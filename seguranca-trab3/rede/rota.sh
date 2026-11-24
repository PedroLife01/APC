#!/bin/sh
# Entrypoint comum de clientes e servidores.
# 1) Troca a rota padrão para o firewall (variável GATEWAY).
#    Assim, qualquer pacote para a OUTRA rede sai pelo firewall.
# 2) Executa o comando principal do contêiner (nginx, python, sleep...).
set -e

if [ -n "$GATEWAY" ]; then
    ip route replace default via "$GATEWAY"
    echo "[rota] $(hostname): rota padrão -> $GATEWAY"
    ip route | sed 's/^/[rota]   /'
fi

exec "$@"
