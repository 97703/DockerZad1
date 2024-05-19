## 3. Część dodatkowa
*Zmodyfikowanie pliku Dockerfile, użycie Docker Buildx oraz Docker Scout*
<br><br><br>

### 3.1. Modyfikacja pliku Dockerfile
Zmodyfikowano plik **« _Dockerfile_ »** z części nieobowiązkowej.

Dodano:
- ```# syntax=docker/dockerfile:1.4``` – odwołanie do oficjalnej i stabilnej wersji obrazu dla rozszerzonych frontend-ów
- do instrukcji **« _RUN_ »** ```apk add --no-cache openssh-client git \``` – instalacja klienta **« _OpenSSH_ »** oraz **« _Git_ »** w kontenerze **« _Docker_ »**
- do instrukcji **« _RUN_ »** ```mkdir -p -m 0700 ~/.ssh``` – utworzenie foldera **« _.ssh_ »** w katalogu domowym z uprawnieniami 700 (pełne uprawnienia dla właściciela katalogu), o ile taki katalog nie istnieje
- do instrukcji **« _RUN_ »** ```ssh-keyscan github.com >> ~/.ssh/known_hosts && eval $(ssh-agent)``` – komentarz
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
