// =============================================================================
//  Software Basico - Trabalho Pratico 1
//  Aluno: Pedro Victor Roriz Sardenberg - Matricula: 212007369
//  Implementacao do montador de DUAS PASSAGENS.
//
//  Etapas:
//    0) Leitura + analise lexica de cada linha (remove comentarios, ignora
//       maiusculas/minusculas, espacos e tabulacoes) e montagem das
//       "sentencas" (rotulo + operacao + argumentos). Rotulos sozinhos numa
//       linha sao juntados com a proxima linha. Aqui tambem sao tratadas as
//       diretivas EQU (tabela de EQU) e IF (descarta a proxima linha).
//    1) Primeira passagem: atribui enderecos (secao TEXT primeiro e depois
//       a secao DATA, independentemente da ordem no arquivo) e monta a
//       tabela de simbolos.
//    2) Segunda passagem: analise sintatica/semantica dos operandos e
//       geracao do codigo objeto.
// =============================================================================
#include "montador_core.hpp"

#include <algorithm>
#include <cctype>
#include <climits>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>

namespace {

// -----------------------------------------------------------------------------
// Tabela de instrucoes (mnemonico, opcode, nro de operandos, tamanho)
// -----------------------------------------------------------------------------
struct InfoInstrucao {
    int opcode;
    int operandos;
    int tamanho;
};

const std::map<std::string, InfoInstrucao> TABELA_INSTRUCOES = {
    {"ADD",    {1, 1, 2}},
    {"SUB",    {2, 1, 2}},
    {"MULT",   {3, 1, 2}},
    {"MUL",    {3, 1, 2}},   // apelido aceito por conveniencia
    {"DIV",    {4, 1, 2}},
    {"JMP",    {5, 1, 2}},
    {"JMPN",   {6, 1, 2}},
    {"JMPP",   {7, 1, 2}},
    {"JMPZ",   {8, 1, 2}},
    {"COPY",   {9, 2, 3}},
    {"LOAD",   {10, 1, 2}},
    {"STORE",  {11, 1, 2}},
    {"INPUT",  {12, 1, 2}},
    {"OUTPUT", {13, 1, 2}},
    {"STOP",   {14, 0, 1}},
};

const std::set<std::string> DIRETIVAS = {
    "SECTION", "SECAO", "SPACE", "CONST", "EQU", "IF",
    "BEGIN", "END", "PUBLIC", "EXTERN"};

const size_t TAM_MAX_IDENT = 30;

// -----------------------------------------------------------------------------
// Analise lexica
// -----------------------------------------------------------------------------
enum class TipoToken { IDENT, NUMERO, DOIS_PONTOS, MAIS, INVALIDO };

struct Token {
    TipoToken tipo;
    std::string texto;
    long valor = 0;   // para NUMERO
};

bool ehIdentificadorValido(const std::string &s) {
    if (s.empty() || s.size() > TAM_MAX_IDENT) return false;
    if (std::isdigit((unsigned char)s[0])) return false;
    for (char c : s)
        if (!(std::isalnum((unsigned char)c) || c == '_')) return false;
    return true;
}

bool ehNumeroDecimal(const std::string &s, long &valor) {
    size_t i = 0;
    if (!s.empty() && s[0] == '-') i = 1;
    if (i >= s.size()) return false;
    for (size_t j = i; j < s.size(); j++)
        if (!std::isdigit((unsigned char)s[j])) return false;
    // limita o tamanho para nao estourar
    if (s.size() - i > 9) return false;
    valor = std::stol(s);
    return true;
}

// Divide uma linha (ja sem comentario e em maiusculas) em tokens.
// ':' e '+' sao tokens proprios. A virgula NAO faz parte da linguagem
// (COPY usa espaco): ela gera erro lexico e e tratada como separador.
std::vector<Token> tokenizar(const std::string &linha, int nroLinha,
                             std::vector<Erro> &erros) {
    std::vector<Token> tokens;
    size_t i = 0, n = linha.size();
    while (i < n) {
        char c = linha[i];
        if (std::isspace((unsigned char)c)) { i++; continue; }
        if (c == ':') { tokens.push_back({TipoToken::DOIS_PONTOS, ":"}); i++; continue; }
        if (c == '+') { tokens.push_back({TipoToken::MAIS, "+"}); i++; continue; }
        if (c == ',') {
            erros.push_back({nroLinha, TipoErro::LEXICO,
                             "caractere invalido ',' (os operandos do COPY devem ser separados por espaco)"});
            i++;
            continue;
        }
        // le uma "palavra" ate um separador
        size_t ini = i;
        while (i < n && !std::isspace((unsigned char)linha[i]) && linha[i] != ':' &&
               linha[i] != '+' && linha[i] != ',')
            i++;
        std::string palavra = linha.substr(ini, i - ini);

        Token t;
        t.texto = palavra;
        long v;
        if (ehNumeroDecimal(palavra, v)) {
            t.tipo = TipoToken::NUMERO;
            t.valor = v;
        } else if (ehIdentificadorValido(palavra)) {
            t.tipo = TipoToken::IDENT;
        } else {
            t.tipo = TipoToken::INVALIDO;
            std::string motivo;
            bool soCaracteresValidos = true;
            for (char ch : palavra)
                if (!(std::isalnum((unsigned char)ch) || ch == '_' || ch == '-'))
                    soCaracteresValidos = false;
            if (!soCaracteresValidos)
                motivo = "token '" + palavra + "' contem caractere invalido";
            else if (std::isdigit((unsigned char)palavra[0]) || palavra[0] == '-')
                motivo = "token '" + palavra + "' invalido (identificador nao pode comecar com numero / numero mal formado)";
            else if (palavra.size() > TAM_MAX_IDENT)
                motivo = "identificador '" + palavra + "' excede 30 caracteres";
            else
                motivo = "token '" + palavra + "' invalido";
            erros.push_back({nroLinha, TipoErro::LEXICO, motivo});
        }
        tokens.push_back(t);
    }
    return tokens;
}

// -----------------------------------------------------------------------------
// Estruturas internas
// -----------------------------------------------------------------------------
enum Secao { SEM_SECAO = 0, TEXT = 1, DATA = 2 };

struct Sentenca {
    int linha = 0;              // linha da operacao
    std::string rotulo;         // rotulo (vazio se nao houver)
    bool rotuloInvalido = false;
    int linhaRotulo = 0;
    Token operacao;             // mnemonico / diretiva
    std::vector<Token> args;
    Secao secao = SEM_SECAO;
    int endereco = 0;
    int tamanho = 0;
};

struct Simbolo {
    int endereco = 0;
    Secao secao = SEM_SECAO;
    bool externo = false;
    bool constante = false;
    long valorConst = 0;
    int tamanhoDados = 0;       // tamanho reservado (SPACE n / CONST = 1)
    int linha = 0;
};

struct Operando {
    std::string simbolo;
    bool simboloInvalido = false;   // token com erro lexico -> nao reportar de novo
    long deslocamento = 0;
};

std::string paraMaiusculas(std::string s) {
    for (char &c : s) c = (char)std::toupper((unsigned char)c);
    return s;
}

class Montador {
public:
    explicit Montador(const std::string &arquivo) : arquivo_(arquivo) {}

