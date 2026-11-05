#ifndef CYBERHAND_TOKEN_HPP
#define CYBERHAND_TOKEN_HPP

#include <string>
#include <string_view>

namespace cyberhand::core {

enum class TokenType {
    // Palabras reservadas del DSL cinemático
    TOKEN_INICIO,
    TOKEN_FIN,
    TOKEN_RUTINA,
    TOKEN_DEDO,
    TOKEN_FALANGE,
    TOKEN_ROTAR,
    TOKEN_ESPERAR,
    TOKEN_MS,
    TOKEN_REPETIR,
    TOKEN_VECES,
    TOKEN_GRADOS,

    // Símbolos y delimitadores
    TOKEN_LLAVE_IZQ,    // {
    TOKEN_LLAVE_DER,    // }
    TOKEN_PAR_IZQ,      // (
    TOKEN_PAR_DER,      // )
    TOKEN_PUNTO_COMA,   // ;
    TOKEN_DOS_PUNTOS,   // :
    TOKEN_COMA,         // ,

    // Operadores aritméticos y asignación
    TOKEN_ASIGNAR,      // =
    TOKEN_MAS,          // +
    TOKEN_MENOS,        // -
    TOKEN_POR,          // *
    TOKEN_DIV,          // /

    // Literales e Identificadores
    TOKEN_NUMERO,
    TOKEN_IDENTIFICADOR,

    // Control del analizador
    TOKEN_ERROR,
    TOKEN_EOF
};

struct Token {
    TokenType   type;
    std::string lexeme;
    double      numeric_value{0.0};
    int         line{1};
    int         column{1};

    Token() = default;
    Token(TokenType t, std::string lex, int l, int c, double val = 0.0)
        : type(t), lexeme(std::move(lex)), numeric_value(val), line(l), column(c) {}
};

[[nodiscard]] constexpr std::string_view tokenTypeToString(TokenType type) noexcept {
    switch (type) {
        case TokenType::TOKEN_INICIO:        return "TOKEN_INICIO";
        case TokenType::TOKEN_FIN:           return "TOKEN_FIN";
        case TokenType::TOKEN_RUTINA:        return "TOKEN_RUTINA";
        case TokenType::TOKEN_DEDO:          return "TOKEN_DEDO";
        case TokenType::TOKEN_FALANGE:       return "TOKEN_FALANGE";
        case TokenType::TOKEN_ROTAR:         return "TOKEN_ROTAR";
        case TokenType::TOKEN_ESPERAR:       return "TOKEN_ESPERAR";
        case TokenType::TOKEN_MS:            return "TOKEN_MS";
        case TokenType::TOKEN_REPETIR:       return "TOKEN_REPETIR";
        case TokenType::TOKEN_VECES:         return "TOKEN_VECES";
        case TokenType::TOKEN_GRADOS:        return "TOKEN_GRADOS";
        case TokenType::TOKEN_LLAVE_IZQ:     return "TOKEN_LLAVE_IZQ";
        case TokenType::TOKEN_LLAVE_DER:     return "TOKEN_LLAVE_DER";
        case TokenType::TOKEN_PAR_IZQ:       return "TOKEN_PAR_IZQ";
        case TokenType::TOKEN_PAR_DER:       return "TOKEN_PAR_DER";
        case TokenType::TOKEN_PUNTO_COMA:    return "TOKEN_PUNTO_COMA";
        case TokenType::TOKEN_DOS_PUNTOS:    return "TOKEN_DOS_PUNTOS";
        case TokenType::TOKEN_COMA:          return "TOKEN_COMA";
        case TokenType::TOKEN_ASIGNAR:       return "TOKEN_ASIGNAR";
        case TokenType::TOKEN_MAS:           return "TOKEN_MAS";
        case TokenType::TOKEN_MENOS:         return "TOKEN_MENOS";
        case TokenType::TOKEN_POR:           return "TOKEN_POR";
        case TokenType::TOKEN_DIV:           return "TOKEN_DIV";
        case TokenType::TOKEN_NUMERO:        return "TOKEN_NUMERO";
        case TokenType::TOKEN_IDENTIFICADOR: return "TOKEN_IDENTIFICADOR";
        case TokenType::TOKEN_ERROR:         return "TOKEN_ERROR";
        case TokenType::TOKEN_EOF:           return "TOKEN_EOF";
    }
    return "TOKEN_DESCONOCIDO";
}

} // namespace cyberhand::core

#endif // CYBERHAND_TOKEN_HPP
