#!/bin/bash
trap "killall -9 kierownik piekarz kasjer klient 2>/dev/null; exit" SIGINT SIGTERM

echo -e "\033[32m>>> Kompilacja...\033[0m"
make clean && make

./kierownik 100 15 &
K_PID=$!

sleep 4
echo -e "\033[32m>>> Wypuszczanie 5000 klientów z koszykami...\033[0m"

for i in {1..5000}; do
   ./klient &
   if (( i % 250 == 0 )); then
       sleep 0.05
   fi
done

wait $K_PID