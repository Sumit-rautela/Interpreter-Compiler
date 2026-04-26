#ifndef LEXER_H
#define LEXER_H

#include <iostream>
#include <vector>
#include <cctype>
using namespace std;

enum TokenType {
    LET, IF, ELSE, WHILE, FOR, BREAK, CONTINUE,
    IDENTIFIER, NUMBER, STRING, CHAR,
    PLUS, MINUS, MUL, DIV,
    EQUAL, EQEQ, NEQ, LT, GT, LE, GE,
    LPAREN, RPAREN,
    LBRACE, RBRACE,
    SEMICOLON,
    END
};

struct Token {
    TokenType type;
    string value;
};

class Lexer {
private:
    string input;
    int pos;

public:
    Lexer(string input) {
        this->input = input;
        pos = 0;
    }

    char currentChar() {
        if (pos >= input.size()) return '\0';
        return input[pos];
    }

    void advance() { pos++; }

    void skipWhitespace() {
        while (isspace(currentChar())) advance();
    }

    Token identifier() {
        string result = "";
        while (isalnum(currentChar()) || currentChar() == '_') {
            result += currentChar();
            advance();
        }

        if (result == "let")
            return {LET, result};
        if (result == "if")
            return {IF, result};
        if (result == "else")
            return {ELSE, result};
        if (result == "while")
            return {WHILE, result};
        if (result == "for")
            return {FOR, result};
        if (result == "break")
            return {BREAK, result};
        if (result == "continue")
            return {CONTINUE, result};

        return {IDENTIFIER, result};
    }

    Token number() {
        string result = "";
        bool hasDot = false;
        while (isdigit(currentChar()) || currentChar() == '.') {
            if (currentChar() == '.') {
                if (hasDot) break;
                hasDot = true;
            }
            result += currentChar();
            advance();
        }
        return {NUMBER, result};
    }

    Token stringLiteral() {
        string result = "";
        advance();

        while (currentChar() != '\0' && currentChar() != '"') {
            if (currentChar() == '\\') {
                advance();
                if (currentChar() == 'n') result += '\n';
                else if (currentChar() == 't') result += '\t';
                else if (currentChar() == '"') result += '"';
                else if (currentChar() == '\\') result += '\\';
                else result += currentChar();
            } else {
                result += currentChar();
            }
            advance();
        }

        if (currentChar() != '"') {
            cout << "Unterminated string literal" << endl;
            exit(1);
        }
        advance();
        return {STRING, result};
    }

    Token charLiteral() {
        string result = "";
        advance();

        if (currentChar() == '\\') {
            advance();
            if (currentChar() == 'n') result += '\n';
            else if (currentChar() == 't') result += '\t';
            else if (currentChar() == '\'') result += '\'';
            else if (currentChar() == '\\') result += '\\';
            else result += currentChar();
            advance();
        } else {
            if (currentChar() == '\0' || currentChar() == '\'') {
                cout << "Invalid char literal" << endl;
                exit(1);
            }
            result += currentChar();
            advance();
        }

        if (currentChar() != '\'') {
            cout << "Invalid char literal" << endl;
            exit(1);
        }

        advance();
        return {CHAR, result};
    }

    vector<Token> tokenize() {
        vector<Token> tokens;

        while (currentChar() != '\0') {
            if (isspace(currentChar())) {
                skipWhitespace();
                continue;
            }

            if (isalpha(currentChar()) || currentChar() == '_') {
                tokens.push_back(identifier());
                continue;
            }

            if (isdigit(currentChar())) {
                tokens.push_back(number());
                continue;
            }

            if (currentChar() == '"') {
                tokens.push_back(stringLiteral());
                continue;
            }

            if (currentChar() == '\'') {
                tokens.push_back(charLiteral());
                continue;
            }

            if (currentChar() == '=') {
                advance();
                if (currentChar() == '=') {
                    tokens.push_back({EQEQ, "=="});
                    advance();
                } else {
                    tokens.push_back({EQUAL, "="});
                }
                continue;
            }

            if (currentChar() == '!') {
                advance();
                if (currentChar() == '=') {
                    tokens.push_back({NEQ, "!="});
                    advance();
                } else {
                    cout << "Unknown char: !" << endl;
                    exit(1);
                }
                continue;
            }

            if (currentChar() == '<') {
                advance();
                if (currentChar() == '=') {
                    tokens.push_back({LE, "<="});
                    advance();
                } else {
                    tokens.push_back({LT, "<"});
                }
                continue;
            }

            if (currentChar() == '>') {
                advance();
                if (currentChar() == '=') {
                    tokens.push_back({GE, ">="});
                    advance();
                } else {
                    tokens.push_back({GT, ">"});
                }
                continue;
            }

            switch (currentChar()) {
                case '+': tokens.push_back({PLUS, "+"}); break;
                case '-': tokens.push_back({MINUS, "-"}); break;
                case '*': tokens.push_back({MUL, "*"}); break;
                case '/': tokens.push_back({DIV, "/"}); break;
                case '(': tokens.push_back({LPAREN, "("}); break;
                case ')': tokens.push_back({RPAREN, ")"}); break;
                case '{': tokens.push_back({LBRACE, "{"}); break;
                case '}': tokens.push_back({RBRACE, "}"}); break;
                case ';': tokens.push_back({SEMICOLON, ";"}); break;
                default:
                    cout << "Unknown char: " << currentChar() << endl;
                    exit(1);
            }
            advance();
        }

        tokens.push_back({END, ""});
        return tokens;
    }
};

#endif