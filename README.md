# Multiple Sequence Alignment

Projekt _Višestruko poravnanje nizova_ iz kolegija Meta-heuristike (2021.)


## Sadržaj

- Uvod
- Sadržaj (trenutnog) direktorija
- Pokretanje programa
	- Pokretanje iz komandne linije
	- Korištenje grafičkog sučelja
- Potrebni alati
- Testni podaci


## Uvod

Program pomoću genetskog algoritma traži poravnanje (alignment) zadanih nizova proteina.
(Svaki protein je reprezentiran nizom slova, praznine u poravnanju se reprezentirane znakovima "-")

Na primjer za sljedeći niz proteina:
```
FFREDLAFLQGKAREFSSEQTRANSPTISSEQTRA
FFREDLAFPQGKAGEFSSEQTRANSPTSRELRVWG
VRQNWPYGKRLQEWTGKFFRVWPLGRSETKKFCAI
KTGGFFRAWPMGKEAPQFPHGPDASGADTNCSPRG
```
jedno poravnanje bi bilo:
```
--FFREDLAFLQGK-AREFSSEQTRA--NSPTISSEQTRA
--FFREDLAFPQGK-AGEFSSEQTRA--NSPTSRELRVWG
VRQNWPYGKRLQEW-TGKFFRVWPLG--RSETKKFCAI--
-----KTGGFFRAWPMGKEAPQFPHGPDASGADTNCSPRG
```

Program će ispisati najbolje pronađeno poravnanje s obzirom na funkciju dobrote.


## Sadržaj (trenutnog) direktorija

- __`podaci`__
- `asci_map.c`
- `asci_map.h`
- `blosum62.c`
- `blosum62.h`
- `blosum62.txt`
- `dokumentacija.txt`
- `input_file.txt`
- `msaga`
- `msaga.c`
- `msaga.exe`
- `msaga-gui.py`
- `pam250.c`
- `pam250.h`
- `pam250.txt`
- `solution_file.txt`

Sam genetski algoritam je napisan u C-u, glavni dio se nalazi u datoteci `msaga.c`.
Ostale datoteke u C-u (`asci_map.c`, `asci_map.h`, `blosum62.c`, `blosum62.h`, 
`pam250.c` i `pam250.h`) služe za čitanje matrica BLOSUM62 i PAM250.

U datotekama `blosum62.txt` i `pam250.txt` su spremljene matrice BLOSUM62 i PAM250.

Izvršni kod se nalazi u datotekama `msaga.exe` (Windows)  i `msaga` (Linux).
(Ako potrebna izvršna datoteka ne rade treba ponovo kompajlirati program.)

Grafičko sučelje napisano u Python-u, program se nalazi u datoteci `msaga-gui.py`.

Datoteke `input_file.txt` i `solution_file.txt` generira `msaga-gui.py` te se
one koriste kao ulazne datoteke za poziv izvršnog koda u C-u.

Direktorij __`podaci`__ sadrži datoteke s ulaznim podacima, podaci su uzeti iz
BAliBASE-e (jedna od najkorištenijih baza podatak za višestruko poravnanje nizova)
te su obrađeni i spremljeni u obliku koje program (u C-u) može prihvatiti.

Datoteka `dokumentacija.txt` sadrži dokumentaciju projekta i programa.


## Pokretanje programa


### Pokretanje iz komandne linije

(U objašnjenju je za izvršnu datoteka korištena `msaga.exe`,
tj. dolje navedeni primjeri su izvedeni na Windows-u.)

Izvršni program u C-u pokrećemo s naredbom oblika:

```
msaga.exe <ime_ulazne_datoteke> [-popnum <broj_populacija>] [-popsize <velicina_populacije>]
	[-gennum <broj_generacija>] [-gapcoeff <koeficijent_praznosti>] [-mparams <mutacijski_parametri>]
	[-fitness <indeks_funkcije_dobrote>] [-score <ime_matrice>] [-solution]
```

- `ime_ulazne_datoteke`:   	naziv ulazne datoteke,
						npr.: `input_file.txt`
						
- `broj_populacija`: 		    broj populacija (prirodan broj),
						npr.: `10`
						
- `velicina_populacije`: 	  veličina populacije (paran broj),
						npr.: `100`
						
- `broj_generacija`: 		    broj generacija (prirodan broj),
						npr.: `1000`
						
- `koeficijent_praznosti`: 	koeficijent (realni broj veći od 1.0) o kojem ovisi
						koliko će praznina biti u poravnanju,
						npr.: `1.2`

- `mutacijski_parametri`: 	niz od 4 parametra (realni brojevi između 0.0 i 1.0)
						koji određuju zastupljenost mutacija,
						npr. `0.1 0.3 0.3 0.3`
						
- `indeks_funkcije_dobrote`:  broj `1` ili `2` koji određuje koja funkcija dobrote se koristiti,
						 npr.: `1`
						 
- `ime_matrice`:			        riječ `blosum62` ili `pam250`, određuje koja matrica se koristi,
						npr.: `blosum62`
						
- Ako je zadan parametar `solution` onda program očekuje da ulazna datoteka sadrži
poravnanje, te će u tom slučaju 10% populacije biti inicijalizirano s tim poravnanjem.
Ako nije zadan parametar `solution` program očekuje da ulazna datoteka
sadrži niz proteina bez praznina.
					
