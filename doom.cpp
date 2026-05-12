/*
===========================================
MINI DOOM STYLE GAME
ЧИСТЫЙ C++ БЕЗ БИБЛИОТЕК
===========================================

Управление:
W - вперед
S - назад
A - влево
D - вправо
Q/E - поворот
SPACE - стрельба

Цель:
- Убить врагов
- Найти выход 'X'

Компиляция:
g++ main.cpp -o game

Запуск:
./game

Windows:
g++ main.cpp -o game.exe
game.exe
*/

#include <iostream>
#include <vector>
#include <cmath>
#include <conio.h>
#include <windows.h>

using namespace std;

const int WIDTH = 120;
const int HEIGHT = 40;

const float FOV = 3.14159f / 4.0f;
const float DEPTH = 16.0f;
const float SPEED = 5.0f;

struct Enemy
{
    float x;
    float y;
    bool alive;
};

vector<Enemy> enemies =
{
    {8.5f, 8.5f, true},
    {14.5f, 5.5f, true},
    {18.5f, 15.5f, true}
};

string mapData;

float playerX = 2.0f;
float playerY = 2.0f;
float playerA = 0.0f;

bool running = true;

void initMap()
{
    mapData += "################";
    mapData += "#..............#";
    mapData += "#....######....#";
    mapData += "#..............#";
    mapData += "#..#####.......#";
    mapData += "#..............#";
    mapData += "#......#####...#";
    mapData += "#..............#";
    mapData += "#.....#........#";
    mapData += "#.....#........#";
    mapData += "#.....#####....#";
    mapData += "#..............#";
    mapData += "#..######......#";
    mapData += "#..............#";
    mapData += "#............X.#";
    mapData += "################";
}

bool wall(float x, float y)
{
    return mapData[(int)y * 16 + (int)x] == '#';
}

void shoot()
{
    for (auto& e : enemies)
    {
        if (!e.alive)
            continue;

        float dx = e.x - playerX;
        float dy = e.y - playerY;

        float dist = sqrt(dx * dx + dy * dy);

        float angle = atan2(dy, dx);

        float diff = fabs(playerA - angle);

        if (diff < 0.15f && dist < 8.0f)
        {
            e.alive = false;
        }
    }
}

