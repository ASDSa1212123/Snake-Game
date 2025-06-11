#include <iostream>
#include <conio.h>
#include <windows.h>
#include <ctime>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>

using namespace std;

enum Direction { STOP = 0, LEFT, RIGHT, UP, DOWN };

struct Difficulty {
    int speed;
    int obstacles;
    int startingLength;
    int foodValue;
};

map<int, Difficulty> difficultyMap = {
    {1, {100, 5, 1, 10}},
    {2, {50, 10, 3, 15}},
    {3, {25, 15, 5, 20}}
};

// ----- UTILITY FUNCTIONS -----
void moveCursorToTopLeft() {
    COORD coord = {0, 0};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void hideCursor() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hOut, &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(hOut, &cursorInfo);
}

void centerText(const string& text, int consoleWidth) {
    int padding = (consoleWidth - text.length()) / 2;
    cout << string(padding, ' ') << text << "\n";
}

void centerBoxText(const string& content, int boxWidth, int consoleWidth) {
    int innerWidth = boxWidth - 2;
    int padding = (innerWidth - content.length()) / 2;
    string line = ".";
    line += string(padding, ' ') + content + string(innerWidth - padding - content.length(), ' ');
    line += ".";
    centerText(line, consoleWidth);
}

void playSound(const string& event) {
    if (event == "food") Beep(1000, 150);
    else if (event == "gameover") Beep(300, 400);
}

// ----- SEGMENT CLASS -----
class Segment {
public:
    int x, y;
    Segment* next;
    Segment(int x, int y) : x(x), y(y), next(NULL) {}
};

// ----- BASE SNAKE CLASS (Polymorphism + Inheritance) -----
class BaseSnake {
protected:
    Segment* head;
    Direction dir;
    int length;

public:
    BaseSnake(int x, int y, int len = 1) {
        head = new Segment(x, y);
        dir = STOP;
        length = len;
        Segment* current = head;
        for (int i = 1; i < length; ++i) {
            current->next = new Segment(x, y + i);
            current = current->next;
        }
    }

    
    BaseSnake(const BaseSnake& other) {
        dir = other.dir;
        length = other.length;
        head = new Segment(other.head->x, other.head->y);
        Segment* currentNew = head;
        Segment* currentOld = other.head->next;
        while (currentOld) {
            currentNew->next = new Segment(currentOld->x, currentOld->y);
            currentNew = currentNew->next;
            currentOld = currentOld->next;
        }
    }

    virtual ~BaseSnake() {
        Segment* current = head;
        while (current) {
            Segment* next = current->next;
            delete current;
            current = next;
        }
    }

    virtual void move() = 0; // Polymorphic method
    virtual void grow() { length++; }
    virtual bool isCollidingWithSelf() {
        Segment* temp = head->next;
        while (temp) {
            if (temp->x == head->x && temp->y == head->y) return true;
            temp = temp->next;
        }
        return false;
    }

    void setDirection(Direction newDir) {
        if ((dir == LEFT && newDir == RIGHT) || (dir == RIGHT && newDir == LEFT) ||
            (dir == UP && newDir == DOWN) || (dir == DOWN && newDir == UP)) return;
        dir = newDir;
    }

    Segment* getHead() { return head; }
    Direction getDirection() { return dir; }
};
// ----- SNAKE CLASS (Single Player) -----
class Snake : public BaseSnake {
public:
    Snake(int x, int y, int len = 1) : BaseSnake(x, y, len) {}

    void move() override {
        if (dir == STOP) return;

        int newX = head->x;
        int newY = head->y;
        if (dir == LEFT) newX--;
        if (dir == RIGHT) newX++;
        if (dir == UP) newY--;
        if (dir == DOWN) newY++;

        Segment* newHead = new Segment(newX, newY);
        newHead->next = head;
        head = newHead;

        Segment* temp = head;
        for (int i = 1; i < length; i++) temp = temp->next;
        delete temp->next;
        temp->next = NULL;
    }
};

// ----- PLAYER SNAKE (Two Player Use) -----
class PlayerSnake : public BaseSnake {
public:
    int score;
    int color;

    PlayerSnake(int x, int y, int color) : BaseSnake(x, y), score(0), color(color) {}

    void move() override {
        if (dir == STOP) return;

        int newX = head->x;
        int newY = head->y;
        if (dir == LEFT) newX--;
        if (dir == RIGHT) newX++;
        if (dir == UP) newY--;
        if (dir == DOWN) newY++;

        Segment* newHead = new Segment(newX, newY);
        newHead->next = head;
        head = newHead;

        Segment* temp = head;
        for (int i = 1; i < length; i++) temp = temp->next;
        delete temp->next;
        temp->next = NULL;
    }

