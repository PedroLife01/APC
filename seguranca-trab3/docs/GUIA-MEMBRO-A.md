# Guia de estudo — Membro A (rede, captura e Módulo 1)

Este guia explica tudo o que a parte A faz, do zero, para você conseguir apresentar
e responder perguntas. Leia na ordem e rode os comandos junto.

---

## 1. A ideia em uma frase

> Montamos duas redes separadas (clientes e servidores) e colocamos um contêiner no
> meio que é o **único caminho** entre elas. Esse contêiner é o firewall, e é nele que
> capturamos o tráfego. No cenário inseguro, a captura mostra a senha do usuário em
> texto claro.

---

## 2. Conceitos que você precisa dominar

### 2.1 Contêiner e namespace de rede
Um contêiner é um processo isolado. Uma das formas de isolamento é o **namespace de
rede**: cada contêiner tem as suas próprias interfaces, IPs, tabela de rotas e regras
de firewall. Para a rede, cada contêiner funciona como um computador separado. Por
isso dá para substituir as "3 VMs" do enunciado por contêineres.

### 2.2 Rede Docker do tipo *bridge*
Cada rede do `docker-compose.yml` (`rede_clientes`, `rede_servidores`) é uma **bridge**
do Linux, um switch virtual. Contêineres na mesma bridge se falam direto (camada 2).
Para ir de uma bridge para outra, alguém precisa **rotear** (camada 3).

### 2.3 Roteador = `ip_forward=1` + interface nas duas redes
Um Linux só repassa pacotes que não são para ele se `net.ipv4.ip_forward=1`. O
firewall tem uma interface em cada rede (10.10.1.254 e 10.10.2.254) e esse parâmetro
ligado (`sysctls` no compose). Com isso, ele vira um roteador.

### 2.4 Rota padrão
O cliente (10.10.1.10) só conhece a própria rede (10.10.1.0/24). Para qualquer outro
destino, ele usa a **rota padrão**. O `rede/rota.sh` faz
`ip route replace default via 10.10.1.254`, que manda tudo o que não é local para o
firewall. O servidor faz o mesmo apontando para 10.10.2.254, para que a **resposta**
também volte pelo firewall. Se só a ida passasse por ele, teríamos roteamento
assimétrico, e o firewall com estado (Módulo 4) não funcionaria.

Para ver isso funcionando:
```sh
docker compose exec cliente-comum ip route
docker compose exec cliente-comum traceroute -n web.lab   # salto 1 = 10.10.1.254
```

### 2.5 Por que `cap_add: NET_ADMIN`
Por padrão, o processo do contêiner não pode mexer nas rotas. O `NET_ADMIN` dá esse
poder para o `rota.sh` funcionar. O firewall também precisa dele para carregar as
regras do nftables.

### 2.6 Captura no namespace do firewall
`network_mode: "service:firewall"` faz o contêiner `captura` **entrar no namespace
de rede do firewall**. O tcpdump dele vê as interfaces eth0/eth1 do firewall. É o
mesmo que rodar o tcpdump no firewall, mas mantendo as ferramentas de análise
(tshark) fora da imagem do firewall. O volume `./evidencias:/evidencias` faz o `.pcap`
aparecer na sua máquina.

### 2.7 tcpdump × tshark × Wireshark
- **tcpdump**: captura e grava o `.pcap`. Também mostra pacotes de forma simples (`-A` = ASCII).
- **tshark**: o Wireshark em linha de comando. Entende HTTP, TLS etc. e extrai campos
  (`-T fields -e http.cookie`).
- **Wireshark**: a interface gráfica. Use no seu computador para abrir o `.pcap` nos slides.

### 2.8 HTTP é texto puro
Uma requisição HTTP é literalmente texto enviado pelo TCP:
```
POST /login HTTP/1.1          <- linha de requisição: método, caminho, versão
Host: web.lab                 <- cabeçalhos
User-Agent: ...
Content-Type: application/x-www-form-urlencoded
                              <- linha em branco separa cabeçalho do corpo
usuario=alice&senha=SenhaSecreta123   <- corpo (o formulário)
```
Quem estiver no caminho e ler os bytes lê tudo. Esse é o Módulo 1.

