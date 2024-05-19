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

#️⃣ Ustawienie portu
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
  <img src="https://raw.githubusercontent.com/97703/DockerZad1/main/Rysunki/build.png?token=GHSAT0AAAAAACRXVHYOSVLLG2MZPCVRBLVQZSJSBPQ" style="width: 80%; height: 80%" /></p>
<p align="center">
  <i>Rys. 3. Zbudowanie obrazu dockerzad1:v1</i>
</p>

### 2.3. Inspekcja obrazu
Wyświetlono liczbę warstw nowoutworzonego obrazu.
```
docker history local/dockerzad1:v1
```
<p align="center">
  <img src="https://raw.githubusercontent.com/97703/DockerZad1/main/Rysunki/warstwy.png?token=GHSAT0AAAAAACRXVHYPHOYIMXDHY75QHPGCZSJSDUQ" style="width: 80%; height: 80%" /></p>
<p align="center">
  <i>Rys. 4. Warstwy obrazu dockerzad1:v1</i>
</p>

Sprawdzono obraz pod względem liczebności i jakości potencjalnych luk bezpieczeństwa.
```
docker scout quickview local/dockerzad1:v1
```
<p align="center">
  <img src="https://raw.githubusercontent.com/97703/DockerZad1/main/Rysunki/scout-quickview-obw.png?token=GHSAT0AAAAAACRXVHYOP72VETPILHBRJJ5UZSJSCOA" style="width: 80%; height: 80%" /></p>
<p align="center">
  <i>Rys. 5. Liczba i jakość luk bezpieczeństwa w obrazie dockerzad1:v1</i>
</p>

Wyświetlono również jego objętość.
```
docker images --filter "reference=local/dockerzad1"
```
<p align="center">
  <img src="https://raw.githubusercontent.com/97703/DockerZad1/main/Rysunki/docker-images-obw.png?token=GHSAT0AAAAAACRXVHYP5DQ3L4ASUB3BSJDCZSJSDCA" style="width: 80%; height: 80%" /></p>
<p align="center">
  <i>Rys. 6. Objętość obrazu części obowiązkowej</i>
</p>

### 2.4. Uruchomienie kontenera
Uruchomiono kontener na podstawie nowoutworzonego obrazu.
```
docker run -p 8080:8080 --name dockerzad1 local/dockerzad1:v1
```

<p align="center">
  <img src="https://raw.githubusercontent.com/97703/DockerZad1/main/Rysunki/docker-run.png?token=GHSAT0AAAAAACRXVHYPMV4WSXSPQDU2JKRWZSJSHUA" style="width: 80%; height: 80%" /></p>
<p align="center">
  <i>Rys. 7. Uruchomienie kontenera dockerzad1</i>
</p>

Sprawdzono dziennik zdarzeń za pomocą
```
docker logs dockerzad1
```

<p align="center">
  <img src="https://raw.githubusercontent.com/97703/DockerZad1/main/Rysunki/docker-logs.png?token=GHSAT0AAAAAACRXVHYOQQI5MTYEAD43WSJ4ZSJSJYA" style="width: 50%; height: 50%" /></p>
<p align="center">
  <i>Rys. 8. Dziennik zdarzeń dla kontenera dockerzad1 (fragment)</i>
</p>

### 2.5. Weryfikacja działania
Uruchomiono przeglądarkę **« _Firefox_ »** i sprawdzono działanie serwera.

<p align="center">
  <img src="https://raw.githubusercontent.com/97703/DockerZad1/main/Rysunki/localhost8080.png?token=GHSAT0AAAAAACRXVHYPFXMQYJWX7O436QDUZSJSEJA" style="width: 50%; height: 50%" /></p>
<p align="center">
  <i>Rys. 9. Efekt działania kontenera</i>
</p>
