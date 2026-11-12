# Software Básico — Trabalho Prático 1: Montador e Ligador

**Aluno:** Pedro Victor Roriz Sardenberg — **Matrícula:** 212007369

Montador de **duas passagens** e ligador, em C++, para a linguagem de montagem
hipotética vista em aula (14 instruções).

## Compilação

Requer `g++` com suporte a C++17 e `make`:

```bash
make            # gera ./montador e ./ligador
make clean      # remove os executáveis
```

Sem o make:

```bash
g++ -std=c++17 -O2 -o montador src/montador.cpp src/montador_core.cpp
g++ -std=c++17 -O2 -o ligador  src/ligador.cpp  src/montador_core.cpp
```

## Uso

### Montador

```bash
./montador myprogram.asm        # gera myprogram.OBJ
```

- A saída é um arquivo **texto** com **uma única linha**: opcodes e operandos
  separados por espaço. O código vem primeiro e os dados (`CONST`/`SPACE`)
  depois, mesmo que a `SECTION DATA` venha antes da `SECTION TEXT` no arquivo.
  O `SPACE` gera `0`.
- Se houver erros, o programa inteiro é analisado mesmo assim, e cada erro
  aparece na tela com a **linha** do arquivo original e o **tipo**
  (LÉXICO, SINTÁTICO ou SEMÂNTICO). Nesse caso o `.OBJ` **não** é gerado.
- Se o arquivo for um módulo (`BEGIN`/`END`), o `.OBJ` também traz as tabelas
  de ligação (veja abaixo).

### Ligador

```bash
./ligador prog1.asm prog2.asm prog3.asm     # no máximo 3 programas
```

1. Chama internamente o montador para cada `.asm`, gerando `progN.OBJ`:
   ```
   D, SIMBOLO endereco      <- tabela de definições (PUBLIC)
   U, SIMBOLO endereco      <- tabela de uso (EXTERN), uma linha por uso
   R, 0 1 0 1 ...           <- relocação (1 = endereço relativo)
   12 8 10 8 5 0 14 ...     <- código
   ```
2. Lê os `.OBJ`, calcula o fator de correção de cada módulo, monta a tabela
   global de definições, aplica a relocação e resolve as referências externas.
3. Gera o executável **`prog1.LIG`** (nome do primeiro arquivo) no mesmo
   formato de uma linha só.

## Instruções e diretivas

| Mnemônico | Opcode | Operandos | Tamanho |
|-----------|:------:|:---------:|:-------:|
| ADD    | 1  | 1 | 2 |
| SUB    | 2  | 1 | 2 |
| MULT   | 3  | 1 | 2 |
| DIV    | 4  | 1 | 2 |
| JMP    | 5  | 1 | 2 |
| JMPN   | 6  | 1 | 2 |
| JMPP   | 7  | 1 | 2 |
| JMPZ   | 8  | 1 | 2 |
| COPY   | 9  | 2 | 3 |
| LOAD   | 10 | 1 | 2 |
| STORE  | 11 | 1 | 2 |
| INPUT  | 12 | 1 | 2 |
| OUTPUT | 13 | 1 | 2 |
| STOP   | 14 | 0 | 1 |

Diretivas: `SECTION TEXT|DATA` (também aceita `SECAO`), `SPACE [n]`,
`CONST n`, `EQU`, `IF`, `BEGIN`, `END`, `PUBLIC`, `EXTERN`.

## O que o montador aceita

- Não diferencia maiúsculas de minúsculas.
- Ignora tabulações, espaços e linhas em branco desnecessários.
- Comentários com `;` em qualquer lugar da linha.
- `CONST` com números decimais positivos e negativos (`CONST -48`).
- `COPY A B`: operandos separados por **espaço**. A vírgula gera erro léxico.
- Vetores: `ADD X + 2` (também `X+2`), com `X: SPACE 3`.
- Rótulo sozinho numa linha vale para a linha seguinte.
- Seção DATA antes ou depois da seção TEXT.
- `EQU` (somente antes de qualquer seção): `TAM: EQU 3`. O símbolo pode ser
  usado em `IF`, `SPACE`, `CONST` e no deslocamento de vetores.