    ResultadoMontagem executar() {
        std::ifstream in(arquivo_);
        if (!in) {
            r_.erros.push_back({0, TipoErro::SEMANTICO,
                                "nao foi possivel abrir o arquivo '" + arquivo_ + "'"});
            return r_;
        }
        lerSentencas(in);
        primeiraPassagem();
        segundaPassagem();
        return r_;
    }

private:
    std::string arquivo_;
    ResultadoMontagem r_;

    std::vector<Sentenca> sentencas_;
    std::map<std::string, long> tabelaEqu_;
    std::map<std::string, int> linhaEqu_;
    std::map<std::string, Simbolo> tabelaSimbolos_;
    std::vector<std::pair<std::string, int>> publicos_;   // simbolo, linha

    bool temSecaoText_ = false, temSecaoData_ = false;
    int linhaBegin_ = 0, linhaEnd_ = 0;

    void erro(int linha, TipoErro t, const std::string &msg) {
        r_.erros.push_back({linha, t, msg});
    }

    // Converte um token em valor numerico (aceita numero ou simbolo de EQU)
    bool valorNumerico(const Token &t, long &v) {
        if (t.tipo == TipoToken::NUMERO) { v = t.valor; return true; }
        if (t.tipo == TipoToken::IDENT) {
            auto it = tabelaEqu_.find(t.texto);
            if (it != tabelaEqu_.end()) { v = it->second; return true; }
        }
        return false;
    }

