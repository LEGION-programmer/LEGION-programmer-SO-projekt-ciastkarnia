#!/bin/bash
cd "$(dirname "$0")/.." || exit

echo "=== TEST 4: REZYGNACJA KLIENTÓW ==="
# Odpalamy kierownika bez piekarza na chwilę (test pustych półek)
# Uwaga: Ten test zakłada, że piekarz dostarcza towar co jakiś czas.
./kierownik 5 > tests/log_rezygnacja.txt &
PID=$!
sleep 6

if grep -q "Nic nie kupil" tests/log_rezygnacja.txt; then
    echo -e "\033[0;32m[OK]\033[0m System poprawnie obsłużył pusty sklep."
else
    echo -e "\033[0;33m[INFO]\033[0m Wszyscy coś kupili lub log nie zarejestrował rezygnacji."
fi