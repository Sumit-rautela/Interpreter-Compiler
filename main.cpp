#include <iostream>
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "codegen.h"
#include "utils.h"

using namespace std;

static void cleanupProgram(vector<Statement*>& program) {
    for (auto stmt : program) {
        delete stmt;
    }
    program.clear();
}

int main(int argc, char* argv[]) {

    if (argc < 3) {
        cout << "Usage: myLang <file> --interpret | --compile\n";
        return 1;
    }

    string filename = argv[1];
    string mode = argv[2];

    string code = readFile(filename);

    Lexer lexer(code);
    vector<Token> tokens = lexer.tokenize();

    Parser parser(tokens);
    vector<Statement*> program = parser.parseProgram();

    if (mode == "--interpret") {
        Interpreter interpreter;
        interpreter.execute(program);
    }
    else if (mode == "--compile") {
        CodeGenerator generator;
        generator.generate(program);
        cout << "Generated output.cpp\n";
    }
    else {
        cout << "Unknown mode: " << mode << "\n";
        cleanupProgram(program);
        return 1;
    }

    cleanupProgram(program);
    return 0;
}