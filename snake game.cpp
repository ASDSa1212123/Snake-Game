#include <iostream>
#include <conio.h>
#include <windows.h>
#include <ctime>
#include <vector>
#include <string>
#include <queue>
#include <stack>
#include <cmath>
#include <algorithm> // For find_if

using namespace std;

// ==========================================
//        DATA STRUCTURES & UTILS
// ==========================================

struct Point {
    int x, y;
    bool operator==(const Point& other) const { return x == other.x && y == other.y; }
};

enum Direction { STOP = 0, LEFT, RIGHT, UP, DOWN };
enum GameMode { CLASSIC = 1, TIME_ATTACK = 2 };
enum MapType { RECTANGLE = 1, CIRCLE = 2, TRIANGLE = 3 };

// ----- CONSOLE UTILS -----
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
    if(padding < 0) padding = 0;
    cout << string(padding, ' ') << text << "\n";
}

void playSound(const string& event) {
    if (event == "food") Beep(1000, 100);
    else if (event == "gameover") Beep(300, 400);
}

// ==========================================
//    [DSA CONCEPT: LOOKUP TABLE] MAPS
// ==========================================
class GameMap {
protected:
    int width, height;
    vector<vector<bool>> validArea; // The Lookup Table

public:
    GameMap(int w, int h) : width(w), height(h) {
        validArea.resize(height, vector<bool>(width, false));
    }
    virtual void generateMap() = 0;
    virtual string getName() = 0;

    // O(1) Access
    bool isValid(int x, int y) {
        if (x < 0 || x >= width || y < 0 || y >= height) return false;
        return validArea[y][x];
    }
    int getWidth() { return width; }
    int getHeight() { return height; }
};

class RectangularMap : public GameMap {
public:
    RectangularMap(int w, int h) : GameMap(w, h) {}
    void generateMap() override {
        for(int y=1; y<height-1; y++)
            for(int x=1; x<width-1; x++)
                validArea[y][x] = true;
    }
    string getName() override { return "Classic Box"; }
};

class CircularMap : public GameMap {
public:
    CircularMap(int w, int h) : GameMap(w, h) {}
    void generateMap() override {
        double h_center = width / 2.0;
        double k_center = height / 2.0;
        double a = (width / 2.0) - 2; 
        double b = (height / 2.0) - 1; 
        
        for(int y=0; y<height; y++) {
            for(int x=0; x<width; x++) {
                double val = (pow(x - h_center, 2) / pow(a, 2)) + (pow(y - k_center, 2) / pow(b, 2));
                if (val <= 1.0) validArea[y][x] = true;
            }
        }
    }
    string getName() override { return " The Colosseum "; }
};

class TriangularMap : public GameMap {
public:
    TriangularMap(int w, int h) : GameMap(w, h) {}
    void generateMap() override {
        double x1 = width / 2.0, y1 = 1;         // Top
        double x2 = 2, y2 = height - 2;          // Bottom Left
        double x3 = width - 3, y3 = height - 2;  // Bottom Right

        for(int y=0; y<height; y++) {
            for(int x=0; x<width; x++) {
                // Simple slope check for a upward pointing triangle
                double slopeL = (y2 - y1) / (x2 - x1);
                double slopeR = (y3 - y1) / (x3 - x1);
                
                bool leftCheck = x >= (x1 + (y - y1) / slopeL);
                bool rightCheck = x <= (x1 + (y - y1) / slopeR);
                bool bottomCheck = y <= y2;

                if (leftCheck && rightCheck && bottomCheck && y > 0) 
                    validArea[y][x] = true;
            }
        }
    }
    string getName() override { return "Pyramid of Doom"; }
};

// ==========================================
//    [DSA CONCEPT: LINKED LIST] SNAKE
// ==========================================
class Segment {
public:
    int x, y;
    Segment* next;
    Segment(int x, int y) : x(x), y(y), next(NULL) {}
};

class Snake {
private:
    Segment* head;
    Direction dir;
    int length;
    // [DSA CONCEPT: STACK] Move History
    stack<Direction> moveHistory; 

public:
    Snake(int x, int y) {
        head = new Segment(x, y);
        dir = STOP;
        length = 1;
        // Start with small body hanging down
        Segment* curr = head;
        for(int i=1; i<3; ++i) {
            curr->next = new Segment(x, y+i);
            curr = curr->next;
            length++;
        }
    }

    ~Snake() {
        Segment* current = head;
        while (current) {
            Segment* next = current->next;
            delete current;
            current = next;
        }
    }

    // --- FIX: Public accessor for the private stack size ---
    int getMoveCount() {
        return moveHistory.size();
    }
    // -----------------------------------------------------

