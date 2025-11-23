# Temat 15 – Ciastkarnia  
## Opis projektu

Projekt polega na symulacji funkcjonowania ciastkarni składającej się z piekarni, samoobsługowego sklepu firmowego, stanowisk kasowych oraz kierownika. Symulacja musi być zrealizowana przy użyciu wielu procesów (`fork()`, `exec()`) oraz mechanizmów IPC: semaforów, pamięci dzielonej, kolejek komunikatów, potoków (FIFO/pipe) i sygnałów.

## Założenia funkcjonalne

- Produkowanych jest **P różnych produktów (P > 10)**, każdy trafia na osobny podajnik o pojemności **Kᵢ**.
- Produkty są pobierane **FIFO**, w kolejności ułożenia przez piekarza.
- Godziny pracy:
  - piekarnia: **Tp → Tk**
  - sklep: **Tp + 30 min → Tk**
- W sklepie jednocześnie może przebywać maksymalnie **N klientów**.
- Działają **2 stanowiska kasowe**, zawsze co najmniej jedno.
- Na każde **K = N/2** klientów musi przypadać jedna czynna kasa.
  - gdy liczba klientów < N/2 → druga kasa zostaje zamknięta (po obsłużeniu własnej kolejki).
- Klienci przychodzą losowo, każdy z listą zakupów (min. 2 różne produkty).
- Jeśli produkt jest niedostępny → klient go nie kupuje.
- Kierownik może wysłać dwa sygnały:
  - **Sygnał 1 – inwentaryzacja:** klienci kończą zakupy normalnie, po zamknięciu kasjerzy tworzą raport sprzedaży.
  - **Sygnał 2 – ewakuacja:** klienci natychmiast opuszczają sklep i odkładają pobrany towar.
- Raport końcowy zawiera:
  - sprzedaż z kas,
  - stan podajników,
  - liczbę wyprodukowanych produktów.

## Procesy w systemie

W systemie funkcjonują procesy reprezentujące:
- kierownika,
- piekarza,
- klientów (wielokrotne procesy),
- kasjerów (co najmniej dwa procesy).

---

# Testy funkcjonalne

Poniżej znajdują się testy wymagane do weryfikacji poprawności działania symulacji.

---

## Test 1 – Limit klientów w sklepie
**Cel:** sprawdzenie nieprzekraczania limitu N klientów.  
**Przebieg:**
1. Uruchomić wielu klientów przychodzących w krótkich odstępach.
2. Weryfikować, że maksymalnie N klientów jest wewnątrz sklepu.
3. Pozostali klienci powinni czekać przed wejściem.

**Oczekiwany wynik:** liczba klientów w sklepie nigdy nie przekracza N.

---

## Test 2 – Automatyczne otwieranie i zamykanie kas
**Cel:** weryfikacja logiki uruchamiania i zamykania kas.  
**Przebieg:**
1. Doprowadzić do sytuacji, w której liczba klientów przekracza N/2.
2. Druga kasa powinna zostać otwarta.
3. Następnie zmniejszyć liczbę klientów poniżej N/2.
4. Kasa powinna zostać zamknięta dopiero po obsłudze bieżącej kolejki.

**Oczekiwany wynik:** zachowanie kas zgodne z zasadami kierownika.

---

## Test 3 – Sygnał 2 (ewakuacja)
**Cel:** sprawdzenie natychmiastowej reakcji systemu na ewakuację.  
**Przebieg:**
1. Uruchomić symulację z aktywnymi klientami.
2. Wysłać sygnał ewakuacyjny.
3. Klienci przerywają zakupy i wychodzą bez przechodzenia przez kasy.

**Oczekiwany wynik:** brak deadlocków, klienci kończą procesy poprawnie, kasy kończą pracę.

---

## Test 4 – Sygnał 1 (inwentaryzacja i raport)
**Cel:** weryfikacja poprawności danych w raporcie końcowym.  
**Przebieg:**
1. Wysłać sygnał inwentaryzacji w trakcie symulacji.
2. Pozwolić symulacji działać do zamknięcia sklepu.
3. Po zamknięciu kasjerzy generują raporty sprzedaży.

**Oczekiwany wynik:**
- dane w raporcie zgodne z liczbą sprzedanych i pozostałych produktów,
- brak niespójności między danymi piekarza i podajników.

---

## Test 5 – Poprawność FIFO i synchronizacji podajników
**Cel:** sprawdzenie prawidłowego działania podajników i semaforów.  
**Przebieg:**
1. Ustawić małą pojemność podajników (małe Kᵢ).
2. Wprowadzić wielu klientów pobierających produkty jednocześnie.

**Oczekiwany wynik:**  
- produkty wydawane w kolejności FIFO,  
- brak nadpisywania lub utraty danych,  
- poprawna synchronizacja dostępu multiprocessing.

---

# Link do repozytorium GitHub  
*(do uzupełnienia)*  
**→ [https://github.com/LEGION-programmer/LEGION-programmer-SO-projekt-ciastkarnia/tree/main]()**

