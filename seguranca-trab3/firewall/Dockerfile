# Firewall/gateway: roteia entre rede_clientes e rede_servidores e filtra com nftables.
FROM ubuntu:24.04
RUN apt-get update \
 && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
      nftables iproute2 \
 && rm -rf /var/lib/apt/lists/*
# Membro D: para registrar bloqueios, acrescente "ulogd2" na lista acima.
# As regras (*.nft) e scripts ficam em ./firewall e são montados em /firewall,
# então editar uma regra não exige reconstruir a imagem.
ENTRYPOINT ["sh", "/firewall/entrypoint.sh"]