    void move() {
        if (dir == STOP) return;
        moveHistory.push(dir); // Store history

        int newX = head->x;
        int newY = head->y;
        if (dir == LEFT) newX--;
        if (dir == RIGHT) newX++;
        if (dir == UP) newY--;
        if (dir == DOWN) newY++;

        // Add new head
        Segment* newHead = new Segment(newX, newY);
        newHead->next = head;
        head = newHead;

        // Remove tail
        Segment* temp = head;
        for (int i = 1; i < length; i++) temp = temp->next;
        delete temp->next;
        temp->next = NULL;
    }

    void grow() { length++; }

    bool isCollidingWithSelf() {
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
    
    vector<Point> getBody() {
        vector<Point> body;
        Segment* t = head;
        while(t) { body.push_back({t->x, t->y}); t=t->next; }
        return body;
    }
};

// ==========================================
//    [DSA CONCEPT: GRAPH BFS] FOOD
// ==========================================
class Food {
public:
    int x, y;

    // Checks if the food location is reachable from the snake's head
    // BFS considers Map Walls, Obstacles, and Snake Body
    bool isReachable(int startX, int startY, int targetX, int targetY, 
                     GameMap* map, const vector<Point>& obstacles, const vector<Point>& body) {
        
        vector<vector<bool>> visited(map->getHeight(), vector<bool>(map->getWidth(), false));

        // Mark obstacles/body as visited (blocked)
        for(const auto& p : obstacles) if(p.y < map->getHeight() && p.x < map->getWidth()) visited[p.y][p.x] = true;
        for(const auto& p : body) if(p.y < map->getHeight() && p.x < map->getWidth()) visited[p.y][p.x] = true;

        if (visited[targetY][targetX]) return false; // Target is inside obstacle

        queue<Point> q;
        q.push({startX, startY});
        visited[startY][startX] = true;

        int dx[] = {0, 0, 1, -1};
        int dy[] = {1, -1, 0, 0};

        while (!q.empty()) {
            Point curr = q.front(); q.pop();
            if (curr.x == targetX && curr.y == targetY) return true;

            for (int i = 0; i < 4; i++) {
                int nx = curr.x + dx[i];
                int ny = curr.y + dy[i];

                if (map->isValid(nx, ny) && !visited[ny][nx]) {
                    visited[ny][nx] = true;
                    q.push({nx, ny});
                }
            }
        }
        return false;
    }

    void respawn(GameMap* map, Snake* snake, const vector<Point>& obstacles) {
        bool valid = false;
        int attempts = 0;
        
        while (!valid && attempts < 100) {
            x = rand() % map->getWidth();
            y = rand() % map->getHeight();
            
            // 1. Must be in valid map area
            if (!map->isValid(x, y)) continue;

            // 2. Must not be on obstacle
            bool onObstacle = false;
            for(auto& o : obstacles) if(o.x == x && o.y == y) onObstacle = true;
            if(onObstacle) continue;

            // 3. Must not be on snake
            bool onSnake = false;
            vector<Point> body = snake->getBody();
            for(auto& b : body) if(b.x == x && b.y == y) onSnake = true;
            if(onSnake) continue;

            // 4. [DSA] BFS Check (Limit checks for performance)
            if (attempts < 5) {
                if(isReachable(snake->getHead()->x, snake->getHead()->y, x, y, map, obstacles, body))
                    valid = true;
            } else {
                valid = true; // Fallback
            }
            attempts++;
        }
    }
};

// ==========================================
//           MAIN GAME ENGINE
// ==========================================
class Game {
private:
    GameMap* map;
    Snake* snake;
    Food* food;
    vector<Point> obstacles;
    
    // [DSA CONCEPT: QUEUE] Input Buffer
    queue<int> inputQueue;

    GameMode mode;
    string playerName;
    bool gameOver;
    int score;
    double timeLeft;
    clock_t lastTime;
    int difficultySpeed;

