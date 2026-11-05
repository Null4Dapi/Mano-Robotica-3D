// ==============================================================================
// PROYECTO: Compilador en C++ y Máquina Virtual Cinemática para Mano Robótica 3D
// ASIGNATURA: Autómatas y Compiladores - Ciclo V
// AUTORES: Equipo N° 1
// ==============================================================================

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <map>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <algorithm>

// ------------------------------------------------------------------------------
// FASE 1: ANALIZADOR LÉXICO (TOKENS Y AUTÓMATA FINITO DETERMINISTA)
// ------------------------------------------------------------------------------
enum class TokenType {
    TOKEN_MOTOR,
    TOKEN_INICIO,
    TOKEN_FIN,
    TOKEN_DEDO,
    TOKEN_FALANGE,
    TOKEN_ROTAR,
    TOKEN_ESPERAR,
    TOKEN_MS,
    TOKEN_REPETIR,
    TOKEN_VECES,
    TOKEN_LLAVE_IZQ,
    TOKEN_LLAVE_DER,
    TOKEN_PUNTO_COMA,
    TOKEN_NUMERO,
    TOKEN_IDENTIFICADOR,
    TOKEN_ERROR,
    TOKEN_EOF
};

std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::TOKEN_MOTOR:        return "TOKEN_MOTOR";
        case TokenType::TOKEN_INICIO:       return "TOKEN_INICIO";
        case TokenType::TOKEN_FIN:          return "TOKEN_FIN";
        case TokenType::TOKEN_DEDO:         return "TOKEN_DEDO";
        case TokenType::TOKEN_FALANGE:      return "TOKEN_FALANGE";
        case TokenType::TOKEN_ROTAR:        return "TOKEN_ROTAR";
        case TokenType::TOKEN_ESPERAR:      return "TOKEN_ESPERAR";
        case TokenType::TOKEN_MS:           return "TOKEN_MS";
        case TokenType::TOKEN_REPETIR:      return "TOKEN_REPETIR";
        case TokenType::TOKEN_VECES:        return "TOKEN_VECES";
        case TokenType::TOKEN_LLAVE_IZQ:    return "TOKEN_LLAVE_IZQ";
        case TokenType::TOKEN_LLAVE_DER:    return "TOKEN_LLAVE_DER";
        case TokenType::TOKEN_PUNTO_COMA:   return "TOKEN_PUNTO_COMA";
        case TokenType::TOKEN_NUMERO:       return "TOKEN_NUMERO";
        case TokenType::TOKEN_IDENTIFICADOR:return "TOKEN_IDENTIFICADOR";
        case TokenType::TOKEN_ERROR:        return "TOKEN_ERROR";
        case TokenType::TOKEN_EOF:          return "TOKEN_EOF";
    }
    return "DESCONOCIDO";
}

struct Token {
    TokenType   type;
    std::string lexeme;
    int         line;
    int         column;
};

class Lexer {
private:
    std::string src;
    size_t cursor = 0;
    int line = 1;
    int column = 1;

public:
    explicit Lexer(std::string source) : src(std::move(source)) {}

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        while (cursor < src.size()) {
            char c = src[cursor];

            // Ignorar espacios en blanco y tabulaciones
            if (std::isspace(static_cast<unsigned char>(c))) {
                if (c == '\n') {
                    line++;
                    column = 1;
                } else {
                    column++;
                }
                cursor++;
                continue;
            }

            // Ignorar comentarios de una sola línea '//'
            if (c == '/' && cursor + 1 < src.size() && src[cursor + 1] == '/') {
                while (cursor < src.size() && src[cursor] != '\n') {
                    cursor++;
                }
                continue;
            }

            // Simbolos especiales de puntuación
            if (c == ';') {
                tokens.push_back({TokenType::TOKEN_PUNTO_COMA, ";", line, column++});
                cursor++;
                continue;
            }
            if (c == '{') {
                tokens.push_back({TokenType::TOKEN_LLAVE_IZQ, "{", line, column++});
                cursor++;
                continue;
            }
            if (c == '}') {
                tokens.push_back({TokenType::TOKEN_LLAVE_DER, "}", line, column++});
                cursor++;
                continue;
            }

            // Identificadores y palabras reservadas
            if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
                tokens.push_back(lexIdentifierOrKeyword());
                continue;
            }

