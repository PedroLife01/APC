# PROVISÓRIO (Membro A) - o Membro B substitui pela versão com TLS (443) e 8080.
FROM ubuntu:24.04
RUN apt-get update \
 && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
      nginx iproute2 \
 && rm -rf /var/lib/apt/lists/*
COPY nginx.conf /etc/nginx/nginx.conf
