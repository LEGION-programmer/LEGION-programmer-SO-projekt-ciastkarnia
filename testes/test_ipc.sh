#!/bin/bash
echo "=== TEST 1: CZYSZCZENIE ZASOBÓW IPC ==="
# Sprawdź stan przed
BEFORE=$(ipcs -m -s -q | grep -c "0x")

# Uruchom i przerwij po 3s
./kierownik > /dev/null &
PID=$!
sleep 3
kill -SIGINT $PID
wait $PID 2>/dev/null

# Sprawdź stan po
AFTER=$(ipcs -m -s -q | grep -c "0x")

if [ "$BEFORE" -eq "$AFTER" ]; then
    echo -e "\033[0;32m[OK]\033[0m Wszystkie semafory, kolejki i pamięć zostały usunięte."
else
    echo -e "\033[0;31m[BŁĄD]\033[0m Wykryto wyciek zasobów IPC!"
    ipcs -u
fi