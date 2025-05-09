#include <iostream>
#include <string>
using namespace std;

enum Color { RED, GREEN, BLUE, YELLOW, NONE };
enum Type { NUMBER, SKIP, REVERSE, DRAW_TWO, WILD, WILD_DRAW_FOUR };

struct Card {
    Color color;
    Type type;
    int number;

    Card(Color c, Type t, int n = -1) {
        color = c;
        type = t;
        number = n;
    }
};

int main() {
    Card c(RED, NUMBER, 5);
    cout << "Card created.\n";
    return 0;
}


