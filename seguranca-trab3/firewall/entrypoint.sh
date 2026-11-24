#!/bin/sh
# Inicialização do firewall.
set -e

echo "[firewall] ip_forward = $(cat /proc/sys/net/ipv4/ip_forward)  (1 = roteando)"
ip -br addr | sed 's/^/[firewall]   /'

# Cenário inseguro -> política aberta; cenário protegido -> política restritiva.
case "${CENARIO:-inseguro}" in
    protegido) POLITICA=protegido ;;
    *)         POLITICA=aberto ;;
esac
if [ ! -f "/firewall/$POLITICA.nft" ]; then
    echo "[firewall] AVISO: /firewall/$POLITICA.nft ainda não existe; usando 'aberto' por enquanto"
    POLITICA=aberto
fi
sh /firewall/aplicar.sh "$POLITICA"

# Ganchos do Membro D (ex.: iniciar o ulogd2 para registrar bloqueios).
[ -f /firewall/extra.sh ] && sh /firewall/extra.sh

exec sleep infinity
