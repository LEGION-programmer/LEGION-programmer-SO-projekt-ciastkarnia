# Dokumentacja Techniczna i Raport Końcowy — Symulacja Ciastkarni
**Autor:** Tomasz Kaczmarek (155269)  
**Temat:** Wieloprocesowa symulacja z wykorzystaniem mechanizmów IPC System V.

---

## 1. Założenia i Opis Projektu

### 1.1. Założenia Projektowe
Celem projektu było stworzenie stabilnego, zdecentralizowanego środowiska wieloprocesowego symulującego pracę ciastkarni. Główne założenia to:
* **Decentralizacja:** Każdy aktor (Piekarz, Klient, Kasjer, Kierownik) działa jako osobny proces (`fork` + `execl`).
* **Synchronizacja:** Wykorzystanie semaforów System V do ochrony sekcji krytycznych w pamięci współdzielonej (SHM).
* **Komunikacja:** Przekazywanie zamówień za pomocą kolejki komunikatów (MSG).
* **Skalowalność:** Dynamiczne uruchamianie procesów kasjerów przez Kierownika w zależności od liczby osób w sklepie.

### 1.2. Co udało się zrealizować
* **Cykl życia produktu:** Implementacja pełnego obiegu: produkcja (Piekarz), magazynowanie (SHM), sprzedaż (Kasjer) oraz konsumpcja (Klient).
* **Pancerna obsługa sygnałów:** Kierownik zarządza grupą procesów (`setpgid`), co pozwala na bezpieczne zamykanie systemu sygnałami `SIGUSR1` (inwentaryzacja) i `SIGUSR2` (ewakuacja).
* **Zautomatyzowane sprzątanie:** Gwarantowane usunięcie wszystkich kluczy IPC (`shmctl`, `semctl`, `msgctl`) po zakończeniu symulacji.
* **Parametryzacja:** Sterowanie limitem osób (N), progiem otwarcia kasy (K) oraz trybem stress-testu z poziomu argumentów `argv`.

### 1.3. Problemy i Zastosowane Rozwiązania
* **Wycieki zasobów IPC:** Rozwiązano poprzez implementację funkcji `sprzatanie_ipc`, która przed usunięciem zasobów wysyła sygnał do całej grupy procesów (`kill(0, sig)`), dając potomkom czas na odłączenie się od pamięci.
* **Race Condition:** Wyeliminowano ryzyko błędnych stanów magazynowych poprzez rygorystyczne stosowanie semaforów binarnych (mutex) z flagą `SEM_UNDO`.
* **Synchronizacja logów:** Zastosowano `setvbuf` dla `stdout`, aby komunikaty z różnych procesów nie nakładały się na siebie w sposób nieczytelny.

---

## 2. Implementacja Funkcji Systemowych



### A. Tworzenie i obsługa plików
* **Zapis raportu końcowego (`fopen`, `fprintf`, `fclose`):** `kierownik.c`
  *Blok realizuje otwarcie strumienia do pliku `raport.txt`, formatowanie danych statystycznych pobranych z SHM oraz weryfikację sum kontrolnych (Wytworzono - Sprzedano = Zostało).*

### B. Tworzenie procesów
* **Cykl życia procesu potomnego (`fork`, `execl`):** `kierownik.c`
  *Wykorzystanie `fork` do tworzenia Piekarza, Kasjerów i Klientów, a następnie podmiana obrazu procesu za pomocą `execl`. Zastosowanie `setpgid(0,0)` dla lidera grupy.*

### C. Obsługa sygnałów
* **Zarządzanie sygnałami (`signal`, `kill`):** `kierownik.c`, `kasjer.c`, `piekarz.c`
  *Implementacja własnych procedur obsługi dla `SIGINT` i `SIGUSR1/2`. Zastosowanie `SIG_IGN` dla sygnałów, które nie powinny przerywać pracy kluczowych sekcji sprzątających.*

### D. Synchronizacja procesów
* **Operacje na semaforach (`semop`, `SEM_UNDO`):** `ciastkarnia.h` (funkcja `sem_op`)
  *Implementacja uniwersalnego mechanizmu blokowania i zwalniania zasobów. Flaga `SEM_UNDO` zapobiega deadlockom w razie awarii procesu.*
* **Inicjalizacja zestawu semaforów (`semget`, `semctl`):** `kierownik.c`
  *Tworzenie zestawu dwóch semaforów: licznikowego (limit osób) oraz binarnego (ochrona SHM).*

### E. Segmenty pamięci dzielonej
* **Alokacja i mapowanie (`shmget`, `shmat`, `shmdt`, `shmctl`):** `kierownik.c`
  *Zarządzanie strukturą `shared_data_t`. Gwarantowane zwolnienie segmentu z jądra (`IPC_RMID`) po zakończeniu symulacji.*

### F. Kolejki komunikatów
* **Wymiana danych (`msgsnd`, `msgrcv`):** `klient.c`, `kasjer.c`
  *Przesyłanie struktur `order_t`. Kasjer realizuje hybrydowy odbiór: blokujący podczas normalnej pracy i nieblokujący (`IPC_NOWAIT`) podczas czyszczenia kolejki w fazie inwentaryzacji.*

---

## 3. Scenariusze Testowe (Weryfikacja automatyczna)



Do projektu dołączono skrypt `testy_ipc.sh`, który automatyzuje proces weryfikacji:

| ID | Test | Opis | Status |
|:---|:---|:---|:---:|
| **T01** | **Stress Test** | 1000 klientów. Sprawdzenie spójności matematycznej raportu. | **PASS** |
| **T02** | **Skalowanie** | Wymuszenie tłumu. Sprawdzenie czy Kierownik otworzy Kasy > 1. | **PASS** |
| **T03** | **Komunikacja** | Weryfikacja poprawności przesyłu danych przez MSG Queue. | **PASS** |
| **T04** | **Wycieki IPC** | Sprawdzenie czy `ipcs` jest puste po zakończeniu programu. | **PASS** |

---

## 4. Uwagi końcowe
Projekt demonstruje pełną kontrolę nad zasobami System V. Dzięki separacji logiki na niezależne moduły binarne oraz ścisłej kontroli dostępu do pamięci, system jest odporny na wyścigi danych i poprawnie zarządza zasobami systemowymi w sytuacjach awaryjnych.