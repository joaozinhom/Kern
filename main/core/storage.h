/*
 * Persistent mnemonic storage
 *
 * Stores KEF-encrypted mnemonics on SPIFFS (flash) or SD card.
 * Flash: raw binary (no encoding overhead on constrained SPIFFS).
 * SD card: base64-encoded (portable, human-inspectable).
 *
 * Flash path:  /spiffs/m_<sanitized_id>.kef
 * SD card path: /sdcard/kern/mnemonics/<sanitized_id>.kef
 */

#ifndef STORAGE_H
#define STORAGE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"

typedef enum {
  STORAGE_FLASH,
  STORAGE_SD,
} storage_location_t;

#define STORAGE_FLASH_BASE_PATH "/spiffs"
#define STORAGE_SD_MNEMONICS_DIR "/sdcard/kern/mnemonics"

#define STORAGE_MAX_SANITIZED_ID_LEN 24
#define STORAGE_MNEMONIC_PREFIX "m_"
#define STORAGE_MNEMONIC_EXT ".kef"

/**
 * Initialize flash storage (mount SPIFFS). Safe to call multiple times.
 */
esp_err_t storage_init(void);

/**
 * Save a KEF envelope. Flash: raw binary. SD: base64-encoded.
 *
 * @param loc           Flash or SD card
 * @param id            Raw KEF ID (sanitized for the filename)
 * @param kef_envelope  Binary KEF envelope
 * @param len           Length of KEF envelope
 */
esp_err_t storage_save_mnemonic(storage_location_t loc, const char *id,
                                const uint8_t *kef_envelope, size_t len);

/**
 * Load a mnemonic file. Flash: raw binary. SD: base64-decoded.
 *
 * @param loc              Flash or SD card
 * @param filename         Filename (e.g. "m_73C5DA0A.kef")
 * @param kef_envelope_out Receives heap-allocated binary KEF envelope
 * @param len_out          Receives length
 */
esp_err_t storage_load_mnemonic(storage_location_t loc, const char *filename,
                                uint8_t **kef_envelope_out, size_t *len_out);

/**
 * List stored mnemonic files.
 *
 * @param loc            Flash or SD card
 * @param filenames_out  Receives array of filename strings (caller frees
 *                       with storage_free_file_list)
 * @param count_out      Receives count
 */
esp_err_t storage_list_mnemonics(storage_location_t loc, char ***filenames_out,
                                 int *count_out);

/**
 * Delete a stored mnemonic file.
 */
esp_err_t storage_delete_mnemonic(storage_location_t loc,
                                  const char *filename);

/**
 * Securely wipe flash storage.
 * Unmounts SPIFFS, erases the entire partition (all bytes -> 0xFF),
 * then remounts with a fresh filesystem.
 */
esp_err_t storage_wipe_flash(void);

/**
 * Check if a mnemonic with the given ID already exists.
 */
bool storage_mnemonic_exists(storage_location_t loc, const char *id);

/**
 * Sanitize a raw ID for use as a filename component.
 *
 * Rules:
 * 1. Replace \ / : * ? " < > | and spaces with _
 * 2. Strip leading/trailing whitespace and dots
 * 3. Collapse consecutive underscores
 * 4. Truncate to STORAGE_MAX_SANITIZED_ID_LEN
 * 5. Fallback to SHA-256 hex prefix if result is empty
 */
void storage_sanitize_id(const char *raw_id, char *out, size_t out_size);

/**
 * Free a file list returned by storage_list_mnemonics.
 */
void storage_free_file_list(char **files, int count);

#endif /* STORAGE_H */
