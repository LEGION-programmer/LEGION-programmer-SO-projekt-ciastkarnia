CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -O2 -pthread
# Tutaj wskazujemy folder z plikami .h
INCLUDE = -Isrc/include

SRC_DIR = src
OBJ_DIR = build
BIN     = ciastkarnia

SRC = $(SRC_DIR)/main.c \
      $(SRC_DIR)/ciastkarnia.c \
      $(SRC_DIR)/procesy.c \
      $(SRC_DIR)/ipc.c

OBJ = $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

all: $(BIN)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Dodałem $(INCLUDE) tutaj, żeby widział pliki .h
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(BIN)

clean:
	rm -rf $(OBJ_DIR) $(BIN)