#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
using namespace std;

enum Color { RED, GREEN, BLUE, YELLOW, NONE };
enum Type { NUMBER, SKIP, REVERSE, DRAW_TWO, WILD, WILD_DRAW_FOUR };

string colorToString(Color c) {
    if (c == RED) return "Red";
    if (c == GREEN) return "Green";
    if (c == BLUE) return "Blue";
    if (c == YELLOW) return "Yellow";
    return "None";
}

struct Card {
    Color color;
    Type type;
    int number;

    Card(Color c, Type t, int n = -1) {
        color = c;
        type = t;
        number = n;
    }

    string toString() {
        if (type == NUMBER) return colorToString(color) + " " + to_string(number);
        if (type == SKIP) return colorToString(color) + " Skip";
        if (type == REVERSE) return colorToString(color) + " Reverse";
        if (type == DRAW_TWO) return colorToString(color) + " Draw Two";
        if (type == WILD) return "Wild";
        if (type == WILD_DRAW_FOUR) return "Wild Draw Four";
        return "Unknown";
    }

    bool equals(Card other) {
        return color == other.color && type == other.type && number == other.number;
    }
};

class Deck {
public:
    vector<Card> cards;

    Deck() {
        generate();
        shuffle();
    }

    void generate() {
        cards.clear();
        for (int c = RED; c <= YELLOW; c++) {
            for (int n = 0; n <= 9; n++)
                cards.push_back(Card((Color)c, NUMBER, n));
            for (int t = SKIP; t <= DRAW_TWO; t++)
                cards.push_back(Card((Color)c, (Type)t));
        }
        for (int i = 0; i < 4; i++) {
            cards.push_back(Card(NONE, WILD));
            cards.push_back(Card(NONE, WILD_DRAW_FOUR));
        }
    }

    void shuffle() {
        srand(time(0));
        for (int i = 0; i < cards.size(); i++) {
            int j = rand() % cards.size();
            Card temp = cards[i];
            cards[i] = cards[j];
            cards[j] = temp;
        }
    }

    Card drawCard() {
        if (cards.empty()) generate();
        Card c = cards.back();
        cards.pop_back();
        return c;
    }
};

int main() {
    Deck d;
    Card c = d.drawCard();
    cout << "Drawn: " << c.toString() << endl;
    return 0;
}
