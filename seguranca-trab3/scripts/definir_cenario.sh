#!/bin/sh
# Grava CENARIO=<inseguro|protegido> no .env sem mexer nas outras variáveis.
set -e
case "$1" in inseguro|protegido) ;; *) echo "uso: $0 inseguro|protegido" >&2; exit 2 ;; esac
[ -f .env ] || cp .env.example .env
grep -v '^CENARIO=' .env > .env.tmp || true
echo "CENARIO=$1" >> .env.tmp
mv .env.tmp .env
echo "[cenario] CENARIO=$1 gravado em .env"
