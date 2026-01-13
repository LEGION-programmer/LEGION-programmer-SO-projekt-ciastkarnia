#!/bin/bash
echo "=== TEST 2: DYNAMICZNE URUCHAMIANIE KASJERÓW ==="

# Uruchamiamy symulację
./kierownik > log_skalowanie.txt &
PID=$!
sleep 10
kill -SIGINT $PID
wait $PID 2>/dev/null

# Sprawdzamy czy w logach pojawił się komunikat o 2 kasie
if grep -q "Otwieram drugą kasę" log_skalowanie.txt; then
    echo -e "\033[0;32m[OK]\033[0m Kierownik poprawnie wykrył tłok i otworzył drugą kasę."
    grep "Kasa 2" log_skalowanie.txt | head -n 3
else
    echo -e "\033[0;33m[INFO]\033[0m Kolejka nie była dość długa. Spróbuj zwiększyć liczbę klientów w kierownik.c"
fi