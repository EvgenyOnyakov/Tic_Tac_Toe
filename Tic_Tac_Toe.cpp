#include <iostream>
#include <stdlib.h>
#include <random>
#include <chrono>
#include <conio.h> 
#define _WIN32_WINNT 0x0500

#include <windows.h>

using namespace std;

size_t player_wins{ 0 }, ai_wins{ 0 }, draws{ 0 }, difficulty{ 0 };

enum ConsoleColor
{
    Black = 0,
    Blue = 1,
    Green = 2,
    Cyan = 3,
    Red = 4,
    Magenta = 5,
    Brown = 6,
    LightGray = 7,
    DarkGray = 8,
    LightBlue = 9,
    LightGreen = 10,
    LightCyan = 11,
    LightRed = 12,
    LightMagenta = 13,
    Yellow = 14,
    White = 15
};

enum Cell : char
{
    EMPTY = '-',
    CROSS = 'X',
    ZERO = 'O'
};

enum Progress
{
    in_game,
    player_won,
    ai_won,
    draw
};

struct Coordinates
{
    size_t y{ 0 };
    size_t x{ 0 };
};

#pragma pack (push, 1)
struct Game
{
    Cell** pField = nullptr;
    size_t SIZE = 3;
    Cell ai;
    Cell player;
    size_t turn{ 0 };
    Progress prog = in_game;
};
#pragma pack (pop)

void windowSetting()
{
    const int col = 40; // ширина окна консоли
    const int row = 25; // высота консоли
    auto h = ::GetStdHandle(STD_OUTPUT_HANDLE);     // программное увеличение размера шрифта
    CONSOLE_FONT_INFOEX cfi = {};
    cfi.cbSize = sizeof(cfi);
    ::GetCurrentConsoleFontEx(h, FALSE, &cfi);
    cfi.dwFontSize.Y *= 2;
    ::SetCurrentConsoleFontEx(h, FALSE, &cfi);

    HANDLE wHnd = GetStdHandle(STD_OUTPUT_HANDLE);
    HANDLE rHnd = GetStdHandle(STD_INPUT_HANDLE);
    SetConsoleTitle(L"Крестики Нолики");             // имя окна консоли    
    SMALL_RECT windowSize = { 0,0,col - 1,row - 1 }; // Задаем размер консоли и его внутреннего буфера 
    SetConsoleWindowInfo(wHnd, TRUE, &windowSize);
    COORD bufferSize = { col,row };                  // размеры внутреннего буфера    
    SetConsoleScreenBufferSize(wHnd, bufferSize);    // Изменение размера внутреннего буфера окна консоли
    HWND consoleWindow = GetConsoleWindow(); SetWindowLong(consoleWindow, GWL_STYLE, GetWindowLong(consoleWindow, GWL_STYLE) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX);

    void* handle = GetStdHandle(STD_OUTPUT_HANDLE);  // прячем курсор консоли
    CONSOLE_CURSOR_INFO structCursorInfo;
    GetConsoleCursorInfo(handle, &structCursorInfo);
    structCursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(handle, &structCursorInfo);
}

void SetColor(int text, int background) // устанавливаем цвет
{
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hStdOut, (WORD)((background << 4) | text));
}

void GotoXY(SHORT X, SHORT Y) // позиция курсора
{
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coord = { X, Y };
    SetConsoleCursorPosition(hStdOut, coord);
}

void logo()  // логотип на начальном экране 
{
    SetColor(10, 0);
    GotoXY(2, 2);  cout << "### ###  ##  ###  #   ##  ###  #  ###" << endl;
    GotoXY(2, 3);  cout << " #   #  #     #  # # #     #  # # #  " << endl;
    GotoXY(2, 4);  cout << " #   #  #     #  ### #     #  # # ## " << endl;
    GotoXY(2, 5);  cout << " #   #  #     #  # # #     #  # # #  " << endl;
    GotoXY(2, 6);  cout << " #  ###  ##   #  # #  ##   #   #  ###" << endl;

    SetColor(8, 0);
    GotoXY(12, 8);   cout << "     |     |     " << endl;
    GotoXY(12, 9);   cout << "  X  |  -  |  -  " << endl;
    GotoXY(12, 10);  cout << "_____|_____|_____" << endl;
    GotoXY(12, 11);  cout << "     |     |     " << endl;
    GotoXY(12, 12);  cout << "  -  |  O  |  X  " << endl;
    GotoXY(12, 13);  cout << "_____|_____|_____" << endl;
    GotoXY(12, 14);  cout << "     |     |     " << endl;
    GotoXY(12, 15);  cout << "  -  |  O  |  -  " << endl;
    GotoXY(12, 16);  cout << "     |     |     " << endl;
}

