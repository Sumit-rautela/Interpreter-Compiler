#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"
#include <vector>
#include <cstddef>

class Parser {
private:
    vector<Token> tokens;
    size_t pos;

    void parseError(const string& message) {
        cout << message << "\n";
        exit(1);
    }

    bool isComparisonOperator(TokenType type) {
        return type == EQEQ || type == NEQ || type == LT || type == GT || type == LE || type == GE;
    }

    void requireToken(TokenType type, const string& message) {
        if (currentToken().type != type) {
            parseError(message);
        }
    }

public:
    Parser(vector<Token> tokens) {
        this->tokens = tokens;
        pos = 0;
    }

    Token currentToken() { return tokens[pos]; }
    void advance() { if (pos < tokens.size()) pos++; }

    Expr* parseFactor() {
        Token token = currentToken();

        if (token.type == NUMBER) {
            advance();
            return new NumberExpr(stod(token.value));
        }

        if (token.type == STRING) {
            advance();
            return new StringExpr(token.value);
        }

        if (token.type == CHAR) {
            advance();
            return new CharExpr(token.value.empty() ? '\0' : token.value[0]);
        }

        if (token.type == MINUS) {
            advance();
            return new BinaryExpr(new NumberExpr(0), "-", parseFactor());
        }

        if (token.type == IDENTIFIER) {
            advance();
            return new VariableExpr(token.value);
        }

        if (token.type == LPAREN) {
            advance();
            Expr* node = parseExpression();

            requireToken(RPAREN, "Expected ')'");
            advance();
            return node;
        }

        parseError("Invalid factor");
        return nullptr;
    }

    Expr* parseTerm() {
        Expr* node = parseFactor();

        while (currentToken().type == MUL || currentToken().type == DIV) {
            string op = currentToken().value;
            advance();
            node = new BinaryExpr(node, op, parseFactor());
        }

        return node;
    }

    Expr* parseArithmetic() {
        Expr* node = parseTerm();

        while (currentToken().type == PLUS || currentToken().type == MINUS) {
            string op = currentToken().value;
            advance();
            node = new BinaryExpr(node, op, parseTerm());
        }

        return node;
    }

    Expr* parseExpression() {
        Expr* node = parseArithmetic();

        while (isComparisonOperator(currentToken().type)) {
            string op = currentToken().value;
            advance();
            node = new BinaryExpr(node, op, parseArithmetic());
        }

        return node;
    }

    Statement* parseLetStatement(bool expectSemicolon = true) {
        advance();

        requireToken(IDENTIFIER, "Expected identifier after 'let'");
        string name = currentToken().value;
        advance();

        requireToken(EQUAL, "Expected '='");
        advance();

        Expr* value = parseExpression();

        if (expectSemicolon) {
            requireToken(SEMICOLON, "Expected ';'");
            advance();
        }

        return new LetStatement(name, value);
    }

    Statement* parseAssignmentStatement(bool expectSemicolon = true) {
        string name = currentToken().value;
        advance();

        requireToken(EQUAL, "Expected '=' in assignment");
        advance();

        Expr* value = parseExpression();

        if (expectSemicolon) {
            requireToken(SEMICOLON, "Expected ';'");
            advance();
        }

        return new AssignmentStatement(name, value);
    }

    Statement* parsePrintStatement() {
        advance();

        requireToken(LPAREN, "Expected '('");
        advance();

        Expr* value = parseExpression();

        requireToken(RPAREN, "Expected ')'");
        advance();

        requireToken(SEMICOLON, "Expected ';'");
        advance();

        return new PrintStatement(value);
    }

    Statement* parseBlockStatement() {
        requireToken(LBRACE, "Expected '{'");
        advance();

        vector<Statement*> statements;
        while (currentToken().type != RBRACE && currentToken().type != END) {
            statements.push_back(parseStatement());
        }

        requireToken(RBRACE, "Expected '}'");
        advance();

        return new BlockStatement(statements);
    }

    Statement* parseIfStatement() {
        advance();

        requireToken(LPAREN, "Expected '(' after if");
        advance();
        Expr* condition = parseExpression();
        requireToken(RPAREN, "Expected ')' after if condition");
        advance();

        Statement* thenBranch = parseBlockStatement();
        Statement* elseBranch = nullptr;

        if (currentToken().type == ELSE) {
            advance();
            elseBranch = parseBlockStatement();
        }

        return new IfStatement(condition, thenBranch, elseBranch);
    }

    Statement* parseWhileStatement() {
        advance();

        requireToken(LPAREN, "Expected '(' after while");
        advance();
        Expr* condition = parseExpression();
        requireToken(RPAREN, "Expected ')' after while condition");
        advance();

        Statement* body = parseBlockStatement();
        return new WhileStatement(condition, body);
    }

    Statement* parseForStatement() {
        advance();

        requireToken(LPAREN, "Expected '(' after for");
        advance();

        Statement* init = nullptr;
        if (currentToken().type != SEMICOLON) {
            if (currentToken().type == LET) {
                init = parseLetStatement(false);
            } else if (currentToken().type == IDENTIFIER) {
                init = parseAssignmentStatement(false);
            } else {
                parseError("Invalid for-loop initializer");
            }
        }

        requireToken(SEMICOLON, "Expected ';' after for-loop initializer");
        advance();

        Expr* condition = nullptr;
        if (currentToken().type != SEMICOLON) {
            condition = parseExpression();
        }

        requireToken(SEMICOLON, "Expected ';' after for-loop condition");
        advance();

        Statement* update = nullptr;
        if (currentToken().type != RPAREN) {
            if (currentToken().type == IDENTIFIER) {
                update = parseAssignmentStatement(false);
            } else {
                parseError("Invalid for-loop update");
            }
        }

        requireToken(RPAREN, "Expected ')' after for-loop update");
        advance();

        Statement* body = parseBlockStatement();
        return new ForStatement(init, condition, update, body);
    }

    Statement* parseStatement() {
        Token token = currentToken();

        if (token.type == LET) {
            return parseLetStatement(true);
        }

        if (token.type == IF) {
            return parseIfStatement();
        }

        if (token.type == WHILE) {
            return parseWhileStatement();
        }

        if (token.type == FOR) {
            return parseForStatement();
        }

        if (token.type == BREAK) {
            advance();
            requireToken(SEMICOLON, "Expected ';' after break");
            advance();
            return new BreakStatement();
        }

        if (token.type == CONTINUE) {
            advance();
            requireToken(SEMICOLON, "Expected ';' after continue");
            advance();
            return new ContinueStatement();
        }

        if (token.type == IDENTIFIER && token.value == "print") {
            return parsePrintStatement();
        }

        if (token.type == IDENTIFIER) {
            return parseAssignmentStatement(true);
        }

        if (token.type == LBRACE) {
            return parseBlockStatement();
        }

        parseError("Unknown statement");
        return nullptr;
    }

    vector<Statement*> parseProgram() {
        vector<Statement*> program;

        while (currentToken().type != END) {
            program.push_back(parseStatement());
        }

        return program;
    }
};

#endif