#!/bin/bash
# Wejdź do folderu głównego (tam gdzie Makefile i binarne)
cd "$(dirname "$0")/.." || exit

echo "=== TEST 1: CZYSZCZENIE ZASOBÓW IPC ==="
BEFORE=$(ipcs -m -s -q | grep -c "0x")

# Odpal na 3 sekundy
./kierownik 3 > /dev/null &
PID=$!
sleep 4

# Wyślij sygnał przerwania
kill -SIGINT $PID 2>/dev/null
wait $PID 2>/dev/null

AFTER=$(ipcs -m -s -q | grep -c "0x")

if [ "$BEFORE" -eq "$AFTER" ]; then
    echo -e "\033[0;32m[OK]\033[0m Zasoby IPC zostały posprzątane."
else
    echo -e "\033[0;31m[BŁĄD]\033[0m Wykryto wyciek zasobów IPC!"
    ipcs -u
fi