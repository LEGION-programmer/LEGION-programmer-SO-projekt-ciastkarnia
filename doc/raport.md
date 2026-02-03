# Dokumentacja Techniczna i Raport Końcowy — Symulacja Ciastkarni

**Autor:** Tomasz Kaczmarek (155269)  
**Temat:** Wieloprocesowa symulacja z wykorzystaniem mechanizmów IPC System V  

---

## 1. Założenia i Opis Projektu

### 1.1. Założenia Projektowe
Celem projektu było stworzenie stabilnego, zdecentralizowanego środowiska wieloprocesowego symulującego pracę ciastkarni. Główne filary systemu to:
* **Decentralizacja:** Każdy aktor (Piekarz, Klient, Kasjer, Kierownik) działa jako osobny proces uruchamiany przez parę funkcji `fork` + `execl`.
* **Synchronizacja:** Wykorzystanie semaforów System V do ochrony sekcji krytycznych w pamięci współdzielonej (SHM).
* **Komunikacja:** Asynchroniczne przekazywanie zamówień za pomocą kolejki komunikatów (MSG).
* **Skalowalność:** Dynamiczne zarządzanie liczbą kasjerów przez Kierownika w zależności od rzeczywistego obciążenia sklepu.

### 1.2. Zrealizowane Funkcjonalności
* **Cykl życia produktu:** Implementacja pełnego obiegu: produkcja (Piekarz), magazynowanie (SHM), sprzedaż (Kasjer) oraz konsumpcja (Klient).
* **Pancerna obsługa sygnałów:** Kierownik zarządza grupą procesów (`setpgid`), co pozwala na bezpieczne zamykanie systemu sygnałami `SIGINT`, `SIGUSR1` (inwentaryzacja) i `SIGUSR2` (ewakuacja).
* **Zautomatyzowane sprzątanie:** Gwarantowane usunięcie wszystkich kluczy IPC (`shmctl`, `semctl`, `msgctl` z flagą `IPC_RMID`) po zakończeniu symulacji.
* **Parametryzacja:** Pełne sterowanie limitem osób ($N$), progiem otwarcia kasy ($K$) oraz trybem stress-testu z poziomu argumentów wywołania programu.

### 1.3. Problemy i Zastosowane Rozwiązania
* **Wycieki zasobów IPC:** Rozwiązano poprzez funkcję `sprzatanie_ipc`, która wysyła sygnał do całej grupy procesów (`kill(0, sig)`), dając potomkom czas na poprawne odłączenie się od pamięci przed jej usunięciem.
* **Race Condition:** Wyeliminowano ryzyko błędnych stanów magazynowych poprzez rygorystyczne stosowanie semaforów binarnych (mutex) z flagą `SEM_UNDO`.
* **Synchronizacja logów:** Zastosowano `setvbuf` dla `stdout`, aby komunikaty z różnych procesów nie nakładały się na siebie w sposób nieczytelny.

---

## 2. Implementacja Funkcji Systemowych

### A. Tworzenie i obsługa plików
* **Zapis raportu (`fopen`, `fprintf`, `fclose`):** Realizowane w `kierownik.c`. Blok odpowiada za otwarcie pliku `raport.txt`, formatowanie danych statystycznych z SHM oraz weryfikację sumy kontrolnej:
  $$Wytworzono = Sprzedano + Zostało + Porzucono$$

### B. Tworzenie procesów
* **Cykl życia (`fork`, `execl`):** Wykorzystanie `fork` do powołania procesów i `execl` do podmiany obrazu procesu na dedykowane moduły. Zastosowanie `setpgid(0,0)` dla lidera grupy pozwala na masowe zarządzanie sygnałami.

### C. Obsługa sygnałów
* **Zarządzanie sygnałami (`signal`, `kill`):** Implementacja własnych procedur obsługi w każdym module. Zastosowanie `SIG_IGN` podczas czyszczenia zapobiega przerwaniu kluczowych operacji zwalniania zasobów.

### D. Synchronizacja procesów
* **Operacje na semaforach (`semop`, `SEM_UNDO`):** Funkcja `sem_op` w `ciastkarnia.h` stanowi uniwersalny interfejs blokowania. Flaga `SEM_UNDO` zapobiega zakleszczeniom (*deadlocks*) w razie awarii któregokolwiek procesu.
* **Inicjalizacja (`semget`, `semctl`):** Tworzenie zestawu dwóch semaforów: licznikowego (limit osób $N$) oraz binarnego (ochrona dostępu do SHM).

### E. Segmenty pamięci dzielonej
* **Zarządzanie SHM (`shmget`, `shmat`, `shmdt`, `shmctl`):** Mapowanie struktury `shared_data_t` we wszystkich procesach. Gwarantowane zwolnienie segmentu z jądra po zakończeniu symulacji.

### F. Kolejki komunikatów
* **Wymiana danych (`msgsnd`, `msgrcv`):** Przesyłanie struktur `order_t` między Klientem a Kasjerem. Kasjer realizuje odbiór blokujący podczas pracy oraz nieblokujący (`IPC_NOWAIT`) podczas inwentaryzacji.

---

## 3. Scenariusze Testowe (Weryfikacja automatyczna)

| ID | Test | Opis | Status |
| :--- | :--- | :--- | :--- |
| **T01** | **Stress Test** | 1000 klientów. Sprawdzenie spójności matematycznej raportu. | **PASS** |
| **T02** | **Skalowanie** | Wymuszenie tłumu. Sprawdzenie czy Kierownik otworzy Kasjerów > 1. | **PASS** |
| **T03** | **Komunikacja** | Weryfikacja poprawności przesyłu danych przez kolejkę komunikatów. | **PASS** |
| **T04** | **Wycieki IPC** | Sprawdzenie narzędziem `ipcs` czy jądro jest czyste po zamknięciu. | **PASS** |

---

## 4. Uwagi końcowe

Projekt demonstruje pełną kontrolę nad zasobami System V w środowisku Linux. Dzięki ścisłej separacji logiki na niezależne moduły binarne oraz kontroli sekcji krytycznych, system wykazuje wysoką odporność na błędy synchronizacji i poprawnie zarządza zasobami systemowymi nawet w sytuacjach awaryjnych.

---
**Link do repozytorium:** [GitHub - Projekt Ciastkarnia](https://github.com/LEGION-programmer/LEGION-programmer-SO-projekt-ciastkarnia/tree/main)