    void grow() override {
        length++;
        score += 10;
    }

    bool isDead(int width, int height) {
        if (head->x < 1 || head->x >= width + 1 || head->y < 0 || head->y >= height)
            return true;
        return isCollidingWithSelf();
    }
};

// ----- FOOD CLASS -----
class Food {
public:
    int x, y;

    Food(int width, int height) {
        respawn(width, height);
    }

    void respawn(int width, int height) {
        x = rand() % width;
        y = rand() % height;
    }
};
void updateScores(const string& playerName, int score, int& personalBest, string& topScorer, int& topScore) {
    try {
        // Step 1: Read all scores
        map<string, int> bestScores;
        topScore = 0;
        topScorer = "N/A";

        ifstream inFile("score.txt");
        if (inFile) {
            string name;
            int s;
            while (inFile >> name >> s) {
                bestScores[name] = max(bestScores[name], s);
                if (bestScores[name] > topScore) {
                    topScore = bestScores[name];
                    topScorer = name;
                }
            }
            inFile.close();
        }

        // Step 2: Update current player
        bestScores[playerName] = max(bestScores[playerName], score);

        // Step 3: Rewrite all to file
        ofstream outFile("scores.txt");
        if (!outFile) throw runtime_error("Failed to open scores.txt for writing.");
        for (const auto& entry : bestScores) {
            outFile << entry.first << " " << entry.second << "\n";
        }
        outFile.close();

        // Step 4: Return personal best and top score
        personalBest = bestScores[playerName];
        topScore = 0;
        for (const auto& entry : bestScores) {
            if (entry.second > topScore) {
                topScore = entry.second;
                topScorer = entry.first;
            }
        }
    } catch (exception& e) {
        cerr << "[ERROR]: " << e.what() << endl;
        personalBest = 0;
        topScorer = "N/A";
        topScore = 0;
    }
}


// ----- GAME CLASS -----
class Game {
private:
    int width, height, score, level, speed, foodValue;
    Snake* snake;
    Food* food;
    vector<pair<int, int>> obstacles;
    bool gameOver;
    string playerName;
    
int timeLeft = 60;  // in seconds
bool bonusFoodVisible = false;
int bonusX = -1, bonusY = -1;

    void generateObstacles(int count) {
        obstacles.clear();
        for (int i = 0; i < count; i++) {
            int x = rand() % width;
            int y = rand() % height;
            obstacles.push_back({x, y});
        }
    }

public:
	bool isTimedMode = false;
    Game(int w, int h, const string& name, const Difficulty& diff, bool timed = false)
    : width(w), height(h), playerName(name), speed(diff.speed), foodValue(diff.foodValue), isTimedMode(timed) {
        snake = new Snake(w / 2, h / 2, diff.startingLength);
        food = new Food(w, h);
        score = 0;
        level = 1;
        gameOver = false;
        generateObstacles(diff.obstacles);
    }

    ~Game() {
        delete snake;
        delete food;
    }

    void draw() {
    moveCursorToTopLeft();
    int consoleWidth = 130;
    int padding = (consoleWidth - (width + 2)) / 2;
    setColor(11);
    cout << string(padding, ' ') << string(width + 2, '#') << "\n";

    for (int y = 0; y < height; y++) {
        cout << string(padding, ' ') << "#";
        for (int x = 0; x < width; x++) {
            if (x == snake->getHead()->x && y == snake->getHead()->y) {
                setColor(10); cout << "O";
            } else if (x == food->x && y == food->y) {
                setColor(14); cout << "@";
            } else if (isTimedMode && bonusFoodVisible && x == bonusX && y == bonusY) {
                setColor(13); cout << "$";  // Bonus food symbol
            } else {
                bool drawn = false;
                for (auto& o : obstacles) {
                    if (x == o.first && y == o.second) {
                        setColor(4); cout << "X"; drawn = true; break;
                    }
                }
                Segment* temp = snake->getHead()->next;
                while (!drawn && temp) {
                    if (temp->x == x && temp->y == y) {
                        setColor(10); cout << "o"; drawn = true; break;
                    }
                    temp = temp->next;
                }
                if (!drawn) setColor(7), cout << " ";
            }
        }
        setColor(11); cout << "#\n";
    }

    cout << string(padding, ' ') << string(width + 2, '#') << "\n";
    setColor(7);
    cout << string(padding, ' ') << "Player: " << playerName
         << " | Score: " << score
         << " | Level: " << level;

    if (isTimedMode) {
        cout << " | Time Left: " << timeLeft << "s";
    }
    cout << "\n";
}


