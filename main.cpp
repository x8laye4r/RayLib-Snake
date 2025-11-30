#include <raylib.h>
#include <spdlog/spdlog.h>
#include <random>
#include <vector>
#include <cmath>
using namespace std;

#define WIDTH 800
#define HEIGHT 900
#define GRIDSIZE 36
#define BACKGROUND CLITERAL(Color) {172, 204, 102, 255}

// #define DEBUG

unsigned int score = 0;

class Grid {
public:
    const int cellSize = 20;
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
        return !(headX < outerX || headY < outerY || headX >= outerX + fieldW || headY >= outerY + fieldH);
    }
};

class Snake {
protected:
    void DrawSegment(const int x, const int y) const {
        DrawRectangleRounded(Rectangle{static_cast<float>(x), static_cast<float>(y), 20, 20}, 0.5f, 8, DARKGRAY);
    }
public:
    int headX, headY;
    int velocityX = 1;
    int velocityY = 0;
    int speed = 20;
    vector<Vector2> body;
    KeyboardKey key = KEY_RIGHT;

    void DrawBody() const {
        for (auto &pos : body) {
            DrawSegment(pos.x, pos.y);
        }
    }

    bool collidingItself() const {
        for (int i = 0; i < body.size(); i++) {
            if (headX == body[i].x && headY == body[i].y)
                return true;
        }
        return false;
    }

    void Update() {
        // Richtung prüfen
        if (IsKeyDown(KEY_UP) && key != KEY_DOWN) { velocityX = 0; velocityY = -1; key = KEY_UP; }
        else if (IsKeyDown(KEY_DOWN) && key != KEY_UP) { velocityX = 0; velocityY = 1; key = KEY_DOWN; }
        else if (IsKeyDown(KEY_LEFT) && key != KEY_RIGHT) { velocityX = -1; velocityY = 0; key = KEY_LEFT; }
        else if (IsKeyDown(KEY_RIGHT) && key != KEY_LEFT) { velocityX = 1; velocityY = 0; key = KEY_RIGHT; }

        // Body verschieben
        if (!body.empty()) {
            for (int i = body.size() - 1; i > 0; --i) {
                body[i] = body[i - 1];
            }
            body[0] = Vector2{static_cast<float>(headX), static_cast<float>(headY)};
        }

        // Kopf bewegen
        headX += velocityX * speed;
        headY += velocityY * speed;
    }

    void Grow() {
        if (!body.empty()) {
            Vector2 tail = body.back();
            body.push_back(tail);
        }
    }
};

class Food {
public:
    int cellX;
    int cellY;

    void SpawnFood(const Grid &grid) {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<int> distInt(0, grid.cells - 1);

        int x = distInt(gen);
        int y = distInt(gen);

        cellX = grid.innerX + (x * grid.cellSize) + 2.5;
        cellY = grid.innerY + (y * grid.cellSize) + 2.5;
    }

    void Draw() {
        DrawRectangleRounded(Rectangle{static_cast<float>(cellX), static_cast<float>(cellY), 15, 15}, 5, 0.8f, RED);
    }

    bool FoodEaten(const int headX, const int headY) const {
        return abs(headX - cellX) < 20 && abs(headY - cellY) < 20;
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

    snake.headX = grid.marginX + grid.thickness;
    snake.headY = grid.marginY + grid.thickness;
    snake.body.push_back(Vector2{static_cast<float>(snake.headX), static_cast<float>(snake.headY)});

    food.SpawnFood(grid);

    while (!WindowShouldClose() && (!lost && !won)) {
        if (!grid.isInGrid(snake.headX, snake.headY)) lost = true;
        else if (snake.body.size() == grid.cellSize) won = true;
        snake.Update();
        if (snake.collidingItself()) lost = true;

        if (food.FoodEaten(snake.headX, snake.headY)) {
            PlaySound(collect);
            spdlog::info("Snake ate food");
            snake.Grow();
            food.SpawnFood(grid);
            score++;
        }

        BeginDrawing();
        ClearBackground(BACKGROUND);

        DrawText("Retro Snake", 40, 25, 40, BLACK);
        DrawText(TextFormat("Score: %i", score), 40, HEIGHT - 110, 40, BLACK);

        grid.Draw();
        food.Draw();
        snake.DrawBody();
        EndDrawing();
    }
    while (!WindowShouldClose()) {
        BeginDrawing();
        if (lost) DrawText(TextFormat("You Lost\nScore: %i", score), WIDTH/2 - 40, HEIGHT/2 - 40, 40, WHITE);
        else if (won) DrawText(TextFormat("You Won\nScore: %i", score), WIDTH/2 - 40, HEIGHT/2 - 40, 40, WHITE);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