    // -------------------------------------------------------------------------
    // Etapa 0: leitura, analise lexica, rotulos, EQU e IF
    // -------------------------------------------------------------------------
    void lerSentencas(std::istream &in) {
        std::string linhaTxt;
        int nro = 0;
        Secao secaoAtual = SEM_SECAO;

        // rotulo pendente (rotulo sozinho numa linha)
        std::string rotuloPendente;
        bool rotuloPendenteInvalido = false;
        int linhaRotuloPendente = 0;
        bool descartarProxima = false;   // efeito de IF falso

        while (std::getline(in, linhaTxt)) {
            nro++;
            if (!linhaTxt.empty() && linhaTxt.back() == '\r') linhaTxt.pop_back();
            // remove comentario
            size_t pc = linhaTxt.find(';');
            if (pc != std::string::npos) linhaTxt = linhaTxt.substr(0, pc);
            linhaTxt = paraMaiusculas(linhaTxt);

            // linha vazia nao conta para o IF
            bool vazia = std::all_of(linhaTxt.begin(), linhaTxt.end(),
                                     [](char c) { return std::isspace((unsigned char)c); });
            if (vazia) continue;

            if (descartarProxima) {   // IF com condicao falsa: ignora a linha
                descartarProxima = false;
                continue;
            }

            std::vector<Token> toks = tokenizar(linhaTxt, nro, r_.erros);
            if (toks.empty()) continue;   // linha so com virgulas, por ex.

            // ---- rotulos ------------------------------------------------------
            size_t pos = 0;
            std::string rotulo;
            bool rotuloInvalido = false;
            int linhaRotulo = 0;
            int qtdRotulos = 0;
            while (pos < toks.size()) {
                if (pos + 1 < toks.size() && toks[pos + 1].tipo == TipoToken::DOIS_PONTOS &&
                    (toks[pos].tipo == TipoToken::IDENT || toks[pos].tipo == TipoToken::INVALIDO)) {
                    qtdRotulos++;
                    if (qtdRotulos == 1) {
                        rotulo = toks[pos].texto;
                        rotuloInvalido = toks[pos].tipo == TipoToken::INVALIDO;
                        linhaRotulo = nro;
                    }
                    pos += 2;
                } else if (toks[pos].tipo == TipoToken::DOIS_PONTOS) {
                    erro(nro, TipoErro::SINTATICO, "':' sem rotulo valido antes");
                    pos += 1;
                } else {
                    break;
                }
            }
            if (qtdRotulos > 1)
                erro(nro, TipoErro::SINTATICO, "mais de um rotulo na mesma linha");

            if (!rotulo.empty() && !rotuloPendente.empty()) {
                erro(nro, TipoErro::SINTATICO,
                     "mais de um rotulo para a mesma linha ('" + rotuloPendente + "' e '" +
                         rotulo + "')");
            }
            if (rotulo.empty() && !rotuloPendente.empty()) {
                rotulo = rotuloPendente;
                rotuloInvalido = rotuloPendenteInvalido;
                linhaRotulo = linhaRotuloPendente;
            }
            rotuloPendente.clear();

            if (pos >= toks.size()) {
                // linha contem somente rotulo: vale para a proxima linha
                rotuloPendente = rotulo;
                rotuloPendenteInvalido = rotuloInvalido;
                linhaRotuloPendente = linhaRotulo;
                continue;
            }

            Sentenca s;
            s.linha = nro;
            s.rotulo = rotulo;
            s.rotuloInvalido = rotuloInvalido;
            s.linhaRotulo = linhaRotulo;
            s.operacao = toks[pos];
            s.args.assign(toks.begin() + pos + 1, toks.end());

            // dois-pontos perdidos no meio dos argumentos
            for (const Token &t : s.args)
                if (t.tipo == TipoToken::DOIS_PONTOS) {
                    erro(nro, TipoErro::SINTATICO, "rotulo definido em posicao invalida");
                    break;
                }

            // Forma alternativa do exemplo do professor: "BEGIN: MOD1"
            if (s.rotulo == "BEGIN" && s.operacao.tipo == TipoToken::IDENT &&
                !TABELA_INSTRUCOES.count(s.operacao.texto) && !DIRETIVAS.count(s.operacao.texto) &&
                s.args.empty()) {
                s.rotulo = s.operacao.texto;
                s.operacao.texto = "BEGIN";
            }

            if (s.operacao.tipo == TipoToken::INVALIDO) {
                // ja reportado como erro lexico; nao gera codigo
                continue;
            }
            if (s.operacao.tipo != TipoToken::IDENT) {
                erro(nro, TipoErro::SINTATICO,
                     "instrucao ou diretiva invalida: '" + s.operacao.texto + "'");
                continue;
            }

            const std::string &op = s.operacao.texto;

            // ---- SECTION --------------------------------------------------------
            if (op == "SECTION" || op == "SECAO") {
                if (!s.rotulo.empty())
                    erro(nro, TipoErro::SINTATICO, "diretiva SECTION nao pode ter rotulo");
                if (s.args.size() != 1 || s.args[0].tipo != TipoToken::IDENT) {
                    erro(nro, TipoErro::SINTATICO,
                         "SECTION deve receber exatamente um operando (TEXT ou DATA)");
                    continue;
                }
                const std::string &nome = s.args[0].texto;
                if (nome == "TEXT") {
                    if (temSecaoText_) erro(nro, TipoErro::SEMANTICO, "secao TEXT repetida");
                    temSecaoText_ = true;
                    secaoAtual = TEXT;
                } else if (nome == "DATA") {
                    if (temSecaoData_) erro(nro, TipoErro::SEMANTICO, "secao DATA repetida");
                    temSecaoData_ = true;
                    secaoAtual = DATA;
                } else {
                    erro(nro, TipoErro::SINTATICO, "secao invalida: '" + nome + "'");
                }
                continue;
            }

            // ---- EQU ------------------------------------------------------------
            if (op == "EQU") {
                if (secaoAtual != SEM_SECAO)
                    erro(nro, TipoErro::SEMANTICO,
                         "EQU deve ser declarado antes de qualquer secao");
                long v = 0;
                bool ok = s.args.size() == 1 && valorNumerico(s.args[0], v);
                if (!ok)
                    erro(nro, TipoErro::SINTATICO,
                         "EQU deve receber exatamente um operando numerico");
                if (s.rotulo.empty()) {
                    erro(nro, TipoErro::SINTATICO, "EQU sem rotulo");
                } else if (!s.rotuloInvalido) {
                    if (tabelaEqu_.count(s.rotulo))
                        erro(nro, TipoErro::SEMANTICO,
                             "rotulo repetido '" + s.rotulo + "' (ja definido na linha " +
                                 std::to_string(linhaEqu_[s.rotulo]) + ")");
                    else {
                        tabelaEqu_[s.rotulo] = v;
                        linhaEqu_[s.rotulo] = nro;
                    }
                }
                continue;
            }

            // ---- IF -------------------------------------------------------------
            if (op == "IF") {
                if (secaoAtual != TEXT)
                    erro(nro, TipoErro::SEMANTICO, "IF so pode ser usado dentro da secao TEXT");
                if (!s.rotulo.empty())
                    erro(nro, TipoErro::SINTATICO, "diretiva IF nao pode ter rotulo");
                long v = 0;
                if (s.args.size() != 1) {
                    erro(nro, TipoErro::SINTATICO, "IF deve receber exatamente um operando");
                } else if (!valorNumerico(s.args[0], v)) {
                    if (s.args[0].tipo == TipoToken::IDENT)
                        erro(nro, TipoErro::SEMANTICO,
                             "simbolo '" + s.args[0].texto + "' do IF nao foi definido por EQU");
                    else if (s.args[0].tipo != TipoToken::INVALIDO)
                        erro(nro, TipoErro::SINTATICO, "operando invalido para IF");
                } else if (v == 0) {
                    descartarProxima = true;
                }
                continue;
            }

            s.secao = secaoAtual;
            sentencas_.push_back(s);
        }

        if (!rotuloPendente.empty())
            erro(linhaRotuloPendente, TipoErro::SINTATICO,
                 "rotulo '" + rotuloPendente + "' sem instrucao/diretiva associada");

        if (!temSecaoText_)
            erro(0, TipoErro::SEMANTICO, "secao TEXT ausente");
    }