            // Números (enteros o flotantes, positivos o negativos)
            if (std::isdigit(static_cast<unsigned char>(c)) || (c == '-' && cursor + 1 < src.size() && std::isdigit(static_cast<unsigned char>(src[cursor + 1])))) {
                tokens.push_back(lexNumber());
                continue;
            }

            // Caracter no reconocido (Token de Error Léxico)
            tokens.push_back({TokenType::TOKEN_ERROR, std::string(1, c), line, column++});
            cursor++;
        }
        tokens.push_back({TokenType::TOKEN_EOF, "EOF", line, column});
        return tokens;
    }

private:
    Token lexIdentifierOrKeyword() {
        size_t start = cursor;
        int startCol = column;
        while (cursor < src.size() && (std::isalnum(static_cast<unsigned char>(src[cursor])) || src[cursor] == '_')) {
            cursor++;
            column++;
        }
        std::string word = src.substr(start, cursor - start);

        // Tabla de palabras clave reservadas
        TokenType type = TokenType::TOKEN_IDENTIFICADOR;
        if (word == "MOTOR")        type = TokenType::TOKEN_MOTOR;
        else if (word == "INICIO")  type = TokenType::TOKEN_INICIO;
        else if (word == "FIN")     type = TokenType::TOKEN_FIN;
        else if (word == "DEDO")    type = TokenType::TOKEN_DEDO;
        else if (word == "FALANGE") type = TokenType::TOKEN_FALANGE;
        else if (word == "ROTAR")   type = TokenType::TOKEN_ROTAR;
        else if (word == "ESPERAR") type = TokenType::TOKEN_ESPERAR;
        else if (word == "MS")      type = TokenType::TOKEN_MS;
        else if (word == "REPETIR") type = TokenType::TOKEN_REPETIR;
        else if (word == "VECES")   type = TokenType::TOKEN_VECES;

        return {type, word, line, startCol};
    }

    Token lexNumber() {
        size_t start = cursor;
        int startCol = column;
        if (src[cursor] == '-') {
            cursor++;
            column++;
        }
        while (cursor < src.size() && (std::isdigit(static_cast<unsigned char>(src[cursor])) || src[cursor] == '.')) {
            cursor++;
            column++;
        }
        std::string numStr = src.substr(start, cursor - start);
        return {TokenType::TOKEN_NUMERO, numStr, line, startCol};
    }
};

// ------------------------------------------------------------------------------
// FASE 2: ANALIZADOR SINTÁCTICO (PARSER Y ÁRBOL DE SINTAXIS ABSTRACTA - AST)
// ------------------------------------------------------------------------------
struct ASTNode {
    virtual ~ASTNode() = default;
    virtual void print(int indent = 0) const = 0;
};

struct RotateNode : public ASTNode {
    std::string finger;       // Ej: "INDICE", "PULGAR"
    std::string phalanx;      // Ej: "PROXIMAL", "DISTAL"
    double      targetAngle;  // Ángulo en grados [0.0 - 90.0]
    int         line;

    RotateNode(std::string f, std::string p, double ang, int l)
        : finger(std::move(f)), phalanx(std::move(p)), targetAngle(ang), line(l) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "-> ROTAR [" << finger << "::" << phalanx 
                  << "] a " << targetAngle << "° (Línea " << line << ")\n";
    }
};

struct WaitNode : public ASTNode {
    int milliseconds;
    int line;

    WaitNode(int ms, int l) : milliseconds(ms), line(l) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "-> ESPERAR " << milliseconds << " ms (Línea " << line << ")\n";
    }
};

struct LoopNode : public ASTNode {
    int iterations;
    std::vector<std::unique_ptr<ASTNode>> body;
    int line;

    LoopNode(int iters, int l) : iterations(iters), line(l) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "-> REPETIR " << iterations << " VECES {\n";
        for (const auto& cmd : body) {
            cmd->print(indent + 4);
        }
        std::cout << std::string(indent, ' ') << "}\n";
    }
};

