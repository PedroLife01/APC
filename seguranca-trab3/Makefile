# Atalhos do laboratório. Rode "make" ou "make help" para ver a lista.
COMPOSE  := docker compose
DONO     := -e HOST_UID=$(shell id -u) -e HOST_GID=$(shell id -g)

# Parâmetros da captura (podem ser trocados na linha de comando):
#   make captura NOME=teste SEG=20 LADO=servidores FILTRO="tcp port 443"
NOME   ?= captura-$(shell date +%Y%m%d-%H%M%S)
SEG    ?= 30
LADO   ?= clientes
FILTRO ?=

.PHONY: help up down inseguro protegido captura captura-iniciar captura-parar \
        analisar m1 test status logs limpar

help:
	@echo "make up          sobe o laboratório no cenário gravado em .env"
	@echo "make down        derruba tudo"
	@echo "make inseguro    cenário inseguro (HTTP liberado, firewall aberto)"
	@echo "make protegido   cenário protegido (HTTPS, HSTS, firewall restritivo)"
	@echo "make captura     captura SEG segundos no firewall -> evidencias/NOME.pcap"
	@echo "make analisar NOME=x  mostra o que está em claro em evidencias/x.pcap"
	@echo "make m1          demonstração completa do Módulo 1 (login HTTP capturado)"
	@echo "make test        roda todos os testes automatizados (tests/test_*.sh)"
	@echo "make status      contêineres, rotas e regras ativas"

.env:
	cp .env.example .env

up: .env
	$(COMPOSE) up -d --build

down:
	$(COMPOSE) down --remove-orphans

inseguro: .env
	sh scripts/definir_cenario.sh inseguro
	$(COMPOSE) up -d --build

protegido: .env
	sh scripts/definir_cenario.sh protegido
	$(COMPOSE) up -d --build

captura:
	$(COMPOSE) exec $(DONO) captura sh /captura/capturar.sh por $(SEG) $(NOME) $(LADO) $(FILTRO)

captura-iniciar:
	$(COMPOSE) exec $(DONO) captura sh /captura/capturar.sh iniciar $(NOME) $(LADO) $(FILTRO)

captura-parar:
	$(COMPOSE) exec $(DONO) captura sh /captura/capturar.sh parar

analisar:
	$(COMPOSE) exec captura sh /captura/analisar_http.sh /evidencias/$(NOME).pcap

m1:
	sh scripts/m1_login_http.sh

test:
	@falhas=0; for t in tests/test_*.sh; do \
	    echo "===== $$t"; sh $$t || falhas=$$((falhas+1)); \
	done; echo "===== arquivos de teste com falha: $$falhas"; [ $$falhas -eq 0 ]

status:
	$(COMPOSE) ps
	@echo "--- rotas do cliente-comum";  $(COMPOSE) exec cliente-comum ip route
	@echo "--- rotas do web";            $(COMPOSE) exec web ip route
	@echo "--- regras do firewall";      $(COMPOSE) exec firewall nft list table inet lab

logs:
	$(COMPOSE) logs -f --tail=50
