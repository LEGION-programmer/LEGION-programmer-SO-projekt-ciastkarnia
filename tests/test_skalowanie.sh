#!/bin/bash
# Wejdź do folderu głównego
cd "$(dirname "$0")/.." || exit

echo "=== TEST 2: DYNAMICZNE URUCHAMIANIE KASJERÓW ==="

# 1. Usuń stare logi
rm -f tests/log_skalowanie.txt

# 2. Uruchom kierownika: 10s trwania, próg 2 osoby
./kierownik 10 2 > tests/log_skalowanie.txt &
K_PID=$!

sleep 1

# 3. Wygeneruj gwałtowny napływ 10 klientów
echo "Generowanie tłoku..."
for i in {1..10}; do
    ./klient > /dev/null &
done

# Czekamy na reakcję kierownika
sleep 4

# 4. Kończymy i sprawdzamy
kill -SIGINT $K_PID 2>/dev/null
wait $K_PID 2>/dev/null

if grep -q "Otwieram Kasę 2" tests/log_skalowanie.txt; then
    echo -e "\033[0;32m[OK]\033[0m Test zdany: Druga kasa została otwarta."
    grep "Kasa 2" tests/log_skalowanie.txt | head -n 3
else
    echo -e "\033[0;31m[BŁĄD]\033[0m Kierownik nie zareagował na tłok."
    echo "Ostatnie linie logu:"
    tail -n 5 tests/log_skalowanie.txt
fi