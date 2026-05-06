# AI Usage – Phases 1 and 2

---

## Phase 1

### Tool folosit

Am folosit ChatGPT pentru cerință.  
L-am folosit doar pentru cele 2 funcții legate de comanda `filter`:

1. `parse_condition(...)`
2. `match_condition(...)`

### Ce i-am cerut

I-am descris structura `Report` și formatul condițiilor de la `filter`, adică:

- condiția are forma `field:operator:value`
- câmpurile acceptate sunt `severity`, `category`, `inspector`, `timestamp`
- operatorii acceptați sunt `==`, `!=`, `<`, `<=`, `>`, `>=`

Apoi i-am cerut două lucruri:

1. să genereze o funcție `parse_condition` care sparge un șir de forma `field:operator:value` în cele 3 componente
2. să genereze o funcție `match_condition` care verifică dacă un `Report` respectă o condiție

### Ce mi-a dat ChatGPT

Răspunsul a fost util ca punct de plecare, dar nu a fost perfect direct de pus în proiect.

Ideea de bază a fost corectă:

- `parse_condition` separă câmpul, operatorul și valoarea
- `match_condition` compară valorile din raport cu cele din condiție

### Ce nu era perfect și ce am schimbat

- a trebuit să mă asigur că operatorii cu 2 caractere, cum sunt `>=`, `<=`, `==`, `!=`, sunt verificați înaintea celor cu un singur caracter
- am adăugat verificări de lungime pentru buffere, ca să nu scrie în afara lor
- pentru `severity` și `timestamp` am folosit conversie numerică și validare, nu comparație ca string
- pentru `category` și `inspector` am păstrat comparația ca string
- am adaptat semnătura și integrarea funcției astfel încât să se potrivească cu structurile și fișierele din proiectul meu
- am scris eu logica din `filter` care citește din `reports.dat`, parcurge toate condițiile și afișează doar rapoartele care le respectă pe toate

### Ce am învățat

- cum se parsează în C un format de tip `field:operator:value`
- de ce trebuie tratate diferit câmpurile numerice față de cele text
- că un răspuns generat de AI trebuie verificat atent și adaptat la proiectul real

---

## Phase 2

### Tool folosit

Am folosit din nou ChatGPT, de această dată pentru a înțelege cum se configurează handlere de semnale cu `sigaction()` în loc de `signal()`.

### Ce i-am cerut

I-am explicat că trebuie să scriu un program care:
- reacționează la `SIGUSR1` printr-un mesaj la stdout
- se oprește la `SIGINT` și face curățenie (șterge un fișier PID)
- nu trebuie să folosească `signal()`, ci `sigaction()`

I-am cerut un exemplu simplu de structură `sigaction` și cum se folosește `pause()` într-un loop de așteptare.

### Ce mi-a dat ChatGPT

Mi-a dat un template de bază cu `sigaction`, doi handleri separați și un loop cu `pause()`. Structura generală era corectă.

### Ce nu era perfect și ce am schimbat

- handlerul pentru `SIGINT` în exemplul primit folosea `printf`, care nu este async-signal-safe; am înlocuit cu un flag `volatile sig_atomic_t` pe care îl verific în main loop, iar mesajul îl afișez după ce ieșim din loop
- handlerul pentru `SIGUSR1` folosea tot `printf`; l-am înlocuit cu `write()` direct pe `STDOUT_FILENO`, care este async-signal-safe
- am adăugat eu logica de creare și ștergere a fișierului `.monitor_pid` cu `open()`/`write()`/`unlink()`
- am integrat în `city_manager` funcția `notify_monitor` care citește PID-ul din `.monitor_pid` și trimite `SIGUSR1` cu `kill()`; logica de tratare a erorilor (fișier lipsă, PID invalid, `kill()` eșuat) am scris-o eu complet

### Ce am învățat

- diferența dintre `signal()` și `sigaction()` și de ce `sigaction` este preferabil
- ce înseamnă async-signal-safe și de ce contează în handleri de semnale
- cum se coordonează două procese separate prin semnale și un fișier PID comun
