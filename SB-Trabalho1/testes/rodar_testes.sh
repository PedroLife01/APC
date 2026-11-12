#!/bin/bash
# Roda todos os testes: compara bin.asm com o objeto de referencia, mostra
# os erros de cada arquivo de erro e liga mod1.asm + mod2.asm.
cd "$(dirname "$0")"
M=../montador; L=../ligador
echo "=== Programa correto (bin.asm) ==="
$M ok/bin.asm && diff -bq ok/bin.OBJ ok/bin_referencia.obj && echo "OK: bin.OBJ igual ao de referencia"
$M ok/completo.asm && cat ok/completo.OBJ
for f in erros/*.asm; do echo; echo "=== $f ==="; $M "$f"; done
echo; echo "=== Ligacao mod1.asm + mod2.asm ==="
$L ligacao/mod1.asm ligacao/mod2.asm && cat ligacao/mod1.LIG
echo; echo "=== Ligacao com erros ==="
$L ligacao/err_a.asm ligacao/err_b.asm
exit 0
