TARGET = ciastkarnia

SRC_DIR = src
INCLUDE_DIR = src/include
BUILD_DIR = build

SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/ciastkarnia.c $(SRC_DIR)/procesy.c
OBJS = $(BUILD_DIR)/main.o $(BUILD_DIR)/ciastkarnia.o $(BUILD_DIR)/procesy.o

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -I$(INCLUDE_DIR)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

.PHONY: all clean
