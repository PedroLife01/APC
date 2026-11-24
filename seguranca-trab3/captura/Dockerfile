# Monitoramento: tcpdump para capturar e tshark (Wireshark em linha de comando)
# para analisar os .pcap. Roda no namespace de rede do firewall.
FROM ubuntu:24.04
RUN apt-get update \
 && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
      tcpdump tshark iproute2 procps \
 && rm -rf /var/lib/apt/lists/*
