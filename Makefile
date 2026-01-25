# Ustawienia kompilatora
CC = gcc
# -Isrc/include mówi kompilatorowi: "szukaj nagłówków w folderze src/include"
CFLAGS = -Wall -Wextra -Isrc/include
TARGETS = kierownik piekarz kasjer klient

# Lokalizacja nagłówka dla mechanizmu zależności
DEPS = src/include/ciastkarnia.h

all: $(TARGETS)

# Kompilacja modułów
kierownik: src/kierownik.c $(DEPS)
	$(CC) $(CFLAGS) src/kierownik.c -o kierownik

piekarz: src/piekarz.c $(DEPS)
	$(CC) $(CFLAGS) src/piekarz.c -o piekarz

kasjer: src/kasjer.c $(DEPS)
	$(CC) $(CFLAGS) src/kasjer.c -o kasjer

klient: src/klient.c $(DEPS)
	$(CC) $(CFLAGS) src/klient.c -o klient

clean:
	rm -f $(TARGETS) raport_koncowy.txt
	@echo "Wyczyszczono pliki binarne i raporty."

run: all
	ipcrm -a || true
	./kierownik

.PHONY: all clean