int32_t getRandomNum(int32_t min, int32_t max) // вместо rand()
{
    const static auto seed = chrono::system_clock::now().time_since_epoch().count();
    static mt19937_64 generator(seed);
    uniform_int_distribution<int32_t> dis(min, max);
    return dis(generator);
}

void clearScr() // очистка экрана
{
    system("cls");
}

void startGame(Game& g)  // запуск игры и выделение под нее памяти
{
    g.pField = new Cell * [g.SIZE];

    for (size_t i = 0; i < g.SIZE; i++)
    {
        g.pField[i] = new Cell[g.SIZE];
    }

    for (size_t y = 0; y < g.SIZE; y++)
    {
        for (size_t x = 0; x < g.SIZE; x++)
        {
            g.pField[y][x] = EMPTY;
        }
    }

    if (getRandomNum(0, 1000) > 500)
    {
        g.player = CROSS;
        g.ai = ZERO;
        g.turn = 0;
    }
    else
    {
        g.player = ZERO;
        g.ai = CROSS;
        g.turn = 1;
    }
}

void stopGame(Game& g) // остановка и удаление игры из памяти 
{
    for (size_t i = 0; i < g.SIZE; i++)
    {
        delete[] g.pField[i];
    }

    delete[] g.pField;
    g.pField = nullptr;
}

void printGameField(Game& g)  // рисуем игровое поле
{
    SetColor(8, 0);
    GotoXY(6, 1); cout << "Для размещения фигуры на поле" << endl;
    GotoXY(4, 2); cout << "необходимо ввести координаты Х и У" << endl;
    GotoXY(11, 3); cout << "затем нажать ENTER" << endl;

    GotoXY(10, 5);
    cout << "       ";
    for (size_t x = 0; x < g.SIZE; x++)
    {
        SetColor(9, 0); cout << "[X" << x + 1 << "]";
    }
    cout << endl;

    for (size_t y = 0; y < g.SIZE; y++)
    {
        GotoXY(10, 6 + y); SetColor(10, 0);
        cout << " [Y" << y + 1 << "]";
        SetColor(15, 0); cout << " | ";
        for (size_t x = 0; x < g.SIZE; x++)
        {
            if (g.pField[y][x] == CROSS)
            {
                SetColor(14, 0);
                cout << g.pField[y][x];
            }
            else if (g.pField[y][x] == ZERO)
            {
                SetColor(12, 0);
                cout << g.pField[y][x];
            }
            else
            {
                SetColor(8, 0);
                cout << g.pField[y][x];
            }

            SetColor(15, 0);
            cout << " | ";
        }
        cout << endl;
    }
    cout << endl;
    SetColor(8, 0); cout << " Фигура игрока: ";
    if (g.player == CROSS)
    {
        SetColor(14, 0); cout << g.player;
    }
    else
    {
        SetColor(12, 0); cout << g.player;
    }
    SetColor(8, 0); cout << "  Фигура компьютера: ";
    if (g.ai == CROSS)
    {
        SetColor(14, 0); cout << g.ai << endl << endl;
    }
    else
    {
        SetColor(12, 0); cout << g.ai << endl << endl;
    }
    SetColor(15, 0);

    GotoXY(9, 20); cout << "Победы игрока:       " << player_wins << endl;
    GotoXY(9, 21); cout << "Победы компьютера:   " << ai_wins << endl;
    GotoXY(9, 22); cout << "Ничьи:               " << draws << endl;

    GotoXY(6, 17); cout << "Уровень сложности: ";
    if (difficulty == 0)
    {
        SetColor(10, 0); cout << "нормально";
    }
    if (difficulty == 1)
    {
        SetColor(9, 0); cout << "сложно";
    }
    if (difficulty == 2)
    {
        SetColor(12, 0); cout << "очень сложно";
    }
}

