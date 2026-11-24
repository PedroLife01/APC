# Laboratório de Segurança de Redes — CIC0201, Trabalho 3

Pequena rede corporativa em contêineres Docker: uma rede de clientes e uma rede de
servidores separadas por um **firewall/gateway** (nftables). Tem um serviço web
(Nginx + aplicação Python) acessível por HTTP e HTTPS e um contêiner de **captura**
(tcpdump/tshark) que vê todo o tráfego que passa pelo firewall.

## Requisitos

- Linux, macOS ou Windows com **Docker Engine 28+** (ou Docker Desktop 4.41+) e o plugin `docker compose`.
- `make` e `sh` (no Windows, use o WSL2).
- Wireshark no host, opcional, para abrir os `.pcap` com interface gráfica.

## Reprodução rápida

```sh
git clone <repo> && cd seguranca-trab3
make inseguro      # constrói as imagens e sobe tudo no cenário inseguro
make test          # verifica topologia, rotas, firewall e captura
make m1            # Módulo 1: captura um login por HTTP e mostra a senha em claro
make protegido     # troca para o cenário protegido (HTTPS, HSTS, firewall restritivo)
make down          # desliga tudo
```

Na primeira vez, o `make` copia `.env.example` para `.env`. Troque a `HMAC_CHAVE` lá
(`openssl rand -hex 32`). O `.env` não vai para o git.

## Comandos do Makefile

| Comando | O que faz |
|---|---|
| `make up` | Sobe o laboratório no cenário gravado em `.env` |
| `make down` | Derruba contêineres e redes |
| `make inseguro` | Grava `CENARIO=inseguro` no `.env` e recria o que mudou (firewall aberto, cookie sem `Secure`) |
| `make protegido` | Grava `CENARIO=protegido` e recria (firewall `protegido.nft`, cookie `Secure`, HTTPS) |
| `make captura` | Captura no firewall por `SEG` segundos em `evidencias/NOME.pcap` |
| `make captura-iniciar` / `make captura-parar` | O mesmo, mas você controla o início e o fim |
| `make analisar NOME=x` | Mostra cabeçalhos, formulários e cookies legíveis em `evidencias/x.pcap` |
| `make m1` | Roteiro completo do Módulo 1 |
| `make test` | Roda todos os `tests/test_*.sh` |
| `make status` | Contêineres, rotas e regras ativas do firewall |

Exemplos de captura:

```sh
make captura NOME=m2-tls13 SEG=15 FILTRO="tcp port 443"     # Módulo 2
make captura NOME=m4-bloqueio SEG=20 LADO=servidores         # Módulo 4: o que chega aos servidores
```

`LADO` escolhe a interface do firewall: `clientes` (padrão), `servidores` ou `any`.
Enquanto a captura roda, gere tráfego em outro terminal, por exemplo:
`docker compose exec cliente-comum curl -v http://web.lab/`.

## Topologia

```
        rede_clientes 10.10.1.0/24                     rede_servidores 10.10.2.0/24
 ┌───────────────────────────┐                      ┌──────────────────────────────┐
 │ cliente-comum  10.10.1.10 │      ┌──────────┐    │ web (Nginx)     10.10.2.10   │
 │                           ├──────┤ firewall ├────┤  :80 :443 :8080 → app:8000  │
 │ cliente-admin  10.10.1.20 │ .254 │ nftables │.254│ app (Python)    10.10.2.20   │
 └───────────────────────────┘      │ +captura │    │  :8000                       │
                                    └──────────┘    └──────────────────────────────┘
```

| Contêiner | Rede / IP | Papel |
|---|---|---|
| `firewall` | 10.10.1.254 e 10.10.2.254 | Roteador entre as redes (`ip_forward=1`) e filtro nftables |
| `captura` | namespace de rede do firewall | tcpdump/tshark; grava em `./evidencias` |
| `cliente-comum` | 10.10.1.10 | Funcionário comum: só usa o site |
| `cliente-admin` | 10.10.1.20 | Administrador: também acessa a porta 8080 |
| `web` | 10.10.2.10 (`web.lab`) | Nginx: HTTP 80, HTTPS 443, admin 8080, proxy para `app:8000` |
| `app` | 10.10.2.20 | Aplicação: login, área protegida, HMAC |

Clientes e servidores trocam a rota padrão para o firewall ao iniciar (`rede/rota.sh`),
então todo pacote entre as redes passa por ele. O host Docker não tem IP nas duas
redes (`inhibit_ipv4`), então não há outro caminho; o `make test` confere isso.

## Estrutura

```
docker-compose.yml   topologia (redes, IPs, serviços)
Makefile             atalhos
rede/rota.sh         entrypoint que aponta a rota padrão para o firewall
firewall/            imagem do firewall, aberto.nft, protegido.nft, aplicar.sh
captura/             imagem tcpdump/tshark, capturar.sh, analisar_http.sh
clientes/            imagem dos clientes (curl, openssl, ping, traceroute, nc)
web/                 Nginx (Membro B)
app/                 aplicação (Membro C)
certs/               CA e certificado do laboratório (gerados pelo Membro B)
scripts/             roteiros de demonstração (m1_login_http.sh...)
tests/               testes automatizados (test_*.sh)
evidencias/          capturas .pcap e saídas dos testes
docs/relatorio/      seções do relatório
```

## Contrato entre as partes

- **Aplicação (C):** `GET /`, `POST /login` com campos `usuario` e `senha`
  (form urlencoded; sucesso = 303 + `Set-Cookie: sessao=...; HttpOnly`, falha = 401),
  `GET /area-protegida` (200 com sessão, 401 sem), `GET /saude`. Escuta em `0.0.0.0:8000`.
  Lê `CENARIO` (ativa `Secure` no protegido) e `HMAC_CHAVE`.
- **Web (B):** comando `nginx -g 'daemon off;'`, certificados em `/certs`, lê `CENARIO`.
- **Firewall (D):** `firewall/protegido.nft` usa a **mesma tabela** `inet lab` de
  `aberto.nft` (declarar, `delete table inet lab` e recriar). Não use `flush ruleset`,
  porque ele apaga as regras do DNS interno do Docker. Para iniciar o `ulogd2`, crie
  `firewall/extra.sh`; ele é chamado pelo entrypoint.
- **Testes:** cada membro cria `tests/test_<parte>.sh`, que retorna código ≠ 0 se falhar.
  O `make test` roda todos.

## Problemas comuns

- **`curl` do cliente fica sem resposta (000):** confira `make status`. O cliente precisa
  ter `default via 10.10.1.254`. Se aparecer `AVISO ... protegido.nft ainda não existe`
  nos logs do firewall, ele está usando a política aberta.
- **Windows:** os `.sh` precisam de fim de linha LF (o `.gitattributes` cuida disso).
  Rode tudo dentro do WSL2.
- **`.pcap` com dono root:** as capturas feitas pelo `make` já vêm com o seu usuário.
  Para corrigir arquivos antigos, use `sudo chown $USER evidencias/*`.
