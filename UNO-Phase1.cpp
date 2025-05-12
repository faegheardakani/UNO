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

    Card() {
        color = NONE;
        type = NUMBER;
        number = -1;
    }

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

class Player {
public:
    vector<Card> hand;
    string name;

    Player(string n) {
        name = n;
    }

    void draw(Deck& deck, int count = 1) {
        for (int i = 0; i < count; i++)
            hand.push_back(deck.drawCard());
    }

    bool canPlay(Card c, Card top, Color currentColor) {
        if (c.type == WILD || c.type == WILD_DRAW_FOUR) return true;
        if (c.color == currentColor) return true;
        if (c.type == top.type && c.type != NUMBER) return true;
        if (c.type == NUMBER && top.type == NUMBER && c.number == top.number) return true;
        return false;
    }

    bool hasPlayableCard(Card top, Color currentColor) {
        for (int i = 0; i < hand.size(); i++) {
            if (canPlay(hand[i], top, currentColor)) return true;
        }
        return false;
    }
};

class Game {
public:
    Deck deck;
    vector<Player> players;
    int currentPlayer;
    int direction;
    Color currentColor;
    Card topCard;

    Game(string playerName) {
        players.push_back(Player(playerName));
        players.push_back(Player("Bot1"));
        players.push_back(Player("Bot2"));
        players.push_back(Player("Bot3"));

        for (int i = 0; i < players.size(); i++)
            players[i].draw(deck, 7);

        topCard = deck.drawCard();
        currentColor = topCard.color;
        currentPlayer = 0;
        direction = 1;

        for (int round = 0; round < 4; round++)
            runTurn();
    }

    void runTurn() {
        Player& p = players[currentPlayer];
        cout << "Top card: " << topCard.toString() << endl;
        cout << p.name << " has these cards:" << endl;
        for (int i = 0; i < p.hand.size(); i++)
            cout << "- " << p.hand[i].toString() << endl;
        if (!p.hasPlayableCard(topCard, currentColor)) {
            cout << p.name << " draws a card." << endl;
            p.draw(deck);
        } else {
            for (int i = 0; i < p.hand.size(); i++) {
                if (p.canPlay(p.hand[i], topCard, currentColor)) {
                    topCard = p.hand[i];
                    currentColor = topCard.color;
                    cout << p.name << " plays " << topCard.toString() << endl;
                    p.hand.erase(p.hand.begin() + i);
                    break;
                }
            }
        }
        advanceTurn();
    }

    void advanceTurn() {
        currentPlayer = (currentPlayer + direction + players.size()) % players.size();
    }
};

int main() {
    Game g("Tester");
    return 0;
}

