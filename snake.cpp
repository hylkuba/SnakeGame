#include <iostream>
#include <thread>
#include <vector>
#include <random>

#ifdef _WIN32
    #define CLEAR_SCREEN "cls"
#else
    #define CLEAR_SCREEN "clear"
#endif

#ifdef _WIN32
    #include <Windows.h>
    #include <conio.h>
#else
    #include <termios.h>
    #include <unistd.h>
#endif

#define WIDTH 100
#define HEIGHT 25

#define DELAY 75
#define TIMEOUT 1000

#define START_X 50
#define START_Y 12
#define INIT_LENGTH 1

// Moves cursor to specific location
void moveCursor(int x, int y) {
    std::cout << "\033[" << y << ";" << x << "H";
}

int generateRandomNumber(int min, int max) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(min, max);

    return dis(gen);
}

void resetTextFormat() {
    std::cout << "\033[0m";
}

struct TSnake {
    TSnake() : m_x(START_X), m_y(START_Y), m_length(INIT_LENGTH) {
        for (int i = 0; i < m_length; i++) {
            prevPos.push_back(std::make_pair(m_x, m_y));
        }
        lastPosToClear = std::make_pair(m_x, m_y);
    }

    void move(std::pair<int, int> val) {
        m_x += val.first;
        m_y += val.second;

        prevPos.push_back(std::make_pair(m_x, m_y));

        // Remove the first element and store the position to be cleaned
        if (!prevPos.empty()) {
            lastPosToClear = prevPos[0];
            prevPos.erase(prevPos.begin());
        }
    }

    void eat() {
        m_length++;
        std::pair<int, int> tmp;
        if(!prevPos.empty()) {
            tmp = prevPos[0];
            prevPos.insert(prevPos.begin(), std::make_pair(tmp.first, tmp.second));
        }

        moveCursor(7, HEIGHT + 1);
        for(int i = 0; i < WIDTH / 2; i++) {
            std::cout << " ";
        } 
        moveCursor(8, HEIGHT + 1);
        std::cout << m_length;
    }

    void print() {
        // Set Color to green
        std::cout << "\033[0;32m";

        for(const auto &pos : prevPos) {
            moveCursor(pos.first, pos.second);
            std::cout << "@";
        }
        moveCursor(lastPosToClear.first, lastPosToClear.second);
        std::cout << " ";

        resetTextFormat();
    }

    bool checkTailHit() {
        if(m_length == 1) return false;

        for(size_t i = 0; i < prevPos.size() - 1; i++) {
            if(prevPos[i] == std::make_pair(m_x, m_y)) return true;
        }
        return false;
    }

    bool outOfBounds() {
        return (m_x >= WIDTH) || (m_x <= 1) ||
                (m_y >= HEIGHT) || (m_y <= 1); 
    }

    std::pair<int, int> getPos() {
        return std::make_pair(m_x, m_y);
    }

    bool inBody(std::pair<int, int> pos) {
        for(const auto &it : prevPos) {
            if(it == pos) return true;
        }
        return false;
    }

private:
    int m_x;
    int m_y;
    int m_length;

    std::vector<std::pair<int, int>> prevPos;

    std::pair<int, int> lastPosToClear;
};

struct TFruit {
    void generate() {
        m_x = generateRandomNumber(2, WIDTH - 1);
        m_y = generateRandomNumber(2, HEIGHT - 1);
    }

    std::pair<int, int> getPos() {
        return std::make_pair(m_x, m_y);
    }

    void print() {
        moveCursor(m_x, m_y);

        std::cout << "\033[0;33m";
        std::cout << "o";

        resetTextFormat();
    }

private:
    int m_x;
    int m_y;
};

void hideCursor() {
    std::cout << "\033[?25l";
}

void showCursor() {
    std::cout << "\033[?25h";
}

