// =============================================================================
//  Software Basico - Trabalho Pratico 1
//  Aluno: Pedro Victor Roriz Sardenberg - Matricula: 212007369
//  Montador (2 passagens) para a linguagem de montagem hipotetica vista em aula
//
//  Este cabecalho declara a interface do montador. Ela e usada tanto pelo
//  programa "montador" quanto pelo "ligador" (que chama o montador
//  internamente para gerar o objeto de cada modulo).
// =============================================================================
#ifndef MONTADOR_CORE_HPP
#define MONTADOR_CORE_HPP

#include <string>
#include <vector>
#include <map>

// Tipos de erro exigidos pela especificacao
enum class TipoErro { LEXICO, SINTATICO, SEMANTICO };

struct Erro {
    int linha;          // linha do arquivo original (0 = erro geral do arquivo)
    TipoErro tipo;
    std::string msg;
};

// Resultado da montagem de um arquivo
struct ResultadoMontagem {
    std::vector<Erro> erros;

    std::vector<int> codigo;       // codigo de maquina (texto + dados)
    std::vector<int> relativos;    // 1 = palavra relocavel, 0 = absoluta

    bool ehModulo = false;         // arquivo possui BEGIN/END
    std::string nomeModulo;

    // Tabela de definicoes (simbolos PUBLIC): simbolo -> endereco
    std::vector<std::pair<std::string, int>> tabelaDefinicoes;
    // Tabela de uso (simbolos EXTERN): simbolo -> endereco onde e usado
    std::vector<std::pair<std::string, int>> tabelaUso;
};

// Monta o arquivo .asm informado. Nao escreve nada em disco.
ResultadoMontagem montar(const std::string &arquivoAsm);

// Imprime os erros (ordenados por linha) na tela.
void imprimirErros(const std::string &arquivo, const std::vector<Erro> &erros);

// Troca a extensao do arquivo (ex.: "prog.asm" -> "prog.OBJ")
std::string trocarExtensao(const std::string &arquivo, const std::string &novaExt);

// Escreve o arquivo objeto.
//  - formatoLigacao = false: uma unica linha com o codigo separado por espacos
//  - formatoLigacao = true : tabelas D (definicoes), U (uso), R (relocacao)
//                            seguidas da linha com o codigo
bool escreverObjeto(const std::string &arquivoObj, const ResultadoMontagem &r,
                    bool formatoLigacao);

#endif
