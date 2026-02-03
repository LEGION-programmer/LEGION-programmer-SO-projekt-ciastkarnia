CC = gcc
CFLAGS = -Wall -Wextra -Isrc/include -Wno-unused-variable

SRCDIR = src
INCDIR = src/include

COMMON = $(SRCDIR)/ciastkarnia.c
COMMON_OBJ = $(COMMON:.c=.o)

TARGETS = kierownik piekarz kasjer klient

all: $(TARGETS)

$(COMMON_OBJ): $(COMMON)
	$(CC) $(CFLAGS) -c $< -o $@

kierownik: $(SRCDIR)/kierownik.c $(COMMON_OBJ)
	$(CC) $(CFLAGS) $^ -o $@

piekarz: $(SRCDIR)/piekarz.c $(COMMON_OBJ)
	$(CC) $(CFLAGS) $^ -o $@

kasjer: $(SRCDIR)/kasjer.c $(COMMON_OBJ)
	$(CC) $(CFLAGS) $^ -o $@

klient: $(SRCDIR)/klient.c $(COMMON_OBJ)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f $(TARGETS) $(COMMON_OBJ) raport.txt
	-killall -9 kierownik piekarz kasjer klient 2>/dev/null || true
	-ipcs -m | grep `whoami` | awk '{print $$2}' | xargs -r ipcrm -m
	-ipcs -s | grep `whoami` | awk '{print $$2}' | xargs -r ipcrm -s
	-ipcs -q | grep `whoami` | awk '{print $$2}' | xargs -r ipcrm -q

test: all
	./tests/testy_ipc.sh

.PHONY: all clean test
