# CyberHand 3D — Engineering & Architecture Guide

## Purpose & Scope
High-performance kinematic compiler implemented in standard C++20 coupled with an interactive 3D WebGL simulation environment for an 18 Degree-of-Freedom (DOF / GDL) articulated robotic hand.

## Architectural Principles
1. **Screaming Architecture**: Folder and file structures reflect the domain of compilers and biomechanics (Lexer, Parser, Semantics, Intermediate Representation, Optimization, Kinematics, Simulation), never artificial framework or academic course labels.
2. **SOLID & Modular Monolith**: Each compiler stage is an isolated module with strict boundaries, decoupled interfaces, and unit/E2E test coverage.
3. **Course-Agnostic Codebase**: The source code, comments, identifiers, CLI outputs, and UI must remain strictly professional engineering software, without mentions of "semana xx", "curso", or academic grading tags.

## Core Technical Pipeline
1. **Predictive Syntax Analysis**: LL(1) deterministic grammar verification (FIRST and FOLLOW set calculation, conflict-free parsing tables).
2. **Bottom-Up Syntax Analysis**: LR(0) canonical automaton (closure, goto transitions, shift/reduce action tables).
3. **Parser & AST Generation**: Flex lexical scanner and GNU Bison LALR parser constructing a strongly-typed Abstract Syntax Tree (AST).
4. **Semantic Safety**: Scope-managed symbol table validating actuator declarations and enforcing strict physical angle boundaries `[0°, 90°]` to prevent mechanical damage.
5. **Intermediate Code Generation (IR)**: Translation of the AST into Three-Address Code (TAC) structured as Quadruples `(operator, arg1, arg2, result)` and Triples.
6. **Basic Block Optimization**: Local constant folding, constant propagation, and dead-movement elimination emitting `trayectoria.json` for 3D visualization.
7. **18 GDL Biomechanical Model**:
   - 15 Finger Phalanxes: 5 fingers (Thumb, Index, Middle, Ring, Pinky) × 3 joints (MCP, PIP, DIP).
   - 3 Wrist Orientations: Radio-carpal axes (Pitch, Roll, Yaw).
