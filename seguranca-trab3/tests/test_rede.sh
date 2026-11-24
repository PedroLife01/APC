#!/bin/sh
# Testes da rede (Membro A): topologia, rotas pelo firewall e captura.
# Rodar na raiz do projeto com o laboratório no ar (make up). Saída != 0 se falhar.
DC="docker compose"
falhas=0
ok()    { echo "  [OK]    $1"; }
falha() { echo "  [FALHA] $1"; falhas=$((falhas+1)); }
checa() { desc="$1"; shift; if "$@" >/dev/null 2>&1; then ok "$desc"; else falha "$desc"; fi; }

echo "-- Contêineres"
for s in firewall captura cliente-comum cliente-admin web app; do
    checa "$s está rodando" sh -c "$DC ps --status running --services | grep -qx $s"
done

echo "-- Firewall como roteador"
checa "firewall tem ip_forward=1" sh -c "$DC exec -T firewall cat /proc/sys/net/ipv4/ip_forward | grep -qx 1"
checa "firewall tem IP nas duas redes (10.10.1.254 e 10.10.2.254)" \
    sh -c "$DC exec -T firewall ip -4 addr | grep -q 10.10.1.254 && $DC exec -T firewall ip -4 addr | grep -q 10.10.2.254"

echo "-- Rotas passam pelo firewall"
for c in cliente-comum cliente-admin; do
    checa "$c -> 10.10.2.10 via 10.10.1.254" \
        sh -c "$DC exec -T $c ip route get 10.10.2.10 | grep -q 'via 10.10.1.254'"
done
for s in web app; do
    checa "$s -> 10.10.1.10 via 10.10.2.254" \
        sh -c "$DC exec -T $s ip route get 10.10.1.10 | grep -q 'via 10.10.2.254'"
done
checa "clientes NÃO têm interface na rede dos servidores" \
    sh -c "! $DC exec -T cliente-comum ip -4 addr | grep -q 10.10.2."
# Tentativa de contornar o firewall usando o host Docker (10.10.1.1) como roteador.
# Deve falhar, porque o host não tem IP nas redes do laboratório (inhibit_ipv4).
desvio=$($DC exec -T cliente-comum sh -c 'ip route add 10.10.2.10/32 via 10.10.1.1 2>/dev/null;
    curl -s -o /dev/null -w "%{http_code}" --max-time 3 http://web.lab/;
    ip route del 10.10.2.10/32 2>/dev/null' 2>/dev/null)
if [ "$desvio" = "000" ]; then ok "não dá para contornar o firewall roteando pelo host"
else falha "desvio pelo host funcionou (HTTP $desvio)"; fi

echo "-- Serviço web alcançável a partir do cliente"
codigo=$($DC exec -T cliente-comum curl -s -o /dev/null -w '%{http_code}' --max-time 5 http://web.lab/ 2>/dev/null)
case "$codigo" in
    200|301|302|308) ok "http://web.lab/ respondeu $codigo" ;;
    *) falha "http://web.lab/ respondeu '$codigo'" ;;
esac

echo "-- Captura no namespace do firewall"
DONO="-e HOST_UID=$(id -u) -e HOST_GID=$(id -g)"
$DC exec -T $DONO captura sh /captura/capturar.sh iniciar teste-rede clientes tcp port 80 >/dev/null
$DC exec -T cliente-comum curl -s -o /dev/null --max-time 5 http://web.lab/ >/dev/null 2>&1
$DC exec -T $DONO captura sh /captura/capturar.sh parar >/dev/null
n=$($DC exec -T captura sh -c 'tcpdump -r /evidencias/teste-rede.pcap -nn 2>/dev/null | wc -l')
if [ "${n:-0}" -gt 0 ]; then ok "captura registrou $n pacotes em evidencias/teste-rede.pcap"
else falha "captura não registrou pacotes"; fi
rm -f evidencias/teste-rede.pcap 2>/dev/null

echo "-- Resultado: $falhas falha(s)"
[ "$falhas" -eq 0 ]
