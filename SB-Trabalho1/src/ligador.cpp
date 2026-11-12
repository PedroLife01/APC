// =============================================================================
//  Software Basico - Trabalho Pratico 1
//  Aluno: Pedro Victor Roriz Sardenberg - Matricula: 212007369
//  Programa LIGADOR
//
//  Uso: ./ligador prog1.asm [prog2.asm] [prog3.asm]   (no maximo 3 programas)
//
//  Funcionamento:
//    1) Chama internamente o montador para cada arquivo .asm, gerando um
//       arquivo objeto (progN.OBJ) com as tabelas de definicao (D), uso (U),
//       informacao de relocacao (R) e o codigo.
//    2) Le os arquivos objeto gerados, calcula o fator de correcao de cada
//       modulo, monta a tabela global de definicoes, aplica a relocacao e
//       resolve as referencias externas.
//    3) Gera o executavel <prog1>.LIG em uma unica linha de texto.
// =============================================================================
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "montador_core.hpp"

struct Objeto {
    std::string arquivo;
    std::vector<std::pair<std::string, int>> definicoes;
    std::vector<std::pair<std::string, int>> usos;
    std::vector<int> relativos;
    std::vector<int> codigo;
};

// Le um arquivo objeto no formato gerado pelo montador (formato de ligacao)
static bool lerObjeto(const std::string &arquivo, Objeto &obj) {
    std::ifstream in(arquivo);
    if (!in) return false;
    obj.arquivo = arquivo;
    std::string linha;
    while (std::getline(in, linha)) {
        if (!linha.empty() && linha.back() == '\r') linha.pop_back();
        if (linha.empty()) continue;
        std::istringstream ss(linha.size() >= 2 && linha[1] == ',' ? linha.substr(2) : linha);
        if (linha.rfind("D,", 0) == 0) {
            std::string s; int end;
            ss >> s >> end;
            obj.definicoes.push_back({s, end});
        } else if (linha.rfind("U,", 0) == 0) {
            std::string s; int end;
            ss >> s >> end;
            obj.usos.push_back({s, end});
        } else if (linha.rfind("R,", 0) == 0) {
            int b;
            while (ss >> b) obj.relativos.push_back(b);
        } else {
            int v;
            while (ss >> v) obj.codigo.push_back(v);
        }
    }
    return true;
}

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 4) {
        std::cerr << "Uso: " << argv[0] << " prog1.asm [prog2.asm] [prog3.asm]\n";
        return 1;
    }
    std::vector<std::string> arquivos(argv + 1, argv + argc);

    // ---- 1) Montagem de cada modulo -----------------------------------------
    bool houveErro = false;
    std::vector<std::string> objetos;
    for (const std::string &asmFile : arquivos) {
        ResultadoMontagem r = montar(asmFile);
        if (!r.erros.empty()) {
            imprimirErros(asmFile, r.erros);
            houveErro = true;
            continue;
        }
        if (arquivos.size() > 1 && !r.ehModulo) {
            std::cout << asmFile << ": ERRO - para ligar mais de um arquivo, todos devem "
                         "ser modulos (BEGIN/END)\n";
            houveErro = true;
            continue;
        }
        std::string obj = trocarExtensao(asmFile, ".OBJ");
        if (!escreverObjeto(obj, r, true)) {
            std::cerr << "Erro: nao foi possivel criar '" << obj << "'\n";
            houveErro = true;
            continue;
        }
        std::cout << "Montado: " << asmFile << " -> " << obj << "\n";
        objetos.push_back(obj);
    }
    if (houveErro) {
        std::cout << "Ligacao abortada: existem erros de montagem.\n";
        return 1;
    }

    // ---- 2) Leitura dos objetos e fatores de correcao ------------------------
    std::vector<Objeto> mods(objetos.size());
    std::vector<int> fator(objetos.size());
    int total = 0;
    for (size_t i = 0; i < objetos.size(); i++) {
        if (!lerObjeto(objetos[i], mods[i])) {
            std::cerr << "Erro: nao foi possivel ler '" << objetos[i] << "'\n";
            return 1;
        }
        fator[i] = total;
        total += (int)mods[i].codigo.size();
    }

    // Tabela global de definicoes
    std::map<std::string, int> tgd;
    std::map<std::string, std::string> origem;
    for (size_t i = 0; i < mods.size(); i++) {
        for (auto &d : mods[i].definicoes) {
            if (tgd.count(d.first)) {
                std::cout << "ERRO DE LIGACAO - simbolo publico '" << d.first
                          << "' definido em mais de um modulo (" << origem[d.first]
                          << " e " << arquivos[i] << ")\n";
                houveErro = true;
                continue;
            }
            tgd[d.first] = d.second + fator[i];
            origem[d.first] = arquivos[i];
        }
    }

    // ---- 3) Relocacao e resolucao de referencias externas --------------------
    std::vector<int> executavel;
    for (size_t i = 0; i < mods.size(); i++) {
        Objeto &m = mods[i];
        std::vector<int> cod = m.codigo;
        for (size_t k = 0; k < cod.size() && k < m.relativos.size(); k++)
            if (m.relativos[k] == 1) cod[k] += fator[i];
        for (auto &u : m.usos) {
            auto it = tgd.find(u.first);
            if (it == tgd.end()) {
                std::cout << "ERRO DE LIGACAO - simbolo externo '" << u.first << "' usado em "
                          << arquivos[i] << " nao e definido (PUBLIC) em nenhum modulo\n";
                houveErro = true;
                continue;
            }
            if (u.second >= 0 && u.second < (int)cod.size()) cod[u.second] += it->second;
        }
        executavel.insert(executavel.end(), cod.begin(), cod.end());
    }
    if (houveErro) {
        std::cout << "Ligacao abortada.\n";
        return 1;
    }

    std::string saida = trocarExtensao(arquivos[0], ".LIG");
    std::ofstream out(saida);
    if (!out) {
        std::cerr << "Erro: nao foi possivel criar '" << saida << "'\n";
        return 1;
    }
    for (size_t i = 0; i < executavel.size(); i++) {
        if (i) out << " ";
        out << executavel[i];
    }
    out << "\n";
    std::cout << "Ligacao concluida. Executavel gerado: " << saida << "\n";
    return 0;
}
