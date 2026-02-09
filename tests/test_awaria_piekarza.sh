#!/bin/bash
# test_glod.sh
make clean && make

# Startujemy kierownika
./kierownik 50 10 0 &
K_PID=$!
sleep 0.5

# ZABIJAMY PIEKARZA ZANIM COKOLWIEK UPIECZE
killall -9 piekarz
echo -e "\033[31m!!! PIEKARZ NIE ŻYJE. LADA JEST PUSTA !!!\033[0m"

# Wypuszczamy 100 klientów
./pilot_klientow.sh <<EOF
100
q
EOF

sleep 2
kill -SIGINT $K_PID