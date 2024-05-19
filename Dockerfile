# Etap 1: Budowanie pliku binarnego serwera C
FROM alpine:3.19.1 AS builder

# Ustawienie zmiennej środowiskowej BASE_VERSION, z domyślną wartościa v1
ARG BASE_VERSION
ENV APP_VERSION=${BASE_VERSION:-v1}

# Instalacja kompilatora C i musl-dev
RUN apk add --no-cache build-base musl-dev \
    && rm -rf /var/cache/apk/*

# Ustawienie katalogu roboczego
WORKDIR /app

# Kopiowanie pliku źródłowego
COPY server.c .

# Kompilacja statyczna, utworzenie pliku binarnego
RUN gcc -static -o server server.c

# Kopiowanie pliku index.html
COPY index.html .

# Etap 2: Tworzenie minimalnego obrazu
FROM scratch as final

# Ustawienie zmiennej środowiskowej BASE_VERSION, z domyślną wartościa v1
ARG BASE_VERSION
ENV APP_VERSION=${BASE_VERSION:-v1}

# Informacje o autorze, dokumentacji i licencji
LABEL org.opencontainers.image.authors="Paweł Pieczykolan"
LABEL org.opencontainers.image.documentation="https://github.com/97703/DockerLabZad1"
LABEL org.opencontainers.image.licenses="GNU v3.0"

# Kopiowanie bin z etapu budowania
COPY --from=builder /app/server /server

# Kopiowanie pliku index.html
COPY --from=builder /app/index.html /index.html

# Ustawienie portu
EXPOSE 8080

# Komenda startowa
ENTRYPOINT ["/server"]