struct ProgramNode : public ASTNode {
    std::string name;
    std::vector<std::unique_ptr<ASTNode>> statements;

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "PROGRAMA_CINEMATICO: " << name << "\n";
        for (const auto& stmt : statements) {
            stmt->print(indent + 2);
        }
    }
};

class Parser {
private:
    std::vector<Token> tokens;
    size_t current = 0;

public:
    explicit Parser(std::vector<Token> tok) : tokens(std::move(tok)) {}

    std::unique_ptr<ProgramNode> parse() {
        match(TokenType::TOKEN_MOTOR);
        Token idToken = match(TokenType::TOKEN_IDENTIFICADOR);
        match(TokenType::TOKEN_PUNTO_COMA);
        match(TokenType::TOKEN_INICIO);

        auto prog = std::make_unique<ProgramNode>();
        prog->name = idToken.lexeme;

        while (!check(TokenType::TOKEN_FIN) && !check(TokenType::TOKEN_EOF)) {
            prog->statements.push_back(parseStatement());
        }

        match(TokenType::TOKEN_FIN);
        return prog;
    }

private:
    std::unique_ptr<ASTNode> parseStatement() {
        if (check(TokenType::TOKEN_DEDO)) {
            match(TokenType::TOKEN_DEDO);
            Token f = match(TokenType::TOKEN_IDENTIFICADOR);
            match(TokenType::TOKEN_FALANGE);
            Token p = match(TokenType::TOKEN_IDENTIFICADOR);
            match(TokenType::TOKEN_ROTAR);
            Token ang = match(TokenType::TOKEN_NUMERO);
            match(TokenType::TOKEN_PUNTO_COMA);
            return std::make_unique<RotateNode>(f.lexeme, p.lexeme, std::stod(ang.lexeme), f.line);
        }
        if (check(TokenType::TOKEN_ESPERAR)) {
            match(TokenType::TOKEN_ESPERAR);
            Token ms = match(TokenType::TOKEN_NUMERO);
            match(TokenType::TOKEN_MS);
            match(TokenType::TOKEN_PUNTO_COMA);
            return std::make_unique<WaitNode>(std::stoi(ms.lexeme), ms.line);
        }
        if (check(TokenType::TOKEN_REPETIR)) {
            match(TokenType::TOKEN_REPETIR);
            Token iters = match(TokenType::TOKEN_NUMERO);
            match(TokenType::TOKEN_VECES);
            match(TokenType::TOKEN_LLAVE_IZQ);

            auto loop = std::make_unique<LoopNode>(std::stoi(iters.lexeme), iters.line);
            while (!check(TokenType::TOKEN_LLAVE_DER) && !check(TokenType::TOKEN_EOF)) {
                loop->body.push_back(parseStatement());
            }
            match(TokenType::TOKEN_LLAVE_DER);
            return loop;
        }

        throw std::runtime_error("Error Sintáctico [Línea " + std::to_string(peek().line) + 
                                 ", Columna " + std::to_string(peek().column) + 
                                 "]: Sentencia inesperada o token '" + peek().lexeme + "'.");
    }

    bool check(TokenType type) const {
        if (current >= tokens.size()) return false;
        return tokens[current].type == type;
    }

    Token peek() const { return tokens[current]; }

    Token match(TokenType type) {
        if (check(type)) {
            return tokens[current++];
        }
        throw std::runtime_error("Error Sintáctico [Línea " + std::to_string(peek().line) + 
                                 ", Columna " + std::to_string(peek().column) + 
                                 "]: Se esperaba " + tokenTypeToString(type) + 
                                 " pero se encontró '" + peek().lexeme + "'.");
    }
};

// ------------------------------------------------------------------------------
// FASE 3: ANALIZADOR SEMÁNTICO Y TABLA DE SÍMBOLOS (VALIDACIÓN BIOMECÁNICA [0°-90°])
// ------------------------------------------------------------------------------
struct JointSymbol {
    std::string fingerName;
    std::string phalanxName;
    double      minAngle;
    double      maxAngle;
    double      currentAngle;
};