    void input() {
        if (_kbhit()) {
            switch (_getch()) {
                case 'a': snake->setDirection(LEFT); break;
                case 'd': snake->setDirection(RIGHT); break;
                case 'w': snake->setDirection(UP); break;
                case 's': snake->setDirection(DOWN); break;
                case 'x': gameOver = true; break;
            }
        }
    }

    void logic() {
        snake->move();
        Segment* head = snake->getHead();
        if (head->x < 0 || head->x >= width || head->y < 0 || head->y >= height || snake->isCollidingWithSelf()) {
            gameOver = true; return;
        }
if (isTimedMode && bonusFoodVisible && head->x == bonusX && head->y == bonusY) {
    timeLeft += 10;
    bonusFoodVisible = false;
}

        for (auto& o : obstacles) {
            if (head->x == o.first && head->y == o.second) {
                gameOver = true; return;
            }
        }

        if (head->x == food->x && head->y == food->y) {
            playSound("food");
            snake->grow();
            score += foodValue;
            food->respawn(width, height);

            if (score % 50 == 0 && speed > 10) {
                level++;
                speed -= 5;
                generateObstacles(level + 10);
            }
        }
    }

    void showWelcomeScreen() {
    system("cls");
    hideCursor();
    const int consoleWidth = 130;

    setColor(6); // Orange-like color (Dark Yellow)

    // Display baby snakes
    string snakeFace[] = {
        "        /^\\/^\\    ",
        "      _|__|  O|   ",
        " \\/     /~    \\_/ ",
        "  \\____|__________/ ",
        "        \\_______     \\ ",
        "               `\\     \\ ",
        "                 |     |",

    };

    for (const string& line : snakeFace) {
        centerText(line, consoleWidth);
    }

    cout << "\n";

    // Main welcome message
    setColor(12); // Light red (more attention-grabbing)
    centerBoxText("WELCOME TO THE SNAKE GAME!", 50, consoleWidth);
    centerBoxText("Use W/A/S/D to move. Eat food. Avoid obstacles.", 80, consoleWidth);
    centerBoxText("Level up every 50 points!", 80, consoleWidth);

    setColor(14); // Bright yellow for the prompt
    centerBoxText("Press any key to start...", 50, consoleWidth);

    _getch();
    system("cls");
    setColor(7); // Reset to default
}


    void run() {
    time_t startTime = time(0);
while (!gameOver) {
    draw();
    input();
    logic();

    if (isTimedMode) {
        timeLeft = 60 - (int)(time(0) - startTime);
        if (timeLeft <= 0) gameOver = true;
        else if (timeLeft <= 15 && !bonusFoodVisible) {
            bonusFoodVisible = true;
            bonusX = rand() % width;
            bonusY = rand() % height;
        }
    }

    Sleep(speed);
}

    }