    void generateObstacles(int count) {
        obstacles.clear();
        int placed = 0;
        int attempts = 0;
        while(placed < count && attempts < 1000) {
            int ox = rand() % map->getWidth();
            int oy = rand() % map->getHeight();
            
            // Obstacle must be inside valid map area
            // And not too close to center (where snake spawns)
            if(map->isValid(ox, oy) && (abs(ox - map->getWidth()/2) > 5 || abs(oy - map->getHeight()/2) > 5)) {
                obstacles.push_back({ox, oy});
                placed++;
            }
            attempts++;
        }
    }

public:
    Game(string name, GameMode gm, MapType mt, int diff) 
        : playerName(name), mode(gm) {
        
        int w = 50, h = 25;
        if (mt == RECTANGLE) map = new RectangularMap(w, h);
        else if (mt == CIRCLE) map = new CircularMap(w, h);
        else map = new TriangularMap(w, h);
        
        map->generateMap();
        snake = new Snake(w/2, h/2);
        
        // Difficulty controls speed and obstacle count
        difficultySpeed = (diff == 1) ? 100 : (diff == 2) ? 60 : 30;
        int obsCount = (diff == 1) ? 5 : (diff == 2) ? 15 : 25;
        generateObstacles(obsCount);

        food = new Food();
        food->respawn(map, snake, obstacles);

        score = 0;
        gameOver = false;
        timeLeft = (mode == TIME_ATTACK) ? 20.0 : 0.0;
        lastTime = clock();
    }

    ~Game() { delete map; delete snake; delete food; }

    void draw() {
        moveCursorToTopLeft();
        
        // --- HUD ---
        setColor(14); // Yellow
        cout << " PLAYER: " << playerName << " | SCORE: " << score << " ";
        if (mode == TIME_ATTACK) {
            setColor(timeLeft < 5.0 ? 12 : 11); // Red if low time
            cout << "| TIME: " << (int)timeLeft << "s  ";
        }
        cout << "\n";
        setColor(8); cout << " --------------------------------------------------\n";

        // --- MAP RENDERING ---
        for(int y=0; y<map->getHeight(); y++) {
            cout << " "; // Margin
            for(int x=0; x<map->getWidth(); x++) {
                
                // 1. Check Map Boundary (Lookup Table)
                if(!map->isValid(x, y)) {
                    setColor(8); cout << "."; // Void
                    continue;
                }

                // 2. Check Objects
                bool isObstacle = false;
                for(auto& o : obstacles) if(o.x == x && o.y == y) { isObstacle=true; break; }
                
                if(isObstacle) {
                    setColor(4); cout << "X"; // Red Obstacle
                }
                else if(x == snake->getHead()->x && y == snake->getHead()->y) {
                    setColor(10); cout << "O"; // Green Head
                }
                else if(x == food->x && y == food->y) {
                    setColor(13); cout << "@"; // Pink Food
                }
                else {
                    bool isBody = false;
                    vector<Point> body = snake->getBody();
                    // Skip head
                    for(size_t i=1; i<body.size(); i++) {
                        if(body[i].x == x && body[i].y == y) { isBody = true; break; }
                    }
                    
                    if(isBody) {
                        setColor(2); cout << "o"; 
                    } else {
                        setColor(7); cout << " "; 
                    }
                }
            }
            cout << "\n";
        }
    }

    void input() {
        if(_kbhit()) {
            inputQueue.push(_getch());
        }
    }

    void logic() {
        // Timer
        clock_t now = clock();
        double elapsed = double(now - lastTime) / CLOCKS_PER_SEC;
        lastTime = now;
        
        if(mode == TIME_ATTACK) {
            timeLeft -= elapsed;
            if(timeLeft <= 0) gameOver = true;
        }

        // Input Processing
        if(!inputQueue.empty()) {
            int k = inputQueue.front(); inputQueue.pop();
            switch(k) {
                case 'w': snake->setDirection(UP); break;
                case 's': snake->setDirection(DOWN); break;
                case 'a': snake->setDirection(LEFT); break;
                case 'd': snake->setDirection(RIGHT); break;
                case 'x': gameOver = true; break;
            }
        }

        snake->move();

        Segment* h = snake->getHead();

        // Check Map Collision
        if(!map->isValid(h->x, h->y)) gameOver = true;
        
        // Check Self Collision
        if(snake->isCollidingWithSelf()) gameOver = true;

        // Check Obstacle Collision
        for(auto& o : obstacles) if(o.x == h->x && o.y == h->y) gameOver = true;

        if(gameOver) return;

        // Eat Food
        if(h->x == food->x && h->y == food->y) {
            playSound("food");
            score += 10;
            snake->grow();
            if(mode == TIME_ATTACK) timeLeft += 3.0; // Bonus time
            food->respawn(map, snake, obstacles);
        }
    }

    void run() {
        while(!gameOver) {
            draw();
            input();
            logic();
            Sleep(difficultySpeed);
        }
        showGameOver();
    }

    string getRank(int s) {
        if (s < 50) return "Garden Worm (Weak)";
        if (s < 100) return "Grass Snake (Average)";
        if (s < 200) return "Python (Dangerous)";
        if (s < 300) return "Anaconda (Deadly)";
        return "LEGENDARY KING COBRA";
    }

