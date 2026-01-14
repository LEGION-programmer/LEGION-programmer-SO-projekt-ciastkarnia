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
- **Wejście:** Podlega kontroli semafora licznikowego (maksymalnie N osób w sklepie).
- **Zakupy:** Wybiera losowe produkty. Jeśli podajnik jest pusty, klient oczekuje (3 próby), dając szansę piekarzowi na dostarczenie towaru.
- **Pusty Koszyk:** Jeśli po wszystkich próbach klient nic nie kupił, opuszcza sklep bez angażowania kasjera (optymalizacja zasobów IPC).
- **Kolejka:** Klienci z produktami przesyłają swoje zamówienie do **kolejki komunikatów (FIFO)**.

### **Kasjer (Skalowanie dynamiczne)**
- Odbiera zamówienia z kolejki komunikatów za pomocą funkcji `msgrcv`.
- Sumuje sprzedane towary w strukturze statystyk znajdującej się w pamięci współdzielonej.
- **Polityka kasy:**
    - **Kasa 1:** Działa od początku symulacji.
    - **Kasa 2:** Uruchamiana przez Kierownika tylko wtedy, gdy liczba komunikatów w kolejce oczekujących przekroczy zdefiniowany próg bezpieczeństwa.

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

Link do repozytorium GitHub
→ https://github.com/LEGION-programmer/LEGION-programmer-SO-projekt-ciastkarnia/tree/main