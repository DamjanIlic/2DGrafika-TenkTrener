# 2DGrafika-TenkTrener
12. Tenk-trener
 Kao novozaposleni inženjer u vojnotehničkom institutu vojske Srbije, zadatak Vam je 
napraviti simulator za obuku nišandžija tenka M-84 koji ima sledeće elemente:
 ● Simulator se pokreće preko čitavog ekrana.  Taster escape zatvara simulator.
 ● Lampica koja je indikator spremnosti topa za paljbu. Ukoliko ova lampica nije 
uključena, nije moguća paljba. 
● Indikator sa 10 podeoka koji predstavljaju preostalu municiju topa. Lijevi klik se 
koristi za paljbu. Nakon paljbe, potrebno je sačekati 7.5 sekundi da bi se top 
pripremio za ponovnu paljbu, ukoliko ima preostale municije. Adekvatno ažurirati 
indikator spremnosti za paljbu. 
● Voltmetar sa kazaljkom koji pokazuje napon hidrauličnog sistema (Kazaljka se 
blago trese). Moguće je povećavati i dodavati napon na tastere plus i minus. 
Kada je hidraulika isključena, kazaljka se ne pomjera, a voltmetar pokazuje 0, i 
okretanje kupole tenka je 10 puta sporije. Brzina okretanja se skalira sa 
naponom. 
● Spoljašnost tenka nije vidljiva kroz kupolu, nego se pritiskom na taster V 
prikazuje pogled vani kroz optički nišan tenka (vidljiva je i cijev tenka), a tasterom 
C vraća u kupolu tenka. 
● Strelice lijevo i desno služe za okretanje kupole tenka lijevo-desno, (što kao 
posljedicu mijenja vidljivi dio spoljašnosti). 
● Optički nišan tenka se sastoji od iksa (x) koji označava tačku ka kojoj je cijev topa 
okrenuta, plusa (+) koji prati kursor miša i služi kao indikator željene pozicije cijevi 
topa, i linije koja spaja ta dva elementa. 
● Kada se kursor miša postavi na neku poziciju, cijev se okreće ka njoj, što je 
uslovljeno hidraulikom. 
● Ograničiti kretanje cijevi na krug u centru ekrana, čiji prečnik ne prelazi visinu 
ekrana. Kupola se okreće samo horizontalno.
 ● Okretanje kupole tenka pomjera vidljivi pejzaž u suprotnom pravcu. Pejzaž može 
biti ili geometrijski teren ili učitana tekstura (panorama slika), sa bar tri okrugle 
mete u nasumičnim pozicijama. 
● Ukoliko se nišan ispravno postavi na metu, nakon čega se puca iz topa, meta će 
se uništiti, tj. nestati sa ekrana. 
● Od pokretanja simulacije, korisnik ima 1 minut da uništi mete, predstavljeno 
numeričkim tajmerom na sredini vrha ekrana, nakon čega se umjersto tajmera 
ispisuje “Uspješna misija” i simulacija zatvara. U suprotnom se ispisuje 
“Neuspijeh”, ali se simulacija se nastavlja. Uništavanje meta nakon neuspijeha 
zatvara aplikaciju.
 U proizvoljnom uglu ekrana napisati ime, prezime i indeks
