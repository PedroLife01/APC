#!/bin/sh
# Captura de tráfego no firewall. Roda DENTRO do contêiner "captura".
#
#   capturar.sh por <segundos> <nome> [lado] [filtro BPF...]   captura por N segundos
#   capturar.sh iniciar <nome> [lado] [filtro BPF...]          inicia em segundo plano
#   capturar.sh parar                                          encerra a captura iniciada
#
# lado: clientes (padrão) | servidores | any
#   clientes   -> interface do firewall na rede 10.10.1.0/24 (o que o cliente envia/recebe)
#   servidores -> interface do firewall na rede 10.10.2.0/24 (o que chega aos servidores)
#   any        -> todas (pacotes encaminhados aparecem duas vezes: entrada e saída)
#
# O arquivo vai para /evidencias/<nome>.pcap (= ./evidencias no host).
set -e

interface_do_lado() {
    case "$1" in
        clientes)   ip -o -4 addr show | awk '$4 ~ /^10\.10\.1\./ {print $2; exit}' ;;
        servidores) ip -o -4 addr show | awk '$4 ~ /^10\.10\.2\./ {print $2; exit}' ;;
        any)        echo any ;;
        *)          echo "lado inválido: $1" >&2; exit 2 ;;
    esac
}

ajustar_dono() {
    # Os arquivos são criados como root; devolve a posse para o usuário do host.
    [ -n "$HOST_UID" ] && chown "$HOST_UID:${HOST_GID:-$HOST_UID}" "$1" 2>/dev/null || true
}

ACAO="${1:?uso: capturar.sh por|iniciar|parar ...}"; shift

case "$ACAO" in
    por)
        SEG="${1:?informe os segundos}"; NOME="${2:?informe o nome}"; LADO="${3:-clientes}"
        if [ $# -ge 3 ]; then shift 3; else shift $#; fi
        IFACE=$(interface_do_lado "$LADO"); ARQ="/evidencias/$NOME.pcap"
        echo "[captura] $SEG s na interface $IFACE ($LADO) -> evidencias/$NOME.pcap  filtro: ${*:-nenhum}"
        # -U/--immediate-mode gravam cada pacote na hora (sem isso, os últimos se perdem
        # ao encerrar); -Z root evita que o tcpdump perca permissão de escrita
        timeout -s INT "$SEG" tcpdump -i "$IFACE" -nn -U --immediate-mode -Z root -w "$ARQ" "$@" || true
        ajustar_dono "$ARQ"
        echo "[captura] pacotes gravados: $(tcpdump -r "$ARQ" -nn 2>/dev/null | wc -l)"
        ;;
    iniciar)
        NOME="${1:?informe o nome}"; LADO="${2:-clientes}"
        if [ $# -ge 2 ]; then shift 2; else shift $#; fi
        IFACE=$(interface_do_lado "$LADO"); ARQ="/evidencias/$NOME.pcap"
        pkill -INT tcpdump 2>/dev/null && sleep 1 || true
        tcpdump -i "$IFACE" -nn -U --immediate-mode -Z root -w "$ARQ" "$@" >/tmp/tcpdump.log 2>&1 &
        echo "$ARQ" > /tmp/captura_atual
        sleep 1   # tempo para o tcpdump abrir a interface
        echo "[captura] iniciada na interface $IFACE ($LADO) -> evidencias/$NOME.pcap"
        ;;
    parar)
        pkill -INT tcpdump 2>/dev/null || { echo "[captura] nenhuma captura em andamento"; exit 0; }
        sleep 1
        ARQ=$(cat /tmp/captura_atual 2>/dev/null || true)
        [ -n "$ARQ" ] && ajustar_dono "$ARQ"
        echo "[captura] encerrada: ${ARQ#/}  ($(tcpdump -r "$ARQ" -nn 2>/dev/null | wc -l) pacotes)"
        ;;
    *)
        echo "ação inválida: $ACAO" >&2; exit 2 ;;
esac
