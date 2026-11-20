#!/usr/bin/env bash
set -euo pipefail

# ==============================================================================
# PRUEBAS END-TO-END (E2E) - CLI COMPILADOR CINEMÁTICO (18 GDL)
# ==============================================================================

BIN="./bin/cyberhand_compiler"
GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Mantener web/trayectoria.json para la simulación del visor 3D web
# (No borrar con trap al finalizar)

TOTAL_TESTS=8
PASSED_TESTS=0

echo -e "${BLUE}=================================================================${NC}"
echo -e "${BLUE}  SUITE E2E: CLI UNIFICADO DEL COMPILADOR CYBERHAND (18 GDL)${NC}"
echo -e "${BLUE}=================================================================${NC}"

if [ ! -f "$BIN" ]; then
    echo -e "${RED}[ERROR]: Binario $BIN no encontrado. Ejecutando make all...${NC}"
    make all
fi

# ------------------------------------------------------------------------------
# Test a: --help
# ------------------------------------------------------------------------------
echo -n "a) Test --help / -h ... "
OUT_HELP=$($BIN --help)
if echo "$OUT_HELP" | grep -q "CYBERHAND COMPILER" && echo "$OUT_HELP" | grep -q "18 GDL"; then
    echo -e "${GREEN}PASSED${NC}"
    PASSED_TESTS=$((PASSED_TESTS + 1))
else
    echo -e "${RED}FAILED${NC}"
    echo "$OUT_HELP"
    exit 1
fi

# ------------------------------------------------------------------------------
# Test b: --analisis-ll1
# ------------------------------------------------------------------------------
echo -n "b) Test --analisis-ll1 (FIRST, FOLLOW, Tabla LL(1), Traza) ... "
OUT_LL1=$($BIN --analisis-ll1)
if echo "$OUT_LL1" | grep -qi "PRIMERO" && \
   echo "$OUT_LL1" | grep -qi "SIGUIENTE" && \
   echo "$OUT_LL1" | grep -qi "TABLA DE ANALISIS SINTACTICO PREDICTIVO LL(1)" && \
   echo "$OUT_LL1" | grep -qi "TRAZA DE SIMULACI"; then
    echo -e "${GREEN}PASSED${NC}"
    PASSED_TESTS=$((PASSED_TESTS + 1))
else
    echo -e "${RED}FAILED${NC}"
    echo "$OUT_LL1"
    exit 1
fi

# ------------------------------------------------------------------------------
# Test c: --analisis-lr0
# ------------------------------------------------------------------------------
echo -n "c) Test --analisis-lr0 (10 Estados, ACTION, GOTO, Shift/Reduce) ... "
OUT_LR0=$($BIN --analisis-lr0)
if echo "$OUT_LR0" | grep -qi "10 ESTADOS" && \
   echo "$OUT_LR0" | grep -qi "ACTION" && \
   echo "$OUT_LR0" | grep -qi "GOTO" && \
   echo "$OUT_LR0" | grep -qi "SHIFT 2" && \
   echo "$OUT_LR0" | grep -qi "ACCEPT"; then
    echo -e "${GREEN}PASSED${NC}"
    PASSED_TESTS=$((PASSED_TESTS + 1))
else
    echo -e "${RED}FAILED${NC}"
    echo "$OUT_LR0"
    exit 1
fi

# ------------------------------------------------------------------------------
# Test d: --tabla-simbolos examples/rutina_valida.rob
# ------------------------------------------------------------------------------
echo -n "d) Test --tabla-simbolos examples/rutina_valida.rob ... "
OUT_SYM=$($BIN --tabla-simbolos examples/rutina_valida.rob)
if echo "$OUT_SYM" | grep -q "Identifier" && \
   echo "$OUT_SYM" | grep -q "ACTUATOR" && \
   echo "$OUT_SYM" | grep -q "pulgar"; then
    echo -e "${GREEN}PASSED${NC}"
    PASSED_TESTS=$((PASSED_TESTS + 1))
