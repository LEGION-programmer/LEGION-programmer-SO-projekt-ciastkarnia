# Ustawienia kompilatora
CC = gcc
CFLAGS = -Wall -Wextra -Isrc/include
LDFLAGS = 

# Lista programów do zbudowania
TARGETS = kierownik piekarz kasjer klient

# Domyślna reguła (wywoływana przez wpisanie 'make')
all: $(TARGETS)

# Kompilacja Kierownika
kierownik: src/kierownik.c
	$(CC) $(CFLAGS) src/kierownik.c -o kierownik

# Kompilacja Piekarza
piekarz: src/piekarz.c
	$(CC) $(CFLAGS) src/piekarz.c -o piekarz

# Kompilacja Kasjera
kasjer: src/kasjer.c
	$(CC) $(CFLAGS) src/kasjer.c -o kasjer

# Kompilacja Klienta
klient: src/klient.c
	$(CC) $(CFLAGS) src/klient.c -o klient

# Czyszczenie plików binarnych (wywoływane przez 'make clean')
clean:
	rm -f $(TARGETS)
	@echo "Pliki binarne usunięte."

# Reguła zapobiegająca problemom z plikami o nazwach takich jak reguły
.PHONY: all clean