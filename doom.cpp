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
    1,2,2,2,2,2,2,2,1,3,3,3,3,3,3,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

bool wall(int x, int y)
{
    if (x < 0 || x >= MW || y < 0 || y >= MH) return true;
    return mapData[y * MW + x] == 1;
}

char wallTex(float dist, int x, int y)
{
    // 🔥 ТЕКСТУРА СТЕН (точки разной плотности)
    int v = (int)(dist * 10 + x * 3 + y * 7) % 6;

    switch (v)
    {
        case 0: return ' ';
        case 1: return '.';
        case 2: return '.';
        case 3: return ':';
        case 4: return ':';
        default: return '#';
    }
}

int main()
{
    wchar_t* screen = new wchar_t[W * H];

    HANDLE h = CreateConsoleScreenBuffer(
        GENERIC_READ | GENERIC_WRITE,
        0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);

    SetConsoleActiveScreenBuffer(h);

    DWORD written;

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
            if (!wall(nx, ny)) px = nx, py = ny;
        }

        if (GetAsyncKeyState('S'))
        {
            float nx = px - sinf(pa) * ms;
            float ny = py - cosf(pa) * ms;
            if (!wall(nx, ny)) px = nx, py = ny;
        }

        for (int i = 0; i < W * H; i++)
            screen[i] = ' ';

        vector<float> z(W);

        for (int x = 0; x < W; x++)
        {
            float rayA = (pa - FOV / 2.0f) + ((float)x / W) * FOV;

            float dist = 0;
            float ox = sinf(rayA);
            float oy = cosf(rayA);

            bool hit = false;

            while (!hit && dist < DEPTH)
            {
                dist += 0.03f;

                int tx = (int)(px + ox * dist);
                int ty = (int)(py + oy * dist);

                if (tx < 0 || tx >= MW || ty < 0 || ty >= MH || wall(tx, ty))
                    hit = true;
            }

            z[x] = dist;

            int ceiling = (H / 2.0f) - H / dist;
            int floor = H - ceiling;

            for (int y = 0; y < H; y++)
            {
                // ===== ПОТОЛОК (тёмный градиент) =====
                if (y < ceiling)
                {
                    float b = 1.0f - (float)y / (H / 2);

                    if (b > 0.7f) screen[y * W + x] = '.';
                    else if (b > 0.4f) screen[y * W + x] = ' ';
                    else screen[y * W + x] = ' ';
                }

                // ===== СТЕНЫ (ТЕКСТУРЫ) =====
                else if (y <= floor)
                {
                    wchar_t c = wallTex(dist, x, y);

                    // 🔥 fog (хоррор эффект)
                    if (dist > 12) c = ' ';
                    else if (dist > 8 && c == '#') c = ':';

                    screen[y * W + x] = c;
                }

                // ===== ПОЛ (перспектива точками) =====
                else
                {
                    float b = (float)(y - H / 2) / (H / 2);

                    if ((x + y + (int)(px + py)) % 11 == 0)
                        screen[y * W + x] = '.';
                    else if (b > 0.6f)
                        screen[y * W + x] = ',';
                    else if (b > 0.3f)
                        screen[y * W + x] = '-';
                    else
                        screen[y * W + x] = ' ';
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
