# PROVISÓRIO (Membro A) - o Membro C substitui pela aplicação final
# (hash Argon2/scrypt, HMAC etc.). Ex.: acrescentar python3-argon2 na lista.
FROM ubuntu:24.04
RUN apt-get update \
 && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
      python3 iproute2 \
 && rm -rf /var/lib/apt/lists/*
COPY app.py /app/app.py
