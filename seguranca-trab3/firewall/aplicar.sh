#!/bin/sh
# Uso: sh /firewall/aplicar.sh aberto|protegido
# Carrega /firewall/<politica>.nft. Os dois arquivos usam a MESMA tabela
# (inet lab), então carregar um substitui o outro por completo.
POLITICA="${1:?uso: aplicar.sh aberto|protegido}"
ARQ="/firewall/${POLITICA}.nft"

if [ ! -f "$ARQ" ]; then
    echo "[firewall] ERRO: $ARQ não existe (o Membro D ainda vai criá-lo?)" >&2
    exit 1
fi

nft -f "$ARQ"
echo "[firewall] política '$POLITICA' carregada de $ARQ"
nft list table inet lab | sed 's/^/[firewall]   /'