else
    echo -e "${RED}FAILED${NC}"
    echo "$OUT_SYM"
    exit 1
fi

# ------------------------------------------------------------------------------
# Test e: --cuadruplas examples/rutina_valida.rob
# ------------------------------------------------------------------------------
echo -n "e) Test --cuadruplas examples/rutina_valida.rob ... "
OUT_QUAD=$($BIN --cuadruplas examples/rutina_valida.rob)
if echo "$OUT_QUAD" | grep -q "TABLA DE CUÁDRUPLAS" && \
   echo "$OUT_QUAD" | grep -q "TABLA DE TRIPLAS" && \
   echo "$OUT_QUAD" | grep -q "ROTAR"; then
    echo -e "${GREEN}PASSED${NC}"
    PASSED_TESTS=$((PASSED_TESTS + 1))
else
    echo -e "${RED}FAILED${NC}"
    echo "$OUT_QUAD"
    exit 1
fi

# ------------------------------------------------------------------------------
# Test f: --optimizar examples/rutina_valida.rob
# ------------------------------------------------------------------------------
echo -n "f) Test --optimizar examples/rutina_valida.rob ... "
OUT_OPT=$($BIN --optimizar examples/rutina_valida.rob)
if echo "$OUT_OPT" | grep -qi "REPORTE DE OPTIMIZA" && \
   echo "$OUT_OPT" | grep -q "Reduccion:"; then
    echo -e "${GREEN}PASSED${NC}"
    PASSED_TESTS=$((PASSED_TESTS + 1))
else
    echo -e "${RED}FAILED${NC}"
    echo "$OUT_OPT"
    exit 1
fi

# ------------------------------------------------------------------------------
# Test g: --compilar examples/rutina_valida.rob -o web/trayectoria.json
# ------------------------------------------------------------------------------
echo -n "g) Test --compilar examples/rutina_valida.rob -o web/trayectoria.json ... "
rm -f web/trayectoria.json
set +e
OUT_COMP=$($BIN --compilar examples/rutina_valida.rob -o web/trayectoria.json 2>&1)
EXIT_CODE_COMP=$?
set -e

if [ $EXIT_CODE_COMP -eq 0 ] && [ -f "web/trayectoria.json" ] && [ -s "web/trayectoria.json" ]; then
    if grep -q "\"programa\":" web/trayectoria.json && grep -q "\"total_gdl\": 18" web/trayectoria.json; then
        echo -e "${GREEN}PASSED (Exit 0, JSON emitido con 18 GDL)${NC}"
        PASSED_TESTS=$((PASSED_TESTS + 1))
    else
        echo -e "${RED}FAILED (Formato JSON invalido)${NC}"
        exit 1
    fi
else
    echo -e "${RED}FAILED (Codigo de salida: $EXIT_CODE_COMP)${NC}"
    echo "$OUT_COMP"
    exit 1
fi

# ------------------------------------------------------------------------------
# Test h: --compilar examples/rutina_error_semantico.rob (Debe fallar con exit code 1)
# ------------------------------------------------------------------------------
echo -n "h) Test --compilar examples/rutina_error_semantico.rob (Assert Exit Code 1) ... "
set +e
OUT_ERR=$($BIN --compilar examples/rutina_error_semantico.rob 2>&1)
EXIT_CODE_ERR=$?
set -e

if [ $EXIT_CODE_ERR -eq 1 ] && echo "$OUT_ERR" | grep -qi "Violacion biomecanica"; then
    echo -e "${GREEN}PASSED (Exit 1 y violacion biomecanica detectada)${NC}"
    PASSED_TESTS=$((PASSED_TESTS + 1))
else
    echo -e "${RED}FAILED (Esperaba exit code 1 y error biomecanico)${NC}"
    echo "Exit code observado: $EXIT_CODE_ERR"
    echo "Salida: $OUT_ERR"
    exit 1
fi

echo -e "\n${GREEN}>> RESULTADO: $PASSED_TESTS / $TOTAL_TESTS pruebas E2E pasaron exitosamente <<${NC}"
exit 0
