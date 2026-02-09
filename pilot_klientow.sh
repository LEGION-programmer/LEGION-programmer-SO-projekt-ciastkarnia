#!/bin/bash
CYAN='\033[0;36m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

while true; do
    echo -e "\n${YELLOW}Ilu klientów wysłać? (q - wyjście):${NC}"
    read input
    [[ "$input" == "q" ]] && break

    if [[ "$input" =~ ^[0-9]+$ ]]; then
        for (( i=1; i<=$input; i++ )); do
            ./klient &
            # Co 50 klientów czekamy 10ms, używając sleep
            if (( i % 50 == 0 )); then
                sleep 0.01
            fi
        done
        echo -e "${GREEN}>>> Wysłano $input klientów.${NC}"
    fi
done