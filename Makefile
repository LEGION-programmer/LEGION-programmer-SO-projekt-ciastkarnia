# ================================
#   Projekt: Ciastkarnia
#   Kompilacja i uruchamianie
# ================================

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -I./src/include
LDFLAGS = -pthread

SRC_DIR = src
BUILD_DIR = build
LOG_DIR = logs

SRC = $(wildcard $(SRC_DIR)/*.c) \
      $(wildcard $(SRC_DIR)/utils/*.c)

OBJ = $(patsubst %.c, $(BUILD_DIR)/%.o, $(notdir $(SRC)))

TARGET = ciastkarnia

# --------------------------------
#  Reguła główna
# --------------------------------
all: prepare $(TARGET)

# Tworzenie katalogów build/ i logs/
prepare:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(LOG_DIR)

# Kompilacja pliku wykonywalnego
$(TARGET): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $@

# Kompilacja poszczególnych plików .c → .o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/utils/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# --------------------------------
#  Czyszczenie projektu
# --------------------------------
clean:
	rm -rf $(BUILD_DIR)/*.o
	rm -f $(TARGET)

# Usuwa również katalogi
distclean: clean
	rm -rf $(BUILD_DIR)
	rm -rf $(LOG_DIR)

run: all
	./$(TARGET)

.PHONY: all clean distclean prepare run