int main()
{
    initMap();

    wchar_t* screen = new wchar_t[WIDTH * HEIGHT];

    HANDLE hConsole = CreateConsoleScreenBuffer(
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        CONSOLE_TEXTMODE_BUFFER,
        NULL
    );

    SetConsoleActiveScreenBuffer(hConsole);

    DWORD dwBytesWritten = 0;

    while (running)
    {
        // Movement
        if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
        {
            playerX += sin(playerA - 1.57f) * SPEED * 0.1f;
            playerY += cos(playerA - 1.57f) * SPEED * 0.1f;

            if (wall(playerX, playerY))
            {
                playerX -= sin(playerA - 1.57f) * SPEED * 0.1f;
                playerY -= cos(playerA - 1.57f) * SPEED * 0.1f;
            }
        }

        if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
        {
            playerX += sin(playerA + 1.57f) * SPEED * 0.1f;
            playerY += cos(playerA + 1.57f) * SPEED * 0.1f;

            if (wall(playerX, playerY))
            {
                playerX -= sin(playerA + 1.57f) * SPEED * 0.1f;
                playerY -= cos(playerA + 1.57f) * SPEED * 0.1f;
            }
        }

        if (GetAsyncKeyState((unsigned short)'W') & 0x8000)
        {
            playerX += sin(playerA) * SPEED * 0.1f;
            playerY += cos(playerA) * SPEED * 0.1f;

            if (wall(playerX, playerY))
            {
                playerX -= sin(playerA) * SPEED * 0.1f;
                playerY -= cos(playerA) * SPEED * 0.1f;
            }
        }

        if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
        {
            playerX -= sin(playerA) * SPEED * 0.1f;
            playerY -= cos(playerA) * SPEED * 0.1f;

            if (wall(playerX, playerY))
            {
                playerX += sin(playerA) * SPEED * 0.1f;
                playerY += cos(playerA) * SPEED * 0.1f;
            }
        }

        if (GetAsyncKeyState((unsigned short)'Q') & 0x8000)
            playerA -= 0.1f;

        if (GetAsyncKeyState((unsigned short)'E') & 0x8000)
            playerA += 0.1f;

        if (GetAsyncKeyState(VK_SPACE) & 0x0001)
            shoot();

        // Raycasting
        for (int x = 0; x < WIDTH; x++)
        {
            float rayAngle =
                (playerA - FOV / 2.0f) +
                ((float)x / (float)WIDTH) * FOV;

            float distanceToWall = 0.0f;

            bool hitWall = false;

            float eyeX = sin(rayAngle);
            float eyeY = cos(rayAngle);

            while (!hitWall && distanceToWall < DEPTH)
            {
                distanceToWall += 0.1f;

                int testX = (int)(playerX + eyeX * distanceToWall);
                int testY = (int)(playerY + eyeY * distanceToWall);

                if (testX < 0 || testX >= 16 ||
                    testY < 0 || testY >= 16)
                {
                    hitWall = true;
                    distanceToWall = DEPTH;
                }
                else
                {
                    if (mapData[testY * 16 + testX] == '#')
                        hitWall = true;
                }
            }

            int ceiling =
                (float)(HEIGHT / 2.0) -
                HEIGHT / ((float)distanceToWall);

            int floor = HEIGHT - ceiling;

            short shade = ' ';

            if (distanceToWall <= DEPTH / 4.0f)
                shade = 0x2588;
            else if (distanceToWall < DEPTH / 3.0f)
                shade = 0x2593;
            else if (distanceToWall < DEPTH / 2.0f)
                shade = 0x2592;
            else if (distanceToWall < DEPTH)
                shade = 0x2591;
            else
                shade = ' ';

            for (int y = 0; y < HEIGHT; y++)
            {
                if (y < ceiling)
                    screen[y * WIDTH + x] = ' ';
                else if (y > ceiling && y <= floor)
                    screen[y * WIDTH + x] = shade;
                else
                {
                    float b =
                        1.0f -
                        (((float)y - HEIGHT / 2.0f) /
                        ((float)HEIGHT / 2.0f));

                    if (b < 0.25)
                        shade = '#';
                    else if (b < 0.5)
                        shade = 'x';
                    else if (b < 0.75)
                        shade = '.';
                    else if (b < 0.9)
                        shade = '-';
                    else
                        shade = ' ';

                    screen[y * WIDTH + x] = shade;
                }
            }
        }

        // Draw enemies
        for (auto& e : enemies)
        {
            if (!e.alive)
                continue;

            float dx = e.x - playerX;
            float dy = e.y - playerY;

            float distance = sqrt(dx * dx + dy * dy);

            float angle = atan2(dx, dy) - playerA;

            if (fabs(angle) < FOV / 2.0f)
            {
                int enemyX =
                    (0.5f * (angle / (FOV / 2.0f)) + 0.5f)
                    * WIDTH;

                int enemyHeight = HEIGHT / distance;

                for (int y = -enemyHeight / 2;
                     y < enemyHeight / 2;
                     y++)
                {
                    int drawY = HEIGHT / 2 + y;

                    if (drawY >= 0 && drawY < HEIGHT)
                    {
                        screen[drawY * WIDTH + enemyX] = 'M';
                    }
                }
            }
        }

        // Crosshair
        screen[(HEIGHT / 2) * WIDTH + WIDTH / 2] = '+';

        // Victory
        int pX = (int)playerX;
        int pY = (int)playerY;

        if (mapData[pY * 16 + pX] == 'X')
        {
            system("cls");
            cout << "YOU WIN!" << endl;
            break;
        }

        // HUD
        swprintf_s(
            screen,
            80,
            L"X=%3.2f Y=%3.2f A=%3.2f ",
            playerX,
            playerY,
            playerA
        );

        screen[WIDTH * HEIGHT - 1] = '\0';

        WriteConsoleOutputCharacterW(
            hConsole,
            screen,
            WIDTH * HEIGHT,
            {0,0},
            &dwBytesWritten
        );
    }

    return 0;
}
