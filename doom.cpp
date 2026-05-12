#include <windows.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>

using namespace std;

const int SCREEN_WIDTH = 240;
const int SCREEN_HEIGHT = 80;

const int MAP_WIDTH = 16;
const int MAP_HEIGHT = 16;

float playerX = 3.0f;
float playerY = 3.0f;
float playerA = 0.0f;

float FOV = 3.14159f / 4.0f;
float DEPTH = 16.0f;
float SPEED = 5.0f;

wstring mapData;

struct Enemy
{
    float x;
    float y;
    bool alive;
};

vector<Enemy> enemies =
{
    {8.0f, 8.0f, true},
    {12.0f, 10.0f, true},
    {6.0f, 13.0f, true}
};

bool IsWall(float x, float y)
{
    if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT)
        return true;

    return mapData[(int)y * MAP_WIDTH + (int)x] == '#';
}

void InitMap()
{
    mapData += L"################";
    mapData += L"#..............#";
    mapData += L"#....######....#";
    mapData += L"#..............#";
    mapData += L"#..####........#";
    mapData += L"#..............#";
    mapData += L"#......####....#";
    mapData += L"#..............#";
    mapData += L"#....#.........#";
    mapData += L"#....#.........#";
    mapData += L"#....#####.....#";
    mapData += L"#..............#";
    mapData += L"#......##......#";
    mapData += L"#..............#";
    mapData += L"#............X.#";
    mapData += L"################";
}

void Shoot()
{
    for (auto& e : enemies)
    {
        if (!e.alive)
            continue;

        float dx = e.x - playerX;
        float dy = e.y - playerY;

        float dist = sqrt(dx * dx + dy * dy);

        float angle = atan2(dy, dx);

        float diff = fabs(angle - playerA);

        while (diff > 3.14159f)
            diff -= 6.28318f;

        diff = fabs(diff);

        if (diff < 0.12f && dist < 10.0f)
        {
            e.alive = false;
        }
    }
}

