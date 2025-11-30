#include <raylib.h>
#include <spdlog/spdlog.h>
#include <random>
#include <vector>
#include <cmath>

using namespace std;

#define WIDTH 800
#define HEIGHT 900
#define GRIDSIZE 36
#define CELL_SIZE 20
#define BACKGROUND CLITERAL(Color) {172, 204, 102, 255}

// #define DEBUG

unsigned int score = 0;

class Grid {
public:
    const int cellSize = CELL_SIZE;
    const int cells = GRIDSIZE;
    const int thickness = 4;
    const int marginX = 40;
    const int marginY = 60;

    int fieldW = cells * cellSize;
    int fieldH = cells * cellSize;

    int outerX = marginX;
    int outerY = marginY;
    int innerX = outerX + thickness;
    int innerY = outerY + thickness;

    void Draw() const {
        // Draw borders
        DrawRectangle(outerX, outerY, fieldW + thickness*2, thickness, BLACK);
        DrawRectangle(outerX, outerY + fieldH + thickness, fieldW + thickness*2, thickness, BLACK);
        DrawRectangle(outerX, outerY, thickness, fieldH + thickness*2, BLACK);
        DrawRectangle(outerX + fieldW + thickness, outerY, thickness, fieldH + thickness*2, BLACK);

#ifdef DEBUG
        for (int x = innerX; x <= innerX + fieldW; x += cellSize) {
            DrawLine(x, innerY, x, innerY + fieldH, DARKGRAY);
        }
        for (int y = innerY; y <= innerY + fieldH; y += cellSize) {
            DrawLine(innerX, y, innerX + fieldW, y, DARKGRAY);
        }
#endif
    }

    bool isInGrid(const int headX, const int headY) const {
        return !(headX < innerX || headY < innerY || headX >= innerX + fieldW || headY >= innerY + fieldH);
    }
};

class Snake {
protected:
    void DrawSegment(const int x, const int y) const {
        DrawRectangleRounded(Rectangle{static_cast<float>(x), static_cast<float>(y), (float)CELL_SIZE, (float)CELL_SIZE}, 0.5f, 8, DARKGRAY);
    }
public:
    int headX, headY;
    int velocityX = 1;
    int velocityY = 0;
    int speed = CELL_SIZE;
    vector<Vector2> body;
    KeyboardKey lastKeyPressed = KEY_RIGHT;

    void Draw() const {
        // Draw Body
        for (const auto &pos : body) {
            DrawSegment((int)pos.x, (int)pos.y);
        }
        // Draw Head
        DrawSegment(headX, headY);
    }

    bool collidingItself() const {
        for (const auto &part : body) {
            if (headX == static_cast<int>(part.x) && headY == static_cast<int>(part.y))
                return true;
        }
        return false;
    }

    void Update() {
        // Input Handling
        if (IsKeyDown(KEY_UP) && lastKeyPressed != KEY_DOWN) { velocityX = 0; velocityY = -1; lastKeyPressed = KEY_UP; }
        else if (IsKeyDown(KEY_DOWN) && lastKeyPressed != KEY_UP) { velocityX = 0; velocityY = 1; lastKeyPressed = KEY_DOWN; }
        else if (IsKeyDown(KEY_LEFT) && lastKeyPressed != KEY_RIGHT) { velocityX = -1; velocityY = 0; lastKeyPressed = KEY_LEFT; }
        else if (IsKeyDown(KEY_RIGHT) && lastKeyPressed != KEY_LEFT) { velocityX = 1; velocityY = 0; lastKeyPressed = KEY_RIGHT; }

        if (!body.empty()) {
            for (int i = body.size() - 1; i > 0; --i) {
                body[i] = body[i - 1];
            }
            body[0] = Vector2{static_cast<float>(headX), static_cast<float>(headY)};
        }

        headX += velocityX * speed;
        headY += velocityY * speed;
    }

    void Grow() {
        if (body.empty()) {
            body.push_back(Vector2{static_cast<float>(headX), static_cast<float>(headY)});
        } else {
            body.push_back(body.back());
        }
    }
};

class Food {
public:
    int cellX;
    int cellY;
    const int size = 15;

    void SpawnFood(const Grid &grid) {
        static random_device rd;
        static mt19937 gen(rd());
        uniform_int_distribution<int> distInt(0, grid.cells - 1);

        int x = distInt(gen);
        int y = distInt(gen);

        float offset = (grid.cellSize - size) / 2.0f;
        cellX = grid.innerX + (x * grid.cellSize) + static_cast<int>(offset);
        cellY = grid.innerY + (y * grid.cellSize) + static_cast<int>(offset);
    }

    void Draw() const {
        DrawRectangleRounded(Rectangle{static_cast<float>(cellX), static_cast<float>(cellY), (float)size, (float)size}, 0.5f, 4, RED);
    }

    bool FoodEaten(const int headX, const int headY) const {
        return abs(headX - cellX) < CELL_SIZE && abs(headY - cellY) < CELL_SIZE;
    }
};

Grid grid;
Snake snake;
Food food;

int main() {
    InitWindow(WIDTH, HEIGHT, "Snake in RayLib");
    InitAudioDevice();

    Sound collect = LoadSound("../collect.mp3");

    SetTargetFPS(8);

    bool lost = false;
    bool won = false;

    snake.headX = grid.innerX + (5 * grid.cellSize);
    snake.headY = grid.innerY + (5 * grid.cellSize);

    snake.body.push_back(Vector2{static_cast<float>(snake.headX - grid.cellSize), static_cast<float>(snake.headY)});

    food.SpawnFood(grid);

    // Game loop
    while (!WindowShouldClose() && !lost && !won) {
        // 1. Update Snake
        snake.Update();

        // 2. Logic
        if (!grid.isInGrid(snake.headX, snake.headY)) {
            lost = true;
        } else if (snake.collidingItself()) {
            lost = true;
        } else if (snake.body.size() >= (GRIDSIZE * GRIDSIZE) - 1) {
            won = true;
        }

        // Food check
        if (food.FoodEaten(snake.headX, snake.headY)) {
            if(IsAudioDeviceReady()) PlaySound(collect);
            spdlog::info("Snake ate food. Score: {}", score + 1);
            snake.Grow();
            food.SpawnFood(grid);
            score++;
        }

        // 3. Draw
        BeginDrawing();
        ClearBackground(BACKGROUND);

        DrawText("Retro Snake", 40, 25, 40, BLACK);
        DrawText(TextFormat("Score: %i", score), 40, HEIGHT - 50, 30, BLACK);

        grid.Draw();
        food.Draw();
        snake.Draw();

        EndDrawing();
    }

    // Endscreen
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BACKGROUND);

        const char* msg = lost ? "You Lost" : "You Won!";
        Color msgColor = lost ? RED : DARKGREEN;

        int textWidth = MeasureText(msg, 50);
        DrawText(msg, WIDTH/2 - textWidth/2, HEIGHT/2 - 50, 50, msgColor);

        const char* scoreMsg = TextFormat("Final Score: %i", score);
        int scoreWidth = MeasureText(scoreMsg, 30);
        DrawText(scoreMsg, WIDTH/2 - scoreWidth/2, HEIGHT/2 + 20, 30, BLACK);

        DrawText("Press ESC to Close", WIDTH/2 - 100, HEIGHT/2 + 80, 20, DARKGRAY);

        EndDrawing();
    }

    // Close everything
    UnloadSound(collect);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}