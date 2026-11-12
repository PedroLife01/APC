// =============================================================================
//  Software Basico - Trabalho Pratico 1
//  Programa MONTADOR
//
//  Uso: ./montador programa.asm
//  Saida: programa.OBJ (uma unica linha, em texto, com o codigo separado por
//         espacos). Se o arquivo for um modulo (BEGIN/END), o .OBJ tambem
//         contem as tabelas de definicao (D), uso (U) e relocacao (R).
// =============================================================================
#include <iostream>
#include "montador_core.hpp"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <arquivo.asm>\n";
        return 1;
    }
    std::string entrada = argv[1];
    ResultadoMontagem r = montar(entrada);

    if (!r.erros.empty()) {
        imprimirErros(entrada, r.erros);
        std::cout << "Montagem finalizada com " << r.erros.size()
                  << " erro(s). Arquivo objeto NAO gerado.\n";
        return 1;
    }

    std::string saida = trocarExtensao(entrada, ".OBJ");
    if (!escreverObjeto(saida, r, r.ehModulo)) {
        std::cerr << "Erro: nao foi possivel criar o arquivo '" << saida << "'\n";
        return 1;
    }
    std::cout << "Montagem concluida sem erros. Arquivo gerado: " << saida << "\n";
    return 0;
}
