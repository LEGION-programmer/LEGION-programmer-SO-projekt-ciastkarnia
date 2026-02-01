# Projekt SO — Ciastkarnia 
**Autor:** Tomasz Kaczmarek  
**Nr albumu:** 155269  
**Temat:** Wieloprocesowa symulacja ciastkarni z wykorzystaniem mechanizmów IPC System V  

---

## 1. Opis symulacji

Symulacja odwzorowuje pracę **ciastkarni z samoobsługowym sklepem**, w której niezależne procesy współpracują w czasie rzeczywistym bez scentralizowanego zarządcy. System demonstruje rozwiązanie klasycznego problemu "Producenta i Konsumenta" w środowisku wieloprocesowym.

* **Piekarz** – działa jako producent; wytwarza produkty (11 typów) i umieszcza je na podajnikach w pamięci współdzielonej.
* **Klienci** – generowani falami; wchodzą do sklepu, jeśli pozwala na to semafor licznikowy (limit N). Pobierają towary, a w przypadku braków stosują mechanizm ponawiania prób (*retry*).
* **Kasjerzy (Dynamiczne stanowiska)** – domyślnie pracuje jeden kasjer. Jeśli kolejka komunikatów przekroczy próg (K = N/2), Kierownik dynamicznie uruchamia drugie stanowisko.
* **Kierownik** – pełni rolę nadrzędną: inicjalizuje zasoby IPC, zarządza sygnałami, monitoruje obciążenie i generuje raport końcowy.

Wszystkie role działają jako **osobne procesy** (uruchamiane przez `fork` i `exec`), komunikujące się poprzez standard IPC System V.

---

## 2. Szczegółowa rola procesów

### **Piekarz**
- Działa w pętli, symulując czas pieczenia poprzez `usleep`.
- Odświeża stan **podajników** w pamięci współdzielonej, zwiększając licznik danego produktu.
- Dostęp do magazynu chroniony jest binarnym semaforem (**mutex**).
- Każda udana operacja pieczenia jest logowana na standardowe wyjście.



### **Klient**
- **Wejście:** Kontrolowane semaforem licznikowym (limit N osób w sklepie).
- **Zakupy:** Wybiera losowe produkty i wysyła zamówienie do kolejki komunikatów.
- **Synchronizacja:** Po wysłaniu zamówienia zwalnia miejsce w sklepie (podnosi semafor), umożliwiając wejście kolejnej osobie.

### **Kasjer (Skalowanie dynamiczne)**
- Odbiera zamówienia z kolejki komunikatów za pomocą funkcji `msgrcv`.
- Sumuje sprzedane towary w strukturze statystyk znajdującej się w pamięci współdzielonej.
- **Polityka kasy:**
    - **Kasa 1:** Działa od początku symulacji.
    - **Kasa 2:** Uruchamiana jest na podstawie liczby klientów aktualnie przebywających w sklepie (monitorowanie semafora licznikowego przez Kierownika).

### **Kierownik**
- **Zarządzanie IPC:** Tworzy i usuwa klucze `ftok`, segmenty pamięci (`shm`), semafory (`sem`) oraz kolejkę (`msg`).
- **Obsługa sygnałów:** Implementuje obsługę `SIGINT` (Ctrl+C), co wyzwala procedurę bezpiecznego zamknięcia: ustawienie flagi zamknięcia, zakończenie procesów potomnych, wyświetlenie raportu i usunięcie zasobów z jądra.
- **Raport końcowy:** Prezentuje tabelaryczne zestawienie produkcji i sprzedaży każdego z 11 typów produktów.

---

## 3. Mechanizmy IPC użyte w projekcie

| Mechanizm | Zastosowanie |
|:--- |:--- |
| **Pamięć dzielona (`shm`)** | Przechowywanie stanu podajników, liczników statystyk i flagi kontrolnej. |
| **Semafory (`sem`)** | Kontrola limitu osób w sklepie oraz ochrona sekcji krytycznej (mutex). |
| **Kolejka komunikatów (`msg`)** | Asynchroniczne przesyłanie zamówień od Klientów do Kasjerów. |
| **Sygnały (`signal`)** | Obsługa zamknięcia systemu i czyszczenia zasobów. |
| **Procesy (`fork/exec`)** | Fizyczny podział na niezależne moduły wykonywalne. |



---

## 4. Instrukcja Kompilacji i Uruchomienia

