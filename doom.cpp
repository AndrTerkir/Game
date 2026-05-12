#include <windows.h>
#include <iostream>
#include <vector>
#include <cmath>

using namespace std;

const int screenWidth = 320;
const int screenHeight = 120;

const int mapWidth = 16;
const int mapHeight = 16;

float playerX = 3.0f;
float playerY = 3.0f;
float playerA = 0.0f;

float FOV = 3.14159f / 4.0f;
float depth = 16.0f;
float speed = 5.0f;

wstring map;

struct Enemy
{
    float x;
    float y;
    bool alive;
};

vector<Enemy> enemies =
{
    {8.0f, 8.0f, true},
    {10.0f, 13.0f, true},
    {13.0f, 5.0f, true}
};

bool IsWall(float x, float y)
{
    return map[(int)y * mapWidth + (int)x] == '#';
}

void Shoot()
{
    for (auto& e : enemies)
    {
        if (!e.alive)
            continue;

        float dx = e.x - playerX;
        float dy = e.y - playerY;

        float distance = sqrt(dx * dx + dy * dy);

        float angle = atan2(dy, dx);

        float diff = fabs(playerA - angle);

        if (diff < 0.15f && distance < 8.0f)
        {
            e.alive = false;
        }
    }
}

int main()
{
    map += L"################";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#.....####.....#";
    map += L"#..............#";
    map += L"#......#.......#";
    map += L"#......#.......#";
    map += L"#......#.......#";
    map += L"#..............#";
    map += L"#....#####.....#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#......##......#";
    map += L"#..............#";
    map += L"#............X.#";
    map += L"################";

    wchar_t* screen = new wchar_t[screenWidth * screenHeight];

    HANDLE console = CreateConsoleScreenBuffer(
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        CONSOLE_TEXTMODE_BUFFER,
        NULL
    );

    SetConsoleActiveScreenBuffer(console);

    DWORD bytesWritten = 0;

    auto tp1 = chrono::system_clock::now();

    while (true)
    {
        auto tp2 = chrono::system_clock::now();
        chrono::duration<float> elapsedTime = tp2 - tp1;
        tp1 = tp2;

        float dt = elapsedTime.count();

        // INPUT

        if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
            playerA -= 1.5f * dt;

        if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
            playerA += 1.5f * dt;

        float moveStep = speed * dt;

        if (GetAsyncKeyState((unsigned short)'W') & 0x8000)
        {
            playerX += sinf(playerA) * moveStep;
            playerY += cosf(playerA) * moveStep;

            if (IsWall(playerX, playerY))
            {
                playerX -= sinf(playerA) * moveStep;
                playerY -= cosf(playerA) * moveStep;
            }
        }

        if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
        {
            playerX -= sinf(playerA) * moveStep;
            playerY -= cosf(playerA) * moveStep;

            if (IsWall(playerX, playerY))
            {
                playerX += sinf(playerA) * moveStep;
                playerY += cosf(playerA) * moveStep;
            }
        }

        if (GetAsyncKeyState(VK_SPACE) & 0x0001)
            Shoot();

        // RENDER

        for (int x = 0; x < screenWidth; x++)
        {
            float rayAngle =
                (playerA - FOV / 2.0f) +
                ((float)x / (float)screenWidth) * FOV;

            float distanceToWall = 0.0f;
            bool hitWall = false;

            float eyeX = sinf(rayAngle);
            float eyeY = cosf(rayAngle);

            while (!hitWall && distanceToWall < depth)
            {
                distanceToWall += 0.05f;

                int testX = (int)(playerX + eyeX * distanceToWall);
                int testY = (int)(playerY + eyeY * distanceToWall);

                if (testX < 0 || testX >= mapWidth ||
                    testY < 0 || testY >= mapHeight)
                {
                    hitWall = true;
                    distanceToWall = depth;
                }
                else
                {
                    if (map[testY * mapWidth + testX] == '#')
                    {
                        hitWall = true;
                    }
                }
            }

            int ceiling =
                (float)(screenHeight / 2.0)
                - screenHeight / ((float)distanceToWall);

            int floor = screenHeight - ceiling;

            short shade = ' ';

            if (distanceToWall <= depth / 4.0f)
                shade = 0x2588;
            else if (distanceToWall < depth / 3.0f)
                shade = 0x2593;
            else if (distanceToWall < depth / 2.0f)
                shade = 0x2592;
            else if (distanceToWall < depth)
                shade = 0x2591;
            else
                shade = ' ';

            for (int y = 0; y < screenHeight; y++)
            {
                if (y <= ceiling)
                {
                    screen[y * screenWidth + x] = ' ';
                }
                else if (y > ceiling && y <= floor)
                {
                    screen[y * screenWidth + x] = shade;
                }
                else
                {
                    float b =
                        1.0f -
                        (((float)y - screenHeight / 2.0f)
                        / ((float)screenHeight / 2.0f));

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

                    screen[y * screenWidth + x] = shade;
                }
            }
        }

        // DRAW ENEMIES

        for (auto& e : enemies)
        {
            if (!e.alive)
                continue;

            float dx = e.x - playerX;
            float dy = e.y - playerY;

            float distance = sqrt(dx * dx + dy * dy);

            float angle =
                atan2(dy, dx) - playerA;

            while (angle > 3.14159f)
                angle -= 2.0f * 3.14159f;

            while (angle < -3.14159f)
                angle += 2.0f * 3.14159f;

            bool inFOV = fabs(angle) < FOV / 2.0f;

            if (inFOV && distance >= 0.5f)
            {
                int enemyCeiling =
                    (float)(screenHeight / 2.0)
                    - screenHeight / distance;

                int enemyFloor =
                    screenHeight - enemyCeiling;

                int enemyHeight =
                    enemyFloor - enemyCeiling;

                int enemyAspectWidth =
                    enemyHeight / 2;

                int middle =
                    (0.5f * (angle / (FOV / 2.0f)) + 0.5f)
                    * (float)screenWidth;

                for (int ex = 0; ex < enemyAspectWidth; ex++)
                {
                    for (int ey = 0; ey < enemyHeight; ey++)
                    {
                        int drawX =
                            middle + ex - enemyAspectWidth / 2;

                        int drawY =
                            enemyCeiling + ey;

                        if (drawX >= 0 &&
                            drawX < screenWidth &&
                            drawY >= 0 &&
                            drawY < screenHeight)
                        {
                            screen[drawY * screenWidth + drawX] = 'M';
                        }
                    }
                }
            }
        }

        // CROSSHAIR

        screen[(screenHeight / 2) * screenWidth + screenWidth / 2] = '+';

        // MINIMAP

        for (int nx = 0; nx < mapWidth; nx++)
        {
            for (int ny = 0; ny < mapHeight; ny++)
            {
                screen[(ny + 1) * screenWidth + nx] =
                    map[ny * mapWidth + nx];
            }
        }

        screen[((int)playerY + 1) * screenWidth + (int)playerX] = 'P';

        // WIN

        if (map[(int)playerY * mapWidth + (int)playerX] == 'X')
        {
            system("cls");
            cout << "YOU WIN!" << endl;
            break;
        }

        screen[screenWidth * screenHeight - 1] = '\0';

        WriteConsoleOutputCharacterW(
            console,
            screen,
            screenWidth * screenHeight,
            {0, 0},
            &bytesWritten
        );
    }

    return 0;
}
