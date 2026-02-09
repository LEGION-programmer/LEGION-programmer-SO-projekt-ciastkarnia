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
* **Skalowalność:** Dynamiczne zarządzanie liczbą kasjerów przez Kierownika w zależności od rzeczywistego obciążenia sklepu (próg $K$).

### 1.2. Zrealizowane Funkcjonalności
* **Cykl życia produktu:** Implementacja pełnego obiegu: produkcja (Piekarz), magazynowanie (SHM), sprzedaż (Kasjer) oraz konsumpcja (Klient).
* **Pancerna obsługa sygnałów:** Kierownik zarządza grupą procesów (`setpgid`), co pozwala na bezpieczne zamykanie systemu sygnałami `SIGINT` (zamknięcie), `SIGUSR1` (sterowanie kasami) i `SIGUSR2` (ewakuacja).
* **Zautomatyzowane sprzątanie:** Gwarantowane usunięcie wszystkich zasobów IPC (`shmctl`, `semctl`, `msgctl` z flagą `IPC_RMID`) po zakończeniu symulacji, realizowane przez proces nadrzędny.
* **Parametryzacja:** Sterowanie limitem osób ($N$), progiem otwarcia kasy ($K$) oraz czasem trwania symulacji z poziomu argumentów wywołania.

### 1.3. Problemy i Zastosowane Rozwiązania
* **Wycieki zasobów IPC:** Rozwiązano poprzez funkcję `sprzataj`, która najpierw wygasza procesy potomne, a dopiero po ich zakończeniu (`wait`) usuwa struktury z jądra.
* **Błąd "User defined signal 2":** Wyeliminowano poprzez mechanizm **maskowania sygnałów** (`sigprocmask`). Kierownik blokuje `SIGUSR2` dla siebie na czas rozsyłania go do grupy, co zapobiega przerwaniu jego pracy przez system.
* **Race Condition:** Wyeliminowano ryzyko błędnych stanów magazynowych poprzez rygorystyczne stosowanie semaforów binarnych (mutex) z flagą `SEM_UNDO` oraz atomowe transakcje w procesie Klienta.

---

## 2. Implementacja Funkcji Systemowych

### A. Tworzenie i obsługa plików
* **Zapis raportu (`fopen`, `fprintf`, `fclose`):** Realizowane w `kierownik.c`. Po otrzymaniu `SIGINT`, proces agreguje dane z SHM i weryfikuje sumę kontrolną:
  $$Wytworzono = Sprzedano + Stan\_podajników + Porzucono$$

### B. Tworzenie procesów
* **Cykl życia (`fork`, `execl`):** Wykorzystanie `fork` do powołania procesów i `execl` do podmiany obrazu procesu. Zastosowanie `setpgid(0,0)` dla lidera grupy pozwala na masowe zarządzanie sygnałami poprzez `kill(0, sig)`.

### C. Obsługa sygnałów
* **Zaawansowana obsługa (`sigaction`):** Zastąpienie funkcji `signal` strukturą `sigaction` z flagą `SA_RESTART`. Pozwala to na uniknięcie przerywania wywołań systemowych przez sygnały (np. przy długim oczekiwaniu na wiadomość w kolejce).



### D. Synchronizacja procesów
* **Operacje na semaforach (`semop`, `SEM_UNDO`):** Funkcja `sem_op` stanowi uniwersalny interfejs. Flaga `SEM_UNDO` zapobiega zakleszczeniom (*deadlocks*) w razie awarii któregokolwiek procesu – jądro automatycznie wycofuje zmiany w semaforach po śmierci procesu.
* **Inicjalizacja (`semget`, `semctl`):** Tworzenie zestawu dwóch semaforów: licznikowego (limit osób $N$) oraz binarnego (mutex dla SHM).

