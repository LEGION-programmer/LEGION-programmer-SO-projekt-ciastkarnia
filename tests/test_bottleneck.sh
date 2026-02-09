#!/bin/bash
make clean && make
./kierownik 3 10 0 & 
K_PID=$!
sleep 2

for j in {1..3}; do
    echo "Fala $j..."
    for i in {1..50}; do ./klient & done
    sleep 3 # DAJEMY CZAS PIERWSZEJ GRUPIE NA WYJŚCIE!
done

echo "Czekam na maruderów..."
sleep 10 # Kluczowe: czekamy aż wszyscy z 150 klientów wyjdą
wait
kill -SIGINT $K_PID