- `IF` (somente na seção TEXT): se o valor for 0, a próxima linha é ignorada.
- Identificadores com até 30 caracteres, com letras, números e `_`, sem
  começar por número.
- Módulos: `MOD: BEGIN` (ou `BEGIN: MOD`, como no exemplo do Moodle), `END`,
  `PUBLIC SIMB` e `SIMB: EXTERN`. O nome do módulo é um símbolo que aponta
  para o início do módulo e pode ser declarado `PUBLIC`.

## Erros detectados

| Erro | Tipo |
|------|------|
| Caractere inválido (ex.: `,` em `COPY A,B`, `$`), token mal formado (`1ABC`, `0x10`), identificador com mais de 30 caracteres | LÉXICO |
| Instrução ou diretiva inválida (`FOO X`) | SINTÁTICO |
| Quantidade de operandos errada (`COPY A`, `STOP 1`, `CONST`) | SINTÁTICO |
| Operando mal formado (número no lugar de rótulo, `X +` sem número), dois rótulos na mesma linha, rótulo sem instrução, `SECTION` inválida | SINTÁTICO |
| Rótulo / declaração ausente (não definido) | SEMÂNTICO |
| Rótulo / declaração repetido | SEMÂNTICO |
| Seção TEXT ausente | SEMÂNTICO |
| Instrução fora da seção TEXT, `SPACE`/`CONST` fora da seção DATA, `EQU` depois de seção, `IF` fora da TEXT | SEMÂNTICO |
| Pulo para rótulo da seção de dados; operando de dados que não está na seção DATA | SEMÂNTICO |
| Modificação de constante (`STORE`/`INPUT`/`COPY` em `CONST`), divisão por constante zero, acesso fora dos limites de vetor | SEMÂNTICO |
| `BEGIN` sem `END` (e vice-versa), `PUBLIC` de símbolo não definido, `PUBLIC`/`EXTERN` fora de módulo | SEMÂNTICO |

O ligador também acusa símbolo `EXTERN` não definido em nenhum módulo e
símbolo `PUBLIC` definido em mais de um módulo.

Exemplo de saída:

```
$ ./montador testes/erros/err_misturado.asm
testes/erros/err_misturado.asm: Linha 2: ERRO LEXICO - caractere invalido ',' (os operandos do COPY devem ser separados por espaco)
testes/erros/err_misturado.asm: Linha 3: ERRO SINTATICO - instrucao ou diretiva invalida: 'FOOBAR'
testes/erros/err_misturado.asm: Linha 5: ERRO SEMANTICO - rotulo repetido 'VAR1' (ja definido na linha 4)
testes/erros/err_misturado.asm: Linha 6: ERRO SEMANTICO - rotulo ausente: 'AUSENTE' nao declarado
Montagem finalizada com 4 erro(s). Arquivo objeto NAO gerado.
```

## Testes

```bash
make && testes/rodar_testes.sh
```

- `testes/ok/bin.asm`: exemplo do Moodle. O `bin.OBJ` gerado é **idêntico** ao
  `bin_referencia.obj` fornecido.
- `testes/ok/completo.asm`: usa EQU, IF, vetores, DATA antes de TEXT,
  minúsculas, tabulações e rótulo em linha separada.
- `testes/erros/`: arquivos de erro do Moodle e alguns extras.
- `testes/ligacao/mod1.asm` (exemplo do Moodle) + `mod2.asm`: juntos formam o
  mesmo algoritmo do `bin.asm`. O `mod1.LIG` gerado foi executado no
  simulador da disciplina e deu a mesma saída do `bin.OBJ` (entrada 10 →
  saída 0 1 0 1).

Observação: o `LIG_referencia.OBJ` do Moodle traz `D, L2 7`, mas pelo código
do próprio exemplo (`12 8 10 8 5 0 14 ...`) o `STOP` rotulado por `L2` fica no
endereço 6. O montador gera `D, L2 6`.

## Estrutura

```
src/montador_core.hpp/.cpp   núcleo do montador (léxico, 2 passagens, objeto)
src/montador.cpp             programa ./montador
src/ligador.cpp              programa ./ligador
testes/                      arquivos de teste
```
