#!/bin/sh
# Analisa um .pcap HTTP e mostra o que um atacante na rede consegue ler.
# Uso (dentro do contêiner captura): sh /captura/analisar_http.sh /evidencias/<nome>.pcap
ARQ="${1:?uso: analisar_http.sh arquivo.pcap}"

linha() { echo "----------------------------------------------------------------------"; }

linha; echo "1) Conversas TCP (quem falou com quem)"; linha
tshark -r "$ARQ" -q -z conv,tcp 2>/dev/null | sed -n '/<->/p'

linha; echo "2) Requisições HTTP: método, URI, Host, User-Agent e Cookie"; linha
tshark -r "$ARQ" -Y http.request -T fields -E header=y -E separator='|' \
    -e frame.number -e ip.src -e http.request.method -e http.request.uri \
    -e http.host -e http.user_agent -e http.cookie 2>/dev/null

linha; echo "3) Corpo do formulário (application/x-www-form-urlencoded)"; linha
tshark -r "$ARQ" -Y urlencoded-form -V -O urlencoded-form 2>/dev/null | grep 'Form item' | sed 's/^ *//'

linha; echo "4) Respostas HTTP: código, Server e Set-Cookie (cookie de sessão)"; linha
tshark -r "$ARQ" -Y http.response -T fields -E header=y -E separator='|' \
    -e frame.number -e http.response.code -e http.server -e http.set_cookie 2>/dev/null

linha; echo "5) Bytes crus do pacote com a senha (tcpdump -A)"; linha
tcpdump -r "$ARQ" -nn -A 2>/dev/null | grep -B8 'senha=' | head -30

linha; echo "6) Fluxo TCP completo da 1a conexão, como no 'Follow TCP Stream' do Wireshark"; linha
tshark -r "$ARQ" -q -z follow,tcp,ascii,0 2>/dev/null | head -60
