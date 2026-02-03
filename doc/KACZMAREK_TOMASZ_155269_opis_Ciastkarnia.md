# Projekt SO — Ciastkarnia 

**Autor:** Tomasz Kaczmarek  
**Nr albumu:** 155269  
**Temat:** Wieloprocesowa symulacja ciastkarni z wykorzystaniem mechanizmów IPC System V  

---

## 1. Opis symulacji

Symulacja odwzorowuje pracę **ciastkarni z samoobsługowym sklepem**, w której niezależne procesy współpracują w czasie rzeczywistym bez scentralizowanego zarządcy. System demonstruje rozwiązanie klasycznego problemu "Producenta i Konsumenta" w środowisku wieloprocesowym.

* **Piekarz** – działa jako producent; wytwarza produkty (11 typów) i umieszcza je na podajnikach w pamięci współdzielonej.
* **Klienci** – generowani falami; wchodzą do sklepu, jeśli pozwala na to semafor licznikowy (limit N). Kupują towary i wysyłają zamówienia do kasy.
* **Kasjerzy (Dynamiczne stanowiska)** – domyślnie pracuje jeden kasjer. Jeśli liczba osób w sklepie przekroczy próg (K), Kierownik dynamicznie uruchamia drugie stanowisko.
* **Kierownik** – pełni rolę nadrzędną: inicjalizuje zasoby IPC, zarządza sygnałami, monitoruje obciążenie i generuje raport końcowy.

Wszystkie role działają jako **osobne procesy** (uruchamiane przez `fork` i `exec`), komunikujące się poprzez standard IPC System V.

---

## 2. Szczegółowa rola procesów

### **Piekarz**
- Działa w pętli, symulując czas pieczenia za pomocą `usleep`.
- Odświeża stan **podajników** w pamięci współdzielonej, zwiększając licznik danego produktu.
- Każda operacja zapisu do magazynu chroniona jest binarnym semaforem (**mutex**).

### **Klient**
- **Wejście:** Kontrolowane semaforem licznikowym (limit N osób w sklepie).
- **Zakupy:** Losuje listę produktów i przesyła strukturę zamówienia do kolejki komunikatów.
- **Synchronizacja:** Po wysłaniu zamówienia spędza w sklepie określony czas (symulacja pakowania), a następnie zwalnia miejsce (podnosi semafor).
- **Ewakuacja:** W przypadku nagłego przerwania pracy, produkty pobrane przez klienta, a jeszcze nieopłacone, są zliczane w polu `porzucono`, co pozwala zachować bilans masy w raporcie.

### **Kasjer (Skalowanie dynamiczne)**
- Odbiera zamówienia z kolejki komunikatów za pomocą funkcji `msgrcv`.
- Weryfikuje stan towarów w pamięci współdzielonej i aktualizuje statystyki sprzedaży.
- **Polityka kasy:** Kasa 2 uruchamiana jest dynamicznie przez Kierownika, gdy liczba osób w sklepie przekroczy zadany próg.

### **Kierownik**
- **Zarządzanie IPC:** Tworzy i usuwa klucze `ftok`, segmenty pamięci (`shm`), semafory (`sem`) oraz kolejkę (`msg`).
- **Obsługa sygnałów:** Implementuje obsługę `SIGINT`, co wyzwala procedurę bezpiecznego zamknięcia systemu: najpierw blokowany jest napływ klientów, a po synchronizacji danych (2s) zamykany jest personel i generowany raport.

---

## 3. Mechanizmy IPC użyte w projekcie



| Mechanizm | Zastosowanie |
| :--- | :--- |
| **Pamięć dzielona (`shm`)** | Stan podajników, liczników statystyk (wytworzono/sprzedano) i flagi kontrolnej. |
| **Semafory (`sem`)** | Limit osób w sklepie (licznikowy) oraz ochrona sekcji krytycznej (binarny mutex). |
| **Kolejka komunikatów (`msg`)** | Asynchroniczne przesyłanie struktur zamówień od Klientów do Kasjerów. |
| **Sygnały (`signal`)** | Obsługa zamknięcia systemu (SIGINT), sterowanie pracą kasjerów (SIGUSR1/2). |
| **Procesy (`fork/exec`)** | Fizyczny podział na niezależne moduły wykonywalne działające w jednej grupie. |