class SymbolTable {
private:
    std::map<std::string, JointSymbol> table;

public:
    SymbolTable() {
        std::vector<std::string> fingers = {"PULGAR", "INDICE", "MEDIO", "ANULAR", "MENIQUE"};
        std::vector<std::string> phalanges = {"PROXIMAL", "MEDIAL", "DISTAL"};

        for (const auto& f : fingers) {
            for (const auto& p : phalanges) {
                std::string key = f + "_" + p;
                table[key] = {f, p, 0.0, 90.0, 0.0};
            }
        }
        // Articulación de la Muñeca (Carpo - 3 Ejes)
        table["MUNICA_ROLL"]  = {"MUNICA", "ROLL", -45.0, 45.0, 0.0};
        table["MUNICA_PITCH"] = {"MUNICA", "PITCH", -45.0, 45.0, 0.0};
        table["MUNICA_YAW"]   = {"MUNICA", "YAW", -45.0, 45.0, 0.0};
    }

    bool hasJoint(const std::string& key) const {
        return table.find(key) != table.end();
    }

    const JointSymbol& getJoint(const std::string& key) const {
        return table.at(key);
    }

    void updateJoint(const std::string& key, double angle) {
        table[key].currentAngle = angle;
    }

    const std::map<std::string, JointSymbol>& getAll() const {
        return table;
    }
};

class SemanticAnalyzer {
private:
    SymbolTable& symTable;
    std::vector<std::string> errors;

public:
    explicit SemanticAnalyzer(SymbolTable& table) : symTable(table) {}

    bool analyze(const ProgramNode* program) {
        errors.clear();
        for (const auto& stmt : program->statements) {
            checkNode(stmt.get());
        }
        return errors.empty();
    }

    const std::vector<std::string>& getErrors() const {
        return errors;
    }

private:
    void checkNode(const ASTNode* node) {
        if (auto rot = dynamic_cast<const RotateNode*>(node)) {
            std::string key = rot->finger + "_" + rot->phalanx;

            // Validación 1: Existencia articular en la Tabla de Símbolos
            if (!symTable.hasJoint(key)) {
                errors.push_back("Error Semántico [Línea " + std::to_string(rot->line) + 
                                 "]: Articulación desconocida '" + rot->finger + " " + rot->phalanx + "'.");
                return;
            }

            const auto& j = symTable.getJoint(key);

            // Validación 2: Restricción física de rango articular [0° a 90°]
            if (rot->targetAngle < j.minAngle || rot->targetAngle > j.maxAngle) {
                std::ostringstream oss;
                oss << "Error Semántico [Línea " << rot->line << "]: El ángulo " 
                    << rot->targetAngle << "° para '" << key 
                    << "' transgrede el rango biomecánico seguro [" 
                    << j.minAngle << "° a " << j.maxAngle << "°]. Riesgo de daño al servomotor.";
                errors.push_back(oss.str());
            }
        }
        else if (auto loop = dynamic_cast<const LoopNode*>(node)) {
            if (loop->iterations <= 0 || loop->iterations > 50) {
                errors.push_back("Error Semántico [Línea " + std::to_string(loop->line) + 
                                 "]: Iteraciones fuera de rango seguro [1 a 50].");
            }
            for (const auto& child : loop->body) {
                checkNode(child.get());
            }
        }
        else if (auto wait = dynamic_cast<const WaitNode*>(node)) {
            if (wait->milliseconds < 0 || wait->milliseconds > 10000) {
                errors.push_back("Error Semántico [Línea " + std::to_string(wait->line) + 
                                 "]: Tiempo de espera inválido (0 a 10000 ms).");
            }
        }
    }
};

// ------------------------------------------------------------------------------
// FASE 4: CÓDIGO INTERMEDIO (TAC - CUÁDRUPLAS DE TRES DIRECCIONES)
// ------------------------------------------------------------------------------
struct Quadruple {
    std::string op;
    std::string arg1;
    std::string arg2;
    std::string result;

    void print() const {
        std::cout << std::left << std::setw(12) << op 
                  << std::setw(20) << arg1 
                  << std::setw(16) << arg2 
                  << std::setw(14) << result << "\n";
    }
};

