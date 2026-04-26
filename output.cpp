#include <iostream>
using namespace std;

int main() {
    auto x = 2.500000;
    auto y = 3.000000;
    auto name = "Sumit";
    auto letter = 'A';
    cout << name + " -> " + letter << endl;
    cout << x + y << endl;
    if (x < y)     {
        cout << 100.250000 << endl;
    }
    else     {
        cout << 200.000000 << endl;
    }
    auto i = 0.000000;
    while (i < 3.000000)     {
        if (i == 1.000000)         {
            i = i + 1.000000;
            continue;
        }
        cout << i << endl;
        i = i + 1.000000;
    }
    for (auto j = 0.000000; j < 3.000000; j = j + 1.000000)     {
        if (j == 4.000000)         {
            break;
        }
        cout << j + 10.000000 << endl;
    }
    return 0;
}