### E. Segmenty pamięci dzielonej
* **Zarządzanie SHM (`shmget`, `shmat`, `shmdt`, `shmctl`):** Mapowanie struktury `shared_data_t` we wszystkich procesach. Dane są chronione semaforem, co zapewnia integralność statystyk przy dużej współbieżności.

### F. Kolejki komunikatów
* **Wymiana danych (`msgsnd`, `msgrcv`):** Przesyłanie struktur `order_t` (zamówienia). Kasjer nr 2 realizuje odbiór nieblokujący (`IPC_NOWAIT`) po otrzymaniu sygnału zamknięcia, aby "wyczyścić" kolejkę przed wyjściem.



---

## 3. Scenariusze Testowe (Weryfikacja automatyczna)

| ID | Test | Opis | Status |
| :--- | :--- | :--- | :--- |
| **T01** | **Stress Test** | 5000 klientów. Weryfikacja spójności danych w pamięci współdzielonej (shm) przy ekstremalnym obciążeniu. Sprawdzenie, czy suma sprzedanych i porzuconych ciastek zgadza się z produkcją. Uruchomić ./tests/test_5000_klientow.sh | **PASS** |
| **T02** | **Awaria pieca** | Symulacja braku towaru poprzez wstrzymanie produkcji Piekarza. Testuje poprawność inkrementacji licznika porzucone_zamowienia oraz zachowanie procesów Klientów przy pustych podajnikach. Przed testem zwiększyć usleep w ./src/piekarz.c, uruchomić ./tests/test_awaria_pieca.sh | **PASS** |
| **T03** | **bottleneck** | Ograniczenie pojemności sklepu (Semafor wejściowy). Weryfikacja mechanizmu blokowania procesów klientów przed wejściem (kolejka systemowa) oraz płynności rotacji przy małym limicie miejsc. Dla lepszego efektu odkomentowac wszystkie usleep w ./src/klient.c. Uruchimić ./tests/test_bottleneck.sh | **PASS** |
| **T04** | **Wyprzedaz** | Test przepustowości IPC. Wysłanie fali 300 klientów przy aktywnych dwóch kasach. Weryfikacja mechanizmu asynchronicznej kolejki komunikatów (Message Queue) jako bufora między klientem a kasjerem. Uruchomić ./tests/test_wyprzedaz.sh | **PASS** |


## 4. Analiza Rozwiązań i Testów

### 4.1. Dodane elementy specjalne
* **Mechanizm Lidera Grupy:** Zastosowanie `setpgid(0,0)` w procesie Kierownika pozwala na utworzenie nowej grupy procesów. Dzięki temu polecenie `kill(0, SIGUSR2)` trafia precyzyjnie do wszystkich procesów składowych symulacji (klientów, kasjerów, piekarza), co pozwala na precyzyjne zarządzanie sygnałami bez ryzyka osierocenia procesów w systemie.
* **Maskowanie sygnałów:** Wykorzystanie `sigprocmask` do blokowania sygnału `SIGUSR2` w procesie Kierownika na czas rozsyłania go do grupy. Pozwala to Kierownikowi "krzyknąć" do wszystkich procesów o ewakuacji, samemu pozostając odpornym na ten sygnał, co jest kluczowe dla poprawnego wygenerowania raportu końcowego.
* **Interfejs:** Implementacja kolorowania wyjścia terminala (**ANSI Escape Codes**). Różne kolory dla ról (Piekarz – Zielony, Kasjer – Niebieski/Cyan, Kierownik – Żółty/Magenta, Błędy – Czerwony) znacząco ułatwiają śledzenie logiki dynamicznego otwierania kas oraz monitorowanie stanów magazynowych w czasie rzeczywistym.

### 4.2. Zauważone problemy z testami
* **Test Ewakuacji (T04):** Podczas wstępnych testów zauważono, że domyślna obsługa sygnału `USR2` (terminacja) zabijała proces Kierownika natychmiast po wywołaniu `kill(0, SIGUSR2)`, zanim ten zdążył odebrać statystyki od klientów. Rozwiązano to poprzez implementację zaawansowanej obsługi `sigaction` oraz wspomniane wcześniej blokowanie sygnału (`SIG_BLOCK`) na czas operacji rozsyłania sygnału do grupy.



