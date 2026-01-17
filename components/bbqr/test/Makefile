CC = gcc
CFLAGS = -Wall -Wextra -g -I../src
LDFLAGS =

SRCS_BBQR = test_bbqr.c ../src/base32.c ../src/miniz.c ../src/bbqr.c
SRCS_BASE32 = test_base32.c ../src/base32.c
SRCS_MINIZ = test_miniz.c ../src/miniz.c
TARGET_BBQR = test_bbqr
TARGET_BASE32 = test_base32
TARGET_MINIZ = test_miniz

all: $(TARGET_BBQR) $(TARGET_BASE32) $(TARGET_MINIZ)

$(TARGET_BBQR): $(SRCS_BBQR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(TARGET_BASE32): $(SRCS_BASE32)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(TARGET_MINIZ): $(SRCS_MINIZ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

run: $(TARGET_BBQR) $(TARGET_BASE32) $(TARGET_MINIZ)
	./$(TARGET_MINIZ)
	./$(TARGET_BASE32)
	./$(TARGET_BBQR)

run-bbqr: $(TARGET_BBQR)
	./$(TARGET_BBQR)

run-base32: $(TARGET_BASE32)
	./$(TARGET_BASE32)

run-miniz: $(TARGET_MINIZ)
	./$(TARGET_MINIZ)

clean:
	rm -f $(TARGET_BBQR) $(TARGET_BASE32) $(TARGET_MINIZ)

.PHONY: all run run-bbqr run-base32 run-miniz clean
