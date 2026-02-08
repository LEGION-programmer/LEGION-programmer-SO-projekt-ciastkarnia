CC = gcc
CFLAGS = -Wall -Wextra -Isrc/include -Wno-unused-variable

all: kierownik piekarz kasjer klient

kierownik: src/kierownik.c
	$(CC) $(CFLAGS) $^ -o $@

piekarz: src/piekarz.c
	$(CC) $(CFLAGS) $^ -o $@

kasjer: src/kasjer.c
	$(CC) $(CFLAGS) $^ -o $@

klient: src/klient.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f kierownik piekarz kasjer klient
	killall -9 kierownik piekarz kasjer klient 2>/dev/null || true
	ipcs -m | grep `whoami` | awk '{print $$2}' | xargs -r ipcrm -m
	ipcs -s | grep `whoami` | awk '{print $$2}' | xargs -r ipcrm -s
	ipcs -q | grep `whoami` | awk '{print $$2}' | xargs -r ipcrm -q