int main()
{
    InitMap();

    wchar_t* screen = new wchar_t[SCREEN_WIDTH * SCREEN_HEIGHT];

    HANDLE console = CreateConsoleScreenBuffer(
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        CONSOLE_TEXTMODE_BUFFER,
        NULL
    );

    SetConsoleActiveScreenBuffer(console);

    DWORD bytesWritten = 0;

    vector<float> depthBuffer(SCREEN_WIDTH);

    auto tp1 = chrono::high_resolution_clock::now();

    while (true)
    {
        auto tp2 = chrono::high_resolution_clock::now();
        chrono::duration<float> elapsed = tp2 - tp1;
        tp1 = tp2;

        float dt = elapsed.count();

        if (GetAsyncKeyState('A') & 0x8000)
            playerA -= 1.8f * dt;

        if (GetAsyncKeyState('D') & 0x8000)
            playerA += 1.8f * dt;

        float moveStep = SPEED * dt;

        if (GetAsyncKeyState('W') & 0x8000)
        {
            float nx = playerX + sinf(playerA) * moveStep;
            float ny = playerY + cosf(playerA) * moveStep;

            if (!IsWall(nx, ny))
            {
                playerX = nx;
                playerY = ny;
            }
        }

        if (GetAsyncKeyState('S') & 0x8000)
        {
            float nx = playerX - sinf(playerA) * moveStep;
            float ny = playerY - cosf(playerA) * moveStep;

            if (!IsWall(nx, ny))
            {
                playerX = nx;
                playerY = ny;
            }
        }

        if (GetAsyncKeyState(VK_SPACE) & 0x0001)
            Shoot();

        for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++)
            screen[i] = ' ';

        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            float rayAngle =
                (playerA - FOV / 2.0f)
                + ((float)x / (float)SCREEN_WIDTH) * FOV;

            float distanceToWall = 0.0f;

            bool hitWall = false;

            float eyeX = sinf(rayAngle);
            float eyeY = cosf(rayAngle);

            while (!hitWall && distanceToWall < DEPTH)
            {
                distanceToWall += 0.03f;

                int testX = (int)(playerX + eyeX * distanceToWall);
                int testY = (int)(playerY + eyeY * distanceToWall);

                if (testX < 0 || testX >= MAP_WIDTH ||
                    testY < 0 || testY >= MAP_HEIGHT)
                {
                    hitWall = true;
                    distanceToWall = DEPTH;
                }
                else if (mapData[testY * MAP_WIDTH + testX] == '#')
                {
                    hitWall = true;
                }
            }

            depthBuffer[x] = distanceToWall;

            int ceiling =
                (SCREEN_HEIGHT / 2.0)
                - SCREEN_HEIGHT / distanceToWall;

            int floor = SCREEN_HEIGHT - ceiling;

            wchar_t shade;

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

            for (int y = 0; y < SCREEN_HEIGHT; y++)
            {
                if (y < ceiling)
                {
                    screen[y * SCREEN_WIDTH + x] = ' ';
                }
                else if (y >= ceiling && y <= floor)
                {
                    screen[y * SCREEN_WIDTH + x] = shade;
                }
                else
                {
                    float b = 1.0f -
                        (((float)y - SCREEN_HEIGHT / 2.0f)
                        / ((float)SCREEN_HEIGHT / 2.0f));

                    wchar_t floorShade;

                    if (b < 0.25)
                        floorShade = '#';
                    else if (b < 0.5)
                        floorShade = 'x';
                    else if (b < 0.75)
                        floorShade = '.';
                    else if (b < 0.9)
                        floorShade = '-';
                    else
                        floorShade = ' ';

                    screen[y * SCREEN_WIDTH + x] = floorShade;
                }
            }
        }

        for (auto& e : enemies)
        {
            if (!e.alive)
                continue;

            float vecX = e.x - playerX;
            float vecY = e.y - playerY;

            float distance = sqrt(vecX * vecX + vecY * vecY);

            float objectAngle = atan2(vecY, vecX) - playerA;

            while (objectAngle < -3.14159f)
                objectAngle += 6.28318f;

            while (objectAngle > 3.14159f)
                objectAngle -= 6.28318f;

            bool inFOV = fabs(objectAngle) < FOV / 2.0f;

            if (inFOV && distance >= 0.5f && distance < DEPTH)
            {
                int enemyCeiling =
                    (float)(SCREEN_HEIGHT / 2.0)
                    - SCREEN_HEIGHT / distance;

                int enemyFloor = SCREEN_HEIGHT - enemyCeiling;

                int enemyHeight = enemyFloor - enemyCeiling;
                int enemyWidth = enemyHeight / 2;

                int middleX =
                    (0.5f * (objectAngle / (FOV / 2.0f)) + 0.5f)
                    * (float)SCREEN_WIDTH;

                for (int ex = 0; ex < enemyWidth; ex++)
                {
                    int drawX = middleX + ex - enemyWidth / 2;

                    if (drawX >= 0 && drawX < SCREEN_WIDTH)
                    {
                        if (depthBuffer[drawX] >= distance)
                        {
                            for (int ey = 0; ey < enemyHeight; ey++)
                            {
                                int drawY = enemyCeiling + ey;

                                if (drawY >= 0 && drawY < SCREEN_HEIGHT)
                                {
                                    screen[drawY * SCREEN_WIDTH + drawX] = 'M';
                                }
                            }
                        }
                    }
                }
            }
        }

        screen[(SCREEN_HEIGHT / 2) * SCREEN_WIDTH + SCREEN_WIDTH / 2] = '+';

        for (int mx = 0; mx < MAP_WIDTH; mx++)
        {
            for (int my = 0; my < MAP_HEIGHT; my++)
            {
                screen[(my + 1) * SCREEN_WIDTH + mx] =
                    mapData[my * MAP_WIDTH + mx];
            }
        }

        screen[((int)playerY + 1) * SCREEN_WIDTH + (int)playerX] = 'P';

        if (mapData[(int)playerY * MAP_WIDTH + (int)playerX] == 'X')
        {
            system("cls");
            cout << "YOU WIN!" << endl;
            break;
        }

        screen[SCREEN_WIDTH * SCREEN_HEIGHT - 1] = '\0';

        WriteConsoleOutputCharacterW(
            console,
            screen,
            SCREEN_WIDTH * SCREEN_HEIGHT,
            {0, 0},
            &bytesWritten
        );
    }

    return 0;
}
