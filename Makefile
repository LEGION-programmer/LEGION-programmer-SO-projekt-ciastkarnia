CC = gcc
# -Wno-unused-variable ukryje ostrzeżenie o PRODUKTY_NAZWY w kliencie
CFLAGS = -Wall -Wextra -Isrc/include -Wno-unused-variable
TARGETS = kierownik piekarz kasjer klient
DEPS = src/include/ciastkarnia.h

all: $(TARGETS)

kierownik: src/kierownik.c $(DEPS)
	$(CC) $(CFLAGS) src/kierownik.c -o kierownik

piekarz: src/piekarz.c $(DEPS)
	$(CC) $(CFLAGS) src/piekarz.c -o piekarz

kasjer: src/kasjer.c $(DEPS)
	$(CC) $(CFLAGS) src/kasjer.c -o kasjer

klient: src/klient.c $(DEPS)
	$(CC) $(CFLAGS) src/klient.c -o klient

clean:
	rm -f *.o $(TARGETS) raport.txt
	-killall -9 kierownik piekarz kasjer klient 2>/dev/null || true
	-ipcs -m | grep `whoami` | awk '{print $$2}' | xargs -r ipcrm -m
	-ipcs -s | grep `whoami` | awk '{print $$2}' | xargs -r ipcrm -s
	-ipcs -q | grep `whoami` | awk '{print $$2}' | xargs -r ipcrm -q

test: all
	chmod +x tests/testy_ipc.sh
	./tests/testy_ipc.sh

.PHONY: all clean test