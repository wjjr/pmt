TARGET=pmt
CC=gcc
CFLAGS=-std=c99 -Wall -Wextra -pedantic-errors -O2 -s
LDFLAGS=-Wl,-gc-sections
SRC_DIR=src
OBJ_DIR=obj
OUT_DIR=bin
EXTRAS=doc/ LICENSE Makefile README.txt
DIST_TGZ=$(TARGET)-dist.tgz

OBJECTS=$(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(wildcard $(SRC_DIR)/*.c $(SRC_DIR)/*/*.c $(SRC_DIR)/*/*/*.c))
HEADERS=$(wildcard $(SRC_DIR)/*.h $(SRC_DIR)/*/*.h $(SRC_DIR)/*/*/*.h)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $(OUT_DIR)/$@

.PRECIOUS: $(TARGET) $(OBJECTS)
.PHONY: default all clean
default: $(TARGET)
all: default
clean:
	@rm -rf $(OBJ_DIR) $(OUT_DIR) $(DIST_TGZ)
dist:
	@tar --create --gzip --owner=0 --group=0 --numeric-owner --xform 's:^\./::' --mtime='$(shell date -Is)' --file=$(DIST_TGZ) -- $(EXTRAS) $(SRC_DIR)
