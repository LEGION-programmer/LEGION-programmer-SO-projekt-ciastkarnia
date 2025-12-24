TARGET = ciastkarnia

SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = $(SRC_DIR)/include

SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/ciastkarnia.c $(SRC_DIR)/procesy.c
OBJS = $(BUILD_DIR)/main.o $(BUILD_DIR)/ciastkarnia.o $(BUILD_DIR)/procesy.o

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -I$(INCLUDE_DIR)

all: $(BUILD_DIR) $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

.PHONY: all clean
