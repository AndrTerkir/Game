#include <iostream>
#include <vector>
#include <conio.h>
#include <windows.h>

using namespace std;

const int WIDTH = 10;
const int HEIGHT = 10;

int playerX = 1;
int playerY = 1;

vector<vector<int>> map =
{
    {1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,2,1},
    {1,0,1,1,1,0,1,0,2,1},
    {1,0,0,0,1,0,1,0,0,1},
    {1,0,1,0,1,0,1,1,0,1},
    {1,0,1,0,0,0,0,1,0,1},
    {1,0,1,1,1,1,0,1,0,1},
    {1,0,0,0,0,1,0,0,0,1},
    {1,2,2,2,0,0,0,1,0,1},
    {1,1,1,1,1,1,1,1,1,1}
};

void hideCursor()
{
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = false;

    SetConsoleCursorInfo(consoleHandle, &info);
}

void setCursorPosition(int x, int y)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    COORD pos;
    pos.X = x;
    pos.Y = y;

    SetConsoleCursorPosition(hConsole, pos);
}

void drawMap()
{
    setCursorPosition(0, 0);

    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            if (x == playerX && y == playerY)
            {
                cout << "@ ";
            }
            else
            {
                switch (map[y][x])
                {
                case 0:
                    cout << ". ";
                    break;

                case 1:
                    cout << "# ";
                    break;

                case 2:
                    cout << "~ ";
                    break;
                }
            }
        }

        cout << endl;
    }

    cout << endl;
    setlocale(LC_ALL, "Rus");
    cout << "Управление: стрелочки" << endl;
}

bool canMove(int x, int y)
{
    return map[y][x] != 1;
}

int main()
{
    hideCursor();

    while (true)
    {
        drawMap();

        int key = _getch();

        if (key == 224)
        {
            key = _getch();

            int newX = playerX;
            int newY = playerY;

            switch (key)
            {
            case 72:
                newY--;
                break;

            case 80:
                newY++;
                break;

            case 75:
                newX--;
                break;

            case 77:
                newX++;
                break;
            }

            if (canMove(newX, newY))
            {
                playerX = newX;
                playerY = newY;
            }
        }
    }

    return 0;
}