void congPrint(Game& g)  // печать итогового игрового поля для окна поздравлений
{
    for (size_t y = 0; y < g.SIZE; y++)
    {
        GotoXY(13, 3 + y); SetColor(8, 0);
        SetColor(15, 0); cout << " | ";
        for (size_t x = 0; x < g.SIZE; x++)
        {
            if (g.pField[y][x] == CROSS)
            {
                SetColor(14, 0);
                cout << g.pField[y][x];
            }
            else if (g.pField[y][x] == ZERO)
            {
                SetColor(12, 0);
                cout << g.pField[y][x];
            }
            else
            {
                SetColor(8, 0);
                cout << g.pField[y][x];
            }

            SetColor(15, 0);
            cout << " | ";
        }
        cout << endl;
    }
}

Progress result(Game& g)  // определение выйгрыша
{
    //строки
    for (size_t y = 0; y < g.SIZE; y++)
    {
        if (g.pField[y][0] == g.pField[y][1] && g.pField[y][0] == g.pField[y][2])
        {
            if (g.pField[y][0] == g.player)
                return player_won;
            if (g.pField[y][0] == g.ai)
                return ai_won;
        }
    }
    // столбцы
    for (size_t x = 0; x < g.SIZE; x++)
    {
        if (g.pField[0][x] == g.pField[1][x] && g.pField[0][x] == g.pField[2][x])
        {
            if (g.pField[0][x] == g.player)
                return player_won;
            if (g.pField[0][x] == g.ai)
                return ai_won;
        }
    }
    //диагональ
    if (g.pField[0][0] == g.pField[1][1] && g.pField[0][0] == g.pField[2][2])
    {
        if (g.pField[1][1] == g.player)
            return player_won;
        if (g.pField[1][1] == g.ai)
            return ai_won;
    }
    if (g.pField[0][2] == g.pField[1][1] && g.pField[2][0] == g.pField[1][1])
    {
        if (g.pField[1][1] == g.player)
            return player_won;
        if (g.pField[1][1] == g.ai)
            return ai_won;
    }

    bool DRAW = true;
    for (size_t y = 0; y < g.SIZE; y++)
    {
        for (size_t x = 0; x < g.SIZE; x++)
        {
            if (g.pField[y][x] == EMPTY)
            {
                DRAW = false;
                break;
            }
        }
        if (!DRAW)
            break;
    }

    if (DRAW)
        return draw;
    return in_game;
}

Coordinates getPlayerCoord(Game& g) // получение координат от игрока
{
    Coordinates c;

    do
    {

        GotoXY(8, 12); SetColor(9, 0); cout << " Введите координату X: ";
        cin >> c.x;
        GotoXY(8, 13); SetColor(10, 0); cout << " Введите координату Y: ";
        cin >> c.y;
        c.x--;
        c.y--;
    } while (c.x > 2 || c.y > 2 || g.pField[c.y][c.x] != EMPTY);

    return c;
}

