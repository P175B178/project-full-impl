# Inžinerinis projektas

**Tipas:** grupinis projektas
**Grupės dydis:** 3 studentai

---

## Apžvalga

Kursite daiktų interneto (IoT) sistemą aplinkai stebėti. Raspberry Pi matuoja temperatūrą ir drėgmę, tikrina, ar neviršytos slenkstinės reikšmės, ir apie būseną iš karto praneša LED indikatoriais bei garsiniu signalizatoriumi. Matavimai perduodami tinklu, todėl įrenginius galima stebėti ir konfigūruoti nuotoliniu būdu iš darbalaukio programos.

Projektas vykdomas trimis etapais. Kiekviename etape yra pagrindiniai reikalavimai, kuriuos būtina įgyvendinti, ir rekomenduojami — neprivalomi, tačiau verti įgyvendinti.

**1 etapas — įterptinis įrenginys.** Autonominis įrenginys, veikiantis be tinklo ryšio.

**2 etapas — prijungtas įrenginys.** Įrenginys siunčia matavimus per MQTT į bendrą brokerį ir iš jo gauna konfigūracijos komandas.

**3 etapas — pilna sistema.** Qt darbalaukio programa rodo vieno ar kelių įrenginių duomenis realiuoju laiku ir leidžia nuotoliniu būdu keisti jų konfigūraciją.

Visos grupės naudoja bendrą brokerį ir vienodą pranešimų formatą, todėl jūsų Qt programa turi galėti stebėti ir kitų grupių įrenginius.

