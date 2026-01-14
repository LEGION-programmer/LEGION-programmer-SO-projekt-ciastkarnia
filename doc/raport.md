# Dokumentacja Techniczna Implementacji — Specyfikacja Kodu
**Projekt:** Symulacja Ciastkarni (IPC System V)  
**Autor:** Tomasz Kaczmarek (155269)

---

## 1. Struktury Danych i Synchronizacja (`ciastkarnia.h`)

### 1.1. Pamięć Współdzielona (`shared_data_t`)
Główna struktura mapowana w pamięci jądra, dostępna dla wszystkich procesów.

| Pole struktury          | Typ       | Opis                                                                  |
|:-----------------------:|:---------:|:---------------------------------------------------------------------:|
| **`stan_podajnika[]`**  | `int[11]` | Aktualna liczba dostępnych sztuk każdego z 11 produktów na półkach.   |
| **`wytworzono[]`**      | `int[11]` | Globalny licznik produkcji (statystyka dla raportu końcowego).        |
| **`sprzedano[]`**       | `int[11]` | Globalny licznik sprzedaży (aktualizowany przez procesy Kasjerów).    |
| **`sklep_otwarty`**     | `int`     | Flaga sterująca (1 = sklep działa, 0 = procedura zamykania systemu).  |

### 1.2. Kolejka Komunikatów (`msg_zamowienie_t`)
Struktura zamówienia przesyłana od Klienta do Kasjera.

| Pole komunikatu  | Typ       | Opis                                                                  |
|:----------------:|:---------:|:---------------------------------------------------------------------:|
| **`mtype`**      | `long`    | Typ komunikatu (wymagany przez System V IPC, ustawiony na 1).         |
| **`klient_id`**  | `int`     | Identyfikator procesu klienta (PID).                                  |
| **`produkty[]`** | `int[11]` | Tablica zawierająca liczbę sztuk produktów zakupionych przez klienta. |

### 1.3. Funkcja Synchronizacyjna
**sem_op(id, num, op)**
 Opis: Uniwersalne opakowanie semop dla operacji atomowych na mutexie i liczniku.
**Link: Zobacz implementację w ciastkarnia.h**