### **Kompilacja**
Projekt wykorzystuje plik `Makefile` do automatyzacji budowania czterech modułów. Aby skompilować system, należy wykonać:
```bash
make clean
make

Uruchomienie

```
./kierownik
```bash

## 5. Scenariusze Testowe

W celu weryfikacji poprawności implementacji mechanizmów synchronizacji oraz odporności systemu na sytuacje graniczne, przeprowadzono następujące testy:

| ID | Nazwa Testu | Cel Testu | Oczekiwany Wynik | Status |
|:---|:---|:---|:---|:---:|
| **T01** | **Synchronizacja (Race Condition)** | Sprawdzenie czy wielu klientów nie pobierze tego samego produktu jednocześnie. | Stan podajnika nigdy nie spada poniżej 0. Dane w raporcie są spójne. | **OK** |
| **T02** | **Dynamiczne Skalowanie** | Weryfikacja reakcji Kierownika na dużą liczbę komunikatów w kolejce. | Po przekroczeniu progu N/2 komunikatów, uruchamiany jest proces Kasjer 2. | **OK** |
| **T03** | **Sprzątanie Zasobów** | Sprawdzenie czy po sygnale SIGINT (Ctrl+C) zasoby IPC są usuwane. | Komenda `ipcs` nie wykazuje żadnych aktywnych zasobów po zamknięciu programu. | **OK** |
| **T04** | **Pusty Koszyk** | Weryfikacja optymalizacji procesów przy braku towaru. | Klient bez produktów opuszcza sklep bez wysyłania wiadomości do kolejki. | **OK** |

### Szczegółowy opis wyników:

#### **Test 1 – Integralność danych**
Dzięki zastosowaniu semafora binarnego (Mutex) jako blokady sekcji krytycznej, operacje na pamięci współdzielonej są atomowe. Testy wykazały, że nawet przy maksymalnym zagęszczeniu klientów, liczba sprzedanych produktów dokładnie odpowiada operacjom dekrementacji stanu podajnika.

#### **Test 2 – Skalowalność (Load Balancing)**
Mechanizm monitorowania kolejki przy użyciu `msgctl` z flagą `IPC_STAT` działa poprawnie. Logi systemowe potwierdzają, że proces Kasjera 2 jest wywoływany tylko w momentach rzeczywistego zatoru, co optymalizuje wykorzystanie procesora.



#### **Test 3 – Zarządzanie sygnałami**
Zaimplementowany interfejs obsługi sygnałów gwarantuje, że proces nadrzędny (Kierownik) przed zakończeniem własnej pracy wysyła sygnały zakończenia do wszystkich procesów potomnych (`kill`) oraz zwalnia identyfikatory IPC (`shmctl`, `semctl`, `msgctl`). Zapobiega to wyciekom pamięci w systemie operacyjnym.

#### **Test 4 – Cierpliwość klienta (Retry logic)**
Klienci nie rezygnują z zakupu natychmiast po napotkaniu pustego podajnika. Mechanizm 3-krotnego ponowienia próby z opóźnieniem `usleep` pozwala na płynną współpracę z Piekarzem, redukując liczbę "pustych" wizyt w sklepie.

### **Uruchomienie**

Symulację uruchamiamy za pomocą procesu nadrzędnego (Kierownika). Program przyjmuje dwa opcjonalne argumenty wiersza poleceń:

```bash
./kierownik [czas_sekundy] [prog_kasy]
czas_sekundy (domyślnie 30): Określa, po jakim czasie sklep zostanie zamknięty dla nowych klientów.

prog_kasy (domyślnie 5): Określa liczbę klientów w kolejce, powyżej której Kierownik otworzy drugą kasę.

Przykład: ./kierownik 60 2 — Sklep działa przez minutę, a druga kasa otworzy się już przy 2 osobach w kolejce.

### 5. Scenariusze Testowe

W celu weryfikacji poprawności implementacji mechanizmów synchronizacji oraz odporności systemu na sytuacje graniczne, przeprowadzono testy automatyczne zawarte w dedykowanym skrypcie `testy_ipc.sh`.

| ID | Nazwa Testu | Cel i Metodyka | Oczekiwany Wynik | Status |
|:---|:---|:---|:---|:---:|
| **T01** | **Stress Test** | Symulacja 1000 klientów działających asynchronicznie w krótkim czasie. | Raport końcowy wykazuje pełną spójność danych (W-S=Z). | **OK** |
| **T02** | **Skalowanie Kas** | Generowanie nagłego tłumu klientów przy niskim progu wydajności **K**. | Kierownik wykrywa obciążenie i uruchamia proces **Kasjer 2**. | **OK** |
| **T03** | **Kolejka Msg** | Weryfikacja przepływu struktur `order_t` od Klienta do Kasjera. | Licznik sprzedaży w raporcie jest większy od zera. | **OK** |
| **T04** | **Czyszczenie IPC** | Sprawdzenie stanu jądra systemu po wymuszeniu raportu sygnałem `SIGINT`. | Narzędzie `ipcs` nie raportuje żadnych aktywnych zasobów projektu. | **OK** |



#### Szczegółowy opis wyników:

