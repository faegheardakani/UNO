#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <cstdlib>
#include <stack>
#include <fstream>
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

enum Color { RED, GREEN, BLUE, YELLOW, NONE };
enum Type { NUMBER, SKIP, REVERSE, DRAW_TWO, WILD, WILD_DRAW_FOUR };

string colorToString(Color c) {
    switch (c) {
        case RED: return "\033[1;31mRed\033[0m";
        case GREEN: return "\033[1;32mGreen\033[0m";
        case BLUE: return "\033[1;34mBlue\033[0m";
        case YELLOW: return "\033[1;33mYellow\033[0m";
        default: return "None";
    }
}

string getTodayDate() {
    time_t t = time(0);
    tm* now = localtime(&t);
    char buf[11];
    strftime(buf, sizeof(buf), "%Y-%m-%d", now);
    return string(buf);
}

json loadPlayerData(string name) {
    ifstream in("player_stats.json");
    json data;
    if (in) {
        in >> data;
        if (data.contains(name)) return data[name];
    }
    return {
        {"name", name},
        {"played_games", 0},
        {"wins", 0},
        {"losses", 0},
        {"history", json::array()}
    };
}

void savePlayerData(json player) {
    json allData;
    ifstream in("player_stats.json");
    if (in) in >> allData;
    allData[player["name"]] = player;
    ofstream out("player_stats.json");
    out << allData.dump(4);
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
    stack<Card> pile;

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

    void placeCard(Card c) {
        pile.push(c);
    }

    Card topCard() {
        return pile.top();
    }
};

class Player {
public:
    vector<Card> hand;
    string name;
    bool isBot;

    Player(string n, bool bot = false) {
        name = n;
        isBot = bot;
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

    Card chooseCard(Card top, Color& newColor, Color currentColor) {
        while (true) {
            cout << "\nYour hand:\n";
            for (int i = 0; i < hand.size(); i++)
                cout << i + 1 << ". " << hand[i].toString() << endl;
            cout << "0. Draw a card\nChoose: ";
            int choice;
            cin >> choice;
            if (choice == 0) return Card(NONE, NUMBER);
            if (choice >= 1 && choice <= hand.size()) {
                Card selected = hand[choice - 1];
                if (canPlay(selected, top, currentColor)) {
                    hand.erase(hand.begin() + (choice - 1));
                    if (selected.type == WILD || selected.type == WILD_DRAW_FOUR) {
                        int col;
                        cout << "Choose color (0=Red, 1=Green, 2=Blue, 3=Yellow): ";
                        cin >> col;
                        newColor = (Color)col;
                    } else {
                        newColor = selected.color;
                    }
                    return selected;
                } else {
                    cout << "Invalid card. Try again.\n";
                }
            }
        }
    }

    Card playCard(Card top, Color currentColor, Color& newColor) {
        if (!isBot) return chooseCard(top, newColor, currentColor);
        else {
            for (int i = 0; i < hand.size(); i++) {
                if (canPlay(hand[i], top, currentColor)) {
                    Card played = hand[i];
                    hand.erase(hand.begin() + i);
                    if (played.type == WILD || played.type == WILD_DRAW_FOUR)
                        newColor = (Color)(rand() % 4);
                    else
                        newColor = played.color;
                    return played;
                }
            }
            return Card(NONE, NUMBER);
        }
    }
};

class Game {
public:
    Deck deck;
    vector<Player> players;
    int currentPlayer;
    int direction;
    Color currentColor;
    bool allDiscardRule;
    string playerName;
    json& playerData;

    Game(string name, json& pdata, bool enableAllDiscard) : playerData(pdata) {
        playerName = name;
        allDiscardRule = enableAllDiscard;

        players.push_back(Player(name));
        players.push_back(Player("Bot1", true));
        players.push_back(Player("Bot2", true));
        players.push_back(Player("Bot3", true));

        for (int i = 0; i < players.size(); i++)
            players[i].draw(deck, 7);

        Card first = deck.drawCard();
        while (first.type == WILD_DRAW_FOUR) first = deck.drawCard();
        if (first.type == WILD)
            currentColor = (Color)(rand() % 4);
        else
            currentColor = first.color;
        deck.placeCard(first);

        currentPlayer = 0;
        direction = 1;

        playerData["played_games"] = int(playerData["played_games"]) + 1;

        mainLoop();
    }

    void endGame(bool won) {
        if (won)
            playerData["wins"] = int(playerData["wins"]) + 1;
        else
            playerData["losses"] = int(playerData["losses"]) + 1;
        playerData["history"].push_back({ {"date", getTodayDate()}, {"result", won ? "win" : "loss"} });
    }

    void showCardCounts() {
        cout << "\nCard counts: ";
        for (int i = 0; i < players.size(); i++) {
            cout << players[i].name << ": " << players[i].hand.size();
            if (players[i].hand.size() == 1) cout << " (UNO!)";
            cout << " | ";
        }
        cout << endl;
    }

    void mainLoop() {
        while (true) {
            Player& p = players[currentPlayer];
            Card top = deck.topCard();
            showCardCounts();
            cout << "\nTop card: " << top.toString() << " | Current color: " << colorToString(currentColor) << endl;

            if (p.isBot && !p.hasPlayableCard(top, currentColor)) {
                cout << p.name << " draws a card.\n";
                p.draw(deck);
                advanceTurn();
                continue;
            }

            Color newColor = currentColor;
            Card played = p.playCard(top, currentColor, newColor);

            if (played.color == NONE && played.type == NUMBER && played.number == -1) {
                cout << p.name << " drew a card.\n";
                p.draw(deck);
                advanceTurn();
                continue;
            }

            cout << p.name << " plays " << played.toString() << endl;
            deck.placeCard(played);
            currentColor = newColor;

            if (allDiscardRule) {
                vector<Card>& hand = p.hand;
                vector<Card> extras;
                for (int i = 0; i < hand.size(); i++) {
                    if (hand[i].color == played.color)
                        extras.push_back(hand[i]);
                }
                for (int i = 0; i < extras.size(); i++) {
                    cout << "-> " << p.name << " also discards " << extras[i].toString() << " (All Discard)\n";
                    for (int j = 0; j < hand.size(); j++) {
                        if (hand[j].equals(extras[i])) {
                            hand.erase(hand.begin() + j);
                            break;
                        }
                    }
                    deck.placeCard(extras[i]);
                }
            }

            if (p.hand.empty()) {
                cout << p.name << " wins the game!\n";
                endGame(p.name == playerName);
                break;
            } else if (p.hand.size() == 1) {
                cout << "UNO! " << p.name << " has one card left!\n";
            }

            if (played.type == REVERSE) direction *= -1;
            else if (played.type == SKIP) advanceTurn();
            else if (played.type == DRAW_TWO) nextPlayer().draw(deck, 2);
            else if (played.type == WILD_DRAW_FOUR) nextPlayer().draw(deck, 4);

            advanceTurn();
        }
    }

    void advanceTurn() {
        currentPlayer = (currentPlayer + direction + 4) % 4;
    }

    Player& nextPlayer() {
        int next = (currentPlayer + direction + 4) % 4;
        return players[next];
    }
};

int main() {
    string name;
    cout << "Enter your name: ";
    cin >> name;
    bool enableAllDiscard;
    cout << "Enable All Discard rule? (1 = Yes, 0 = No): ";
    cin >> enableAllDiscard;

    json player = loadPlayerData(name);
    Game g(name, player, enableAllDiscard);
    savePlayerData(player);
    return 0;
}



