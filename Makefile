TARGET = ciastkarnia
SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = src/include

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -I$(INCLUDE_DIR)

SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/ciastkarnia.c $(SRC_DIR)/procesy.c
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build $(TARGET)

.PHONY: all clean