void printBorders() {
    // Set color for borders to Dark red
    std::cout << "\033[0;31m";

    for (int i = 0; i < WIDTH; i++) {
        std::cout << "#";
    }
    std::cout << std::endl;
    
    for (int i = 0; i < HEIGHT; i++) {
       moveCursor(0, i);
       std::cout << "#";
       moveCursor(WIDTH, i);
       std::cout << "#";
    }

    moveCursor(0, HEIGHT);
    for (int i = 0; i < WIDTH; i++) {
        std::cout << "#";
    }

    resetTextFormat();
}

bool isLeftArrowPressed() {
#ifdef _WIN32
    SHORT keyState = GetAsyncKeyState(VK_LEFT);

    // Check if the key is currently pressed (high-order bit set)
    if (keyState & 0x8000) {
        // Reset the value (clear the high-order bit)
        keyState &= 0x7FFF;
        return true;
    }
    return false;
#else
    struct termios oldt, newt;
    int ch;
    bool result = false;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    setbuf(stdin, NULL);  // Disable input buffering

    ch = getchar();
    if (ch == 27) {
        ch = getchar();
        if (ch == 91) {
            ch = getchar();
            result = (ch == 68); // Check for left arrow key
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return result;
#endif
}

bool isRightArrowPressed() {
#ifdef _WIN32
    // Get the state of the right arrow key
    SHORT keyStateRight = GetAsyncKeyState(VK_RIGHT);

    // Check if the key is currently pressed (high-order bit set)
    if (keyStateRight & 0x8000) {
        // Reset the value (clear the high-order bit)
        keyStateRight &= 0x7FFF;
        return true;
    }
    return false;
#else
    struct termios oldt, newt;
    int ch;
    bool result = false;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    setbuf(stdin, NULL);  // Disable input buffering

    ch = getchar();
    if (ch == 27) {
        ch = getchar();
        if (ch == 91) {
            ch = getchar();
            result = (ch == 67); // Check for right arrow key
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return result;
#endif
}

void rotate(std::pair<int, int> &move, int rotation) {
    if(rotation < 0) {
        // LEFT
        if(move == std::make_pair(0, 1)) {
            // DOWN -> RIGHT
            move = {1, 0};
        } else if(move == std::make_pair(0, -1)) {
            // UP -> LEFT
            move = {-1, 0};
        } else if(move == std::make_pair(1, 0)) {
            // RIGHT -> UP
            move = {0, -1};
        } else if(move == std::make_pair(-1, 0)) {
            // LEFT -> DOWN
            move = {0, 1};
        }
    } else {
        // RIGHT
        if(move == std::make_pair(0, 1)) {
            // DOWN -> LEFT
            move = {-1, 0};
        } else if(move == std::make_pair(0, -1)) {
            // UP -> RIGHT
            move = {1, 0};
        } else if(move == std::make_pair(1, 0)) {
            // RIGHT -> DOWN
            move = {0, 1};
        } else if(move == std::make_pair(-1, 0)) {
            // LEFT -> UP
            move = {0, -1};
        }
    }
}

int main(void) {
    hideCursor();
    std::system(CLEAR_SCREEN);

    printBorders();

    TSnake snake;

    // Set initial move to go UP
    std::pair<int, int> move {0, -1};

    TFruit fruit;

    do {
        fruit.generate();
    } while(snake.inBody(fruit.getPos()));

    fruit.print();

    moveCursor(0, HEIGHT + 1);
    std::cout << "Score: " << 1;

    // Game cycle
    while(true) {
        snake.move(move);

        if(snake.outOfBounds() || (snake.checkTailHit() && snake.getPos() != fruit.getPos())) {
            std::system(CLEAR_SCREEN);
            std::cout << "You died!!" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(TIMEOUT));
            break;
        }

        snake.print();

        if(snake.getPos() == fruit.getPos()) {
            snake.eat();

            do {
                fruit.generate();
            } while(snake.inBody(fruit.getPos()));

            fruit.print();
        }

        if(isLeftArrowPressed()) {
            rotate(move, -1);
        } else if(isRightArrowPressed()) {
            rotate(move, 1);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));
    }

    std::system(CLEAR_SCREEN);
    showCursor();
    return 0;
}