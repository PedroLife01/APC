# 2. Arquitetura do ambiente

## 2.1 Visão geral

O ambiente simula uma pequena rede corporativa usando contêineres Docker
orquestrados por um único arquivo `docker-compose.yml`. Escolhemos contêineres em vez
de três máquinas virtuais por três motivos: o ambiente inteiro sobe com um comando
(`make inseguro`), roda em qualquer máquina com Docker e cada contêiner tem a sua pilha
de rede (namespace de rede) com interfaces, rotas e regras de firewall próprias. Para
o experimento, isso equivale a ter máquinas separadas.

A topologia tem **duas redes isoladas** ligadas por um único ponto, o firewall:

```
        rede_clientes 10.10.1.0/24                     rede_servidores 10.10.2.0/24
 ┌───────────────────────────┐                      ┌──────────────────────────────┐
 │ cliente-comum  10.10.1.10 │      ┌──────────┐    │ web (Nginx)     10.10.2.10   │
 │                           ├──────┤ firewall ├────┤  :80 :443 :8080 → app:8000  │
 │ cliente-admin  10.10.1.20 │ .254 │ nftables │.254│ app (Python)    10.10.2.20   │
 └───────────────────────────┘      │ +captura │    │  :8000                       │
                                    └──────────┘    └──────────────────────────────┘
```
*Figura 1 – Topologia lógica do laboratório.*

| Componente | Endereço | Tecnologia | Função |
|---|---|---|---|
| cliente-comum | 10.10.1.10 | Ubuntu 24.04, curl, openssl | Usuário comum da intranet |
| cliente-admin | 10.10.1.20 | Ubuntu 24.04, curl, openssl | Administrador (também acessa a porta 8080) |
| firewall | 10.10.1.254 / 10.10.2.254 | nftables, `ip_forward=1` | Gateway e filtro entre as redes |
| captura | namespace do firewall | tcpdump, tshark | Monitoramento do tráfego |
| web | 10.10.2.10 (`web.lab`) | Nginx | HTTP (80), HTTPS (443), administração (8080) |
| app | 10.10.2.20 | Python 3 | Login, área protegida, HMAC |

*Tabela 1 – Componentes, endereços e funções.*

## 2.2 Roteamento pelo firewall

Cada rede Docker é uma *bridge* do Linux, um switch virtual. Contêineres na mesma
rede se comunicam direto. Para chegar à outra rede, precisam de um roteador. Fizemos
do contêiner `firewall` esse roteador:

1. Ele está nas duas redes (uma interface em cada) e tem o encaminhamento de pacotes
   IPv4 ligado (`sysctl net.ipv4.ip_forward=1`).
2. Ao iniciar, clientes e servidores executam `rede/rota.sh`, que troca a rota padrão
   para o IP do firewall na sua rede (`ip route replace default via 10.10.1.254` nos
   clientes e `via 10.10.2.254` nos servidores). A ida e a volta passam pelo firewall,
   o que é necessário para o filtro com estado (*conntrack*) do Módulo 4.
3. O firewall não faz NAT. O servidor vê o IP real do cliente, e as regras e os logs
   podem identificar quem é `cliente-comum` e quem é `cliente-admin`.

Duas opções das redes Docker garantem que o firewall é o **único** caminho:

- `inhibit_ipv4`: o host Docker não recebe IP nas bridges. Num primeiro teste, sem essa
  opção, um cliente com privilégio de rede apontou a rota para o host (10.10.1.1) e
  alcançou o servidor sem passar pelo firewall. Com a opção, esse desvio falha, e o
  `tests/test_rede.sh` confere isso automaticamente. As redes também ficam sem acesso
  à internet, isoladas como pede o enunciado.
- `gateway_mode_ipv4=nat-unprotected`: a partir da versão 28, o Docker cria no host
  uma regra anti-spoofing (`iptables -t raw ... -d 10.10.2.10 ! -i <bridge dos
  servidores> -j DROP`). Como o pacote do cliente para 10.10.2.10 atravessa primeiro a
  bridge dos *clientes* até chegar ao firewall, essa regra o descartava. A opção
  desliga essa proteção do host, e quem filtra passa a ser o nosso firewall.

A Tabela 2 resume a verificação do roteamento.

| Verificação | Comando | Resultado |
|---|---|---|
| Rota do cliente | `ip route` (cliente-comum) | `default via 10.10.1.254` |
| Rota do servidor | `ip route` (web) | `default via 10.10.2.254` |
| Caminho até o servidor | `traceroute -n web.lab` | `1 10.10.1.254`, `2 10.10.2.10` |
| Tráfego contado no firewall | `nft list table inet lab` | contadores > 0 nos dois sentidos |
| Desvio pelo host | rota via 10.10.1.1 + `curl` | falha (sem resposta) |

*Tabela 2 – Evidências de que o tráfego passa pelo firewall.*

## 2.3 Cenários inseguro e protegido

O mesmo ambiente roda em dois cenários, escolhidos pela variável `CENARIO` no
arquivo `.env` (`make inseguro` / `make protegido`):

| Aspecto | Inseguro | Protegido |
|---|---|---|
| Firewall | `aberto.nft`: encaminha tudo | `protegido.nft`: *forward* em `drop` por padrão |
| Web | HTTP na porta 80 | HTTP redireciona para HTTPS; HSTS |
| Cookie de sessão | `HttpOnly` | `HttpOnly; Secure` |

*Tabela 3 – Diferenças entre os cenários.*

As duas políticas de firewall usam a mesma tabela nftables (`inet lab`), então
carregar uma substitui a outra por completo, sem precisar de `flush ruleset`, que
apagaria as regras do DNS interno do Docker dentro do contêiner.

## 2.4 Monitoramento

O contêiner `captura` usa `network_mode: service:firewall`, ou seja, compartilha o
namespace de rede do firewall e enxerga as mesmas interfaces. É o mesmo que rodar o
tcpdump no próprio gateway, mas sem instalar ferramentas de análise no equipamento de
segurança. As capturas vão para `./evidencias/*.pcap` no host e podem ser abertas no
Wireshark.

O script `captura/capturar.sh` escolhe a interface pelo IP (lado dos clientes ou dos
servidores). Assim dá para comparar o que o cliente envia com o que de fato chega aos
servidores, o que é útil para mostrar bloqueios no Módulo 4. Usamos
`tcpdump -U --immediate-mode` porque, sem isso, os últimos pacotes ficavam no buffer
do kernel e se perdiam quando a captura era encerrada logo depois do tráfego.

## 2.5 Reprodutibilidade

O ambiente inteiro é descrito em arquivos de texto versionados: `docker-compose.yml`,
Dockerfiles, regras `.nft`, `nginx.conf` e scripts. Um `Makefile` reúne as operações
(`up`, `down`, `inseguro`, `protegido`, `captura`, `test`), e `make test` executa as
verificações automatizadas de todos os módulos. As instruções completas estão no
`README.md`.