* **Test 1 – Integralność danych (Concurrency):**
    Mimo ekstremalnego obciążenia (1000 procesów), użycie semaforów binarnych z flagą `SEM_UNDO` wyeliminowało zjawisko wyścigów (*race conditions*). Pamięć współdzielona została zaktualizowana poprawnie, co potwierdza komunikat "Weryfikacja: OK" w raporcie końcowym.

* **Test 2 – Skalowalność (Load Balancing):**
    Test potwierdził poprawność algorytmu monitorowania obciążenia. Kierownik, odczytując stan semafora licznikowego (liczba osób aktualnie przebywających w sklepie), poprawnie zinterpretował zator i powołał nowy proces Kasjera za pomocą funkcji `fork()` i `exec()`.

* **Test 3 – Komunikacja asynchroniczna:**
    Weryfikacja wykazała, że kolejka komunikatów poprawnie buforuje zamówienia. Dzięki zastosowaniu `usleep` u Kasjera (symulacja czasu obsługi), procesy Klientów mogły opuścić sklep natychmiast po wysłaniu zamówienia, co udowadnia poprawność asynchronicznego modelu komunikacji IPC.

* **Test 4 – Zarządzanie sygnałami i zasobami:**
    Dzięki grupowaniu procesów (`setpgid`) i mechanizmowi ignorowania sygnałów u lidera grupy w trakcie procedury sprzątania, proces Kierownika poprawnie obsłużył sygnał `SIGINT`. Wszystkie segmenty pamięci współdzielonej (`shm`), kolejki komunikatów (`msg`) oraz zestawy semaforów (`sem`) zostały poprawnie usunięte z jądra systemu.

## 6. Analiza synchronizacji i bezpieczeństwa (Techniczne)

Podczas implementacji systemu Ciastkarnia, szczególną uwagę poświęcono stabilności procesów oraz odporności na błędy typowe dla środowisk wieloprocesowych.

### 1. Atomowość i ochrona przed wyścigami (Race Conditions)
Każda modyfikacja struktur w pamięci współdzielonej (np. aktualizacja stanu podajnika przez Piekarza czy liczników sprzedaży przez Kasjera) odbywa się wewnątrz sekcji krytycznej chronionej przez **semafor binarny (Mutex)**. Gwarantuje to, że w danym momencie tylko jeden proces może zmieniać dane w `shared_data_t`, co eliminuje błędy spójności przy dużej liczbie klientów.

### 2. Wykorzystanie flagi `SEM_UNDO`
Wszystkie operacje na semaforach (blokowanie i zwalnianie) wykorzystują flagę `SEM_UNDO`. Jest to kluczowy mechanizm bezpieczeństwa: jeśli proces Klienta lub Kasjera zostanie nagle przerwany (np. przez błąd systemu lub sygnał `SIGKILL`), jądro systemu automatycznie wycofa zmiany wprowadzone przez ten proces w semaforach. Zapobiega to trwałemu zablokowaniu (*deadlock*) zasobów dla pozostałych procesów.



### 3. "Pancerna" obsługa sygnałów i zarządzanie grupą
W celu uniknięcia błędów typu `User defined signal 2` (wynikających z wyścigów sygnałów), zastosowano dwie zaawansowane techniki:
* **`setpgid(0, 0)`**: Kierownik tworzy nową grupę procesów, stając się jej liderem. Pozwala to na precyzyjne adresowanie sygnałów do wszystkich potomków jednocześnie.
* **Blokada Sygnałów (`SIG_IGN`)**: W trakcie procedury sprzątania, Kierownik tymczasowo ignoruje sygnały przychodzące do niego samego. Dzięki temu nie ginie od sygnału wysłanego do całej grupy, co pozwala mu na bezpieczne wygenerowanie raportu końcowego i usunięcie struktur IPC.

### 4. Asynchroniczność i buforowanie zamówień
Zastosowanie kolejki komunikatów (`msg`) oddziela proces Klienta od procesu Kasjera. Klient po wykonaniu operacji `msgsnd()` może natychmiast opuścić system, nie czekając na zakończenie transakcji. Kasjerzy pobierają zamówienia asynchronicznie, co pozwala na płynną obsługę nawet przy chwilowych skokach obciążenia.



### 5. Gwarancja czystości systemu (Garbage Collection)
Projekt implementuje rygorystyczną politykę czyszczenia zasobów. Niezależnie od tego, czy symulacja zakończy się naturalnie, czy zostanie przerwana przez użytkownika (`SIGINT`), wywoływana jest funkcja `shmctl`, `semctl` oraz `msgctl` z flagą `IPC_RMID`. Gwarantuje to, że po zakończeniu pracy w systemie nie zostają żadne osierocone segmenty pamięci ani kolejki, co jest kluczowe dla stabilności środowiska operacyjnego.

Link do repozytorium GitHub
→ https://github.com/LEGION-programmer/LEGION-programmer-SO-projekt-ciastkarnia/tree/main