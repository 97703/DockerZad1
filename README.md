<img src="https://github.com/97703/DockerLab3/blob/main/rysunki/loga_weii.png?raw=true" style="width: 40%; height: 40%" />

> **Programowanie Aplikacji w Chmurze Obliczeniowej**

      dr inż. Sławomir Wojciech Przyłucki

<br>
Termin zajęć:

      wtorek, godz. 14:15,

Numer na liście:

      9,

Imię i nazwisko:

      Paweł Pieczykolan,
      III rok studiów inżynierskich, IO 6.7.

# Budowanie, uruchamianie i publikowanie obrazu serwera języka C o małej wadze

**1. [Informacje ogólne](#1-informacje-ogólne)**\
\
**2. [Część obowiązkowa](#2-część-obowiązkowa)**\
\
» **2.1 [Przygotowanie wymaganych plikówy](#21-przygotowanie-wymaganych-plików)**\
\
» **2.2 [Zbudowanie obrazu](#22-zbudowanie-obrazu)**\
\
» **2.3 [Inspekcja obrazu](#23-inspekcja-obrazu)**\
\
» **2.4 [Uruchomienie kontenera](#24-uruchomienie-kontenera)**\
\
» **2.5 [Weryfikacja działania](#25-weryfikacja-działania)**\
\
**3. [Część dodatkowa](#3-część-dodatkowa)**\
\
» **3.1 [Modyfikacja pliku Dockerfile](#31-modyfikacja-pliku-dockerfile)**\
\
» **3.2 [Utworzenie i użycie sterownika Docker Buildx](#32-utworzenie-i-użycie-sterownika-docker-buildx)**\
\
» **3.3 [Zbudowanie obrazu z użyciem Docker Buildx](#33-zbudowanie-obrazu-z-użyciem-docker-buildx)**\
\
» **3.4 [Inspekcja obrazu](#34-inspekcja-obrazu)**\
\
» **3.5 [Uruchomienie kontenera](#35-uruchomienie-kontenera)**\
\
» **3.6 [Weryfikacja działania](#36-weryfikacja-działania)**\
\
» **3.7 [Porównanie obrazów](#37-porównanie-obrazów)**\
\
**4. [Podsumowanie](#4-podsumowanie)**

<br>

## 1. Informacje ogólne
*Podstawowe informacje dotyczące opracowania*
<br><br><br>


W części obowiązkowej należało stworzyć serwer w dowolnym języku programowania, który działa w kontenerze **« _Docker_ »** i realizuje określoną funkcjonalność: po uruchomieniu kontenera, serwer zapisuje w logach datę uruchomienia, imię i nazwisko autora (studenta) oraz port **« _TCP_ »**, na którym nasłuchuje na zgłoszenia klienta. Na podstawie adresu **« _IP_ »** klienta, serwer wyświetla stronę informującą o adresie **« _IP_ »** klienta oraz o dacie i godzinie w jego strefie czasowej.
Należało opracować plik Dockerfile, który umożliwia zbudowanie obrazu kontenera z serwerem, z optymalizacją wieloetapowego budowania obrazu, użycia warstwy scratch, optymalizacji cache, zawartości i ilości warstw. Plik **« _Dockerfile_ »** miał zawierać informację o autorze.

Aplikacja serwerowa, która wyświetla stronę internetową **« _HTML_ »**, została napisana w języku **« _C_ »**. Wykorzystano obrazy [**« _Alpine_ »**](https://hub.docker.com/layers/library/alpine/3.19.1/images/sha256-6457d53fb065d6f250e1504b9bc42d5b6c65941d57532c072d929dd0628977d0) w wersji ```3.19.1``` oraz [**« _Scratch_ »**](https://hub.docker.com/_/scratch) w celu drastycznego zmniejszenia objętości obrazu (> 1 MB).

Część dodatkowa wymagała zbudowania obrazu kontenera z aplikacją opracowaną w części obowiązkowej, które działałyby na architekturach: ```linux/arm64``` oraz ```linux/amd64```, wykorzystując sterownik **« _Docker Container_ »**. **« _Dockerfile_ »** powinien wykorzystywać rozszerzony frontend, zawierać deklaracje wykorzystania cache i umożliwiać bezpośrednie wykorzystanie kodów aplikacji umieszczonych w publicznym repozytorium na [GitHub](https://github.com/). Opracowane obrazy należało przesłać do swojego repozytorium na [DockerHub](https://hub.docker.com/).

W sprawozdaniu należało podać wykorzystane instrukcje wraz z wynikiem ich działania oraz ewentualnymi komentarzami, unikając przekazywania danych wrażliwych. Wszystkie informacje wymagane w sprawozdaniu z części nieobowiązkowej należało opracować w postaci pliku zadanie1_dod.md. W repozytorium należało umieścić opracowane źródła dla serwera, przygotowany plik **« _Dockerfile_ »** oraz inne niezbędne pliki. Jako sprawozdanie należało przekazać wyłącznie plik tekstowy zawierający linki do użytego repozytorium na [GitHub](https://github.com/) oraz [DockerHub](https://hub.docker.com/).

> [!NOTE]
> W realizacji zadania napisania serwera w dowolnym języku, wykorzystano informacje pochodzące ze strony [dev.to](https://dev.to/jeffreythecoder/how-i-built-a-simple-http-server-from-scratch-using-c-739).
>
> Zadanie wykonano na maszynie wirtualnej [VM VirtualBox](https://www.virtualbox.org/), na systemie Ubuntu w wersji [20.04 LTS](https://ubuntu.com/download/desktop).

---

## 2. Część obowiązkowa
*Przygotowanie plików serwerowych, zbudowanie obrazu, uruchomienie kontenera oraz weryfikacja działania serwera*
<br><br><br>

### 2.1. Przygotowanie wymaganych plików
Utworzono plik **« _Dockerfile_ »** spełniający wymogi zadania.

```diff
#️⃣ Etap 1: Budowanie pliku binarnego serwera C
FROM alpine:3.19.1 AS builder

#️⃣ Ustawienie zmiennej środowiskowej BASE_VERSION, z domyślną wartościa v1
ARG BASE_VERSION
ENV APP_VERSION=${BASE_VERSION:-v1}

#️⃣ Instalacja kompilatora C i musl-dev
RUN apk add --no-cache build-base musl-dev \
    && rm -rf /var/cache/apk/*

#️⃣ Ustawienie katalogu roboczego
WORKDIR /app

#️⃣ Kopiowanie pliku źródłowego
COPY server.c .

#️⃣ Kompilacja statyczna, utworzenie pliku binarnego
RUN gcc -static -o server server.c

#️⃣ Kopiowanie pliku index.html
COPY index.html .

#️⃣ Etap 2: Tworzenie minimalnego obrazu
FROM scratch as final

#️⃣ Ustawienie zmiennej środowiskowej BASE_VERSION, z domyślną wartościa v1
ARG BASE_VERSION
ENV APP_VERSION=${BASE_VERSION:-v1}

#️⃣ Informacje o autorze, dokumentacji i licencji
LABEL org.opencontainers.image.authors="Paweł Pieczykolan"
LABEL org.opencontainers.image.documentation="https://github.com/97703/DockerLabZad1"
LABEL org.opencontainers.image.licenses="GNU v3.0"

#️⃣ Kopiowanie bin z etapu budowania
COPY --from=builder /app/server /server

#️⃣ Kopiowanie pliku index.html
COPY --from=builder /app/index.html /index.html

#️⃣ Ustawienie informacji o porcie
EXPOSE 8080

#️⃣ Komenda startowa
ENTRYPOINT ["/server"]
```
<p align="center">
  <i>Rys. 1. Plik Dockerfile</i>
</p>

<br>

Utworzono prosty plik serwera w języku **« _C_ »** [server.c](https://github.com/97703/DockerZad1/blob/main/server.c) oraz stronę internetową **« _HTML_ »** [index.html](https://github.com/97703/DockerZad1/blob/main/index.html) korzystającą ze skryptu **« _Javascript_ »**.

```diff
<!DOCTYPE html>
<html lang="pl">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Informacje o Kliencie</title>
    <script>
        #️⃣ Wywołanie API do pobrania adresu IP
        fetch("https://api.ipify.org?format=json")
            #️⃣ Konwersja odpowiedzi na format JSON
            .then(response => response.json())
            #️⃣ Obsługa danych
            .then(data => {
                #️⃣ Wyświetlenie adresu IP
                document.getElementById("ip").innerText = data.ip;
                #️⃣ Pobranie danych dotyczących czasu związanych ze strefą czasową ze strony worldtimeapi
                return fetch("https://worldtimeapi.org/api/ip/" + data.ip);
            })
            #️⃣ Konwersja odpowiedzi na format JSON
            .then(response => response.json())
            #️⃣ Obsługa danych
            .then(data => {
                #️⃣ Utworzenie obiektu czasu
                var czas = new Date(data.utc_datetime);
                #️⃣ Wyświetlenie czasu
                document.getElementById("czas").innerText = czas.toLocaleString();
                #️⃣ Wyświetlenie strefy czasowej
                document.getElementById("strefa").innerText = data.timezone;
            })
            #️⃣ Obsługa błędów
            .catch(error => console.error('Błąd:', error));
    #️⃣ Zamknięcie sekcji skryptu
    </script>
</head>
<body>
    <h1>Informacje o Kliencie</h1>
    <p>Adres IP klienta: <span id="ip"></span></p>
    <p>Czas związany ze strefą czasową tego IP: <span id="czas"></span></p>
    <p>Strefa czasowa: <span id="strefa"></span></p>
</body>
</html>

```
<p align="center">
  <i>Rys. 2. Plik index.html, analiza skryptu Javascript (komentarze)</i>
</p>


### 2.2. Zbudowanie obrazu
Zbudowano obraz korzystając z pliku **« _Dockerfile_ »**.
```
docker build --build-arg BASE_VERSION=v1.0 -t local/dockerzad1:v1
```

<p align="center">
  <img src="https://github.com/97703/DockerZad1/blob/main/Rysunki/build.png" style="width: 80%; height: 80%" /></p>
<p align="center">
  <i>Rys. 3. Zbudowanie obrazu dockerzad1:v1</i>
</p>

### 2.3. Inspekcja obrazu
Wyświetlono liczbę warstw nowoutworzonego obrazu.
```
docker history local/dockerzad1:v1
```
<p align="center">
  <img src="https://github.com/97703/DockerZad1/blob/main/Rysunki/warstwy.png" style="width: 80%; height: 80%" /></p>
<p align="center">
  <i>Rys. 4. Warstwy obrazu dockerzad1:v1</i>
</p>

Sprawdzono obraz pod względem liczebności i jakości potencjalnych luk bezpieczeństwa.
```
docker scout quickview local/dockerzad1:v1
```
<p align="center">
  <img src="https://github.com/97703/DockerZad1/blob/main/Rysunki/scout-quickview-obw.png" style="width: 80%; height: 80%" /></p>
<p align="center">
  <i>Rys. 5. Liczba i jakość luk bezpieczeństwa w obrazie dockerzad1:v1</i>
</p>

Wyświetlono również jego objętość.
```
docker images --filter "reference=local/dockerzad1"
```
<p align="center">
  <img src="https://github.com/97703/DockerZad1/blob/main/Rysunki/docker-images-obw.png" style="width: 80%; height: 80%" /></p>
<p align="center">
  <i>Rys. 6. Objętość obrazu części obowiązkowej</i>
</p>

### 2.4. Uruchomienie kontenera
Uruchomiono kontener na podstawie nowoutworzonego obrazu.
```
docker run -p 8080:8080 --name dockerzad1 local/dockerzad1:v1
```

<p align="center">
  <img src="https://github.com/97703/DockerZad1/blob/main/Rysunki/docker-run.png" style="width: 80%; height: 80%" /></p>
<p align="center">
  <i>Rys. 7. Uruchomienie kontenera dockerzad1</i>
</p>

Sprawdzono dziennik zdarzeń za pomocą
```
docker logs dockerzad1
```

<p align="center">
  <img src="https://github.com/97703/DockerZad1/blob/main/Rysunki/docker-logs.png" style="width: 50%; height: 50%" /></p>
<p align="center">
  <i>Rys. 8. Dziennik zdarzeń dla kontenera dockerzad1 (fragment)</i>
</p>

### 2.5. Weryfikacja działania
Uruchomiono przeglądarkę **« _Firefox_ »** i sprawdzono działanie serwera.

<p align="center">
  <img src="https://github.com/97703/DockerZad1/blob/main/Rysunki/localhost8080.png" style="width: 50%; height: 50%" /></p>
<p align="center">
  <i>Rys. 9. Efekt działania kontenera</i>
</p>

---

## 3. Część dodatkowa
*Zmodyfikowanie pliku Dockerfile, użycie Docker Buildx oraz Docker Scout*
<br><br><br>

### 3.1. Modyfikacja pliku Dockerfile
Zmodyfikowano plik **« _Dockerfile_ »** z części nieobowiązkowej.

Dodano:
- ```# syntax=docker/dockerfile:1.4``` – odwołanie do oficjalnej i stabilnej wersji obrazu dla rozszerzonych frontend-ów
- do instrukcji **« _RUN_ »** ```apk add --no-cache openssh-client git \``` – instalacja klienta **« _OpenSSH_ »** oraz **« _Git_ »** w kontenerze **« _Docker_ »**
- do instrukcji **« _RUN_ »** ```mkdir -p -m 0700 ~/.ssh``` – utworzenie foldera **« _.ssh_ »** w katalogu domowym z uprawnieniami 700 (pełne uprawnienia dla właściciela katalogu), o ile taki katalog nie istnieje
- do instrukcji **« _RUN_ »** ```ssh-keyscan github.com >> ~/.ssh/known_hosts && eval $(ssh-agent)``` – konfiguracja środowiska do korzystania z GitHuba poprzez SSH; uruchomienie agenta SSH
- ```RUN --mount=type=ssh git clone git@github.com:97703/DockerZad1.git DockerZad1``` – klonowanie prywatnego repozytorium **« _Git_ »**, przy wykorzystaniu bezpiecznego dostępu **« _SSH_ »**
- ```mv /app/DockerZad1/server.c /app/DockerZad1/index.html /app``` – przeniesienie plików ze sklonowanego repozytorium
- do pierwszej instrukcji **« _RUN_ »** etapu pierwszego ```--mount=type=cache,target=/var/cache/apk``` – wskazuje folder **« _apk_ »** dla mechanizmu cache'owania
- do drugiej instrukcji **« _RUN_ »** etapu pierwszego ```--mount=type=cache,target=/var/cache/gcc``` – wskazuje folder **« _gcc_ »** dla mechanizmu cache'owania

```diff
+# syntax=docker/dockerfile:1.4
#️⃣ Etap 1: Budowanie pliku binarnego serwera C
FROM alpine:3.19.1 AS builder

#️⃣ Ustawienie zmiennej środowiskowej BASE_VERSION, z domyślną wartościa v1
ARG BASE_VERSION
ENV APP_VERSION=${BASE_VERSION:-v1}

#️⃣ Instalacja kompilatora C i musl-dev
+RUN --mount=type=cache,target=/var/cache/apk \
    apk add --no-cache build-base musl-dev \
+    #️⃣ Czyszczenie pamięci podręcznej apk
    && rm -rf /var/cache/apk/* \
+    #️⃣ Instalacja klienta SSH i GITa
+    && apk add --no-cache openssh-client git \
+    #️⃣ Konfiguracja SSH dla dostępu do GitHub
+    && mkdir -p -m 0700 ~/.ssh && ssh-keyscan github.com >> ~/.ssh/known_hosts && eval $(ssh-agent)

#️⃣ Ustawienie katalogu roboczego
WORKDIR /app

-#️⃣ Kopiowanie pliku źródłowego
-COPY server.c .

-#️⃣ Kompilacja statyczna, utworzenie pliku binarnego
-RUN gcc -static -o server server.c

-#️⃣ Kopiowanie pliku index.html
-COPY index.html .

+#️⃣ Sklonowanie repozytorium i przeniesienie plików
+RUN --mount=type=ssh git clone git@github.com:97703/DockerZad1.git DockerZad1 \
+    && mv /app/DockerZad1/server.c /app/DockerZad1/index.html /app

+#️⃣ Kompilacja statyczna, utworzenie pliku binarnego
+RUN --mount=type=cache,target=/var/cache/gcc \
+    gcc -static -o server server.c

#️⃣ Etap 2: Tworzenie minimalnego obrazu
FROM scratch as final

#️⃣ Ustawienie zmiennej środowiskowej BASE_VERSION, z domyślną wartościa v1
ARG BASE_VERSION
ENV APP_VERSION=${BASE_VERSION:-v1}

#️⃣ Informacje o autorze, dokumentacji i licencji
LABEL org.opencontainers.image.authors="Paweł Pieczykolan"
LABEL org.opencontainers.image.documentation="https://github.com/97703/DockerLabZad1"
LABEL org.opencontainers.image.licenses="GNU v3.0"

#️⃣ Kopiowanie bin z etapu budowania
COPY --from=builder /app/server /server

#️⃣ Kopiowanie pliku index.html
COPY --from=builder /app/index.html /index.html

#️⃣ Ustawienie portu
EXPOSE 8080

#️⃣ Komenda startowa
ENTRYPOINT ["/server"]

```
<p align="center">
  <i>Rys. 10. Zmiany w pliku Dockerfile</i>
</p>

### 3.2. Utworzenie i użycie sterownika Docker Buildx
Utworzono nowy sterownik [Docker Buildx](https://docs.docker.com/reference/cli/docker/buildx/) o nazwie ```dockerzad1```.
```
docker buildx create --name dockerzad1 --driver docker-container --bootstrap
```
Nowoutworzony sterownik ustawiono jako domyślny za pomocą
```
docker buildx use dockerzad1
```

<p align="center">
  <img src="https://github.com/97703/DockerZad1/blob/main/Rysunki/buildx-create.png" style="width: 80%; height: 80%" /></p>
<p align="center">
  <i>Rys. 11. Utworzenie i konfiguracja sterownika Docker Container</i>
</p>

### 3.3. Zbudowanie obrazu z użyciem Docker Buildx
Zbudowano obraz w oparciu o polecenie [Docker Buildx](https://docs.docker.com/reference/cli/docker/buildx/)
```
docker buildx build -f Dockerfile-2 --sbom=true --provenance=mode=max --ssh default=$SSH_AUTH_SOCK --build-arg BASE_VERSION=v.1.0 --platform linux/arm64,linux/amd64 --cache-from=type=registry,ref=docker.io/ppieczykolan/dockerzad1-2:cache --cache-to=type=registry,ref=docker.io/ppieczykolan/dockerzad1-2:cache -t ppieczykolan/dockerzad1-2 --push .
```
, gdzie:
- ```docker buildx build``` – polecenie służace do budowania obrazów przy użyciu [BuildKit](https://docs.docker.com/reference/cli/docker/buildx/build/)
- ```-f Dockerfile-2``` – określa nazwę pliku **« _Dockerfile_ »**
- ```--sbom=true``` – generowanie zestawienia materiałów oprogramowania (spisu komponentów oprogramowania użytych w celu zbudowania danej aplikacji)
- ```provenance=mode=max``` – określa poziom szczegółowości rejestrowania etapów kompilacji
- ```ssh default=$SSH_AUTH_SOCK``` – umożliwia procesowi kompilacji korzystanie z uwierzytelnienia **« _SSH_ »**
- ```--build-arg BASE_VERSION=v.1.0``` – przekazuje argument wersji do pliku **« _Dockerfile_ »**
- ```--platform linux/arm64,linux/amd64``` – określa platformy docelowe; platformy dla których obraz jest budowany
- ```--cache-from=type=registry,ref=docker.io/ppieczykolan/dockerzad1-2:cache``` – określa źródło z którego można pobrać pamięć podręczną **« _cache_ »**
- ```--cache-to=type=registry,ref=docker.io/ppieczykolan/dockerzad1-2:cache``` – określa miejsce docelowe pamięci podręcznej **« _cache_ »**; miejsce udostępnienia pamięci **« _cache_ »**
- ```-t ppieczykolan/dockerzad1-2``` – określa miejsce docelowe obrazu (reporyztorium [DockerHub](https://hub.docker.com/) lub dysk) 
- ```--push``` – nakazuje przesłanie obrazu do repozytorium
- ```.``` – określa kontekst kompliacji

<p align="center">
  <img src="https://github.com/97703/DockerZad1/blob/main/Rysunki/buildx-build-1.png" style="width: 80%; height: 80%" /></p>
<p align="center">
  <i>Rys. 12. Zbudowanie obrazu ze zmodyfikowanego pliku Dockerfile – ½</i>
</p>

<p align="center">
  <img src="https://github.com/97703/DockerZad1/blob/main/Rysunki/buildx-build-2.png" style="width: 80%; height: 80%" /></p>
<p align="center">
  <i>Rys. 13. Zbudowanie obrazu ze zmodyfikowanego pliku Dockerfile – ²⁄₂</i>
</p>

> [!WARNING]
> Błąd
>```
>ERROR importing cache manifest from docker.io/ppieczykolan/dockerzad1-2:cache
>```
>nie jest istotny, gdyż jest to pierwsze budowanie obrazu – nie możemy pobrać **« _cache_ »** z serwera ponieważ nie został on nigdy wcześniej udostępniony.

### 3.4. Inspekcja obrazu
Zezwolono na analizowanie obrazu przez usługę [Docker Scout](https://docs.docker.com/scout/)
<p align="center">
  <img src="https://github.com/97703/DockerZad1/blob/main/Rysunki/docker-scout.png" style="width: 80%; height: 80%" /></p>
<p align="center">
  <i>Rys. 14. Podłączenie usługi Docker Scout pod obraz dockerzad1-2</i>
</p>

Zweryfikowano zbudowany obraz pod kątem samej budowy.
```
docker buildx imagetools inspect ppieczykolan/dockerzad1-2
```
<p align="center">
  <img src="https://github.com/97703/DockerZad1/blob/main/Rysunki/buildx-inspect.png" style="width: 80%; height: 80%" /></p>
<p align="center">
  <i>Rys. 15. Sprawdzenie obecności i liczebności luk bezpieczeństwa</i>
</p>

Sprawdzono liczbę warstw dla nowego obrazu.
```
docker history ppieczykolan/dockerzad1-2
```
<p align="center">
  <img src="https://github.com/97703/DockerZad1/blob/main/Rysunki/warstwy-2.png" style="width: 80%; height: 80%" /></p>
<p align="center">
  <i>Rys. 16. Warstwy obrazu części dodatkowej</i>
</p>


Sprawdzono również obecność luk bezpieczeństwa.
```
docker scout quickview ppieczykolan/dockerzad1-2
```
<p align="center">
  <img src="https://github.com/97703/DockerZad1/blob/main/Rysunki/scout-quickview.png" style="width: 80%; height: 80%" /></p>
<p align="center">
  <i>Rys. 17. Sprawdzenie obecności i liczebności luk bezpieczeństwa</i>
</p>

Obraz jest dostępny pod [tym adresem](https://hub.docker.com/repository/docker/ppieczykolan/dockerzad1-2/general). 

### 3.5. Uruchomienie kontenera
Zaciągnięto obraz ze zdalnego repozytorium za pomocą
```
docker pull ppieczykolan/dockerzad1-2
```
<p align="center">
  <img src="https://github.com/97703/DockerZad1/blob/main/Rysunki/docker-pull.png" style="width: 80%; height: 80%" /></p>
<p align="center">
  <i>Rys. 18. Zaciągnięcie obrazu dockerzad1-2</i>
</p>

Uruchomiono kontener na podstawie zaciągniętego obrazu.
```
docker run -p 8080:8080 --name dockerzad1-2 ppieczykolan/dockerzad1-2:latest
```

<p align="center">
  <img src="https://github.com/97703/DockerZad1/blob/main/Rysunki/docker-run-2.png" style="width: 80%; height: 80%" /></p>
<p align="center">
  <i>Rys. 19. Uruchomienie kontenera części dodatkowej</i>
</p>

### 3.6. Weryfikacja działania
Zweryfikowano działanie kontenera części dodatkowej.
Uruchomiono przeglądarkę i wpisano w pasku adresowym **« _http://localhost:8080_ »**.

<p align="center">
  <img src="https://github.com/97703/DockerZad1/blob/main/Rysunki/localhost8080-2.png" style="width: 50%; height: 50%" /></p>
<p align="center">
  <i>Rys. 20. Efekt działania kontenera części dodatkowej</i>
</p>

### 3.7. Porównanie obrazów
Porównano utworzone w tym rozwiązaniu obrazy pod względem objętości.
```
docker images
```

<p align="center">
  <img src="https://github.com/97703/DockerZad1/blob/main/Rysunki/docker-images.png" style="width: 80%; height: 80%" /></p>
<p align="center">
  <i>Rys. 21. Porównanie obrazów</i>
</p>

---

## 4. Podsumowanie
*Wnioski*
<br><br><br>

Zadanie zostało zrealizowane pomyślnie.

W części obowiązkowej opracowano plik **« _Dockerfile_ »** oraz przygotowano prosty serwer w języku **« _C_ »**, który działa w kontenerze **« _Docker_ »**. Serwer ten wyświetla informacje o adresie **« _IP_ »** klienta oraz o dacie i godzinie w jego strefie czasowej.

W części dodatkowej zbudowano obraz kontenera z aplikacją opracowaną wcześniej, który działa na różnych architekturach. Ponadto zastosowano rozszerzony frontend, mechanizm cache'owania oraz udostępniono obraz z cachem na [DockerHub](https://hub.docker.com/repository/docker/ppieczykolan/dockerzad1-2/general).

Wszystkie cele zadania zostały osiągnięte. Dzięki wykorzystaniu narzędzi takich jak [Docker Buildx](https://docs.docker.com/reference/cli/docker/buildx/) i [Docker Scout](https://docs.docker.com/scout/), możliwe było zautomatyzowanie, ulepszenie procesu budowy obrazów i analiza obrazów pod kątem bezpieczeństwa, a zastosowanie warstwy [Scratch](https://hub.docker.com/_/scratch) w drugim etapie budowy pozwoliło na zmniejszenie objętości pliku do **315kB** bez potencjalnych luk bezpieczeństwa.