class QuadGenerator {
private:
    std::vector<Quadruple> quadruples;
    int tempCounter = 1;
    int labelCounter = 1;

public:
    std::vector<Quadruple> generate(const ProgramNode* program) {
        quadruples.clear();
        tempCounter = 1;
        labelCounter = 1;

        quadruples.push_back({"INICIO_PROGRAMA", program->name, "-", "-"});
        for (const auto& stmt : program->statements) {
            generateStmt(stmt.get());
        }
        quadruples.push_back({"FIN_PROGRAMA", "-", "-", "-"});
        return quadruples;
    }

    void optimize() {
        // Optimización de bloques: remoción de cuádruplas redundantes consecutivas
        if (quadruples.size() < 3) return;
        std::vector<Quadruple> opt;
        for (size_t i = 0; i < quadruples.size(); ++i) {
            if (i + 1 < quadruples.size() &&
                quadruples[i].op == "ROTAR" && quadruples[i+1].op == "ROTAR" &&
                quadruples[i].arg1 == quadruples[i+1].arg1) {
                // Se reemplaza por la última instrucción (actualización redundante)
                continue;
            }
            opt.push_back(quadruples[i]);
        }
        quadruples = std::move(opt);
    }

private:
    void generateStmt(const ASTNode* node) {
        if (auto rot = dynamic_cast<const RotateNode*>(node)) {
            std::string tVar = "T" + std::to_string(tempCounter++);
            std::string jointKey = rot->finger + "_" + rot->phalanx;
            // Cuádrupla 1: Carga inmediata
            quadruples.push_back({"ASIGNAR", std::to_string(rot->targetAngle), "-", tVar});
            // Cuádrupla 2: Despacho articular
            quadruples.push_back({"ROTAR", jointKey, tVar, "ACTUADOR"});
        }
        else if (auto wait = dynamic_cast<const WaitNode*>(node)) {
            quadruples.push_back({"ESPERAR", std::to_string(wait->milliseconds), "-", "TEMPORIZADOR"});
        }
        else if (auto loop = dynamic_cast<const LoopNode*>(node)) {
            for (int i = 0; i < loop->iterations; ++i) {
                for (const auto& child : loop->body) {
                    generateStmt(child.get());
                }
            }
        }
    }
};

// ------------------------------------------------------------------------------
// FASE 5: MÁQUINA VIRTUAL Y GENERADOR DE TELEMETRÍA CINEMÁTICA (18 GDL)
// ------------------------------------------------------------------------------
class VirtualMachine {
private:
    std::map<std::string, double> joints;
    std::map<std::string, double> memory; // Registros temporales Tn de la VM
    struct Frame {
        int timeMs;
        std::map<std::string, double> state;
    };
    std::vector<Frame> trajectory;
    int currentTimeMs = 0;

public:
    VirtualMachine() {
        std::vector<std::string> fingers = {"PULGAR", "INDICE", "MEDIO", "ANULAR", "MENIQUE"};
        std::vector<std::string> phalanges = {"PROXIMAL", "MEDIAL", "DISTAL"};

        for (const auto& f : fingers) {
            for (const auto& p : phalanges) {
                joints[f + "_" + p] = 0.0;
            }
        }
        joints["MUNICA_ROLL"]  = 0.0;
        joints["MUNICA_PITCH"] = 0.0;
        joints["MUNICA_YAW"]   = 0.0;
    }

    void execute(const std::vector<Quadruple>& quads) {
        trajectory.clear();
        memory.clear();
        currentTimeMs = 0;
        trajectory.push_back({currentTimeMs, joints});

        for (const auto& q : quads) {
            if (q.op == "ASIGNAR") {
                memory[q.result] = std::stod(q.arg1);
            }
            else if (q.op == "ROTAR") {
                double target = memory.count(q.arg2) ? memory[q.arg2] : std::stod(q.arg2);
                joints[q.arg1] = target;
                currentTimeMs += 100; // Paso temporal de 100 ms
                trajectory.push_back({currentTimeMs, joints});
            }
            else if (q.op == "ESPERAR") {
                currentTimeMs += std::stoi(q.arg1);
                trajectory.push_back({currentTimeMs, joints});
            }
        }
    }

