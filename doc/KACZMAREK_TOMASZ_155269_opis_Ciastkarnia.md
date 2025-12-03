# Projekt SO — Ciastkarnia  
Autor: **Tomasz Kaczmarek, Nr albumu: 155269**  
Temat: **Ciastkarnia**  

---

# 1. Opis symulacji

Symulacja odwzorowuje pracę **ciastkarni z samoobsługowym sklepem**, w której jednocześnie działają:

- **Piekarz(e)** – produkują *losową liczbę sztuk* różnych produktów (P > 10) i odkładają je na *podajniki*.  
- **Klienci** – wchodzą do sklepu jeśli liczba osób < N; pobierają towary z podajników *w kolejności FIFO*, zgodnie z listą zakupów.  
- **Kasjerzy (2 stanowiska)** – obsługują klientów, ale:  
  - zawsze czynne jest minimum 1 stanowisko,  
  - na każde K = N/2 klientów powinno przypadać minimum 1 czynna kasa,  
  - kasę można zamknąć dopiero po obsłużeniu wszystkich oczekujących.  
- **Kierownik** – generuje sygnały, zamyka sklep, robi inwentaryzację i zapisuje raport.

Wszystkie role działają jako **osobne procesy**, komunikujące się poprzez:

- **pamięć dzieloną** – stan podajników, liczba klientów w sklepie, stan kas,
- **semafory System V** – synchronizacja dostępu do podajników,
- **kolejkę komunikatów** – zgłoszenia klientów do obsługi,
- **sygnały** – inwentaryzacja i ewakuacja,
- **pipe / FIFO** – centralne logowanie.

System **nie stosuje scentralizowanego algorytmu**, procesy działają niezależnie.

---

# 2. Szczegółowa rola procesów

## **Piekarz**
- Co określony losowy czas tworzy *losową liczbę sztuk* różnych produktów.  
- Każdy produkt odkłada na swój **podajnik Pi**, ale **tylko do maksymalnej pojemności Ki**.
- Dostęp do podajnika jest chroniony semaforem.
- Na koniec raportuje, ile sztuk *każdego produktu* wyprodukował.

---

## **Klient**
- Próbuje wejść do sklepu – jeśli jest już N klientów, czeka.  
- Ma *losową listę zakupów* obejmującą co najmniej 2 różne produkty.  
- Dla każdego produktu:  
  - jeśli podajnik nie jest pusty → bierze towar,  
  - jeśli brak → pomija pozycję.  
- Po zakupach ustawia się w **kolejce do kas (FIFO)**.  
- Oczekuje na obsługę przez kasjera.  
- Po zapłacie opuszcza sklep.  
- Zapisuje log swojej aktywności.

---

## **Kasjer (2 stanowiska)**
- Odbiera klientów z kolejki komunikatów.  
- Obsługuje ich zgodnie z dynamiczną polityką kierownika:
  - jeśli klientów < N/2 → jedna kasa zostaje zamknięta,
  - jeśli klientów ≥ N/2 → obie są otwarte,
  - zamknięcie kasy następuje dopiero po obsłużeniu klientów oczekujących w jej kolejce.
- Po obsłudze zgłasza sprzedaż (ile sztuk każdego produktu).  
- Na koniec przekazuje podsumowanie sprzedaży do kierownika.

---

## **Kierownik**
- Obsługuje sygnały:
  - `SIGUSR1` – *inwentaryzacja*, ale wykonywana **po zamknięciu sklepu**,
  - `SIGUSR2` – *ewakuacja natychmiastowa*: klienci przerywają zakupy, odkładają pobrane towary do kosza i wychodzą bez płacenia,
  - `SIGINT` – łagodne zakończenie pracy całego systemu.
- Po zakończeniu pracy:
  - zbiera informacje od piekarzy i kasjerów,
  - sumuje pozostały towar na podajnikach,
  - tworzy **raport końcowy** do pliku tekstowego,
  - usuwa zasoby IPC.

---

# 3. Mechanizmy IPC użyte w projekcie

| Mechanizm | Zastosowanie |
|----------|--------------|
| **Pamięć dzielona** | Podajniki, liczba klientów, stan kas |
| **Semafory** | Ochrona dostępu do podajników |
| **Kolejka komunikatów** | Klienci → kasjerzy |
| **Pipe / FIFO** | Logowanie |
| **Sygnały** | Inwentaryzacja i ewakuacja |
| **Procesy** | Piekarz, klient, kasjer, kierownik, logger |

---

# 4. Parametry programu

Uruchomienie:

./ciastkarnia -P <piekarze> -N <maks. klienci> -K <pojemność podajników> -t <czas działania>

shell
Skopiuj kod

### Przykład:

./ciastkarnia -P 2 -N 20 -K 10 -t 60

yaml
Skopiuj kod

Walidacja obejmuje:

- czy parametry istnieją,  
- czy są > 0,  
- czy nie przekraczają logicznych limitów.

---

# 5. Kompilacja i uruchomienie

### **Wymagane środowisko**
Linux (Ubuntu/Debian):

sudo apt install build-essential

markdown
Skopiuj kod

### **Kompilacja**
make

markdown
Skopiuj kod

### **Uruchomienie**
./ciastkarnia -P 2 -N 30 -K 10 -t 45

markdown
Skopiuj kod

### **Czyszczenie**
make clean

yaml
Skopiuj kod

---

# 6. Testy końcowe

## **Test 1 – Normalna praca**
20 klientów, 2 kasjerów, 1 piekarz.  

**Oczekiwane:**  
- brak deadlocków  
- każdy klient obsłużony  
- poprawne działanie FIFO kas  

---

## **Test 2 – Przepełnienie podajników**
3 piekarzy, pojemność Ki = 5.  

**Oczekiwane:**  
- piekarze czekają, gdy podajnik pełny  
- brak nadpisania pamięci  

---

## **Test 3 – Duże obciążenie kas**
30 klientów, 3 piekarzy.  

**Oczekiwane:**  
- dynamiczne otwieranie i zamykanie kas  
- brak blokad IPC  

---

## **Test 4 – Sygnały**
Podczas działania:

kill -USR1 <PID>
kill -USR2 <PID>

yaml
Skopiuj kod

**Oczekiwane:**  
- prawidłowa inwentaryzacja  
- natychmiastowa ewakuacja  
- poprawne zwolnienie zasobów IPC  

---


# Link do repozytorium GitHub  
**→ [https://github.com/LEGION-programmer/LEGION-programmer-SO-projekt-ciastkarnia/tree/main]()**