### 2.9 Cookie de sessão
Depois do login, o servidor manda `Set-Cookie: sessao=<token>`. O navegador reenvia
`Cookie: sessao=<token>` em toda requisição, e o servidor usa o token para saber que é
a alice. **Quem tem o token é a alice para o servidor.** Se o token vaza, a sessão
pode ser sequestrada (*session hijacking*). É o passo 5 do `make m1`.

---

## 3. Arquivo por arquivo (o que é seu)

| Arquivo | O que faz |
|---|---|
| `docker-compose.yml` | Define as 2 redes com IPs fixos e os 6 serviços. O bloco `x-roteado` é reaproveitado por quem precisa de rota pelo firewall |
| `rede/rota.sh` | Entrypoint de clientes e servidores: troca a rota padrão e depois executa o programa (`exec "$@"`) |
| `firewall/Dockerfile` | Ubuntu + nftables + iproute2 |
| `firewall/entrypoint.sh` | Mostra `ip_forward`, carrega a política conforme o `CENARIO` e fica rodando |
| `firewall/aplicar.sh` | Carrega `/firewall/<politica>.nft` |
| `firewall/aberto.nft` | Política aberta: aceita tudo e só conta pacotes nos dois sentidos |
| `captura/Dockerfile` | Ubuntu + tcpdump + tshark |
| `captura/capturar.sh` | `por N segundos` / `iniciar` / `parar`. Escolhe a interface pelo IP (lado clientes/servidores) |
| `captura/analisar_http.sh` | Extrai do `.pcap` cabeçalhos, formulário, cookies e o fluxo TCP |
| `clientes/Dockerfile` | Ferramentas de teste: curl, openssl, ping, traceroute, nc, dig |
| `scripts/m1_login_http.sh` | Roteiro completo do Módulo 1 |
| `tests/test_rede.sh` | 16 verificações automáticas da rede e da captura |
| `Makefile` | Atalhos `up down inseguro protegido captura test` (+ `m1`, `analisar`, `status`) |
| `web/*` e `app/*` | **Provisórios**, só para o M1 funcionar até o Gabriel e o Alex entregarem as versões deles |

### Detalhes que valem pontos na apresentação
1. **IPs fixos** (`ipv4_address`): as regras do firewall (Membro D) usam IPs, então eles
   não podem mudar a cada `up`.
2. **`extra_hosts`**: o "DNS" do laboratório. O cliente resolve `web.lab` → 10.10.2.10
   pelo `/etc/hosts`, já que o DNS do Docker só resolve nomes de contêineres da mesma rede.
3. **Sem NAT no firewall**: o servidor vê o IP real do cliente, e por isso o Membro D
   consegue diferenciar `cliente-comum` de `cliente-admin`.
4. **Não publicamos portas no host** (`ports:`): o laboratório fica isolado, e o acesso
   é sempre por dentro (`docker compose exec cliente-comum ...`).

---

## 4. Os dois problemas que encontramos (ótimo para a seção "Dificuldades")

### 4.1 Os pacotes não chegavam ao firewall
**Sintoma:** rotas corretas, mas o `curl` dava timeout e os contadores do nftables
ficavam em 0. **Causa:** desde a versão 28, o Docker cria no **host** a regra
`iptables -t raw -A PREROUTING -d 10.10.2.10 ! -i <bridge-servidores> -j DROP`
(anti-spoofing: "pacote para um contêiner só pode chegar pela bridge dele"). O pacote
do cliente para 10.10.2.10 atravessa a bridge dos **clientes** a caminho do firewall
e, como o módulo `br_netfilter` faz o iptables do host ver o tráfego das bridges, foi
descartado. **Solução:** `com.docker.network.bridge.gateway_mode_ipv4: nat-unprotected`.

### 4.2 Dava para contornar o firewall
**Teste:** no cliente, `ip route add 10.10.2.10 via 10.10.1.1` (o IP do host Docker na
bridge) e `curl http://web.lab/` funcionou **sem passar pelo firewall**. **Causa:** o
host também é um roteador entre as bridges. **Solução:**
`com.docker.network.bridge.inhibit_ipv4: "true"`: o host não recebe IP nessas
bridges, e não sobra outro caminho. Isso virou um teste automático
("não dá para contornar o firewall roteando pelo host").

### 4.3 (menor) A captura perdia pacotes
Ao parar o tcpdump logo depois do tráfego, o `.pcap` vinha vazio. O libpcap entrega os
pacotes ao programa em blocos, e os que estavam no buffer se perdiam. **Solução:**
`tcpdump -U --immediate-mode`.

