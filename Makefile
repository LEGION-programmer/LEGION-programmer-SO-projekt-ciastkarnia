CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -I./src/include

SRC = src/main.c src/ciastkarnia.c src/ipc.c
OBJ = $(patsubst src/%.c, build/%.o, $(SRC))

TARGET = ciastkarnia

all: build $(TARGET)

build:
	mkdir -p build
	mkdir -p logs

build/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

clean:
	rm -rf build
	rm -f $(TARGET)
	rm -rf logs

.PHONY: all clean