### 1.4. Protokół Komunikacji (Kolejka)
Definicja struktury zgodna ze standardem msgsnd/msgrcv.
**Link do definicji:**[struct msg_zamowienie_t](https://github.com/LEGION-programmer/LEGION-programmer-SO-projekt-ciastkarnia/blob/3d959858d1cec48501cb52056cd6b19987d67da4/src/include/ciastkarnia.h#L33-L37)

## 2. Analiza Funkcji i Modułów
### 2.1. Moduł: Kierownik (src/kierownik.c)
**main()** 
Opis: Inicjalizacja zasobów IPC, zarządzanie procesami oraz monitoring obciążenia kolejki.
**Link:** [Zaimplementowana funkcja main w src/kierownik.c](https://github.com/LEGION-programmer/LEGION-programmer-SO-projekt-ciastkarnia/blob/3d959858d1cec48501cb52056cd6b19987d67da4/src/kierownik.c#L33-L78)

**sprzatanie()**
Opis: Handler SIGINT: ustawia flagę zamknięcia i usuwa zasoby z jądra (shmctl, semctl).
**Link:** [Przejdź do procedury sprzątania](https://github.com/LEGION-programmer/LEGION-programmer-SO-projekt-ciastkarnia/blob/3d959858d1cec48501cb52056cd6b19987d67da4/src/kierownik.c#L21-L31)

**raport_koncowy()**
Opis: Generuje tabelę inwentaryzacyjną na podstawie liczników w pamięci SHM.
**Link:** [Przejdź do generowania raportu](https://github.com/LEGION-programmer/LEGION-programmer-SO-projekt-ciastkarnia/blob/3d959858d1cec48501cb52056cd6b19987d67da4/src/kierownik.c#L8-L19)

## 2.2. Moduł: Klient (src/klient.c)
**sem_op(..., 0, -1)**
Opis: Kontrola gęstości ruchu: blokuje wejście, gdy w sklepie jest komplet osób.
**Link do definicji:** [Przejdź do kontroli wejścia](https://github.com/LEGION-programmer/LEGION-programmer-SO-projekt-ciastkarnia/blob/3d959858d1cec48501cb52056cd6b19987d67da4/src/ciastkarnia.c#L10-L19)Przykład użycia (Sekcja Krytyczna w Kliencie):
**Plik: src/klient.c**
Opis: Klient wywołuje tę funkcję, aby "podnieść" lub "opuścić" semafor licznikowy (wejście do sklepu) oraz semafor binarny (dostęp do konkretnego podajnika w pamięci współdzielonej).
**Przykład użycia:** [Przejdź do kontroli wejścia](https://github.com/LEGION-programmer/LEGION-programmer-SO-projekt-ciastkarnia/blob/3d959858d1cec48501cb52056cd6b19987d67da4/src/klient.c#L14)

**Pętla Retry**
1. Pętla zewnętrzna (i < 3): Odpowiada za listę zakupów (klient próbuje kupić 3 różne/losowe ciastka).
2. Pętla wewnętrzna (r < 2): To jest właśnie Twoja Pętla Retry (jeśli wylosowanego ciastka nie ma, klient czeka i próbuje jeszcze raz).
**Opis logiki:** Mechanizm składa się z dwóch etapów. Pętla główna definiuje liczbę prób zakupu różnych towarów. Wewnątrz niej znajduje się właściwa pętla ponawiania (retry), która w przypadku braku towaru na podajniku (shm->stan_podajnika[typ] == 0) wstrzymuje proces klienta za pomocą usleep(400000).
**Synchronizacja:** Każde sprawdzenie stanu podajnika i jego inkrementacja odbywa się wewnątrz sekcji krytycznej, zabezpieczonej semaforem binarnym (indeks 1). Zapobiega to sytuacji, w której dwóch klientów pobiera ostatnią sztukę towaru jednocześnie.
**Zastosowane zmienne:** * typ – losowo wybrany produkt.
r – licznik ponowień (2 próby czekania na dostawę).
suma_produktow – licznik zebranych towarów, decydujący o późniejszym udaniu się do kasy.
**Link:** [Przejdź do logiki zakupów](https://github.com/LEGION-programmer/LEGION-programmer-SO-projekt-ciastkarnia/blob/3d959858d1cec48501cb52056cd6b19987d67da4/src/klient.c#L23C5-L38C6)

**Warunek Zakupu (Optymalizacja "Pustego Koszyka")**
**Opis logiki:** Po zakończeniu pętli zakupowej proces sprawdza wartość zmiennej suma_produktow. Jeśli klientowi nie udało się pobrać żadnego towaru (np. z powodu braków na półkach po wszystkich próbach retry), następuje ominięcie fazy płatności. Proces zwalnia semafor wejściowy i kończy działanie.
**Cel optymalizacji:** Zapobieganie przesyłaniu pustych struktur msg_zamowienie_t do kolejki komunikatów (IPC Message Queue). Dzięki temu procesy Kasjerów przetwarzają wyłącznie realne zamówienia, co minimalizuje narzut komunikacyjny i zapobiega sztucznemu wydłużaniu kolejki.
**Kluczowe instrukcje:** Warunek if (suma_produktow > 0) przed wywołaniem msgsnd().
**Link:** [Przejdź do algorytmu wyjścia](https://github.com/LEGION-programmer/LEGION-programmer-SO-projekt-ciastkarnia/blob/3d959858d1cec48501cb52056cd6b19987d67da4/src/klient.c#L40-L47)

## 2.3. Moduł: Kasjer (src/kasjer.c)
**msgrcv() (Odbiór zamówień i obsługa kolejki)**
**Opis logiki:** Kasjer pracuje w pętli while, dopóki flaga shm->sklep_otwarty wynosi 1. Kluczowym elementem jest użycie funkcji msgrcv() z flagą IPC_NOWAIT. Dzięki niej, jeśli kolejka jest pusta, funkcja nie blokuje procesu na nieskończony czas, lecz natychmiast zwraca błąd ENOMSG. Pozwala to Kasjerowi na ponowne sprawdzenie flagi zamknięcia sklepu w nagłówku pętli.
**Przebieg procesu:** 1. Próba pobrania komunikatu o typie 1. 2. Jeśli komunikat istnieje: przejście do sekcji aktualizacji statystyk. 3. Jeśli komunikatu brak: krótkie wstrzymanie (usleep) i powrót do sprawdzenia stanu sklepu.
**Zaleta techniczna:** Zapobiega to sytuacji zawieszenia się procesu Kasjera na kolejce w momencie, gdy Kierownik chce zakończyć symulację i usunąć zasoby.
**Link:** [Przejdź do obsługi kolejki](https://github.com/LEGION-programmer/LEGION-programmer-SO-projekt-ciastkarnia/blob/3d959858d1cec48501cb52056cd6b19987d67da4/src/kasjer.c#L12-L23)

**Sekcja Krytyczna (Aktualizacja globalnych statystyk sprzedaży)**
**Opis logiki:** Po odebraniu poprawnego zamówienia, Kasjer musi dodać zakupione produkty do globalnej tablicy shm->sprzedano. Ponieważ w systemie może pracować więcej niż jeden Kasjer, operacja ta jest objęta semaforem binarnym (Mutexem o indeksie 1).
**Mechanizm zabezpieczenia:** 1. sem_op(semid, 1, -1) – Kasjer "zamyka drzwi" do pamięci współdzielonej dla innych procesów. 2. Aktualizacja tablicy w pętli for. 3. sem_op(semid, 1, 1) – Kasjer "otwiera drzwi", pozwalając innym procesom (np. drugiemu Kasjerowi lub Piekarzowi) na dostęp do danych.
**Cel:** Zapobieganie utracie danych (Race Condition). Bez tego zabezpieczenia, gdyby dwaj kasjerzy dodali wartości w tym samym momencie, jeden z nich mógłby nadpisać wynik drugiego, co zafałszowałoby raport końcowy.
**Link:** [Przejdź do sekcji krytycznej kasjera](https://github.com/LEGION-programmer/LEGION-programmer-SO-projekt-ciastkarnia/blob/3d959858d1cec48501cb52056cd6b19987d67da4/src/kasjer.c#L16-L19)

## 2.4. Moduł: Piekarz (src/piekarz.c)
**Pętla Produkcji**
**Opis logiki:** Piekarz pracuje w niekończącej się pętli (dopóki flaga shm->sklep_otwarty jest aktywna). W każdej iteracji następuje losowanie rodzaju towaru z dostępnego asortymentu P_TYPY. Proces symuluje czas przygotowania wypieków za pomocą funkcji usleep(), a następnie przechodzi do fazy dostawy towaru na podajniki.
**Synchronizacja i Statystyki:** Każde udane "upieczenie" ciastka skutkuje dwiema operacjami w sekcji krytycznej:
    Inkrementacja shm->stan_podajnika[typ] (dostępność dla klientów).
    Inkrementacja shm->wytworzono[typ] (globalna statystyka produkcji dla Kierownika).
**Link:** [Przejdź do pętli piekarza](https://github.com/LEGION-programmer/LEGION-programmer-SO-projekt-ciastkarnia/blob/3d959858d1cec48501cb52056cd6b19987d67da4/src/piekarz.c#L9-L20)

**Limit Pojemności**
**Opis logiki:** Przed umieszczeniem upieczonego towaru w pamięci współdzielonej, proces Piekarza sprawdza warunek if (shm->stan_podajnika[typ] < POJEMNOSC_PODAJNIKA). Dzięki temu produkcja nie odbywa się w nieskończoność – system posiada fizyczny limit miejsca na "półkach" (podajnikach).
**Synchronizacja i Statystyki:** Jeśli miejsce jest dostępne, Piekarz inkrementuje stan konkretnego podajnika oraz aktualizuje globalny licznik wytworzonych towarów (shm->wytworzono). Wszystko to dzieje się wewnątrz sekcji krytycznej sterowanej semaforem binarnym o indeksie 1, co gwarantuje, że w tym samym momencie Klient nie odczyta nieaktualnego stanu magazynu.
**Cel:** Uniknięcie błędów przepełnienia (overflow) oraz realistyczne odwzorowanie ograniczeń logistycznych ciastkarni.
**Link:** [Przejdź do sprawdzania limitów](https://github.com/LEGION-programmer/LEGION-programmer-SO-projekt-ciastkarnia/blob/3d959858d1cec48501cb52056cd6b19987d67da4/src/piekarz.c#L14-L18)
---

## 3. Parametry Systemowe i System Calls

### 3.1. Stałe Globalne
| Nazwa Stałej          | Wartość | Opis                                                              |
|:---------------------:|:-------:|:-----------------------------------------------------------------:|
| **`PROJEKT_ID`**      | 99      | Ziarno dla `ftok` służące do generowania kluczy IPC.              |
| **`P_TYPY`**          | 11      | Liczba zdefiniowanych typów wyrobów piekarniczych.                |
| **`MAX_KLIENCI`**     | 5       | Maksymalna liczba klientów przebywających jednocześnie w sklepie. |
| **`LIMIT_PODAJNIKA`** | 10      | Maksymalna pojemność pojedynczego podajnika produktu.             |

### 3.2. Wykorzystane Funkcje Systemowe
| Wywołanie              | Zastosowanie w projekcie                                                        |
|:----------------------:|:-------------------------------------------------------------------------------:|
| **`shmget / shmat`**   | Alokacja i dołączenie segmentu pamięci dla struktur danych wspólnych.           |
| **`semget / semctl`**  | Tworzenie zestawu semaforów i inicjalizacja ich wartości (`SETVAL`).            |
| **`msgget / msgsnd`**  | Zarządzanie kolejnością zgłoszeń klientów do kasy (FIFO).                       |
| **`fork / execl`**     | Dynamiczne tworzenie procesów i ładowanie modułów binarnych.                    |
| **`msgctl(IPC_STAT)`** | Pobieranie metadanych o kolejce w celu dynamicznego skalowania liczby kasjerów. |

---

## 4. Schemat Komunikacji i Przepływu Danych
1. **Komunikacja Punktowa**: Klient przesyła zamówienie do Kasjera przez **Kolejkę Komunikatów**.
2. **Komunikacja Rozproszona**: Wszystkie procesy synchronizują stan magazynowy przez **Pamięć Współdzieloną**.
3. **Synchronizacja Sygnałowa**: Kierownik nadzoruje cykl życia systemu poprzez **Sygnały jądra** (SIGINT/SIGTERM) oraz flagi w SHM.

---