Ispravni primjeri poziva programa:

```
msaga.exe input_file.txt
```

```
msaga.exe input_file.txt -popnum 2 -popsize 50
```

```
msaga.exe input_file.txt -popnum 1 -popsize 50 -gennum 5000
```

```
msaga.exe input_file.txt -popnum 1 -gapcoeff 1.3
```

```
msaga.exe input_file.txt -popnum 1 -gapcoeff 1.3 -mparams 0.2 0.1 0.3 0.25
```

```
msaga.exe input_file.txt -popnum 1 -gapcoeff 1.3 -fitness 1
```

```
msaga.exe input_file.txt -popnum 1 -gapcoeff 1.3 -fitness 1 -score pam250
```

```
msaga.exe input_file.txt  -popnum 1 -popsize 50 -gennum 2000 -gapcoeff 1.25 -mparams 0.3 0.2 0.2 0.4 -fitness 1 -score pam250
```

```
msaga.exe solution_file.txt
```

```
msaga.exe solution_file.txt -popnum 2 -popsize 50
```

```
msaga.exe solution_file.txt -popnum 1 -popsize 50 -gennum 5000
```

```
msaga.exe solution_file.txt -popnum 1 -mparams 0.2 0.1 0.3 0.25 -fitness 1 -score pam250
```

```
msaga.exe solution_file.txt  -popnum 1 -popsize 50 -gennum 2000 -mparams 0.3 0.2 0.2 0.4 -fitness 1 -score pam250
```

### Korištenje grafičkog sučelja

Grafičko sučelje možemo pokrenuti desnim klikom na `msaga-gui.py` i odabirom
`Open With -> Python`. (Alternativno, otvorimo datoteku `msaga-gui.py` u IDLE-u
te pokrenemo program koristeći izbornik `Run -> Run Module` ili pritiskom tipke `F5`).

Na vrhu prozora biramo parametre (`Number of generations`, `Number of populations`, 
`Population size`, `Gap coefficient`, `Mutation probability`, `Mutation #1 probability`,
`Mutation #2 probability`, `Mutation #3 probability`, `Fitness function`, `Scoring matrix`,
ovi parametri redom odgovaraju parametrima programa u C-u: `gennum`, `popnum`, `popsize`,
`gapcoeff`, `mparams` (sva četiri), `fitness`, `score`).

U sredini prozora imamo okvir za unos niza proteina koji će se koristiti kao ulaz.
U donjem desnom kutu okvira se nalazi gumb `Choose file` pomoću kojeg možemo odabrati
datoteku u kojoj je spremljen niz proteina (npr. `podaci/input_BB11001.txt`).

Na dnu prozora se nalaze tri gumba.

Gumb `Add Solution` otvara novi prozor s okvirom u koji možemo dodati poravnanje
koje će biti spremljeno u `solution_file.txt` te će se pri pozivu programa u C-u
koristiti parametar `solution`. U donjem desnom kutu ovog okvira se također 
nalazi gumb `Choose file` pomoću kojeg možemo odabrati datoteku u kojoj
je spremljeno poravnanje (npr. `podaci/solution_BB11001.txt`).
(Ako je već postavljeno poravnanje klikom na `Add Solution`, tekst gumba će se
promijeniti u `Change Solution`)

Gumb `RUN` pokreće program u C-u te po završetku izvođenja ispisuje najbolje 
(s obzirom na funkciju dobrote) pronađeno poravnanje unutar okvira u sredini prozora.

Gumb `Show Console` dodaje okvir s desne strane prozora u kojem se po pokretanju
programa simulira ispis u konzoli.
(Nakon što kliknemo `Show Console` i pojavi se okvir s desne strane, tekst gumba
će se promijeniti u `Hide Console` te ponovnim klikom se spomenuti okvir sakriva.)

<br />

![Grafičko sučelje](/msaga_gui.png?raw=true)


## Potrebni alati

### gcc (ili neki drugi kompajler, nije nužno potreban)

Za pokretanje program u C-u dovoljno je pokrenutu izvršnu datoteku
(na Windows-u je to `msaga.exe`) u komandnoj liniji s odgovarajućim parametrima.
Ako ova izvršna datoteka ne rade treba ponovo kompajlirati program,
na primjer koristeći gcc (na Windows-u):
```
gcc msaga.c asci_map.c blosum62.c pam250.c -o msaga.exe
```
U tom slučaju potreban je C kompajler, npr. gcc.


### Python 3

Za pokretanje grafičkog sučelja potrebno je imati instaliran Python 3.
Program je testiran na verziji Python 3.7 i 3.9, a trebao bi raditi
s bilo kojom verzijom Python 3.
(Program (grafičkog sučelja) se ne može pokrenuti s verzijom Python 2 jer je za realizaciju
grafičkog sučelja korišten modul Tkinter koji se znatno promijenio u verziji Python 3.)


## Testni podaci

Testni podaci se nalaze u direktoriju __`podaci`__ unutar kojeg su datoteke koje
započinju sa "input" te sadrže nizove proteina, npr. `podaci/input_BB11001.txt`.
(Većina datoteka BAliBASE-e sadrže velik broj sekvenci te je zato preporučljivo koristiti
samo dio sekvenci iz tih datoteka kako bi se program izvršio u razumnom vremenu.)
