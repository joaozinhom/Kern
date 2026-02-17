// Persistent mnemonic storage — SPIFFS and SD card

#include "storage.h"
#include "crypto_utils.h"

#include <dirent.h>
#include <esp_partition.h>
#include <esp_spiffs.h>
#include <mbedtls/base64.h>
#include <sd_card.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define SPIFFS_PARTITION_LABEL "storage"

static bool spiffs_mounted = false;

/* ---------- Initialization ---------- */

esp_err_t storage_init(void) {
  if (spiffs_mounted)
    return ESP_OK;

  esp_vfs_spiffs_conf_t conf = {
      .base_path = STORAGE_FLASH_BASE_PATH,
      .partition_label = SPIFFS_PARTITION_LABEL,
      .max_files = 5,
      .format_if_mount_failed = true,
  };

  esp_err_t ret = esp_vfs_spiffs_register(&conf);
  if (ret == ESP_OK)
    spiffs_mounted = true;
  return ret;
}

/* ---------- ID sanitization ---------- */

void storage_sanitize_id(const char *raw_id, char *out, size_t out_size) {
  if (!raw_id || !out || out_size == 0) {
    if (out && out_size > 0)
      out[0] = '\0';
    return;
  }

  const char *p = raw_id;

  /* Strip leading whitespace and dots */
  while (*p == ' ' || *p == '\t' || *p == '.')
    p++;

  size_t max_len = out_size - 1;
  if (max_len > STORAGE_MAX_SANITIZED_ID_LEN)
    max_len = STORAGE_MAX_SANITIZED_ID_LEN;

  size_t j = 0;
  bool last_underscore = false;

  for (size_t i = 0; p[i] && j < max_len; i++) {
    char c = p[i];

    /* Replace filesystem-unsafe characters with underscore */
    if (c == '\\' || c == '/' || c == ':' || c == '*' || c == '?' ||
        c == '"' || c == '<' || c == '>' || c == '|' || c == ' ') {
      /* Collapse consecutive underscores */
      if (!last_underscore) {
        out[j++] = '_';
        last_underscore = true;
      }
    } else {
      out[j++] = c;
      last_underscore = false;
    }
  }

  /* Strip trailing underscores and dots */
  while (j > 0 && (out[j - 1] == '_' || out[j - 1] == '.'))
    j--;

  out[j] = '\0';

  /* Fallback: first 8 hex chars of SHA-256(raw_id) */
  if (j == 0) {
    uint8_t hash[CRYPTO_SHA256_SIZE];
    crypto_sha256((const uint8_t *)raw_id, strlen(raw_id), hash);
    for (size_t i = 0; i < 4 && (i * 2 + 1) < max_len; i++)
      snprintf(out + i * 2, 3, "%02X", hash[i]);
  }
}

/* ---------- Path helpers ---------- */

static void build_filename(storage_location_t loc, const char *sanitized_id,
                           char *out, size_t out_size) {
  if (loc == STORAGE_FLASH)
    snprintf(out, out_size, "%s%s%s", STORAGE_MNEMONIC_PREFIX, sanitized_id,
             STORAGE_MNEMONIC_EXT);
  else
    snprintf(out, out_size, "%s%s", sanitized_id, STORAGE_MNEMONIC_EXT);
}

static void build_path(storage_location_t loc, const char *filename, char *out,
                       size_t out_size) {
  if (loc == STORAGE_FLASH)
    snprintf(out, out_size, "%s/%s", STORAGE_FLASH_BASE_PATH, filename);
  else
    snprintf(out, out_size, "%s/%s", STORAGE_SD_MNEMONICS_DIR, filename);
}

static esp_err_t ensure_sd_dirs(void) {
  mkdir("/sdcard/kern", 0775);
  mkdir(STORAGE_SD_MNEMONICS_DIR, 0775);
  return ESP_OK;
}

static esp_err_t init_location(storage_location_t loc) {
  if (loc == STORAGE_FLASH)
    return storage_init();

  if (!sd_card_is_mounted()) {
    esp_err_t ret = sd_card_init();
    if (ret != ESP_OK)
      return ret;
  }
  return ensure_sd_dirs();
}

/* ---------- File operations ---------- */

