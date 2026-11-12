X: EQU 2
SECTION TEXT
JMP DADO          ; pulo para a secao de dados
STORE C           ; modificacao de constante
DIV ZERO          ; divisao por zero
LOAD VET + 5      ; fora dos limites
ADD L             ; operando nao e dado
L: SPACE          ; diretiva na secao errada
IF NAOEQU
Y: EQU 3          ; EQU depois de secao
STOP 1            ; operandos a mais
L5:
L6: ADD DADO      ; dois rotulos
SECTION DATA
DADO: SPACE
C: CONST 5
ZERO: CONST 0
VET: SPACE 2
1ERRADO: SPACE    ; erro lexico
NOME_GRANDE_DEMAIS_PARA_UM_ROTULO_X: SPACE
A$B: CONST 1
CONST 0x10
ADD DADO          ; instrucao na secao de dados
