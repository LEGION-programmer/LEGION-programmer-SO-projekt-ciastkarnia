#!/bin/bash

# Kolory dla lepszej czytelności
GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Funkcja czyszcząca na wypadek przerwania skryptu
cleanup() {
    pkill -9 kierownik 2>/dev/null
    pkill -9 piekarz 2>/dev/null
    pkill -9 kasjer 2>/dev/null
    pkill -9 klient 2>/dev/null
}
trap cleanup EXIT

# Kompilacja przed testami
echo -e "${BLUE}--- PRZYGOTOWANIE: Kompilacja projektu ---${NC}"
make clean && make > /dev/null
if [ $? -ne 0 ]; then
    echo -e "${RED}[ERROR] Kompilacja nie powiodła się!${NC}"
    exit 1
fi

echo -e "\n${BLUE}--- START PEŁNYCH TESTÓW IPC (4 SCENARIUSZE) ---${NC}"

# ---------------------------------------------------------
# TEST 1: Maksymalne obciążenie
# ---------------------------------------------------------
echo -n "Test 1: Maksymalne obciążenie (1000 klientów)... "
./kierownik 20 5 1000 > /dev/null 2>&1 &
K_PID=$!
sleep 10
kill -SIGINT $K_PID 2>/dev/null
wait $K_PID 2>/dev/null

if [ -f raport.txt ] && grep -q "OK" raport.txt; then
    echo -e "${GREEN}[PASS]${NC}"
else
    echo -e "${RED}[FAIL]${NC} (Błąd spójności danych)"
fi

# ---------------------------------------------------------
# TEST 2: Dynamika kas (Sygnały i Fork)
# ---------------------------------------------------------
echo -n "Test 2: Sprawdzanie dynamiki kas (Sygnały)... "
# N=20, K=2 (nowa kasa co 2 klientów)
./kierownik 20 2 0 > /dev/null 2>&1 &
K_PID=$!
sleep 2

# Generujemy zmasowany tłum, aby wymusić otwarcie kasy 2
for i in {1..12}; do 
    ./klient > /dev/null 2>&1 & 
done

sleep 2 # Czas dla Kierownika na reakcję
KASY_COUNT=$(ps -ef | grep kasjer | grep -v grep | wc -l)
kill -SIGINT $K_PID 2>/dev/null
wait $K_PID 2>/dev/null

if [ "$KASY_COUNT" -ge 2 ]; then
    echo -e "${GREEN}[PASS]${NC} (Wykryto kasy: $KASY_COUNT)"
else
    echo -e "${RED}[FAIL]${NC} (Otwarto tylko $KASY_COUNT kasy, a powinny być min. 2)"
fi

# ---------------------------------------------------------
# TEST 3: Kolejka Komunikatów (Klient -> Kasjer)
# ---------------------------------------------------------
echo -n "Test 3: Przesyłanie zamówień (Kolejka komunikatów)... "
# Odpalamy 15 klientów natychmiastowych
./kierownik 10 5 15 > /dev/null 2>&1 &
K_PID=$!
sleep 6 # Czekamy aż piekarz upiecze a kasjer obsłuży
kill -SIGINT $K_PID 2>/dev/null
wait $K_PID 2>/dev/null

SPRZEDANO=$(grep "SUMA" raport.txt | awk -F'|' '{print $3}' | xargs)
if [[ "$SPRZEDANO" != "" && "$SPRZEDANO" -gt 0 ]]; then
    echo -e "${GREEN}[PASS]${NC} (Sprzedano towarów: $SPRZEDANO)"
else
    echo -e "${RED}[FAIL]${NC} (Licznik sprzedaży stoi na 0)"
fi

# ---------------------------------------------------------
# TEST 4: Oczyszczanie zasobów (Wycieki IPC)
# ---------------------------------------------------------
echo -n "Test 4: Sprawdzanie wycieków IPC... "
./kierownik 5 2 5 > /dev/null 2>&1 &
K_PID=$!
sleep 2
kill -SIGINT $K_PID 2>/dev/null
wait $K_PID 2>/dev/null
sleep 1 # Czas na ostateczne usunięcie przez system

USER_NAME=$(whoami)
# Sprawdzamy wszystkie trzy typy zasobów
POZOSTALOSCI=$(ipcs -m -s -q | grep "$USER_NAME")

if [ -z "$POZOSTALOSCI" ]; then
    echo -e "${GREEN}[PASS]${NC} (Brak osieroconych segmentów)"
else
    echo -e "${RED}[FAIL]${NC} (Zasoby nadal wiszą w systemie!)"
    ipcs -m -s -q | grep "$USER_NAME"
fi

echo -e "${BLUE}--- KONIEC TESTÓW ---${NC}\n"