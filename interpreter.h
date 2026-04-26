#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ast.h"
#include <iostream>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

using namespace std;

class Interpreter {
private:
    struct Value {
        variant<double, string, char> data;

        Value() : data(0.0) {}
        Value(double v) : data(v) {}
        Value(const string& v) : data(v) {}
        Value(char v) : data(v) {}
    };

    struct BreakSignal {};
    struct ContinueSignal {};

    unordered_map<string, Value> variables;
    int loopDepth = 0;

    void runtimeError(const string& message) {
        cout << "Runtime error: " << message << "\n";
        exit(1);
    }

    Value getVariableValue(const string& name) {
        if (variables.find(name) == variables.end()) {
            runtimeError("Undefined variable '" + name + "'");
        }
        return variables[name];
    }

    bool isNumeric(const Value& v) {
        return holds_alternative<double>(v.data);
    }

    double asNumber(const Value& v) {
        if (!holds_alternative<double>(v.data)) {
            runtimeError("Expected numeric value");
        }
        return get<double>(v.data);
    }

    string asString(const Value& v) {
        if (holds_alternative<string>(v.data)) {
            return get<string>(v.data);
        }
        if (holds_alternative<char>(v.data)) {
            return string(1, get<char>(v.data));
        }
        return to_string(get<double>(v.data));
    }

    bool isTruthy(const Value& v) {
        if (holds_alternative<double>(v.data)) {
            return get<double>(v.data) != 0.0;
        }
        if (holds_alternative<string>(v.data)) {
            return !get<string>(v.data).empty();
        }
        return get<char>(v.data) != '\0';
    }

    string formatValue(const Value& v) {
        if (holds_alternative<double>(v.data)) {
            double num = get<double>(v.data);
            string text = to_string(num);
            while (!text.empty() && text.back() == '0') text.pop_back();
            if (!text.empty() && text.back() == '.') text.pop_back();
            return text.empty() ? "0" : text;
        }
        if (holds_alternative<string>(v.data)) {
            return get<string>(v.data);
        }
        return string(1, get<char>(v.data));
    }

public:
    Value eval(Expr* node) {
        if (auto num = dynamic_cast<NumberExpr*>(node))
            return Value(num->value);

        if (auto str = dynamic_cast<StringExpr*>(node))
            return Value(str->value);

        if (auto ch = dynamic_cast<CharExpr*>(node))
            return Value(ch->value);

        if (auto var = dynamic_cast<VariableExpr*>(node))
            return getVariableValue(var->name);

        if (auto bin = dynamic_cast<BinaryExpr*>(node)) {
            Value left = eval(bin->left);
            Value right = eval(bin->right);

            if (bin->op == "+") {
                if (isNumeric(left) && isNumeric(right)) {
                    return Value(asNumber(left) + asNumber(right));
                }
                return Value(asString(left) + asString(right));
            }
            if (bin->op == "-") return Value(asNumber(left) - asNumber(right));
            if (bin->op == "*") return Value(asNumber(left) * asNumber(right));
            if (bin->op == "/") {
                double r = asNumber(right);
                if (r == 0.0) runtimeError("Division by zero");
                return Value(asNumber(left) / r);
            }

            if (bin->op == "==") {
                if (isNumeric(left) && isNumeric(right)) return Value(asNumber(left) == asNumber(right) ? 1.0 : 0.0);
                return Value(asString(left) == asString(right) ? 1.0 : 0.0);
            }
            if (bin->op == "!=") {
                if (isNumeric(left) && isNumeric(right)) return Value(asNumber(left) != asNumber(right) ? 1.0 : 0.0);
                return Value(asString(left) != asString(right) ? 1.0 : 0.0);
            }
            if (bin->op == "<") return Value(asNumber(left) < asNumber(right) ? 1.0 : 0.0);
            if (bin->op == ">") return Value(asNumber(left) > asNumber(right) ? 1.0 : 0.0);
            if (bin->op == "<=") return Value(asNumber(left) <= asNumber(right) ? 1.0 : 0.0);
            if (bin->op == ">=") return Value(asNumber(left) >= asNumber(right) ? 1.0 : 0.0);
        }

        runtimeError("Invalid expression");
        return Value();
    }

    void executeStatement(Statement* stmt) {
        if (auto letStmt = dynamic_cast<LetStatement*>(stmt)) {
            variables[letStmt->name] = eval(letStmt->value);
            return;
        }

        if (auto assignStmt = dynamic_cast<AssignmentStatement*>(stmt)) {
            if (variables.find(assignStmt->name) == variables.end()) {
                runtimeError("Assignment to undefined variable '" + assignStmt->name + "'");
            }
            variables[assignStmt->name] = eval(assignStmt->value);
            return;
        }

        if (auto printStmt = dynamic_cast<PrintStatement*>(stmt)) {
            cout << formatValue(eval(printStmt->value)) << endl;
            return;
        }

        if (dynamic_cast<BreakStatement*>(stmt)) {
            if (loopDepth <= 0) runtimeError("'break' used outside loop");
            throw BreakSignal();
        }

        if (dynamic_cast<ContinueStatement*>(stmt)) {
            if (loopDepth <= 0) runtimeError("'continue' used outside loop");
            throw ContinueSignal();
        }

        if (auto blockStmt = dynamic_cast<BlockStatement*>(stmt)) {
            for (auto child : blockStmt->statements) {
                executeStatement(child);
            }
            return;
        }

        if (auto ifStmt = dynamic_cast<IfStatement*>(stmt)) {
            if (isTruthy(eval(ifStmt->condition))) {
                executeStatement(ifStmt->thenBranch);
            } else if (ifStmt->elseBranch != nullptr) {
                executeStatement(ifStmt->elseBranch);
            }
            return;
        }

        if (auto whileStmt = dynamic_cast<WhileStatement*>(stmt)) {
            loopDepth++;
            while (isTruthy(eval(whileStmt->condition))) {
                try {
                    executeStatement(whileStmt->body);
                } catch (BreakSignal&) {
                    break;
                } catch (ContinueSignal&) {
                    continue;
                }
            }
            loopDepth--;
            return;
        }

        if (auto forStmt = dynamic_cast<ForStatement*>(stmt)) {
            if (forStmt->init != nullptr) {
                executeStatement(forStmt->init);
            }

            loopDepth++;
            while (forStmt->condition == nullptr || isTruthy(eval(forStmt->condition))) {
                try {
                    executeStatement(forStmt->body);
                } catch (BreakSignal&) {
                    break;
                } catch (ContinueSignal&) {
                }

                if (forStmt->update != nullptr) {
                    executeStatement(forStmt->update);
                }
            }
            loopDepth--;
            return;
        }

        runtimeError("Unknown statement type");
    }

    void execute(const vector<Statement*>& program) {
        for (auto stmt : program) {
            executeStatement(stmt);
        }
    }
};

#endif