    // -------------------------------------------------------------------------
    // Primeira passagem: enderecos e tabela de simbolos
    // -------------------------------------------------------------------------
    void definirRotulo(const Sentenca &s, const Simbolo &sim) {
        if (s.rotulo.empty() || s.rotuloInvalido) return;
        if (tabelaEqu_.count(s.rotulo)) {
            erro(s.linhaRotulo, TipoErro::SEMANTICO,
                 "rotulo repetido '" + s.rotulo + "' (ja definido por EQU na linha " +
                     std::to_string(linhaEqu_[s.rotulo]) + ")");
            return;
        }
        auto it = tabelaSimbolos_.find(s.rotulo);
        if (it != tabelaSimbolos_.end()) {
            erro(s.linhaRotulo, TipoErro::SEMANTICO,
                 "rotulo repetido '" + s.rotulo + "' (ja definido na linha " +
                     std::to_string(it->second.linha) + ")");
            return;
        }
        Simbolo novo = sim;
        novo.linha = s.linhaRotulo;
        tabelaSimbolos_[s.rotulo] = novo;
    }

    void processarPrimeiraPassagem(Sentenca &s, int &contador) {
        const std::string &op = s.operacao.texto;
        s.endereco = contador;
        s.tamanho = 0;

        Simbolo sim;
        sim.endereco = contador;
        sim.secao = s.secao;

        // ---- diretivas de ligacao (BEGIN/END/PUBLIC/EXTERN) -------------------
        if (op == "BEGIN") {
            if (linhaBegin_) erro(s.linha, TipoErro::SEMANTICO, "BEGIN repetido");
            linhaBegin_ = s.linha;
            r_.ehModulo = true;
            r_.nomeModulo = s.rotulo;
            if (!s.args.empty())
                erro(s.linha, TipoErro::SINTATICO, "BEGIN nao recebe operandos");
            // o nome do modulo e um simbolo que aponta para o inicio do modulo
            // (pode ser declarado PUBLIC e usado como ponto de entrada)
            sim.endereco = 0;
            sim.secao = TEXT;
            definirRotulo(s, sim);
            return;
        }
        if (op == "END") {
            if (linhaEnd_) erro(s.linha, TipoErro::SEMANTICO, "END repetido");
            linhaEnd_ = s.linha;
            if (!s.args.empty())
                erro(s.linha, TipoErro::SINTATICO, "END nao recebe operandos");
            if (!s.rotulo.empty())
                erro(s.linha, TipoErro::SINTATICO, "END nao pode ter rotulo");
            return;
        }
        if (op == "EXTERN") {
            if (!s.args.empty())
                erro(s.linha, TipoErro::SINTATICO, "EXTERN nao recebe operandos");
            if (s.rotulo.empty()) {
                erro(s.linha, TipoErro::SINTATICO, "EXTERN sem rotulo");
                return;
            }
            sim.externo = true;
            sim.endereco = 0;
            definirRotulo(s, sim);
            return;
        }
        if (op == "PUBLIC") {
            if (!s.rotulo.empty())
                erro(s.linha, TipoErro::SINTATICO, "PUBLIC nao pode ter rotulo");
            if (s.args.size() != 1 || (s.args[0].tipo != TipoToken::IDENT &&
                                       s.args[0].tipo != TipoToken::INVALIDO)) {
                erro(s.linha, TipoErro::SINTATICO, "PUBLIC deve receber exatamente um rotulo");
                return;
            }
            if (s.args[0].tipo == TipoToken::IDENT)
                publicos_.push_back({s.args[0].texto, s.linha});
            return;
        }

        // ---- instrucoes ---------------------------------------------------------
        auto itInstr = TABELA_INSTRUCOES.find(op);
        if (itInstr != TABELA_INSTRUCOES.end()) {
            if (s.secao != TEXT)
                erro(s.linha, TipoErro::SEMANTICO,
                     "instrucao '" + op + "' fora da secao TEXT");
            s.tamanho = itInstr->second.tamanho;
            definirRotulo(s, sim);
            contador += s.tamanho;
            return;
        }

        // ---- SPACE / CONST ------------------------------------------------------
        if (op == "SPACE" || op == "CONST") {
            if (s.secao != DATA)
                erro(s.linha, TipoErro::SEMANTICO,
                     "diretiva '" + op + "' fora da secao DATA");
            if (op == "SPACE") {
                long n = 1;
                if (s.args.size() > 1) {
                    erro(s.linha, TipoErro::SINTATICO,
                         "SPACE recebe no maximo um operando");
                } else if (s.args.size() == 1) {
                    if (!valorNumerico(s.args[0], n) || n < 1) {
                        if (s.args[0].tipo != TipoToken::INVALIDO)
                            erro(s.linha, TipoErro::SINTATICO,
                                 "operando de SPACE deve ser um inteiro positivo");
                        n = 1;
                    }
                }
                s.tamanho = (int)n;
            } else {
                s.tamanho = 1;
                sim.constante = true;
                // valor e validado na segunda passagem
                long v;
                Token t = s.args.size() == 2 && s.args[0].tipo == TipoToken::MAIS
                              ? s.args[1] : (s.args.size() == 1 ? s.args[0] : Token{});
                if ((s.args.size() == 1 || s.args.size() == 2) && valorNumerico(t, v))
                    sim.valorConst = v;
            }
            sim.tamanhoDados = s.tamanho;
            definirRotulo(s, sim);
            contador += s.tamanho;
            return;
        }

        // ---- nao reconhecida ----------------------------------------------------
        erro(s.linha, TipoErro::SINTATICO, "instrucao ou diretiva invalida: '" + op + "'");
        // define o rotulo mesmo assim para evitar erros em cascata
        definirRotulo(s, sim);
    }

