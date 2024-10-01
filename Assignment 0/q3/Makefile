BUILD_DIR := .
SRCS := $(wildcard *.c)
TARGETS := $(SRCS:%.c=$(BUILD_DIR)/%)

CC := gcc 
CFLAGS := -Wall -g -O0
LIBS := -lm -lpthread

.PHONY: all clean

all: $(TARGETS)

$(BUILD_DIR)/%: %.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@ $(LIBS)

clean:
	-rm -f $(TARGETS)