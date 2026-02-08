#!/bin/bash

# Kolory dla lepszej czytelności
CYAN='\033[0;36m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${CYAN}=== PILOT GENERATORA KLIENTÓW ===${NC}"
echo "Upewnij się, że Kierownik działa w innym terminalu."

while true; do
    echo -e "\n${YELLOW}Ilu klientów wysłać do sklepu? (wpisz 'q' aby wyjść):${NC}"
    read input

    if [[ "$input" == "q" ]]; then
        echo "Zamykanie pilota."
        break
    fi

    # Sprawdzenie czy wpisano liczbę
    if [[ "$input" =~ ^[0-9]+$ ]]; then
        echo -e "${GREEN}>>> Wypuszczam $input klientów...${NC}"
        
        for (( i=1; i<=$input; i++ )); do
            # Uruchamiamy klienta w tle
            ./klient &
            
            # Małe opóźnienie co 50 klientów, żeby system "nie złapał zadyszki"
            if (( i % 50 == 0 )); then
                usleep 10000 # 10ms
            fi
        done
        
        echo -e "${GREEN}>>> Wysłano.${NC}"
    else
        echo -e "${RED}Błąd: Wpisz poprawną liczbę!${NC}"
    fi
done