Darbas pradedamas nuo [projekto šablono](https://github.com/P175B178/project-template).

### Aparatūra

| Komponentas | Modelis |
| ----------- | ------- |
| Mikrokompiuteris | Raspberry Pi 5, 2 GB RAM |
| Ekranas | Raspberry Pi Touch Display 2 |
| Temperatūros ir drėgmės jutiklis | AM2302 (DHT22, MOD-02637) |
| Garsinis signalizatorius | 23 mm pasyvus garsinis signalizatorius (EAK-04187) |

Kaip sujungti kontaktus, nurodyta [A priede](#a-priedas-aparatūros-sujungimai).

Aparatūros valdymo sąsajos aprašytos [B priede](#b-priedas-aparatūros-sąsajos).

MQTT brokeris veikia nuotoliniame serveryje — prie jo jungiasi ir Qt programa studentų kompiuteriuose, ir Raspberry Pi įrenginys.

### Pranešimų formatas

MQTT temos ir pranešimų formatas apibrėžti projekto šablono kataloge [`protocol/`](https://github.com/P175B178/project-template/tree/main/protocol): temos ir taisyklės aprašytos `protocol/README.md` faile, o kiekvieno pranešimo pavyzdys pateiktas `protocol/examples/` kataloge. Šio formato privalo tiksliai laikytis visos grupės — tik tada bet kurios grupės Qt programa gali stebėti bet kurios kitos grupės įrenginį.

---

## Nefunkciniai reikalavimai

Taikomi visiems trims etapams.

### Privalomieji

- Kodas laikosi [C++ Core Guidelines](https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md).
- Ištekliai valdomi RAII principu; vengiama tiesioginių `new`/`delete` ir paprastų rodyklių, kurios valdo objektų gyvavimą.
- Nuoseklus `const` naudojimas.
- Globalių kintamųjų vengiama; jei jų prireikia, tai turi būti pagrįsta.
- Klaidos neignoruojamos — jos registruojamos arba perduodamos aukštyn.
- Kodas suformatuotas pagal `.clang-format`, `clang-tidy` nerodo įspėjimų.
- Aparatūra ir MQTT klientas pasiekiami tik per abstrakčias sąsajas.
- Vienetų testai veikia be aparatūros ir tinklo ryšio.
- Įrenginio ir darbalaukio programos yra atskiri CMake projektai (`device/` ir `desktop/` katalogai toje pačioje repozitorijoje); kiekvienas kompiliuojamas viena `cmake --build` komanda.

### Rekomenduojami

Neprivaloma, tačiau verta įgyvendinti:

- `std::expected` klaidoms grąžinti, `std::optional` — rezultatui, kurio gali nebūti
- `[[nodiscard]]` funkcijoms, grąžinančioms klaidas ar rezultatus
- Debug konfigūracijoje įjungti `AddressSanitizer` ir `UndefinedBehaviorSanitizer` (`-fsanitize=address,undefined`); testai vykdomi su jais
- Struktūrizuotas žurnalavimas (pvz., `spdlog`)
- Visi nustatymai laikomi konfigūracijos faile, o ne programos kode
- Programą galima valdyti komandinės eilutės parametrais: konfigūracijos failo kelias, `--help`, `--version` (pvz., su `CLI11`)
- Git *pre-commit hook*, prieš kiekvieną *commit* tikrinantis formatavimą
- GitHub Actions CI — kompiliavimas, vienetų testai ir `clang-tidy` vykdomi po kiekvieno *push*
- Aiškūs ir glausti *commit* pranešimai: kas pakeista ir kodėl

---

<div style="page-break-before: always"></div>

## 1 etapas — įterptinis įrenginys

Darbas su aparatūra. Šio etapo pabaigoje Pi matuoja, tikrina slenkstines reikšmes ir valdo LED bei garsinį signalizatorių (be tinklo ryšio).

Iš šablono: `device/` — CMake projektas ir aparatūros sąsajos (`device/include/interfaces/`).

### Pagrindiniai reikalavimai

Pi periodiškai nuskaito temperatūrą ir drėgmę iš AM2302 (DHT22) jutiklio ir išlygina rezultatus — skaičiuoja paskutinių N matavimų vidurkį. Kiekvienas matavimas išvedamas į `stdout` struktūrizuotu, žmogui lengvai skaitomu formatu.

Pagal matavimus LED rodo vieną iš trijų būsenų:

| Būsena | Sąlyga | LED spalva |
| ------ | ------ | ---------- |
| *Normal* | Nė viena slenkstinė reikšmė neviršyta | Žalia |
| *Warning* | Viršyta lygiai viena slenkstinė reikšmė | Oranžinė |
| *Alert* | Viršytos abi slenkstinės reikšmės | Raudona (mirksinti) |

Pereinant į *Alert* būseną, garsinis signalizatorius trumpai pypteli tris kartus, išeinant iš jos — vieną kartą ilgai.

Paleista programa iš konfigūracijos failo nuskaito matavimo intervalą, vidurkinimo lango dydį, temperatūros ir drėgmės slenksčius.

### Nefunkciniai reikalavimai

- Kiekvienas aparatūros komponentas valdomas RAII klase (žr. [B priedą](#b-priedas-aparatūros-sąsajos))
- Gavusi `SIGINT` ar `SIGTERM`, programa tvarkingai baigia darbą
- Vienetų testai pagrindinei logikai

### Rekomenduojami

Neprivaloma, tačiau verta įgyvendinti:

- Matavimų tikrinimas: fiziškai neįtikėtinos reikšmės atmetamos ir registruojamos žurnale
- *Fault* būsena: jutiklio gedimą rodo atskira, ketvirta būsena (aprašyta po sąrašu)
- Jutiklio skaitymas atskiroje gijoje: lėtas ar nepavykęs nuskaitymas nestabdo LED ir garsinio signalizatoriaus
- Pakavimas: programa ir konfigūracijos failas supakuojami į `.deb` paketą su CPack

*Fault* būsena: kai keli matavimai iš eilės nepavyksta arba atmetami (jų skaičius nurodomas konfigūracijos faile), įrenginys pereina į *Fault* būseną; LED tada šviečia raudonai ir nemirksi. Perėjimas iš *Alert* į *Fault* nėra *Alert* būsenos pabaiga, todėl ilgas pyptelėjimas neskamba. Gavus tinkamą matavimą, būsena vėl nustatoma pagal slenkstines reikšmes.

### Demo

Programa paleidžiama Pi, jos išvestis stebima per SSH. Sukelti *Warning* ir *Alert* būsenas (pvz., kvėpuojant į jutiklį) ir parodyti:

- Matavimai rodomi `stdout`
- LED pereina Žalia → Oranžinė → Raudona (mirksinti)
- Garsinis signalizatorius pypsi pereinant į *Alert* būseną ir iš jos
- Slenksčiai ir intervalas nuskaitomi iš konfigūracijos failo, o ne įrašyti programos kode

---

<div style="page-break-before: always"></div>

## 2 etapas — prijungtas įrenginys

Pi publikuoja matavimus ir būsenos pokyčius į MQTT brokerį ir priima konfigūracijos komandas.

Iš šablono: `protocol/` — pranešimų formatas ir pavyzdžiai; `broker/` — vietinis brokeris kūrimui.

### Pagrindiniai reikalavimai

Temos ir pranešimų formatas aprašyti skyriuje [„Pranešimų formatas“](#pranešimų-formatas). Pi prisijungia prie brokerio ir po kiekvieno matavimo publikuoja temperatūros bei drėgmės reikšmes JSON pranešimais. Publikuojamos išlygintos (vidurkintos) reikšmės — tos pačios, pagal kurias įrenginys tikrina slenksčius, o ne neapdoroti jutiklio rodmenys. Būsenos pranešimas siunčiamas prisijungus ir po kiekvieno būsenos pokyčio.

Pi prenumeruoja konfigūracijos komandas ir reaguoja į matavimo intervalo, temperatūros bei drėgmės slenksčių pakeitimus. Klaidingas ar neišsamus pranešimas registruojamas ir ignoruojamas — programa dėl jo nesustoja.

LED ir garsinio signalizatoriaus elgesys iš 1 etapo išlieka. Naudojamas MQTT testamentas (angl. *Last Will and Testament*, LWT): įrenginiui netikėtai atsijungus, brokeris pats paskelbia `offline` būseną jo būsenos temoje.

### Nefunkciniai reikalavimai

- TLS ryšys su brokeriu; autentifikacija vartotojo vardu ir slaptažodžiu. Slaptažodis nelaikomas programos kode ir nepatenka į repozitoriją
- Automatinis pakartotinis prisijungimas prie brokerio praradus ryšį
- Vienetų testai MQTT logikai — pranešimų formavimas ir komandų analizavimas

### Rekomenduojami

Neprivaloma, tačiau verta įgyvendinti:

- Būsenos priverstinis nustatymas: įrenginys prenumeruoja `config/<group-id>/state` ir pereina į prašomą būseną, kuri išlieka iki kito tinkamo matavimo
- Tvarkingas išjungimas: prieš atsijungdamas įrenginys pats publikuoja `offline` būseną
- Didėjantis delsimas: nepavykus prisijungti, laikas iki kito bandymo ilgėja (pvz., nuo 1 iki 30 s)
- Paleidimas įjungus Pi: `.deb` paketas įdiegia systemd tarnybą, kuri paleidžia programą ir po strigties paleidžia ją iš naujo

### Demo

Matavimai stebimi MQTT Explorer programoje. Parodyti:

- Pakeitus slenkstį per MQTT Explorer, Pi LED į tai sureaguoja
- Įrenginio būsenos tema atspindi esamą būseną — ir tada, kai MQTT Explorer prisijungia vėliau už įrenginį
- Įrenginiui atsijungus, brokeris gauna `offline` būseną

---

<div style="page-break-before: always"></div>

## 3 etapas — pilna sistema

Qt darbalaukio programa jungiasi prie brokerio, realiuoju laiku rodo visų prijungtų įrenginių matavimus ir grafikus ir leidžia keisti jų konfigūraciją.

Iš šablono: `desktop/` — Qt programos ruošinys.

### Pagrindiniai reikalavimai

Qt programa jungiasi prie MQTT brokerio (adresą galima nustatyti programoje; jis išsaugomas kitam paleidimui) ir automatiškai aptinka įrenginius prenumeruodama `sensors/+/temperature`, `sensors/+/humidity` ir `sensors/+/status`. `<group-id>` nustatomas pagal temos kelią, o rodomas įrenginio pavadinimas imamas iš pranešimo lauko `deviceId`.

Kiekvienam įrenginiui rodomi dabartiniai matavimai ir spalva pažymėta būsena; įrenginys, dar nepranešęs būsenos, rodomas kaip nežinomos būsenos. Matavimų istorija kaupiama lentelėje, slenksčius viršijančios reikšmės joje išryškinamos. Temperatūros ir drėgmės grafikai rodo paskutinių kelių minučių duomenis (slenkantis laiko langas); keli įrenginiai rodomi skirtingomis spalvomis tame pačiame grafike.

Kiekvienam įrenginiui galima keisti slenkstines reikšmes ir matavimo intervalą. Kai įrenginys neprisijungęs, jo valdikliai automatiškai išjungiami. Sistema turi veikti su bent dviem vienu metu prijungtais įrenginiais.

Neteisingas pranešimas registruojamas ir ignoruojamas, kaip ir įrenginyje (žr. [pranešimų formatą](#pranešimų-formatas)); priežastis matoma programos lange, ne tik terminale.

### Nefunkciniai reikalavimai

- TLS ryšys su brokeriu; autentifikacija vartotojo vardu ir slaptažodžiu. Slaptažodis neišsaugomas atviru tekstu
- Automatinis pakartotinis prisijungimas prie brokerio praradus ryšį
- Struktūrizuoti duomenys rodomi pagal Qt Model-View architektūrą
- Vienetų testai duomenų sluoksniui — įrenginių būsenos valdymas, pranešimų analizavimas ir kita sudėtingesnė modelio logika

### Rekomenduojami

Neprivaloma, tačiau verta įgyvendinti:

- Būsenos priverstinis nustatymas: pasirinktam įrenginiui siunčiama `config/<group-id>/state` komanda
- Įspėjimų žurnalas: įrenginių būsenos pokyčių sąrašas — kiekvienas įrašas su laiku, įrenginiu, ankstesne ir nauja būsena
- Eksportas: bet kurio įrenginio matavimų istorija išsaugoma CSV faile, grafikai — PNG paveikslėliu
- Pakavimas: programa supakuojama į `.deb` paketą su CPack

### Galutiniai rezultatai

#### Pristatymas (skaidrės, iki 10 minučių)

- Sistemos architektūros diagrama — kaip tarpusavyje sujungti visi trys komponentai
- MQTT temų struktūra — visas temų sąrašas, kryptis, QoS ir ar pranešimas išsaugomas brokeryje (angl. *retained*)
- Duomenų formatas — JSON pavyzdžiai kiekvienam pranešimo tipui
- Būsenų diagrama — įrenginio būsenos ir perėjimai
- Darbo pasidalijimas — kas ką sukūrė
- Išvados — kas buvo sunkiau nei tikėtasi, ką darytumėte kitaip

#### Demo (iki 5 minučių, gyvai po pristatymo)

- Pi matavimai matomi Qt programoje realiuoju laiku
- Viršyti slenkstį — sureaguoja ir LED, ir Qt programa
- Konfigūracijos pakeitimas iš Qt atsispindi įrenginyje be paleidimo iš naujo
- Neprisijungę įrenginiai rodomi kaip neprisijungę, jų valdikliai išjungti
- Grafikas su bent dviem įrenginiais

---

<div style="page-break-before: always"></div>

## A priedas: Aparatūros sujungimai

Šiame priede pateikti fiziniai sujungimai tarp Raspberry Pi 5 ir naudojamos aparatūros. Visi komponentai jungiami prie 40 kontaktų GPIO jungties.

| Komponentas | Funkcija | BCM GPIO | Kontakto Nr. |
| ----------- | -------- | -------- | ----------------- |
| Touch Display 2 | 5 V | — | 2 |
| Touch Display 2 | GND | — | 6 |
| DHT22 | VCC (3,3 V) | — | 1 |
| DHT22 | DATA | GPIO 4 | 7 |
| DHT22 | GND | — | 14 |
| Garsinis signalizatorius | Signalas | GPIO 17 | 11 |
| Garsinis signalizatorius | GND | — | 20 |

Ekranas dar jungiamas DSI juostiniu kabeliu prie Pi DSI0 prievado — lentelėje nurodyti GPIO jungties kontaktai jam tik tiekia maitinimą.

---

<div style="page-break-before: always"></div>

## B priedas: Aparatūros sąsajos

Įrenginius galima valdyti keliais skirtingais būdais — per failų sistemą, naudojant specialias bibliotekas arba per tvarkykles, kurios paslepia sudėtingesnę logiką.

Šiame projekte sąmoningai naudojami skirtingi metodai, kad galėtumėte susipažinti su įvairiais aparatūros valdymo būdais.

Žemo lygio aparatūros sąsajų detalių nagrinėti nereikia: likęs kodas neturėtų žinoti, kaip aparatūra iš tikrųjų valdoma.

| Komponentas | Sąsaja | Kelias |
| ----------- | ------ | ------ |
| Būsenos LED (integruoti ACT + PWR) | sysfs LED posistemė | `/sys/class/leds/ACT/`, `/sys/class/leds/PWR/` |
| AM2302 (DHT22) jutiklis | Branduolio IIO tvarkyklė (sysfs) | `/sys/bus/iio/devices/iio:device0/` |
| Garsinis signalizatorius | libgpiod (simbolinis įrenginys) | `/dev/gpiochip4` |

### Būsenos LED

Pi 5 turi du integruotus LED — ACT (žalias) ir PWR (raudonas) — valdomus per branduolio LED posistemę. Jie valdomi tiesiog įrašant reikšmes į failus.

Kartu jie leidžia parodyti tris spalvas:

| Būsena | ACT | PWR | Spalva |
| ------ | --- | --- | ------ |
| *Normal* | įjungtas | išjungtas | Žalia |
| *Warning* | įjungtas | įjungtas | Oranžinė |
| *Alert* | išjungtas | įjungtas | Raudona |

Prieš valdant LED reikia išjungti jų numatytąjį branduolio trigerį:

```text
/sys/class/leds/ACT/trigger  ← įrašyti "none"
/sys/class/leds/PWR/trigger  ← įrašyti "none"
```

### DHT22 jutiklis

AM2302 naudoja vieno laido protokolą, kuriam reikalingas labai tikslus laiko valdymas. Dėl to jį patikimai įgyvendinti pačioje programoje sudėtinga.

Šį darbą atlieka branduolio tvarkyklė: jutiklio duomenis ji pateikia kaip paprastus failus. Programai juos tereikia nuskaityti.

Pi reikia vieną kartą paruošti: į `/boot/firmware/config.txt` įrašyti eilutę (ta pati tvarkyklė skirta ir DHT22) ir paleisti Pi iš naujo:

```text
dtoverlay=dht11,gpiopin=4
```

Paleidus iš naujo, matavimai pasiekiami šiuose failuose:

```text
/sys/bus/iio/devices/iio:device0/in_temp_input             (mililaipsniai Celsijaus)
/sys/bus/iio/devices/iio:device0/in_humidityrelative_input (miliprocentai)
```

### Garsinis signalizatorius

GPIO išvestis, prijungta prie pasyvaus garsinio signalizatoriaus. GPIO galima valdyti per failų sistemą (`/sys/class/gpio/`), tačiau ta sąsaja yra pasenusi ir naujame kode nenaudotina. Dabar įprastas būdas — **libgpiod** biblioteka, dirbanti per GPIO simbolinio įrenginio sąsają. Garsinis signalizatorius prijungtas prie `/dev/gpiochip4` (Pi 5 RP1 lustas), GPIO 17.
