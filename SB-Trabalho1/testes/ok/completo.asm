; Testa: EQU, IF, DATA antes de TEXT, vetor (X + 2), rotulo em linha
; separada, minusculas, tabulacoes, CONST negativo e comentarios
TRUE:  EQU 1   ; constantes simbolicas
FALSE: EQU 0
TAM:   equ 3

section data
	vetor:	space TAM      ; vetor de 3 posicoes
menos:  const   -48
UM: CONST 1
    resultado:   SPACE

SECTION TEXT
inicio:
	INPUT vetor
	input   vetor + 1 ; comentario no meio
INPUT vetor+2
LOAD vetor
ADD VETOR + 1
add vetor + 2
IF TRUE
ADD menos         ; incluida  (soma -48)
IF FALSE
ADD UM            ; descartada
STORE resultado
OUTPUT resultado
COPY resultado vetor
	STOP