Coordinates getAICoord(Game& g) // варианты ходов компьютера
{
    if (difficulty == 1 || difficulty == 2) // при выбранной сложности "сложно"
    {
        // вдруг победа?
        for (size_t y = 0; y < g.SIZE; y++)
        {
            for (size_t x = 0; x < g.SIZE; x++)
            {
                if (g.pField[y][x] == EMPTY)
                {
                    g.pField[y][x] = g.ai;
                    if (result(g) == ai_won)
                    {
                        g.pField[y][x] = EMPTY;
                        return { y, x };
                    }
                    g.pField[y][x] = EMPTY;
                }
            }
        }
    }
    if (difficulty == 2) // очень сложно
    {
        //вдруг поражение?
        for (size_t y = 0; y < g.SIZE; y++)
        {
            for (size_t x = 0; x < g.SIZE; x++)
            {
                if (g.pField[y][x] == EMPTY)
                {
                    g.pField[y][x] = g.player;
                    if (result(g) == player_won)
                    {
                        g.pField[y][x] = EMPTY;
                        return { y, x };
                    }
                    g.pField[y][x] = EMPTY;
                }
            }
        }
    }

    // центр
    if (g.pField[1][1] == EMPTY)
        return { 1, 1 };
    // углы
    Coordinates buff[4];
    size_t num = 0;
    if (g.pField[0][0] == EMPTY)
    {
        buff[num] = { 0, 0 };
        num++;
    }
    if (g.pField[0][2] == EMPTY)
    {
        buff[num] = { 0, 2 };
        num++;
    }
    if (g.pField[2][0] == EMPTY)
    {
        buff[num] = { 2, 0 };
        num++;
    }
    if (g.pField[2][2] == EMPTY)
    {
        buff[num] = { 2, 2 };
        num++;
    }

    if (num > 0)
    {
        const size_t index = getRandomNum(0, 1000) % num;
        return buff[index];
    }

    //неуглы
    num = 0;
    if (g.pField[0][1] == EMPTY)
    {
        buff[num] = { 0, 1 };
        num++;
    }
    if (g.pField[1][0] == EMPTY)
    {
        buff[num] = { 1, 0 };
        num++;
    }
    if (g.pField[1][2] == EMPTY)
    {
        buff[num] = { 1, 2 };
        num++;
    }
    if (g.pField[2][1] == EMPTY)
    {
        buff[num] = { 2, 1 };
        num++;
    }

    if (num > 0)
    {
        const size_t index = getRandomNum(0, 1000) % num;
        return buff[index];
    }
}

void congratulation(Game& g) // поздравление победителя
{

    if (g.prog == player_won)
    {
        player_wins++;
        congPrint(g); GotoXY(14, 8); SetColor(6, 0);
        cout << "Вы выйграли!!!" << endl << endl;
        SetColor(14, 0);
        GotoXY(14, 12); cout << R"(   _______   )";
        GotoXY(14, 13); cout << R"(  |   W   |  )";
        GotoXY(14, 14); cout << R"( (|   I   |) )";
        GotoXY(14, 15); cout << R"(  |   N   |  )";
        GotoXY(14, 16); cout << R"(   \  !  /   )";
        GotoXY(14, 17); cout << R"(    `---'    )";
        GotoXY(14, 18); cout << R"(    _|_|_    )";
        GotoXY(10, 22); SetColor(15, 0); cout << "Нажмите любую клавишу";
        SetColor(0, 0);
        GotoXY(1, 24);
    }
    if (g.prog == ai_won)
    {
        ai_wins++;
        congPrint(g); GotoXY(14, 7); SetColor(12, 0);
        cout << "Вы проиграли!!!" << endl;

        SetColor(4, 0);
        GotoXY(12, 9);  cout << R"(   (\-"````"-/)   )";
        GotoXY(12, 10); cout << R"(   //^\    /^\\   )";
        GotoXY(12, 11); cout << R"(  ;/ ~_\  /_~ \;  )";
        GotoXY(12, 12); cout << R"(  |  / \\// \  |  )";
        GotoXY(12, 13); cout << R"( (,  \0/  \0/  ,) )";
        GotoXY(12, 14); cout << R"(  |   /    \   |  )";
        GotoXY(12, 15); cout << R"(  | (_\.__./_) |  )";
        GotoXY(12, 16); cout << R"(   \ \v-..-v/ /   )";
        GotoXY(12, 17); cout << R"(    \ `====' /    )";
        GotoXY(12, 18); cout << R"(     `\\\///'     )";
        GotoXY(12, 19); cout << R"(      '\\//'      )";
        GotoXY(12, 20); cout << R"(        \/        )";
        GotoXY(10, 23); SetColor(15, 0); cout << "Нажмите любую клавишу";
        SetColor(0, 0);
        GotoXY(1, 24);
    }
    if (g.prog == draw)
    {
        draws++;
        congPrint(g); GotoXY(17, 7); SetColor(7, 0);
        cout << "Ничья =/" << endl;
        SetColor(8, 0);
        GotoXY(8, 9);  cout << R"(      ___   ..   ___      )";
        GotoXY(8, 10); cout << R"(   o-~   ~=[UU]=~   ~-o   )";
        GotoXY(8, 11); cout << R"(   |        ||        |   )";
        GotoXY(8, 12); cout << R"(   |        ||        |   )";
        GotoXY(8, 13); cout << R"(  /^\       ||       /^\  )";
        GotoXY(8, 14); cout << R"( (___)      ||      (___) )";
        GotoXY(8, 15); cout << R"(            ||            )";
        GotoXY(8, 16); cout << R"(            ||            )";
        GotoXY(8, 17); cout << R"(           /VV\           )";
        GotoXY(8, 18); cout << R"(         ~'~~~~`~         )";
        GotoXY(10, 22); SetColor(15, 0); cout << "Нажмите любую клавишу";
        SetColor(0, 0);
        GotoXY(1, 24);
    }
}