    void primeiraPassagem() {
        int contador = 0;
        // Secao TEXT primeiro (o objeto sempre tem o codigo antes dos dados)
        for (Secao alvo : {SEM_SECAO, TEXT, DATA}) {
            for (Sentenca &s : sentencas_) {
                if (s.secao != alvo) continue;
                processarPrimeiraPassagem(s, contador);
            }
        }

        // validacoes de modulo
        if (linhaBegin_ && !linhaEnd_)
            erro(linhaBegin_, TipoErro::SEMANTICO, "BEGIN sem END correspondente");
        if (linhaEnd_ && !linhaBegin_)
            erro(linhaEnd_, TipoErro::SEMANTICO, "END sem BEGIN correspondente");
        bool usaLigacao = !publicos_.empty();
        for (auto &kv : tabelaSimbolos_) usaLigacao |= kv.second.externo;
        if (usaLigacao && !linhaBegin_)
            erro(0, TipoErro::SEMANTICO,
                 "uso de PUBLIC/EXTERN em arquivo que nao e modulo (falta BEGIN/END)");

        std::set<std::string> jaPublico;
        for (auto &p : publicos_) {
            if (jaPublico.count(p.first)) {
                erro(p.second, TipoErro::SEMANTICO, "simbolo '" + p.first + "' declarado PUBLIC mais de uma vez");
                continue;
            }
            jaPublico.insert(p.first);
            auto it = tabelaSimbolos_.find(p.first);
            if (it == tabelaSimbolos_.end()) {
                erro(p.second, TipoErro::SEMANTICO,
                     "rotulo ausente: '" + p.first + "' declarado PUBLIC mas nao definido");
            } else if (it->second.externo) {
                erro(p.second, TipoErro::SEMANTICO,
                     "simbolo '" + p.first + "' nao pode ser EXTERN e PUBLIC ao mesmo tempo");
            } else {
                r_.tabelaDefinicoes.push_back({p.first, it->second.endereco});
            }
        }
    }

