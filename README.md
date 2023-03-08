Nume: Hodoboc-Velescu Tudor
Grupă: 335CC

# Tema 2 Bibliotecă stdio

Organizare
-
Ideea temei a fost de a simula biblioteca stdio. Astfel, am incercat sa fac
toate operatiile cat de apropriate am putut de modul in care am inteles ca
functioneaza aceasta bibloteca. Structura principala, SO_FILE, contine doua
buffere, unul pentru scriere si unul pentru citire, pe cat si pozitiile in aceste
doua buffere si lungimea datelor stocate acolo. Am ales sa folosesc doua buffere
in loc de unul singur deoarece mi s-a parut mai usor de impartit operatiile pe
doua buffere, desi probabil era un pic mai eficient din punct de vedere al
memoriei sa folosesc un singur buffer. Pe langa aceste campuri, structura mai
contine si file descriptor-ul de unde va scrie/citi date, cateva variabile ce tin
locaul unor flag-uri pentru eroari/eof, respectiv pentru a stoca ultima operatie
facuta si pentru a retine pozita in fisier. In cazul in care as fi implementat si
functiile so_popen si so_pclose ar fi trebuit sa mai adaug cateva campuri printre
care unul pentru pipe-ul de comunicare intre procesul copil si cel parinte.
Consider ca tema a fost destul de utila pentru o intelegere mai buna a bibliotecii
stdio.
Implementarea nu este cea mai buna. Un prim motiv ar fi cel specificat mai
sus(folosirea a doua buffere in loc de unul). Altul ar fi faptul ca unele operatii
nu mi se par cele mai optime(implementarea fread/fwrite cred ca se putea face fara
apelearea fgetc/fputc sau folosirea unor functii auxiliare pentru a optimiza unele
operatii cum ar fi stocarea datelor in buffere).


Implementare
-
Nu am implementat functiile so_popen si so_pclose deoarece nu am mai avut timp si
nici multe idei. Restul functiilor au fost implementate doar pe Linux.
In enunt nu scrie daca flag-ul de eroare se poate modifica inapoi daca eroarea a
fost rezolvata, astfel aceasta optiune nu este implementata. De asemenea, la
operatiile de fread/fwrite se specifica ca daca apare vreo eroare la scriere, se
va intoarce SO_EOF. Cu toate acestea, am considerat ca, deoarece o eroare apare
si la ajungerea la finalul fisierulu si, cu toate astea, este posibil sa fi fost
citite/scrise cateva elemente, am ales sa intorc numarul de elemente citite/scrise
chiar daca nu este testat acest lucru.
Functia so_fflush am implementat-o pornind de la functia xwrite din laboratorul
2.
Nu stiu daca exista functionalitati lipsa, stiu sigur ca testul 43 nu ar trebui
sa treaca pentru ca functia nu face nimic, intoarce -1 mereu.
Ca dificultati intalnite au fost rularea checker-ului si intelegerea testelor(nu
este foarte explicit daca un test iti pica daca nu ai implementat corect ce se
testeaza sau daca exista o alta functie ce ar fi trebuit sa o implementezi
inainte).
Lucruri interesante descoperite pe parcurs: Valgrind is a bitch.

Cum se compilează și cum se rulează?
-
Se folosesc bibliotecile: string.h, fcntl.h, unistd.h. Pentru compilare se pot
da comenzile make/make build. In urma compilarii se creaza o biblioteca dinamica
ce poate fi linkata la alte programe pentru a folosi functionalitatile
implementate.

Bibliografie
-
Laboratorul 2 SO: https://ocw.cs.pub.ro/courses/so/laboratoare/laborator-02
