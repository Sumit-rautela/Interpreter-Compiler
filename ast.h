#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
using namespace std;

class Expr {
public:
    virtual ~Expr() {}
};

class NumberExpr : public Expr {
public:
    double value;
    NumberExpr(double v) : value(v) {}
};

class StringExpr : public Expr {
public:
    string value;
    StringExpr(string v) : value(v) {}
};

class CharExpr : public Expr {
public:
    char value;
    CharExpr(char v) : value(v) {}
};

class VariableExpr : public Expr {
public:
    string name;
    VariableExpr(string n) : name(n) {}
};

class BinaryExpr : public Expr {
public:
    Expr* left;
    string op;
    Expr* right;

    BinaryExpr(Expr* l, string o, Expr* r)
        : left(l), op(o), right(r) {}

    ~BinaryExpr() {
        delete left;
        delete right;
    }
};

// Statements
class Statement {
public:
    virtual ~Statement() {}
};

class LetStatement : public Statement {
public:
    string name;
    Expr* value;

    LetStatement(string n, Expr* v)
        : name(n), value(v) {}

    ~LetStatement() {
        delete value;
    }
};

class AssignmentStatement : public Statement {
public:
    string name;
    Expr* value;

    AssignmentStatement(string n, Expr* v)
        : name(n), value(v) {}

    ~AssignmentStatement() {
        delete value;
    }
};

class PrintStatement : public Statement {
public:
    Expr* value;

    PrintStatement(Expr* v)
        : value(v) {}

    ~PrintStatement() {
        delete value;
    }
};

class BlockStatement : public Statement {
public:
    vector<Statement*> statements;

    BlockStatement(vector<Statement*> stmts)
        : statements(stmts) {}

    ~BlockStatement() {
        for (auto stmt : statements) {
            delete stmt;
        }
    }
};

class IfStatement : public Statement {
public:
    Expr* condition;
    Statement* thenBranch;
    Statement* elseBranch;

    IfStatement(Expr* cond, Statement* thenStmt, Statement* elseStmt)
        : condition(cond), thenBranch(thenStmt), elseBranch(elseStmt) {}

    ~IfStatement() {
        delete condition;
        delete thenBranch;
        delete elseBranch;
    }
};

class WhileStatement : public Statement {
public:
    Expr* condition;
    Statement* body;

    WhileStatement(Expr* cond, Statement* bodyStmt)
        : condition(cond), body(bodyStmt) {}

    ~WhileStatement() {
        delete condition;
        delete body;
    }
};

class ForStatement : public Statement {
public:
    Statement* init;
    Expr* condition;
    Statement* update;
    Statement* body;

    ForStatement(Statement* initStmt, Expr* condExpr, Statement* updateStmt, Statement* bodyStmt)
        : init(initStmt), condition(condExpr), update(updateStmt), body(bodyStmt) {}

    ~ForStatement() {
        delete init;
        delete condition;
        delete update;
        delete body;
    }
};

class BreakStatement : public Statement {
public:
    BreakStatement() {}
};

class ContinueStatement : public Statement {
public:
    ContinueStatement() {}
};

#endif