    // -------------------------------------------------------------------------
    // Segunda passagem: geracao de codigo
    // -------------------------------------------------------------------------

    // Le os operandos "ROTULO" ou "ROTULO + N". Retorna false em erro sintatico.
    bool lerOperandos(const Sentenca &s, std::vector<Operando> &ops) {
        size_t i = 0;
        const auto &a = s.args;
        while (i < a.size()) {
            Operando op;
            if (a[i].tipo == TipoToken::IDENT || a[i].tipo == TipoToken::INVALIDO) {
                op.simbolo = a[i].texto;
                op.simboloInvalido = a[i].tipo == TipoToken::INVALIDO;
                i++;
            } else if (a[i].tipo == TipoToken::NUMERO) {
                erro(s.linha, TipoErro::SINTATICO,
                     "operando invalido '" + a[i].texto + "' (era esperado um rotulo)");
                return false;
            } else {
                erro(s.linha, TipoErro::SINTATICO,
                     "operando invalido '" + a[i].texto + "'");
                return false;
            }
            if (i < a.size() && a[i].tipo == TipoToken::MAIS) {
                i++;
                long v;
                if (i < a.size() && valorNumerico(a[i], v)) {
                    if (v < 0) {
                        erro(s.linha, TipoErro::SINTATICO, "deslocamento de vetor negativo");
                        return false;
                    }
                    op.deslocamento = v;
                    i++;
                } else {
                    if (!(i < a.size() && a[i].tipo == TipoToken::INVALIDO))
                        erro(s.linha, TipoErro::SINTATICO,
                             "esperado numero apos '+' no operando");
                    return false;
                }
            }
            ops.push_back(op);
        }
        return true;
    }

