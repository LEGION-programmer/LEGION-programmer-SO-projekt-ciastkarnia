#!/bin/bash
echo "=== TEST 3: LOGIKA PUSTEGO KOSZYKA ==="

./kierownik > log_zakupy.txt &
PID=$!
sleep 12
kill -SIGINT $PID
wait $PID 2>/dev/null

# Liczymy ile razy klienci wyszli bez zakupów
BRAK=$(grep -c "Nic nie kupił" log_zakupy.txt)
ZAKUPY=$(grep -c "Idzie do kasy" log_zakupy.txt)

echo "Klienci z zakupami: $ZAKUPY"
echo "Klienci bez zakupów: $BRAK"

if [ "$BRAK" -gt 0 ]; then
    echo -e "\033[0;32m[OK]\033[0m System poprawnie obsługuje klientów rezygnujących z zakupów."
else
    echo -e "\033[0;33m[INFO]\033[0m Wszyscy klienci coś kupili. Zmniejsz tempo Piekarza, aby wymusić braki."
fi