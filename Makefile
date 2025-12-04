# Nazwa pliku wykonywalnego
TARGET = ciastkarnia

# Katalogi
SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = $(SRC_DIR)/include
LOG_DIR = logs

# Pliki źródłowe
SRCS = $(wildcard $(SRC_DIR)/*.c)

# Pliki obiektowe w katalogu build
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# Flagi kompilatora
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -I$(INCLUDE_DIR)

# Domyślny cel
all: $(TARGET)

# Tworzenie pliku wykonywalnego
$(TARGET): $(OBJS) | $(BUILD_DIR) $(LOG_DIR)
	$(CC) $(OBJS) -o $(TARGET)

# Kompilacja plików źródłowych do obiektowych
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Tworzenie katalogów
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(LOG_DIR):
	mkdir -p $(LOG_DIR)

# Czyszczenie plików obiektowych i pliku wykonywalnego
clean:
	rm -rf $(BUILD_DIR)/*.o $(TARGET)
	rm -rf $(LOG_DIR)/*

# Phony targets
.PHONY: all clean