    void exportJSON(const std::string& filename, const std::string& programName) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error al abrir archivo para exportar JSON: " << filename << "\n";
            return;
        }

        file << "{\n";
        file << "  \"programa\": \"" << programName << "\",\n";
        file << "  \"total_gdl\": 18,\n";
        file << "  \"duracion_total_ms\": " << currentTimeMs << ",\n";
        file << "  \"frames\": [\n";

        for (size_t i = 0; i < trajectory.size(); ++i) {
            file << "    {\n";
            file << "      \"tiempo_ms\": " << trajectory[i].timeMs << ",\n";
            file << "      \"articulaciones\": {\n";

            size_t j = 0;
            for (const auto& [joint, angle] : trajectory[i].state) {
                file << "        \"" << joint << "\": " << std::fixed << std::setprecision(1) << angle;
                if (++j < trajectory[i].state.size()) file << ",";
                file << "\n";
            }
            file << "      }\n";
            file << "    }";
            if (i + 1 < trajectory.size()) file << ",";
            file << "\n";
        }

        file << "  ]\n";
        file << "}\n";
        file.close();
        std::cout << "[MÁQUINA VIRTUAL]: Trayectoria cinemática exportada a '" << filename << "' con éxito.\n";
    }

    void printState() const {
        std::cout << "\n+-------------------------------------------------------------+\n";
        std::cout << "| ESTADO ARTICULAR DE LOS 18 GRADOS DE LIBERTAD (MÁQUINA VM)   |\n";
        std::cout << "+-----------------------+----------+--------------------------+\n";
        for (const auto& [name, angle] : joints) {
            std::cout << "| " << std::left << std::setw(21) << name << " | " 
                      << std::right << std::setw(6) << std::fixed << std::setprecision(1) << angle << "° | ";
            // Barra gráfica ASCII
            int bars = static_cast<int>(angle / 5.0);
            std::string barStr = std::string(bars, '#') + std::string(18 - bars, '.');
            std::cout << "[" << barStr << "] |\n";
        }
        std::cout << "+-------------------------------------------------------------+\n";
    }
};

