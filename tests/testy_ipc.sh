#!/bin/bash

# Kolory dla lepszej czytelności
GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

# Funkcja czyszcząca przed i po testach
cleanup() {
    echo -e "${YELLOW}Sprzątanie zasobów...${NC}"
    pkill -9 kierownik 2>/dev/null
    pkill -9 piekarz 2>/dev/null
    pkill -9 kasjer 2>/dev/null
    pkill -9 klient 2>/dev/null
    # Usuwanie pamięci, semaforów i kolejek użytkownika
    ipcs -m | grep $(whoami) | awk '{print $2}' | xargs -r ipcrm -m 2>/dev/null
    ipcs -s | grep $(whoami) | awk '{print $2}' | xargs -r ipcrm -s 2>/dev/null
    ipcs -q | grep $(whoami) | awk '{print $2}' | xargs -r ipcrm -q 2>/dev/null
}

trap cleanup EXIT
cleanup

echo -e "${BLUE}=== START TESTÓW SYSTEMU CIASTKARNIA ===${NC}"

# Kompilacja
make clean && make
if [ $? -ne 0 ]; then
    echo -e "${RED}Błąd kompilacji!${NC}"
    exit 1
fi

# ---------------------------------------------------------
# TEST 1: Obciążenie (1000 klientów) i Bilans
# ---------------------------------------------------------
echo -e "\n${BLUE}Test 1: Obciążenie (1000 klientów) i spójność danych...${NC}"
# N=200, K=50, Czas=0 (brak auto-stopu)
./kierownik 200 50 0 > /dev/null 2>&1 &
K_PID=$!

sleep 4 # Czekamy na Tp + otwarcie sklepu

echo "Generowanie 1000 klientów (może to chwilę potrwać)..."
for i in {1..10}; do
    for j in {1..100}; do
        ./klient > /dev/null 2>&1 &
    done
    sleep 0.1
done

echo "Czekanie na obsługę wszystkich transakcji..."
sleep 20 # Kluczowy czas dla bilansu

kill -SIGINT $K_PID
wait $K_PID 2>/dev/null

if [ -f raport.txt ] && grep -q "Weryfikacja: OK" raport.txt; then
    echo -e "${GREEN}[PASS] Test 1 zakończony sukcesem.${NC}"
else
    echo -e "${RED}[FAIL] Test 1: Błąd bilansu w raport.txt!${NC}"
fi

# ---------------------------------------------------------
# TEST 2: Dynamika kas (DEBUG VERSION)
# ---------------------------------------------------------
echo -e "\n${BLUE}Test 2: Sprawdzanie dynamiki kas...${NC}"
./kierownik 10 2 0 & 
K_PID=$!

sleep 4 

echo "Wpuszczanie klientów..."
for i in {1..12}; do
    ./klient & 
done

sleep 2

# Sprawdźmy co widzi system
K_COUNT=$(pgrep -f "kasjer" | wc -l)
if [ "$K_COUNT" -ge 2 ]; then
    echo -e "${GREEN}[PASS] Znaleziono $K_COUNT procesy kasjera.${NC}"
else
    echo -e "${RED}[FAIL] Widzę tylko $K_COUNT kasjerów.${NC}"
    echo "Aktywne procesy Twojego projektu:"
    ps aux | grep -E "kierownik|kasjer|piekarz" | grep -v grep
fi

kill -SIGINT $K_PID 2>/dev/null

# ---------------------------------------------------------
# TEST 3: Sprawdzanie wycieków IPC
# ---------------------------------------------------------
cleanup
echo -e "\n${BLUE}Test 3: Sprawdzanie wycieków IPC po zamknięciu...${NC}"
./kierownik 10 5 0 > /dev/null 2>&1 &
K_PID=$!
sleep 4
kill -SIGINT $K_PID
sleep 2

LEAKS=$(ipcs -a | grep $(whoami))
if [ -z "$LEAKS" ]; then
    echo -e "${GREEN}[PASS] Zasoby IPC zostały poprawnie usunięte.${NC}"
else
    echo -e "${RED}[FAIL] Wykryto wycieki IPC!${NC}"
    echo "$LEAKS"
fi

echo -e "\n${BLUE}=== KONIEC TESTÓW ===${NC}"

# ---------------------------------------------------------
# TEST 4: Ewakuacja (SIGUSR2)
# ---------------------------------------------------------
echo -e "\n${BLUE}Test 4: Sprawdzanie procedury ewakuacji (SIGUSR2)...${NC}"
./kierownik 50 25 0 > /dev/null 2>&1 &
K_PID=$!
sleep 2

echo "Wpuszczanie 40 klientów..."
for i in {1..40}; do
    ./klient > /dev/null 2>&1 &
done
sleep 1

echo -e "${RED}Wysyłanie sygnału EWAKUACJI do $K_PID...${NC}"
kill -USR2 $K_PID
sleep 2

# Wywolujemy raport
kill -INT $K_PID
sleep 2

if [ -f raport.txt ]; then
    PORZUCONO=$(grep "P:" raport.txt | awk -F'P:' '{sum+=$2} END {print sum}' | tr -d ' |')
    if [ "$PORZUCONO" -gt 0 ]; then
        echo -e "${GREEN}[PASS] Produkty porzucone: $PORZUCONO${NC}"
    else
        echo -e "${RED}[FAIL] Brak porzuconych produktow w raporcie!${NC}"
    fi
    grep "Weryfikacja: OK" raport.txt > /dev/null && echo -e "${GREEN}[PASS] Bilans OK${NC}" || echo -e "${RED}[FAIL] Bilans ERR${NC}"
fi