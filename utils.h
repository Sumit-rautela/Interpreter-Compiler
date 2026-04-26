#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

string readFile(string filename) {
    ifstream file(filename);
    if (!file) {
        cout << "Error: Cannot open file\n";
        exit(1);
    }

    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

#endif