# AI Usage

## Tool folosit

Am folosit ChatGPT pentru cerință.  
L-am folosit doar pentru cele 2 funcții legate de comanda `filter`:

1. `parse_condition(...)`
2. `match_condition(...)`


## Ce i-am cerut

I-am descris structura `Report` și formatul condițiilor de la `filter`, adică:

- condiția are forma `field:operator:value`
- câmpurile acceptate sunt `severity`, `category`, `inspector`, `timestamp`
- operatorii acceptați sunt `==`, `!=`, `<`, `<=`, `>`, `>=`

Apoi i-am cerut două lucruri:

1. să genereze o funcție `parse_condition` care sparge un șir de forma `field:operator:value` în cele 3 componente
2. să genereze o funcție `match_condition` care verifică dacă un `Report` respectă o condiție

## Ce mi-a dat ChatGPT

Răspunsul a fost util ca punct de plecare, dar nu a fost perfect direct de pus în proiect.

Ideea de bază a fost corectă:

- `parse_condition` separă câmpul, operatorul și valoarea
- `match_condition` compară valorile din raport cu cele din condiție

## Ce nu era perfect și ce am schimbat

Aici a fost partea importantă, pentru că a trebuit să verific și să adaptez manual:

- a trebuit să mă asigur că operatorii cu 2 caractere, cum sunt `>=`, `<=`, `==`, `!=`, sunt verificați înaintea celor cu un singur caracter
- am adăugat verificări de lungime pentru buffere, ca să nu scrie în afara lor
- pentru `severity` și `timestamp` am folosit conversie numerică și validare, nu comparație ca string
- pentru `category` și `inspector` am păstrat comparația ca string
- am adaptat semnătura și integrarea funcției astfel încât să se potrivească cu structurile și fișierele din proiectul meu
- am scris eu logica din `filter` care citește din `reports.dat`, parcurge toate condițiile și afișează doar rapoartele care le respectă pe toate

Cu alte cuvinte, ChatGPT nu a oferit soluția finală exact cum trebuia, ci mai degrabă un schelet bun pe care l-am verificat și corectat.

## Ce am învățat

Din partea asta am înțeles mai bine:

- cum se parsează în C un format de tip `field:operator:value`
- de ce trebuie tratate diferit câmpurile numerice față de cele text
- că un răspuns generat de AI trebuie verificat atent și adaptat la proiectul real

## Concluzie

AI-ul a fost folosit strict în limita permisă de cerință, adică doar pentru ideea de implementare a funcțiilor `parse_condition` și `match_condition`.  
Codul final a fost verificat, modificat și integrat manual în proiect.
