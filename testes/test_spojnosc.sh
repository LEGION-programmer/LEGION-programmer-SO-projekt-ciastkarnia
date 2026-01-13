#!/bin/bash
echo "=== TEST 4: SPÓJNOŚĆ RAPORTU KOŃCOWEGO ==="

./kierownik > log_raport.txt &
PID=$!
sleep 15
kill -SIGINT $PID
wait $PID 2>/dev/null

echo "Analiza danych z raportu..."
# Prosty check: czy suma Sprzedano nie jest większa niż Wypieczono
# Wyciągamy liczby z tabeli raportu (zakładając format z kierownik.c)
grep "|" log_raport.txt | grep -v "Produkt" | grep -v "\-\-\-" > dane.tmp

ERROR=0
while read -r line; do
    WYP=$(echo "$line" | awk -F'|' '{print $2}' | xargs)
    SPR=$(echo "$line" | awk -F'|' '{print $3}' | xargs)
    if [ "$SPR" -gt "$WYP" ]; then
        echo -e "\033[0;31m[BŁĄD]\033[0m Sprzedano ($SPR) więcej niż wypieczono ($WYP)!"
        ERROR=1
    fi
done < dane.tmp

if [ $ERROR -eq 0 ]; then
    echo -e "\033[0;32m[OK]\033[0m Dane w pamięci współdzielonej są spójne."
fi
rm dane.tmp