## 5. Wykaz wykorzystanych konstrukcji (Linki do GitHub)

Poniższe linki prowadzą do istotnych fragmentów kodu obrazujących wymagane konstrukcje systemowe:

### a. Tworzenie i obsługa procesów
* [`fork()` i `execl()` – tworzenie procesów roboczych](https://github.com/LEGION-programmer/LEGION-programmer-SO-projekt-ciastkarnia/blob/main/kierownik.c)
* [`wait()` i `waitpid()` – synchronizacja zakończenia](https://github.com/LEGION-programmer/LEGION-programmer-SO-projekt-ciastkarnia/blob/main/kierownik.c)

### b. Obsługa sygnałów
* [`sigaction()` – zaawansowane handlery](https://github.com/LEGION-programmer/LEGION-programmer-SO-projekt-ciastkarnia/blob/main/kierownik.c)
* [`kill(0, sig)` – sterowanie grupą procesów](https://github.com/LEGION-programmer/LEGION-programmer-SO-projekt-ciastkarnia/blob/main/kierownik.c)
* [`sigprocmask()` – maskowanie sygnałów](https://github.com/LEGION-programmer/LEGION-programmer-SO-projekt-ciastkarnia/blob/main/kierownik.c)

### c. Synchronizacja procesów (Semafory)
* [`semget()` i `semctl()` – inicjalizacja zasobów](https://github.com/LEGION-programmer/LEGION-programmer-SO-projekt-ciastkarnia/blob/main/kierownik.c)
* [`semop()` – operacje P i V (sekcje krytyczne)](https://github.com/LEGION-programmer/LEGION-programmer-SO-projekt-ciastkarnia/blob/main/ciastkarnia.h)

### d. Segmenty pamięci dzielonej (SHM)
* [`shmget()` i `shmat()` – tworzenie i mapowanie pamięci](https://github.com/LEGION-programmer/LEGION-programmer-SO-projekt-ciastkarnia/blob/main/kierownik.c)
* [`shmctl(IPC_RMID)` – czyszczenie zasobów](https://github.com/LEGION-programmer/LEGION-programmer-SO-projekt-ciastkarnia/blob/main/kierownik.c)

### e. Kolejki komunikatów (MSG)
* [`msgsnd()` – wysyłanie zamówień przez klienta](https://github.com/LEGION-programmer/LEGION-programmer-SO-projekt-ciastkarnia/blob/main/klient.c)
* [`msgrcv()` – odbieranie danych przez kasjera](https://github.com/LEGION-programmer/LEGION-programmer-SO-projekt-ciastkarnia/blob/main/kasjer.c)

### f. Obsługa plików i błędów
* [`fopen()` i `fprintf()` – generowanie raportu](https://github.com/LEGION-programmer/LEGION-programmer-SO-projekt-ciastkarnia/blob/main/kierownik.c)
* [`perror()` i `errno` – funkcja obsługi błędów](https://github.com/LEGION-programmer/LEGION-programmer-SO-projekt-ciastkarnia/blob/main/ciastkarnia.h)


---

## 6. Uwagi końcowe

Projekt demonstruje pełną kontrolę nad zasobami System V w środowisku Linux. Zastosowanie lidera grupy procesów, maskowania sygnałów oraz nowoczesnej obsługi `sigaction` pozwoliło na stworzenie systemu odpornego na typowe błędy synchronizacji. System poprawnie zarządza zasobami nawet w sytuacjach awaryjnych (ewakuacja), zachowując przy tym pełną spójność danych statystycznych.

---

**Link do repozytorium:** [GitHub - Projekt Ciastkarnia](https://github.com/TomaszKaczmarek-Dev/SO-ciastkarnia)