---

## 4. Instrukcja Kompilacji i Uruchomienia

### **Kompilacja**
```bash
make clean
make

Uruchomienie
Bash

./kierownik <limit_N> <prog_K> [stress_test]
# Przykład: ./kierownik 100 10 0

## 5. Scenariusze Testowe

W celu weryfikacji poprawności implementacji mechanizmów synchronizacji oraz odporności systemu na sytuacje graniczne, przeprowadzono automatyczne testy integracyjne.

| ID | Nazwa Testu | Cel i Metodyka | Oczekiwany Wynik | Status |
| :--- | :--- | :--- | :--- | :---: |
| **T01** | **Stress Test** | Symulacja 1000 klientów działających asynchronicznie w paczkach po 100. | Pełna spójność danych: Wytworzono = Sprzedano + Na stanie + Porzucono. | **OK** |
| **T02** | **Skalowanie Kas** | Generowanie nagłego tłumu klientów przy niskim progu wydajności **K**. | Kierownik wykrywa obciążenie i poprawnie uruchamia proces **Kasjer 2**. | **OK** |
| **T03** | **Integralność danych** | Weryfikacja ochrony SHM przed wyścigami (Race Conditions). | Brak błędów spójności; poprawne blokowanie semaforem binarnym. | **OK** |
| **T04** | **Czyszczenie IPC** | Sprawdzenie stanu jądra systemu po wymuszeniu zamknięcia sygnałem `SIGINT`. | Narzędzie `ipcs` nie raportuje żadnych aktywnych zasobów po zakończeniu. | **OK** |

---

## 6. Analiza synchronizacji i bezpieczeństwa (Techniczne)

### 1. Atomowość i ochrona przed wyścigami
Każda modyfikacja struktur w pamięci współdzielonej (np. aktualizacja stanu podajnika przez Piekarza czy liczników sprzedaży przez Kasjera) odbywa się wewnątrz sekcji krytycznej chronionej przez **semafor binarny (mutex)**. Gwarantuje to spójność danych statystycznych nawet przy bardzo dużej intensywności operacji, eliminując ryzyko jednoczesnego zapisu.

### 2. Wykorzystanie flagi SEM_UNDO
Wszystkie operacje na semaforach wykorzystują flagę `SEM_UNDO`. Jest to kluczowy mechanizm bezpieczeństwa: jeśli proces zostanie nagle przerwany (np. przez błąd lub sygnał `SIGKILL`), jądro systemu automatycznie wycofa zmiany wprowadzone przez ten proces w semaforach. Zapobiega to trwałemu zablokowaniu zasobów (**deadlock**) dla pozostałych procesów.

### 3. Zarządzanie grupą procesów
Dzięki wywołaniu `setpgid(0, 0)`, Kierownik staje się liderem nowej grupy procesów. Pozwala to na precyzyjne zarządzanie sygnałami i gwarantuje, że żaden proces potomny nie zostanie "osierocony" w systemie operacyjnym po zamknięciu aplikacji głównej. Cała grupa jest sprzątana jednocześnie.

### 4. Asynchroniczność i buforowanie
Zastosowanie kolejki komunikatów (`msg`) skutecznie oddziela proces Klienta od procesu Kasjera. Klient po wykonaniu operacji `msgsnd()` może kontynuować działanie lub opuścić sklep, nie czekając na fizyczne zakończenie transakcji przez Kasjera. Zwiększa to przepustowość symulacji i lepiej oddaje realne warunki handlowe.

---

**Link do repozytorium GitHub:** [https://github.com/LEGION-programmer/LEGION-programmer-SO-projekt-ciastkarnia/tree/main](https://github.com/LEGION-programmer/LEGION-programmer-SO-projekt-ciastkarnia/tree/main)