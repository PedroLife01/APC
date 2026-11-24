# 3. Módulo 1 — Comunicação insegura e análise de vulnerabilidades

## 3.1 Objetivo e procedimento

Mostrar o que um observador no caminho da comunicação consegue ler quando a
aplicação usa HTTP sem criptografia. O "observador" é o próprio firewall: ele
encaminha todo o tráfego entre as redes e, por isso, ocupa a mesma posição de um
roteador comprometido, de um Wi-Fi público ou de alguém fazendo *ARP spoofing* na rede
local.

Com o laboratório no cenário inseguro, o roteiro `make m1`
(`scripts/m1_login_http.sh`) faz o seguinte:

1. Inicia o tcpdump na interface do firewall voltada para os clientes, com o filtro
   `tcp port 80`.
2. No `cliente-comum`, faz login com `curl` (`POST /login`, campos `usuario` e
   `senha`) e depois acessa `/area-protegida` com o cookie recebido.
3. Encerra a captura (`evidencias/m1-login-http.pcap`) e a analisa com o tshark.
4. A partir do `cliente-admin`, reutiliza o cookie extraído do `.pcap` para acessar a
   área protegida.

## 3.2 Resultados

A captura tem 20 pacotes em duas conexões TCP (10.10.1.10 → 10.10.2.10:80): o
*three-way handshake*, a requisição, a resposta e o encerramento de cada uma. Como o
HTTP não cifra nada, o Wireshark/tshark decodifica a aplicação inteira. A Figura 2
mostra o conteúdo do pacote 4, que é o login, como aparece no "Follow TCP Stream":

```
POST /login HTTP/1.1
Host: web.lab
User-Agent: Mozilla/5.0 (cliente-comum)
Accept: */*
Content-Length: 35
Content-Type: application/x-www-form-urlencoded

usuario=alice&senha=SenhaSecreta123
```
*Figura 2 – Requisição de login capturada (pacote 4).*

```
HTTP/1.1 303 See Other
Server: nginx/1.24.0 (Ubuntu)
Location: /area-protegida
Set-Cookie: sessao=6434ed544f0e14362071298011376625; Path=/; HttpOnly
```
*Figura 3 – Resposta do servidor com o cookie de sessão (pacote 6).*

> Na versão final, substituir as figuras por capturas de tela do Wireshark abrindo
> `evidencias/m1-login-http.pcap` (filtro `http`, botão direito → *Follow* → *HTTP Stream*).

A Tabela 4 lista o que ficou exposto.

| Informação exposta | Onde aparece | Risco |
|---|---|---|
| Usuário e senha (`alice` / `SenhaSecreta123`) | Corpo do `POST /login` | Roubo de credencial, reaproveitada em outros sistemas |
| Cookie de sessão `sessao=...` | `Set-Cookie` (resposta) e `Cookie` (requisições seguintes) | Sequestro de sessão sem saber a senha |
| URLs acessadas (`/login`, `/area-protegida`) | Linha de requisição | Mapeamento da aplicação e do comportamento do usuário |
| Software e versão do servidor (`nginx/1.24.0 (Ubuntu)`) | Cabeçalho `Server` | Busca de vulnerabilidades conhecidas da versão |
| Cliente usado (`User-Agent`) e `Host` | Cabeçalhos da requisição | *Fingerprinting* do usuário |
| Conteúdo das páginas (`Bem-vinda, alice! Dados internos.`) | Corpo da resposta | Vazamento de dados confidenciais |
| IPs, portas e horários | Cabeçalhos IP/TCP | Metadados (continuam visíveis mesmo com TLS) |

*Tabela 4 – Informações observáveis no tráfego HTTP.*

**Sequestro de sessão.** No passo 4, o `cliente-admin`, fazendo o papel do atacante,
enviou `Cookie: sessao=6434ed54...` copiado da captura e recebeu
`Bem-vinda, alice! Dados internos.`, sem nunca ter digitado a senha. O atributo
`HttpOnly` não ajuda aqui: ele só impede que o JavaScript da página leia o cookie,
mas não impede que o cookie seja lido na rede.

## 3.3 Análise

Os resultados quebram duas propriedades de segurança:

- **Confidencialidade.** Credenciais, cookie e conteúdo trafegam em texto claro. Qualquer
  nó entre o cliente e o servidor pode lê-los. Nesse caso, a captura passiva já basta:
  o atacante não precisa alterar nenhum pacote.
- **Autenticação/integridade.** Nada garante ao cliente que quem respondeu é o
  `web.lab` verdadeiro, nem que a página não foi alterada no caminho. Um atacante ativo
  poderia trocar o formulário ou injetar código.

O Módulo 2 resolve isso com TLS: a mesma captura no cenário protegido mostra apenas
o *handshake* e registros cifrados (*Application Data*). Os metadados da Tabela 4
(IPs, portas, horários e o nome `web.lab` no SNI) continuam visíveis, o que é uma
limitação do TLS que discutimos nas conclusões.
