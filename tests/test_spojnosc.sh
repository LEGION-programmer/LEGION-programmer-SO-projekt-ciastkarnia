#!/bin/bash
cd "$(dirname "$0")/.." || exit

echo "=== TEST 3: SPÓJNOŚĆ RAPORTU ==="
./kierownik 10 > /dev/null

# Czytamy raport z głównego folderu
REPORT="raport_koncowy.txt"
if [ ! -f "$REPORT" ]; then echo "Błąd: Brak raportu!"; exit 1; fi

ERROR=0
# Analiza wierszy (pomijamy nagłówki)
while read -r line; do
    # Wyciągamy: Wytworzono ($2), Sprzedano ($3), Zostalo ($4)
    WYP=$(echo "$line" | awk -F'|' '{print $2}' | xargs)
    SPR=$(echo "$line" | awk -F'|' '{print $3}' | xargs)
    ZOS=$(echo "$line" | awk -F'|' '{print $4}' | xargs)
    
    SUMA=$((SPR + ZOS))
    if [ "$WYP" -ne "$SUMA" ]; then
        PROD=$(echo "$line" | awk -F'|' '{print $1}' | xargs)
        echo -e "\033[0;31m[BŁĄD]\033[0m $PROD: $WYP != $SPR + $ZOS"
        ERROR=1
    fi
done < <(grep "|" "$REPORT" | grep -vE "Produkt|---")

if [ $ERROR -eq 0 ]; then
    echo -e "\033[0;32m[OK]\033[0m Bilans produktów w raporcie jest poprawny."
fi