// ------------------------------------------------------------------------------
// FUNCIÓN PRINCIPAL DE COMPILACIÓN (DRIVER DEL COMPILADOR)
// ------------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    std::cout << "\n";
    std::cout << "===================================================================\n";
    std::cout << " COMPILADOR C++ & VM CINEMÁTICA - MANO ROBÓTICA 3D (18 GDL)\n";
    std::cout << " Asignatura: Autómatas y Compiladores - Ciclo V (2026)\n";
    std::cout << "===================================================================\n\n";

    std::string sourceCode;
    std::string filename = "rutina.rob";

    if (argc > 1) {
        filename = argv[1];
        std::ifstream f(filename);
        if (!f.is_open()) {
            std::cerr << "[ERROR]: No se pudo abrir el archivo fuente: " << filename << "\n";
            return 1;
        }
        std::stringstream buffer;
        buffer << f.rdbuf();
        sourceCode = buffer.str();
    } else {
        // Código fuente de prueba integrado
        sourceCode = 
            "MOTOR ManoBionica_V1;\n"
            "INICIO\n"
            "    // Movimientos cinemáticos articulares (Rango seguro: 0° a 90°)\n"
            "    DEDO INDICE FALANGE PROXIMAL ROTAR 45;\n"
            "    DEDO INDICE FALANGE MEDIAL ROTAR 30;\n"
            "    DEDO INDICE FALANGE DISTAL ROTAR 20;\n"
            "    DEDO PULGAR FALANGE PROXIMAL ROTAR 60;\n"
            "    DEDO MEDIO FALANGE PROXIMAL ROTAR 90;\n"
            "    ESPERAR 500 MS;\n"
            "    REPETIR 2 VECES {\n"
            "        DEDO ANULAR FALANGE PROXIMAL ROTAR 40;\n"
            "        DEDO MENIQUE FALANGE PROXIMAL ROTAR 50;\n"
            "    }\n"
            "FIN\n";
    }

    std::cout << "[ARCHIVO FUENTE]: " << filename << "\n";
    std::cout << "-------------------------------------------------------------------\n";
    std::cout << sourceCode << "\n";
    std::cout << "-------------------------------------------------------------------\n\n";

    try {
        // FASE 1: LÉXICO
        std::cout << ">> [FASE 1: ANÁLISIS LÉXICO (AFD)]\n";
        Lexer lexer(sourceCode);
        std::vector<Token> tokens = lexer.tokenize();
        std::cout << "   Total de tokens generados: " << tokens.size() << "\n";
        for (size_t i = 0; i < std::min(tokens.size(), size_t(8)); ++i) {
            std::cout << "   - [" << std::left << std::setw(20) << tokenTypeToString(tokens[i].type) 
                      << "] : '" << tokens[i].lexeme << "' (Línea " << tokens[i].line << ")\n";
        }
        if (tokens.size() > 8) std::cout << "   ... (" << tokens.size() - 8 << " tokens adicionales)\n";
        std::cout << "   [LÉXICO OK]\n\n";

        // FASE 2: SINTÁCTICO
        std::cout << ">> [FASE 2: ANÁLISIS SINTÁCTICO (GLC & AST)]\n";
        Parser parser(tokens);
        auto ast = parser.parse();
        std::cout << "   Árbol de Sintaxis Abstracta generado con éxito:\n";
        ast->print(4);
        std::cout << "   [SINTÁCTICO OK]\n\n";

        // FASE 3: SEMÁNTICO Y TABLA DE SÍMBOLOS
        std::cout << ">> [FASE 3: ANÁLISIS SEMÁNTICO & TABLA DE SÍMBOLOS]\n";
        SymbolTable symTable;
        SemanticAnalyzer semantic(symTable);
        bool semanticOk = semantic.analyze(ast.get());

        if (!semanticOk) {
            std::cerr << "\n   [FALLO SEMÁNTICO]: Se detectaron los siguientes errores:\n";
            for (const auto& err : semantic.getErrors()) {
                std::cerr << "   x " << err << "\n";
            }
            std::cerr << "\n   [COMPILACIÓN ABORTADA]: El código contiene instrucciones que romperían el robot.\n";
            return 2;
        }
        std::cout << "   Regla biomecánica [0° a 90°]: VERIFICADA Y CUMPLIDA.\n";
        std::cout << "   Ámbitos y actuadores de la mano: EXISTENTES Y VÁLIDOS.\n";
        std::cout << "   [SEMÁNTICO OK]\n\n";

        // FASE 4: GENERACIÓN Y OPTIMIZACIÓN DE CUÁDRUPLAS
        std::cout << ">> [FASE 4: GENERACIÓN DE CÓDIGO INTERMEDIO (CUÁDRUPLAS TAC)]\n";
        QuadGenerator quadGen;
        auto quads = quadGen.generate(ast.get());
        std::cout << "   Cuádruplas generadas (" << quads.size() << " instrucciones):\n";
        std::cout << "   " << std::left << std::setw(12) << "OPERADOR" 
                  << std::setw(20) << "ARGUMENTO 1" 
                  << std::setw(16) << "ARGUMENTO 2" 
                  << std::setw(14) << "RESULTADO" << "\n";
        std::cout << "   --------------------------------------------------------------\n";
        for (const auto& q : quads) {
            std::cout << "   ";
            q.print();
        }
        quadGen.optimize();
        std::cout << "   [CÓDIGO INTERMEDIO OK]\n\n";

        // FASE 5: MÁQUINA VIRTUAL Y EXPORTACIÓN CINEMÁTICA
        std::cout << ">> [FASE 5: MÁQUINA VIRTUAL & CINEMÁTICA (18 GDL)]\n";
        VirtualMachine vm;
        vm.execute(quads);
        vm.printState();
        vm.exportJSON("trayectoria.json", ast->name);
        std::cout << "\n===================================================================\n";
        std::cout << " [COMPILACIÓN Y EJECUCIÓN EXITOSA AL 100%]\n";
        std::cout << "===================================================================\n";

    } catch (const std::exception& ex) {
        std::cerr << "\n" << ex.what() << "\n";
        std::cerr << "[COMPILACIÓN FALLIDA]: Se detuvo el pipeline formal.\n";
        return 1;
    }

    return 0;
}
