#include <windows.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>

using namespace std;

const int W = 240;
const int H = 80;

const int MW = 16;
const int MH = 16;

float px = 3.0f, py = 3.0f;
float pa = 0.0f;

float FOV = 3.14159f / 4.0f;
float DEPTH = 16.0f;
float SPEED = 5.0f;

// ===== MAP (0 = empty, 1 = wall, 2-4 = rooms) =====
int mapData[MW * MH] =
{
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,
    1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,
    1,2,2,2,1,1,1,2,3,3,3,3,3,2,2,1,
    1,2,2,2,1,0,1,2,3,3,3,3,3,2,2,1,
    1,2,2,2,1,0,1,2,3,3,3,3,3,2,2,1,
    1,2,2,2,1,0,1,2,3,3,3,3,3,2,2,1,
    1,2,2,2,1,0,1,2,2,2,2,2,2,2,2,1,
    1,2,2,2,1,0,1,1,1,1,1,2,2,2,2,1,
    1,2,2,2,1,0,0,0,0,0,1,2,2,2,2,1,
    1,2,2,2,1,1,1,1,1,0,1,2,2,2,2,1,
    1,2,2,2,2,2,2,2,1,0,1,2,2,2,2,1,
    1,2,2,2,2,2,2,2,1,0,1,2,2,2,2,1,
    1,2,2,2,2,2,2,2,1,0,1,2,2,2,2,1,
    1,2,2,2,2,2,2,2,1,4,4,4,4,4,4,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

// ===== SECTORS =====
struct Sector
{
    float floor;
    float ceil;
};

Sector sectors[5] =
{
    {0, 0},     // 0 empty
    {0, 0},     // 1 walls
    {0.0f, 3.0f}, // room 2 (low)
    {0.0f, 5.0f}, // room 3 (tall)
    {0.0f, 1.5f}  // room 4 (corridor low ceiling)
};

bool isWall(int x, int y)
{
    if (x < 0 || x >= MW || y < 0 || y >= MH) return true;
    return mapData[y * MW + x] == 1;
}

int getSector(int x, int y)
{
    if (x < 0 || x >= MW || y < 0 || y >= MH) return 1;
    return mapData[y * MW + x];
}

int main()
{
    wchar_t* screen = new wchar_t[W * H];

    HANDLE h = CreateConsoleScreenBuffer(
        GENERIC_READ | GENERIC_WRITE,
        0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);

    SetConsoleActiveScreenBuffer(h);

    DWORD written = 0;

    auto tp1 = chrono::high_resolution_clock::now();

    while (true)
    {
        auto tp2 = chrono::high_resolution_clock::now();
        float dt = chrono::duration<float>(tp2 - tp1).count();
        tp1 = tp2;

        if (GetAsyncKeyState('A')) pa -= 2.0f * dt;
        if (GetAsyncKeyState('D')) pa += 2.0f * dt;

        float ms = SPEED * dt;

        if (GetAsyncKeyState('W'))
        {
            float nx = px + sinf(pa) * ms;
            float ny = py + cosf(pa) * ms;
            if (!isWall(nx, ny)) px = nx, py = ny;
        }

        if (GetAsyncKeyState('S'))
        {
            float nx = px - sinf(pa) * ms;
            float ny = py - cosf(pa) * ms;
            if (!isWall(nx, ny)) px = nx, py = ny;
        }

        for (int i = 0; i < W * H; i++)
            screen[i] = ' ';

        vector<float> zBuffer(W);

        // ===== RAYCAST =====
        for (int x = 0; x < W; x++)
        {
            float rayA = (pa - FOV / 2.0f) + ((float)x / W) * FOV;

            float dist = 0;
            bool hit = false;

            float ox = sinf(rayA);
            float oy = cosf(rayA);

            int sectorHit = 1;

            while (!hit && dist < DEPTH)
            {
                dist += 0.02f;

                int tx = (int)(px + ox * dist);
                int ty = (int)(py + oy * dist);

                if (tx < 0 || tx >= MW || ty < 0 || ty >= MH)
                {
                    hit = true;
                    dist = DEPTH;
                }
                else if (isWall(tx, ty))
                {
                    hit = true;
                    sectorHit = getSector(tx, ty);
                }
            }

            zBuffer[x] = dist;

            Sector sec = sectors[sectorHit];

            // ===== HEIGHT SYSTEM =====
            int ceiling = (H / 2.0f) - H / dist - sec.ceil * 10;
            int floor = (H / 2.0f) + H / dist + sec.floor * 10;

            wchar_t shade;

            if (dist < 3) shade = 0x2588;
            else if (dist < 6) shade = 0x2593;
            else if (dist < 9) shade = 0x2592;
            else shade = 0x2591;

            for (int y = 0; y < H; y++)
            {
                // CEILING
                if (y < ceiling)
                {
                    if (sectorHit == 3)
                        screen[y * W + x] = '^'; // high room ceiling
                    else if (sectorHit == 4)
                        screen[y * W + x] = '-'; // corridor low ceiling
                    else
                        screen[y * W + x] = ' ';
                }

                // WALL
                else if (y >= ceiling && y <= floor)
                {
                    screen[y * W + x] = shade;
                }

                // FLOOR
                else
                {
                    if (sectorHit == 2)
                        screen[y * W + x] = '.';
                    else if (sectorHit == 3)
                        screen[y * W + x] = ',';
                    else
                        screen[y * W + x] = '_';
                }
            }
        }

        // CROSSHAIR
        screen[(H / 2) * W + W / 2] = '+';

        WriteConsoleOutputCharacterW(
            h,
            screen,
            W * H,
            {0,0},
            &written
        );
    }
}
