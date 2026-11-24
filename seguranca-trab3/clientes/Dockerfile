# Cliente interno: ferramentas para acessar serviços e fazer os testes.
FROM ubuntu:24.04
RUN apt-get update \
 && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
      curl ca-certificates openssl iproute2 iputils-ping traceroute dnsutils netcat-openbsd \
 && rm -rf /var/lib/apt/lists/*
WORKDIR /root
