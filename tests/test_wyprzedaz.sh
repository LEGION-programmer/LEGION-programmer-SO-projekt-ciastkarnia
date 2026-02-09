#!/bin/bash
# 1. Gruntowne czyszczenie przed startem
killall -9 kierownik piekarz kasjer klient 2>/dev/null
ipcrm -a

echo -e "\033[35m>>> START TESTU: WIELKA WYPRZEDAŻ <<<\033[0m"
# Startujemy: sklep 50 os., Kasa 2 przy 20 os., Kasa 3 przy 0 (brak)
./kierownik 50 20 0 &
K_PID=$!
sleep 2

echo -e "\033[33m>>> Wylewamy falę 300 klientów na sklep o pojemności 50...\033[0m"
# Wysyłamy bardzo dużo klientów naraz
for i in {1..300}; do 
    ./klient & 
done

# Dajemy im czas na "przemielenie" się przez semafory i kolejkę
echo -e "\033[36m>>> Czekam, aż semafory wejścia zaczną wpuszczać kolejnych klientów...\033[0m"
sleep 15

echo -e "\033[32m>>> Kończymy zakupy. Sprawdzamy statystyki...\033[0m"
kill -SIGINT $K_PID