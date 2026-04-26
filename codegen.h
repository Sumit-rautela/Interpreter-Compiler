#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include <fstream>
using namespace std;

class CodeGenerator {
public:
    string indent(int level) {
        return string(level * 4, ' ');
    }

    string escapeForCppString(const string& value) {
        string out;
        for (char c : value) {
            if (c == '\\') out += "\\\\";
            else if (c == '"') out += "\\\"";
            else if (c == '\n') out += "\\n";
            else if (c == '\t') out += "\\t";
            else out += c;
        }
        return out;
    }

    string escapeForCppChar(char c) {
        if (c == '\\') return "\\\\";
        if (c == '\'') return "\\'";
        if (c == '\n') return "\\n";
        if (c == '\t') return "\\t";
        return string(1, c);
    }

    string generateExpr(Expr* node) {
        if (auto num = dynamic_cast<NumberExpr*>(node))
            return to_string(num->value);

        if (auto str = dynamic_cast<StringExpr*>(node))
            return string("\"") + escapeForCppString(str->value) + "\"";

        if (auto ch = dynamic_cast<CharExpr*>(node))
            return string("'") + escapeForCppChar(ch->value) + "'";

        if (auto var = dynamic_cast<VariableExpr*>(node))
            return var->name;

        if (auto bin = dynamic_cast<BinaryExpr*>(node)) {
            return generateExpr(bin->left) + " " + bin->op + " " + generateExpr(bin->right);
        }

        return "";
    }

    string generateInlineStatement(Statement* stmt) {
        if (auto letStmt = dynamic_cast<LetStatement*>(stmt)) {
            return "auto " + letStmt->name + " = " + generateExpr(letStmt->value);
        }

        if (auto assignStmt = dynamic_cast<AssignmentStatement*>(stmt)) {
            return assignStmt->name + " = " + generateExpr(assignStmt->value);
        }

        return "";
    }

    void generateStatement(Statement* stmt, ofstream& out, int level) {
        if (auto letStmt = dynamic_cast<LetStatement*>(stmt)) {
            out << indent(level) << "auto " << letStmt->name << " = "
                << generateExpr(letStmt->value) << ";\n";
            return;
        }

        if (auto assignStmt = dynamic_cast<AssignmentStatement*>(stmt)) {
            out << indent(level) << assignStmt->name << " = "
                << generateExpr(assignStmt->value) << ";\n";
            return;
        }

        if (auto printStmt = dynamic_cast<PrintStatement*>(stmt)) {
            out << indent(level) << "cout << "
                << generateExpr(printStmt->value)
                << " << endl;\n";
            return;
        }

        if (dynamic_cast<BreakStatement*>(stmt)) {
            out << indent(level) << "break;\n";
            return;
        }

        if (dynamic_cast<ContinueStatement*>(stmt)) {
            out << indent(level) << "continue;\n";
            return;
        }

        if (auto blockStmt = dynamic_cast<BlockStatement*>(stmt)) {
            out << indent(level) << "{\n";
            for (auto child : blockStmt->statements) {
                generateStatement(child, out, level + 1);
            }
            out << indent(level) << "}\n";
            return;
        }

        if (auto ifStmt = dynamic_cast<IfStatement*>(stmt)) {
            out << indent(level) << "if (" << generateExpr(ifStmt->condition) << ") ";
            if (dynamic_cast<BlockStatement*>(ifStmt->thenBranch)) {
                generateStatement(ifStmt->thenBranch, out, level);
            } else {
                out << "{\n";
                generateStatement(ifStmt->thenBranch, out, level + 1);
                out << indent(level) << "}\n";
            }

            if (ifStmt->elseBranch != nullptr) {
                out << indent(level) << "else ";
                if (dynamic_cast<BlockStatement*>(ifStmt->elseBranch)) {
                    generateStatement(ifStmt->elseBranch, out, level);
                } else {
                    out << "{\n";
                    generateStatement(ifStmt->elseBranch, out, level + 1);
                    out << indent(level) << "}\n";
                }
            }
            return;
        }

        if (auto whileStmt = dynamic_cast<WhileStatement*>(stmt)) {
            out << indent(level) << "while (" << generateExpr(whileStmt->condition) << ") ";
            if (dynamic_cast<BlockStatement*>(whileStmt->body)) {
                generateStatement(whileStmt->body, out, level);
            } else {
                out << "{\n";
                generateStatement(whileStmt->body, out, level + 1);
                out << indent(level) << "}\n";
            }
            return;
        }

        if (auto forStmt = dynamic_cast<ForStatement*>(stmt)) {
            out << indent(level) << "for (";
            if (forStmt->init != nullptr) {
                out << generateInlineStatement(forStmt->init);
            }
            out << "; ";
            if (forStmt->condition != nullptr) {
                out << generateExpr(forStmt->condition);
            }
            out << "; ";
            if (forStmt->update != nullptr) {
                out << generateInlineStatement(forStmt->update);
            }
            out << ") ";

            if (dynamic_cast<BlockStatement*>(forStmt->body)) {
                generateStatement(forStmt->body, out, level);
            } else {
                out << "{\n";
                generateStatement(forStmt->body, out, level + 1);
                out << indent(level) << "}\n";
            }
            return;
        }
    }

    void generate(vector<Statement*> program) {
        ofstream out("output.cpp");

        out << "#include <iostream>\nusing namespace std;\n\n";
        out << "int main() {\n";

        for (auto stmt : program) {
            generateStatement(stmt, out, 1);
        }

        out << "    return 0;\n}";
        out.close();
    }
};

#endif