---

## 5. Roteiro da sua parte na apresentação (~4 min)

1. **Topologia (1 min)**: mostre o desenho do README e diga: "duas redes, o firewall é o
   único caminho, e a captura roda dentro dele".
2. **Prova do caminho (30 s)**:
   ```sh
   docker compose exec cliente-comum traceroute -n web.lab
   ```
   "O primeiro salto é o firewall."
3. **Módulo 1 ao vivo (2 min)**:
   ```sh
   make inseguro
   make m1
   ```
   Pare no bloco 3: **"aqui está a senha da alice"**. No passo 5: "outra máquina usou o
   cookie capturado e entrou como alice sem saber a senha".
4. **Wireshark (30 s)**: abra `evidencias/m1-login-http.pcap`, filtro `http`, botão
   direito no POST → *Follow* → *HTTP Stream*. Mostre também o filtro `urlencoded-form`.
5. **Gancho para o Gabriel (M2)**: "com HTTPS, essa mesma captura só mostra o
   handshake e dados cifrados".

**Plano B:** se algo falhar ao vivo, use o `evidencias/m1-login-http-analise.txt` já
gerado e o `.pcap` no Wireshark.

---

## 6. Perguntas prováveis da professora (e respostas)

**Por que contêineres e não VMs?**
O enunciado permite os dois. Cada contêiner tem o seu namespace de rede, ou seja,
interfaces, rotas e firewall próprios. Para a rede, é equivalente a máquinas
separadas. O ganho é reprodutibilidade: um `make inseguro` sobe tudo igual em
qualquer máquina.

**Como vocês garantem que o tráfego passa pelo firewall?**
Três evidências: (1) a rota padrão dos clientes e servidores aponta para o firewall;
(2) o `traceroute` mostra o firewall como primeiro salto e os contadores do nftables
sobem; (3) o host não tem IP nas redes (`inhibit_ipv4`), então não há outro roteador.
Testamos o desvio pelo host e ele falha (teste automático).

**O que é exatamente o "atacante" no M1?**
Qualquer nó no caminho: um roteador comprometido, o dono de um Wi-Fi público, ou alguém
na mesma rede local fazendo ARP spoofing para se colocar no meio. Usamos o próprio
firewall porque ele está nessa posição. O ataque é **passivo**: só lê.

**O `HttpOnly` não protege o cookie?**
Não contra isso. O `HttpOnly` impede que **JavaScript** leia o cookie (proteção contra
XSS). Na rede o cookie passa em claro mesmo assim. Quem resolve é o TLS, mais o
atributo `Secure`, que impede o navegador de mandar o cookie por HTTP.

**O que continua visível mesmo com HTTPS?**
IPs, portas, horários, tamanho dos pacotes e o nome do site no SNI do
ClientHello. O TLS protege o conteúdo, não os metadados.

**Por que o firewall não faz NAT?**
Para que servidores e logs vejam o IP real de cada cliente. As regras do Módulo 4
diferenciam `cliente-comum` (10.10.1.10) de `cliente-admin` (10.10.1.20).

**Por que `-i any` não é o padrão na captura?**
No `any`, todo pacote encaminhado aparece duas vezes (entrando por eth0 e saindo por
eth1). Capturar só o lado dos clientes deixa a análise limpa. O `LADO=servidores`
mostra o que de fato chegou aos servidores, o que é útil para provar bloqueios.

**Qual a diferença entre tcpdump e Wireshark?**
Os dois usam a libpcap e o formato `.pcap`. O tcpdump captura e mostra o básico. O
Wireshark/tshark tem dissecadores de protocolo que entendem HTTP, TLS etc.

**O que é `ip_forward`?**
Um parâmetro do kernel. Com 0, o Linux descarta pacotes que não são para ele. Com 1,
ele os repassa conforme a tabela de rotas, ou seja, funciona como roteador. Antes de
repassar, o pacote passa pela *hook* `forward` do netfilter, que é onde o nftables do
Membro D filtra.

---

## 7. Comandos úteis para treinar

```sh
make status                                         # visão geral
docker compose logs firewall                        # o que o firewall carregou
docker compose exec firewall nft list table inet lab   # regras + contadores
docker compose exec cliente-comum curl -v http://web.lab/   # -v mostra os cabeçalhos
make captura NOME=treino SEG=15                     # e, em outro terminal, gere tráfego
make analisar NOME=treino
```