esp_err_t storage_save_mnemonic(storage_location_t loc, const char *id,
                                const uint8_t *kef_envelope, size_t len) {
  if (!id || !kef_envelope || len == 0)
    return ESP_ERR_INVALID_ARG;

  esp_err_t ret = init_location(loc);
  if (ret != ESP_OK)
    return ret;

  char sanitized[STORAGE_MAX_SANITIZED_ID_LEN + 1];
  storage_sanitize_id(id, sanitized, sizeof(sanitized));

  char filename[32];
  build_filename(loc, sanitized, filename, sizeof(filename));

  char path[80];
  build_path(loc, filename, path, sizeof(path));

  if (loc == STORAGE_FLASH) {
    /* Raw binary on flash */
    FILE *f = fopen(path, "wb");
    if (!f)
      return ESP_FAIL;
    size_t written = fwrite(kef_envelope, 1, len, f);
    fclose(f);
    return (written == len) ? ESP_OK : ESP_FAIL;
  }

  /* SD card: base64 encode */
  size_t b64_len = 0;
  mbedtls_base64_encode(NULL, 0, &b64_len, kef_envelope, len);

  unsigned char *b64 = malloc(b64_len);
  if (!b64)
    return ESP_ERR_NO_MEM;

  if (mbedtls_base64_encode(b64, b64_len, &b64_len, kef_envelope, len) != 0) {
    free(b64);
    return ESP_FAIL;
  }

  ret = sd_card_write_file(path, b64, b64_len);
  free(b64);
  return ret;
}

esp_err_t storage_load_mnemonic(storage_location_t loc, const char *filename,
                                uint8_t **kef_envelope_out, size_t *len_out) {
  if (!filename || !kef_envelope_out || !len_out)
    return ESP_ERR_INVALID_ARG;

  *kef_envelope_out = NULL;
  *len_out = 0;

  esp_err_t ret = init_location(loc);
  if (ret != ESP_OK)
    return ret;

  char path[80];
  build_path(loc, filename, path, sizeof(path));

  if (loc == STORAGE_FLASH) {
    /* Raw binary on flash */
    FILE *f = fopen(path, "rb");
    if (!f)
      return ESP_ERR_NOT_FOUND;

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (fsize <= 0) {
      fclose(f);
      return ESP_ERR_INVALID_SIZE;
    }

    uint8_t *data = malloc((size_t)fsize);
    if (!data) {
      fclose(f);
      return ESP_ERR_NO_MEM;
    }

    size_t nread = fread(data, 1, (size_t)fsize, f);
    fclose(f);

    *kef_envelope_out = data;
    *len_out = nread;
    return ESP_OK;
  }

  /* SD card: base64 decode */
  uint8_t *raw = NULL;
  size_t raw_len = 0;

  ret = sd_card_read_file(path, &raw, &raw_len);
  if (ret != ESP_OK)
    return ret;

  size_t decoded_len = 0;
  if (mbedtls_base64_decode(NULL, 0, &decoded_len, raw, raw_len) !=
      MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL) {
    free(raw);
    return ESP_ERR_INVALID_RESPONSE;
  }

  uint8_t *decoded = malloc(decoded_len);
  if (!decoded) {
    free(raw);
    return ESP_ERR_NO_MEM;
  }

  if (mbedtls_base64_decode(decoded, decoded_len, &decoded_len, raw,
                             raw_len) != 0) {
    free(raw);
    free(decoded);
    return ESP_ERR_INVALID_RESPONSE;
  }

  free(raw);
  *kef_envelope_out = decoded;
  *len_out = decoded_len;
  return ESP_OK;
}