    void showGameOver() {
        playSound("gameover");
        Sleep(500); // Dramatic pause
        system("cls");
        
        // Hide cursor just in case
        hideCursor();

        int cw = 60; // Center width referencing

        // --- FIX: Use the public function accessor instead of private variable ---
        int totalMoves = snake->getMoveCount(); 
        string rank = getRank(score);

        // 2. DRAW THE TOMBSTONE (ASCII ART)
        setColor(8); // Gray for stone
        cout << "\n\n";
        centerText("       _______       ", cw);
        centerText("      /       \\      ", cw);
        centerText("     /         \\     ", cw);
        centerText("    /           \\    ", cw);
        centerText("   |   R. I. P.  |   ", cw);
        centerText("   |             |   ", cw);
        centerText("   |  GAME OVER  |   ", cw);
        centerText("   |             |   ", cw);
        
        // Dynamic Player Name on Tombstone
        setColor(12); // Red text
        centerText("   | " + playerName + " |   ", cw);
        
        setColor(8); // Gray
        centerText("   |             |   ", cw);
        centerText("   |_____________|   ", cw);
        cout << "\n";

        // 3. STATS DISPLAY
        setColor(14); // Yellow
        centerText("-----------------------------", cw);
        centerText(" FINAL SCORE: " + to_string(score), cw);
        centerText(" SURVIVAL RANK: " + rank, cw);
        centerText("-----------------------------", cw);
        cout << "\n";

        // 4. DSA INSIGHTS
        setColor(11); // Cyan
        centerText("[ MEMORY DUMP ]", cw);
        centerText("Moves Stack Size: " + to_string(totalMoves), cw);
        centerText("Map Lookup Cycles: " + to_string(totalMoves * 1), cw); // O(1) per move
        
        // 5. DRAMATIC FOOTER
        cout << "\n";
        setColor(12); // Red
        if (mode == TIME_ATTACK && timeLeft <= 0) {
            centerText("CAUSE OF DEATH: TIME RAN OUT", cw);
        } else {
            centerText("CAUSE OF DEATH: COLLISION", cw);
        }

        cout << "\n\n";
        setColor(7); // White
        centerText("Press ANY KEY to return to Menu...", cw);
        _getch();
    }
};

// ==========================================
//          VISUAL MENUS
// ==========================================

void showStylishIntro() {
    system("cls");
    hideCursor();
    const int cw = 80;

    string art[] = {
        "   /^\\/^\\                                             ",
        " _|__|  O|                                            ",
        "\\/      /~ \\___      W E L C O M E   T O              ",
        " \\____|__________/  \\                                 ",
        "        \\_______      \\    S N A K E   M A S T E R    ",
        "                `\\     \\                              ",
        "                  |     |                                ",
        "                 /      /                                 "
    };

    setColor(10); // Green
    for(auto& s : art) { centerText(s, cw); Sleep(50); }
    
    setColor(14);
    cout << "\n";
    centerText("Press ANY KEY to Start...", cw);
    _getch();
}

int showMenu(string title, vector<string> opts) {
    system("cls");
    setColor(13); // Magenta
    cout << "\n === " << title << " === \n\n";
    setColor(7);
    for(size_t i=0; i<opts.size(); i++) {
        cout << " " << i+1 << ". " << opts[i] << "\n";
    }
    cout << "\n Choice: ";
    int c; cin >> c;
    return c;
}

int main() {
    srand(time(0));
    hideCursor();
    showStylishIntro();

    while(true) {
        system("cls");
        string name;
        setColor(11);
        cout << "\n Enter Player Name: "; 
        cin >> name;

        // 1. Select Mode
        int m = showMenu("SELECT MODE", {"Classic Survival", "Time Attack (Race Against Clock)"});
        GameMode mode = (m == 2) ? TIME_ATTACK : CLASSIC;

        // 2. Select Map
        int mp = showMenu("SELECT MAP SHAPE", {"Classic Box", "The Colosseum (Circle)", "Pyramid (Triangle)"});
        MapType mapType = (mp == 2) ? CIRCLE : (mp == 3) ? TRIANGLE : RECTANGLE;

        // 3. Select Difficulty
        int d = showMenu("SELECT DIFFICULTY", {"Easy (Slow, Few Obstacles)", "Medium (Normal)", "Hard (Fast, Many Obstacles)"});
        
        // Run Game
        system("cls");
        Game game(name, mode, mapType, d);
        game.run();

        // Replay?
        system("cls");
        setColor(14);
        cout << "\n Play Again? (y/n): ";
        char r; cin >> r;
        if(r == 'n' || r == 'N') break;
    }

    return 0;
}
