#ifndef CYBERHAND_TUI_HPP
#define CYBERHAND_TUI_HPP

#include <string_view>
#include <string>

namespace cyberhand::ui {

// Paleta calma y densa estilo IDE (ANSI SGR)
inline constexpr std::string_view RESET     = "\033[0m";
inline constexpr std::string_view BOLD      = "\033[1m";
inline constexpr std::string_view DIM       = "\033[2m";
inline constexpr std::string_view GRAY      = "\033[90m";   // Bordes estructurales, divisores
inline constexpr std::string_view WHITE     = "\033[37m";   // Identificadores y datos
inline constexpr std::string_view CYAN      = "\033[36m";   // Opcodes, palabras clave, no terminales
inline constexpr std::string_view GREEN     = "\033[32m";   // Éxito, terminales válidos
inline constexpr std::string_view YELLOW    = "\033[33m";   // Números, métricas, porcentajes
inline constexpr std::string_view RED       = "\033[31m";   // Errores, fallas semánticas
inline constexpr std::string_view BG_GRAY   = "\033[100m";  // Insignias y chips de estado

// Caracteres de Box-Drawing ligeros Unicode
inline constexpr std::string_view H_LINE    = "─";
inline constexpr std::string_view V_LINE    = "│";
inline constexpr std::string_view TOP_L     = "┌";
inline constexpr std::string_view TOP_R     = "┐";
inline constexpr std::string_view BOT_L     = "└";
inline constexpr std::string_view BOT_R     = "┘";
inline constexpr std::string_view T_DOWN    = "┬";
inline constexpr std::string_view T_UP      = "┴";
inline constexpr std::string_view T_RIGHT   = "├";
inline constexpr std::string_view T_LEFT    = "┤";
inline constexpr std::string_view CROSS     = "┼";

// Generador de líneas horizontales
[[nodiscard]] inline std::string hRule(size_t length) {
    std::string s;
    s.reserve(length * 3);
    for (size_t i = 0; i < length; ++i) s += H_LINE;
    return s;
}

} // namespace cyberhand::ui

#endif // CYBERHAND_TUI_HPP