esp_err_t storage_list_mnemonics(storage_location_t loc, char ***filenames_out,
                                 int *count_out) {
  if (!filenames_out || !count_out)
    return ESP_ERR_INVALID_ARG;

  *filenames_out = NULL;
  *count_out = 0;

  esp_err_t ret = init_location(loc);
  if (ret != ESP_OK)
    return ret;

  if (loc == STORAGE_SD) {
    /* SD card: list directory and filter for .kef files */
    char **all_files = NULL;
    int all_count = 0;

    ret = sd_card_list_files(STORAGE_SD_MNEMONICS_DIR, &all_files, &all_count);
    if (ret != ESP_OK)
      return ret;

    char **filtered = NULL;
    int filtered_count = 0;

    for (int i = 0; i < all_count; i++) {
      size_t flen = strlen(all_files[i]);
      if (flen >= 4 &&
          strcmp(all_files[i] + flen - 4, STORAGE_MNEMONIC_EXT) == 0) {
        char **tmp =
            realloc(filtered, (size_t)(filtered_count + 1) * sizeof(char *));
        if (!tmp) {
          storage_free_file_list(filtered, filtered_count);
          sd_card_free_file_list(all_files, all_count);
          return ESP_ERR_NO_MEM;
        }
        filtered = tmp;
        filtered[filtered_count] = strdup(all_files[i]);
        if (!filtered[filtered_count]) {
          storage_free_file_list(filtered, filtered_count);
          sd_card_free_file_list(all_files, all_count);
          return ESP_ERR_NO_MEM;
        }
        filtered_count++;
      }
    }

    sd_card_free_file_list(all_files, all_count);
    *filenames_out = filtered;
    *count_out = filtered_count;
    return ESP_OK;
  }

  /* Flash: enumerate SPIFFS directory */
  DIR *dir = opendir(STORAGE_FLASH_BASE_PATH);
  if (!dir)
    return ESP_FAIL;

  char **files = NULL;
  int count = 0;
  struct dirent *entry;

  while ((entry = readdir(dir)) != NULL) {
    const char *name = entry->d_name;
    size_t nlen = strlen(name);

    /* Must match m_*.kef pattern */
    if (nlen < 7)
      continue;
    if (strncmp(name, STORAGE_MNEMONIC_PREFIX, 2) != 0)
      continue;
    if (strcmp(name + nlen - 4, STORAGE_MNEMONIC_EXT) != 0)
      continue;

    char **tmp = realloc(files, (size_t)(count + 1) * sizeof(char *));
    if (!tmp) {
      storage_free_file_list(files, count);
      closedir(dir);
      return ESP_ERR_NO_MEM;
    }
    files = tmp;
    files[count] = strdup(name);
    if (!files[count]) {
      storage_free_file_list(files, count);
      closedir(dir);
      return ESP_ERR_NO_MEM;
    }
    count++;
  }

  closedir(dir);
  *filenames_out = files;
  *count_out = count;
  return ESP_OK;
}

esp_err_t storage_delete_mnemonic(storage_location_t loc,
                                  const char *filename) {
  if (!filename)
    return ESP_ERR_INVALID_ARG;

  esp_err_t ret = init_location(loc);
  if (ret != ESP_OK)
    return ret;

  char path[80];
  build_path(loc, filename, path, sizeof(path));

  if (loc == STORAGE_FLASH)
    return (unlink(path) == 0) ? ESP_OK : ESP_FAIL;

  return sd_card_delete_file(path);
}

esp_err_t storage_wipe_flash(void) {
  if (spiffs_mounted) {
    esp_vfs_spiffs_unregister(SPIFFS_PARTITION_LABEL);
    spiffs_mounted = false;
  }

  const esp_partition_t *part = esp_partition_find_first(
      ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_SPIFFS,
      SPIFFS_PARTITION_LABEL);
  if (!part)
    return ESP_ERR_NOT_FOUND;

  esp_err_t ret = esp_partition_erase_range(part, 0, part->size);
  if (ret != ESP_OK)
    return ret;

  /* Remount — format_if_mount_failed creates a fresh filesystem */
  return storage_init();
}

bool storage_mnemonic_exists(storage_location_t loc, const char *id) {
  if (!id)
    return false;

  char sanitized[STORAGE_MAX_SANITIZED_ID_LEN + 1];
  storage_sanitize_id(id, sanitized, sizeof(sanitized));

  char filename[32];
  build_filename(loc, sanitized, filename, sizeof(filename));

  char path[80];
  build_path(loc, filename, path, sizeof(path));

  if (loc == STORAGE_FLASH) {
    if (storage_init() != ESP_OK)
      return false;
    struct stat st;
    return (stat(path, &st) == 0);
  }

  if (!sd_card_is_mounted())
    return false;
  bool exists = false;
  sd_card_file_exists(path, &exists);
  return exists;
}

void storage_free_file_list(char **files, int count) {
  if (!files)
    return;
  for (int i = 0; i < count; i++)
    free(files[i]);
  free(files);
}