void TICTACTOE() // вся игра в сборе
{
    clearScr();
    Game g;
    startGame(g);
    printGameField(g);

    do
    {
        if (g.turn % 2 == 0)
        {
            Coordinates c = getPlayerCoord(g);
            g.pField[c.y][c.x] = g.player;
        }
        else
        {
            Coordinates c = getAICoord(g);
            g.pField[c.y][c.x] = g.ai;
        }
        g.turn++;
        clearScr();
        printGameField(g);
        g.prog = result(g);

    } while (g.prog == in_game);

    clearScr();
    congratulation(g);
    stopGame(g);
}

void mainmenu();

void settings() // выбор сложности
{
    clearScr();

    int key = 0, index = 0;
    const int size = 3;
    const char* menu[size] = { " нормально"," сложно", " очень сложно" };

    for (; key != 27;) {
        clearScr();
        logo();
        GotoXY(7, 18); cout << "Выбранный уровень сложности: ";
        if (difficulty == 0)
        {
            GotoXY(16, 19); SetColor(10, 0); cout << "нормально";
        }
        if (difficulty == 1)
        {
            GotoXY(18, 19); SetColor(9, 0); cout << "сложно";
        }
        if (difficulty == 2)
        {
            GotoXY(15, 19); SetColor(12, 0); cout << "очень сложно";
        }
        for (int i = 0; i < size; i++)
            if (index == i)
            {
                SetColor(6, 0); GotoXY(12, 21 + i);
                cout << (char)26 << menu[i] << "\n";
            }
            else
            {
                SetColor(15, 0); GotoXY(12, 21 + i);
                cout << " " << menu[i] << "\n";
            }
        key = _getch();
        if (key == 224 || key == 0)
            key = _getch();
        switch (key) {
        case 72:
            if (index > 0)
                index--;
            else
                index = size - 1;
            break;
        case 80:
            if (index < size - 1)
                index++;
            else
                index = 0;
            break;
        case 13:
            switch (index) {
            case 0:difficulty = 0; mainmenu(); break;
            case 1:difficulty = 1; mainmenu(); break;
            case 2:difficulty = 2; mainmenu(); break;
            case 3: cout << "Выход\n";
                key = 27; break;
            }
            system("pause");
        }
    }

}

void mainmenu() // меню игры
{
    int key = 0, index = 0;
    const int size = 3;
    const char* menu[size] = { " НАЧАТЬ ИГРУ"," УРОВЕНЬ СЛОЖНОСТИ"," ВЫХОД" };

    for (; key != 27;) {
        clearScr();
        logo();
        for (int i = 0; i < size; i++)
            if (index == i)
            {
                SetColor(6, 0); GotoXY(12, 19 + i);
                cout << (char)26 << menu[i] << "\n";
            }
            else
            {
                SetColor(15, 0); GotoXY(12, 19 + i);
                cout << " " << menu[i] << "\n";
            }
        key = _getch();
        if (key == 224 || key == 0)
            key = _getch();
        switch (key) {
        case 72:
            if (index > 0)
                index--;
            else
                index = size - 1;
            break;
        case 80:
            if (index < size - 1)
                index++;
            else
                index = 0;
            break;
        case 13:
            switch (index) {
            case 0: TICTACTOE(); break;
            case 1: settings(); break;
            case 2: exit(0); break;
            case 3: cout << "Выход\n";
                key = 27; break;
            }
            system("pause");
        }
    }
}

int main()
{
    setlocale(LC_ALL, "Russian");
    windowSetting();
    mainmenu();
    return 0;
}