   ;

bool gameOverScreen() {
    system("cls");
    hideCursor();
    int consoleWidth = 100;
    string border(consoleWidth, '=');

    int personalBest = 0;
    string topScorer;
    int topScore = 0;
    updateScores(playerName, score, personalBest, topScorer, topScore);

    string snakeFace[] = {
        "        /^\\/^\\    ",
        "      _|__|  O|   ",
        " \\/     /~    \\_/ ",
        "  \\____|__________/ ",
        "        \\_______     \\ ",
        "               `\\     \\ ",
        "                 |     |",
        "                /      / ",
        "               /     /    ",
        "            /     /       ",
        "           /     /        ",
        "          /     /         ",
        "         (     (          ",
        "          \\     ~-____    ",
        "           ~-_        ~-__"
    };

    setColor(10);
    for (const auto& line : snakeFace) {
        centerText(line, consoleWidth);
        Sleep(40);
    }

    Beep(300, 200); Beep(200, 200); Beep(150, 300);

    setColor(13);
    cout << "\n";
    centerText(" GAME OVER ", consoleWidth);
    setColor(7);
    Sleep(300);

    string labels[] = {
        " FINAL SCORE  : " + to_string(score),
        " PERSONAL BEST: " + to_string(personalBest),
        " TOP SCORER   : " + topScorer + " (" + to_string(topScore) + ")"
    };

    for (const string& label : labels) {
        setColor(rand() % 7 + 9);  // Bright colors
        centerText(label, consoleWidth);
        Beep(600, 100);
        Sleep(400);
    }

    setColor(14);
    cout << "\n";
    centerText(" THANKS FOR SLITHERING WITH US ", consoleWidth);
    cout << "\n";

    setColor(11);
    centerText(" Want to play again? (y/n): ", consoleWidth);
    setColor(14);
    cout << string((consoleWidth / 2) - 2, ' ');
    char input;
    cin >> input;
    system("cls");
    return (input == 'y' || input == 'Y');
}
  

};
// ----- TWO PLAYER MODE FUNCTION -----
void runTwoPlayerMode() {
	system("cls");
    int width = 60, height = 25;
    PlayerSnake* p1 = new PlayerSnake(width / 3, height / 2, 10);
    PlayerSnake* p2 = new PlayerSnake(2 * width / 3, height / 2, 11);
    Food food(width, height);
    bool gameOver = false;

    while (!gameOver) {
        moveCursorToTopLeft();
        for (int i = 0; i < width + 2; i++) cout << "#";
        cout << "\n";

        for (int y = 0; y < height; y++) {
            for (int x = 0; x <= width + 1; x++) {
                if (x == 0 || x == width + 1) {
                    setColor(11); cout << "#";
                } else if (x == p1->getHead()->x && y == p1->getHead()->y) {
                    setColor(p1->color); cout << "O";
                } else if (x == p2->getHead()->x && y == p2->getHead()->y) {
                    setColor(p2->color); cout << "O";
                } else if (x == food.x && y == food.y) {
                    setColor(14); cout << "@";
                } else {
                    bool printed = false;
                    Segment* t1 = p1->getHead()->next;
                    while (t1) {
                        if (t1->x == x && t1->y == y) {
                            setColor(p1->color); cout << "o"; printed = true; break;
                        }
                        t1 = t1->next;
                    }

                    Segment* t2 = p2->getHead()->next;
                    while (!printed && t2) {
                        if (t2->x == x && t2->y == y) {
                            setColor(p2->color); cout << "o"; printed = true; break;
                        }
                        t2 = t2->next;
                    }

                    if (!printed) cout << " ";
                }
            }
            cout << "\n";
        }

        for (int i = 0; i < width + 2; i++) cout << "#";
        cout << "\n";
        setColor(7);
        cout << "Player 1 (WASD): " << p1->score << "    Player 2 (Arrow Keys): " << p2->score << "\n";

        // Input
        if (_kbhit()) {
            int key = _getch();
            if (key == 224) {
                switch (_getch()) {
                    case 72: p2->setDirection(UP); break;
                    case 80: p2->setDirection(DOWN); break;
                    case 75: p2->setDirection(LEFT); break;
                    case 77: p2->setDirection(RIGHT); break;
                }
            } else {
                switch (key) {
                    case 'w': p1->setDirection(UP); break;
                    case 's': p1->setDirection(DOWN); break;
                    case 'a': p1->setDirection(LEFT); break;
                    case 'd': p1->setDirection(RIGHT); break;
                    case 'x': gameOver = true; break;
                }
            }
        }

        p1->move(); p2->move();

        if (p1->getHead()->x == food.x && p1->getHead()->y == food.y) {
            playSound("food"); p1->grow(); food.respawn(width, height);
        } else if (p2->getHead()->x == food.x && p2->getHead()->y == food.y) {
            playSound("food"); p2->grow(); food.respawn(width, height);
        }

        if (p1->isDead(width, height) || p2->isDead(width, height)) gameOver = true;
        Sleep(60);
    }

    playSound("gameover");
    system("cls");
    cout << "Game Over!\n";
    cout << "Player 1 Score: " << p1->score << "\n";
    cout << "Player 2 Score: " << p2->score << "\n";
    cout << "Press any key to exit...";
    _getch();

    delete p1;
    delete p2;
}


void showStylishIntro() {
    system("cls");
    hideCursor();
    const int consoleWidth = 100;

    string snakeArt[] = {
        "     /^\\/^\\                                              ",
        "   _|__|  O|                                              ",
        "\\/     /~   \\___                                         ",
        " \\____|__________/  \\                                    ",
        "        \\_______      \\                                  ",
        "                `\\     \\                 \\                ",
        "                  |     |                  \\              ",
        "                 /      /                    \\            ",
        "                /     /                       \\\\         ",
        "              /      /                         \\ \\       ",
        "             /     /                            \\  \\     ",
        "           /     /             _----_            \\   \\   ",
        "          /     /           _-~      ~-_         |   |   ",
        "         (      (        _-~    _--_    ~-_     _/   |   ",
        "          \\      ~-____-~    _-~    ~-_    ~-_-~    /    ",
        "            ~-_           _-~          ~-_       _-~     ",
        "               ~--______-~                ~-___-~        ",
        "                                                       ",
        "               << S N A K E   G A M E >>               "
    };

    setColor(10);  // Bright green
    for (const auto& line : snakeArt) {
        centerText(line, consoleWidth);
        Sleep(60);
    }

    setColor(14);
    cout << "\n";
    centerText("Press any key to begin slithering...", consoleWidth);
    setColor(7);
    _getch();
    system("cls");
}