    void emitir(int valor, int relativo) {
        r_.codigo.push_back(valor);
        r_.relativos.push_back(relativo);
    }

    // Resolve um operando e faz as verificacoes semanticas
    void emitirOperando(const Sentenca &s, const Operando &op, int indiceOperando) {
        const std::string &mn = s.operacao.texto;
        int opcode = TABELA_INSTRUCOES.at(mn).opcode;
        int enderecoPalavra = (int)r_.codigo.size();

        if (op.simboloInvalido) { emitir(0, 0); return; }

        if (tabelaEqu_.count(op.simbolo)) {
            erro(s.linha, TipoErro::SEMANTICO,
                 "simbolo '" + op.simbolo + "' definido por EQU nao e um endereco de memoria");
            emitir(0, 0);
            return;
        }
        auto it = tabelaSimbolos_.find(op.simbolo);
        if (it == tabelaSimbolos_.end()) {
            erro(s.linha, TipoErro::SEMANTICO, "rotulo ausente: '" + op.simbolo + "' nao declarado");
            emitir(0, 0);
            return;
        }
        const Simbolo &sim = it->second;
        if (sim.externo) {
            r_.tabelaUso.push_back({op.simbolo, enderecoPalavra});
            emitir((int)op.deslocamento, 0);
            return;
        }

        bool ehPulo = opcode >= 5 && opcode <= 8;
        if (ehPulo) {
            if (sim.secao != TEXT)
                erro(s.linha, TipoErro::SEMANTICO,
                     "pulo para rotulo invalido '" + op.simbolo + "' (nao pertence a secao TEXT)");
        } else {
            if (sim.secao != DATA)
                erro(s.linha, TipoErro::SEMANTICO,
                     "operando '" + op.simbolo + "' de " + mn + " deve ser um rotulo da secao DATA");
            else {
                if (op.deslocamento >= sim.tamanhoDados)
                    erro(s.linha, TipoErro::SEMANTICO,
                         "acesso fora dos limites de '" + op.simbolo + "' (tamanho " +
                             std::to_string(sim.tamanhoDados) + ", deslocamento " +
                             std::to_string(op.deslocamento) + ")");
                bool escreve = opcode == 11 || opcode == 12 || (opcode == 9 && indiceOperando == 1);
                if (escreve && sim.constante)
                    erro(s.linha, TipoErro::SEMANTICO,
                         "modificacao de valor constante '" + op.simbolo + "'");
                if (opcode == 4 && sim.constante && sim.valorConst == 0)
                    erro(s.linha, TipoErro::SEMANTICO,
                         "divisao por zero (constante '" + op.simbolo + "' vale 0)");
            }
        }
        emitir(sim.endereco + (int)op.deslocamento, 1);
    }

