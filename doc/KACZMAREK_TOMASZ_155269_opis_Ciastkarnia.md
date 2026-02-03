# Dokumentacja Projektu: Wieloprocesowa Ciastkarnia IPC

**Autor:** Tomasz Kaczmarek  
**Nr albumu:** 155269  
**System operacyjny:** Linux / Unix  
**Język:** C  

---

## 1. Architektura Systemu

Projekt realizuje model **Producenta i Konsumenta** w środowisku rozproszonym, wykorzystując mechanizmy komunikacji międzyprocesowej (IPC) System V. System składa się z niezależnych jednostek wykonawczych (procesów), które synchronizują dostęp do zasobów bez współdzielenia przestrzeni adresowej (poza dedykowanym segmentem pamięci współdzielonej).



### Główne komponenty:
1.  **Kierownik (Director):** Proces nadrzędny, lider grupy procesów (`setpgid`). Odpowiada za cykl życia zasobów IPC i orkiestrację sygnałów.
2.  **Piekarz (Baker):** Proces produkcyjny, uzupełniający podajniki towarów do limitu $K_i$.
3.  **Klient (Customer):** Krótkotrwałe procesy generowane falami, realizujące atomowe zakupy.
4.  **Kasjer (Cashier):** Procesy serwisowe obsługujące zamówienia asynchronicznie.

---

## 2. Mechanizmy Komunikacji i Synchronizacji

### 2.1 Pamięć Współdzielona (`shm`)
Przechowuje strukturę `shared_data_t`, która jest "źródłem prawdy" dla systemu:
* **Podajniki:** Aktualna liczba dostępnych ciastek każdego typu.
* **Liczniki Statystyczne:** `wytworzono`, `sprzedano`, `porzucono`.
* **Flagi:** Stan otwarcia sklepu (sterowanie wejściem klientów).

### 2.2 Semafory (`sem`)
Wykorzystywany jest zestaw dwóch semaforów:
1.  **Semafor Licznikowy (Pojemność N):** Kontroluje liczbę klientów przebywających jednocześnie w sklepie.
2.  **Semafor Binarny (Mutex):** Gwarantuje atomowość operacji na pamięci współdzielonej, eliminując zjawisko wyścigów (*race conditions*).

### 2.3 Kolejka Komunikatów (`msg`)
Służy do przesyłania struktur `order_t` od klientów do kasjerów. Pozwala to na:
* **Asynchroniczność:** Klient opuszcza sklep po wysłaniu zamówienia, nie czekając na jego fizyczne rozliczenie.
* **Buforowanie:** Zamówienia czekają w kolejce jądra, co zapobiega utracie danych przy dużym obciążeniu.

---

## 3. Obsługa Sygnałów i Bezpieczeństwo

System wykorzystuje zaawansowane techniki obsługi sygnałów (`sigaction`) w celu zapewnienia stabilności i spójności danych.

| Sygnał | Odbiorca | Funkcja |
| :--- | :--- | :--- |
| **SIGINT** | Kierownik | Inicjuje procedurę czyszczenia i generuje raport końcowy. |
| **SIGUSR1** | Kasjer 2 | Informuje o spadku obciążenia i nakazuje zamknięcie stanowiska. |
| **SIGUSR2** | Klient | Procedura ewakuacji – bezpieczne porzucenie zakupów i aktualizacja statystyk. |
| **SIGALRM** | Kierownik | Automatyczne zakończenie symulacji po zadanym czasie. |

### Mechanizm "Pancernej Ewakuacji":
W procesie Kierownika zastosowano maskowanie sygnałów (`sigprocmask`). Podczas rozsyłania sygnału ewakuacji do grupy (`kill(0, SIGUSR2)`), Kierownik blokuje ten sygnał dla samego siebie, co zapobiega jego gwałtownemu zamknięciu przez system (błąd `User defined signal 2`) i pozwala na poprawne wygenerowanie raportu.



---

## 4. Analiza Bilansu (Weryfikacja Statystyczna)

Kluczowym elementem projektu jest zapewnienie 100% spójności towarów. Raport końcowy weryfikuje równanie:
$$W = S + Z + P$$
Gdzie:
* $W$: Łączna liczba wytworzonych produktów.
* $S$: Produkty sprzedane i opłacone w kasie.
* $Z$: Produkty, które pozostały na podajnikach (zapas).
* $P$: Produkty porzucone przez klientów podczas ewakuacji lub w wyniku braku towaru.

Dzięki zastosowaniu **atomowych transakcji** w kodzie klienta (pobranie towaru i jego ewentualny zwrot odbywają się w jednej sekcji krytycznej), bilans pozostaje poprawny nawet przy symulacji tysięcy klientów jednocześnie.

---

## 5. Instrukcja Obsługi

### Kompilacja:
```bash
make clean && make

Uruchomienie:
Bash

./kierownik <N> <K> [czas]
N: Maksymalna liczba klientów w sklepie.

K: Próg otwarcia drugiej kasy.

czas: (Opcjonalnie) Czas trwania symulacji w sekundach.

Przykład testowy:
Bash

./kierownik 50 15 30
Otwiera sklep na 50 osób, druga kasa aktywuje się przy 15 klientach, symulacja kończy się po 30 sekundach.

Tomasz Kaczmarek 2026