Difficulty selectDifficulty(int& modeChoice) {
    system("cls");
    hideCursor();
    const int consoleWidth = 100;
    string border(consoleWidth, '=');
    int selectedOption = 1;
    const int optionCount = 5;

    string options[] = {
        "Easy      - Slow and steady",
        "Medium    - Balanced for most players",
        "Hard      - Fast, tight corners, more danger",
        "Two Player Mode",
        "Timed Challenge - 1 minute blitz with bonus!"
    };

    while (true) {
        system("cls");
        setColor(13);
        cout << border << "\n";
        centerText("== SNAKE GAME - SELECT YOUR VENOM ==", consoleWidth);
        cout << border << "\n\n";

        setColor(10);
        centerText("Use UP / DOWN arrows and press ENTER", consoleWidth);
        cout << "\n";

        for (int i = 0; i < optionCount; ++i) {
            if (i == selectedOption - 1) {
                setColor(14);
                cout << "\n";
                centerText(">> " + options[i] + " <<", consoleWidth);
                Beep(700 + i * 100, 50);
            } else {
                setColor(7);
                centerText("   " + options[i], consoleWidth);
            }
        }

        cout << "\n";
        setColor(13);
        cout << border << "\n";

        int key = _getch();
        if (key == 224) {
            int arrow = _getch();
            if (arrow == 72 && selectedOption > 1) selectedOption--;  // Up
            else if (arrow == 80 && selectedOption < optionCount) selectedOption++;  // Down
        } else if (key == 13) {
            break;  // Enter pressed
        }
    }

    modeChoice = selectedOption;
    return difficultyMap.find(modeChoice) != difficultyMap.end()
           ? difficultyMap[modeChoice]
           : difficultyMap[2];
}

void promptPlayerName(string& playerName) {
    system("cls");
    hideCursor();
    const int consoleWidth = 100;
    string border(consoleWidth, '=');

    string snakeLeft[] = {
        "   /^\\/^\\",
        " _|__|  O|",
        "\\/     /~ \\___",
        " \\____|__________/  \\",
        "        \\_______      \\",
        "                `\\     \\",
        "                  |     |",
        "                 /      /",
        "                /     /"
    };

    string snakeRight[] = {
        "                           /^\\/^\\   ",
        "                          |O  |__|_ ",
        "                    ___/~ \\     \\/ ",
        "                  /  \\__________|___/",
        "                 /      _______/     ",
        "               /     /`             ",
        "              |     |               ",
        "               \\     \\             ",
        "                \\     \\           "
    };

    // Display top border
    setColor(13);
    cout << border << "\n";

    // Display snake art left + right
    for (size_t i = 0; i < snakeLeft->size(); ++i) {
        setColor(10);
        cout << snakeLeft[i];
        int space = consoleWidth - snakeLeft[i].length() - snakeRight[i].length();
        cout << string(space, ' ');
        setColor(10);
        cout << snakeRight[i] << "\n";
        Sleep(70);
    }

    // Title
    setColor(14);
    cout << "\n";
    centerText("WELCOME TO THE KING COBRA'S DEN", consoleWidth);
    centerText("~~~~ SLITHER TO BEGIN YOUR HUNT ~~~~", consoleWidth);
    Beep(600, 150);
    Sleep(300);

    // Name prompt
    setColor(11);
    cout << "\n\n";
    centerText("ENTER YOUR NAME, BRAVE SNAKE:", consoleWidth);
    setColor(14);
    centerText(">>> ", consoleWidth);
    setColor(7);
    cout << flush;

    getline(cin, playerName);
    system("cls");
}


// ----- MAIN FUNCTION -----
int main() {
    srand(time(NULL));
    hideCursor();
    showStylishIntro();

    bool playAgain = true;
    while (playAgain) {
        string playerName;
        cin.ignore();
        promptPlayerName(playerName);

        int choice;
        Difficulty selected = selectDifficulty(choice);

        if (choice == 4) {
            runTwoPlayerMode();
        } else {
            Game game(40, 20, playerName, selected);
            if (choice == 5) {
                game.isTimedMode = true;  //  Enable timed mode
            }
            game.showWelcomeScreen();
            game.run();
            playAgain = game.gameOverScreen();
        }
    }

    return 0;
}