    void segundaPassagem() {
        for (Secao alvo : {SEM_SECAO, TEXT, DATA}) {
            for (const Sentenca &s : sentencas_) {
                if (s.secao != alvo) continue;
                const std::string &mn = s.operacao.texto;

                auto it = TABELA_INSTRUCOES.find(mn);
                if (it != TABELA_INSTRUCOES.end()) {
                    const InfoInstrucao &info = it->second;
                    std::vector<Operando> ops;
                    bool ok;
                    if (info.operandos == 0 && !s.args.empty()) {
                        // ex.: "STOP 1"
                        erro(s.linha, TipoErro::SINTATICO,
                             "quantidade de operandos errada para " + mn + " (esperado 0)");
                        ok = false;
                    } else {
                        ok = lerOperandos(s, ops);
                    }
                    if (ok && (int)ops.size() != info.operandos) {
                        erro(s.linha, TipoErro::SINTATICO,
                             "quantidade de operandos errada para " + mn + " (esperado " +
                                 std::to_string(info.operandos) + ", recebido " +
                                 std::to_string(ops.size()) + ")");
                        ok = false;
                    }
                    emitir(info.opcode, 0);
                    for (int k = 0; k < info.operandos; k++) {
                        if (ok) emitirOperando(s, ops[k], k);
                        else emitir(0, 0);
                    }
                    continue;
                }

                if (mn == "SPACE") {
                    for (int k = 0; k < s.tamanho; k++) emitir(0, 0);
                    continue;
                }
                if (mn == "CONST") {
                    long v = 0;
                    bool ok = false;
                    if (s.args.size() == 1) ok = valorNumerico(s.args[0], v);
                    else if (s.args.size() == 2 && s.args[0].tipo == TipoToken::MAIS)
                        ok = valorNumerico(s.args[1], v);
                    bool temInvalido = false;
                    for (auto &t : s.args) temInvalido |= t.tipo == TipoToken::INVALIDO;
                    if (!ok && !temInvalido) {
                        if (s.args.empty() || s.args.size() > 2)
                            erro(s.linha, TipoErro::SINTATICO,
                                 "quantidade de operandos errada para CONST (esperado 1, recebido " +
                                     std::to_string(s.args.size()) + ")");
                        else
                            erro(s.linha, TipoErro::SINTATICO,
                                 "operando de CONST deve ser um numero decimal");
                    }
                    emitir((int)v, 0);
                    continue;
                }
                // demais diretivas nao geram codigo
            }
        }
    }
};

} // namespace

// =============================================================================
// Interface publica
// =============================================================================
ResultadoMontagem montar(const std::string &arquivoAsm) {
    Montador m(arquivoAsm);
    return m.executar();
}

static const char *nomeTipo(TipoErro t) {
    switch (t) {
        case TipoErro::LEXICO: return "LEXICO";
        case TipoErro::SINTATICO: return "SINTATICO";
        default: return "SEMANTICO";
    }
}

void imprimirErros(const std::string &arquivo, const std::vector<Erro> &errosIn) {
    std::vector<Erro> erros = errosIn;
    std::stable_sort(erros.begin(), erros.end(),
                     [](const Erro &a, const Erro &b) { return a.linha < b.linha; });
    for (const Erro &e : erros) {
        std::cout << arquivo << ": ";
        if (e.linha > 0) std::cout << "Linha " << e.linha << ": ";
        else std::cout << "Arquivo: ";
        std::cout << "ERRO " << nomeTipo(e.tipo) << " - " << e.msg << "\n";
    }
}

std::string trocarExtensao(const std::string &arquivo, const std::string &novaExt) {
    size_t barra = arquivo.find_last_of("/\\");
    size_t ponto = arquivo.find_last_of('.');
    if (ponto == std::string::npos || (barra != std::string::npos && ponto < barra))
        return arquivo + novaExt;
    return arquivo.substr(0, ponto) + novaExt;
}

bool escreverObjeto(const std::string &arquivoObj, const ResultadoMontagem &r,
                    bool formatoLigacao) {
    std::ofstream out(arquivoObj);
    if (!out) return false;
    if (formatoLigacao) {
        for (auto &d : r.tabelaDefinicoes) out << "D, " << d.first << " " << d.second << "\n";
        for (auto &u : r.tabelaUso) out << "U, " << u.first << " " << u.second << "\n";
        out << "R,";
        for (int b : r.relativos) out << " " << b;
        out << "\n";
    }
    for (size_t i = 0; i < r.codigo.size(); i++) {
        if (i) out << " ";
        out << r.codigo[i];
    }
    out << "\n";
    return true;
}
