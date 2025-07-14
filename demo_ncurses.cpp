#include <ncurses.h>
#include <string>

const int ROWS = 5, BOARD_COLS = 5;
int cursorY = 0, cursorX = 0;
int state[ROWS][BOARD_COLS] = {};      // 0: oculta, 1: revelada, 2: bandera
bool running = true;

void drawCell(int y, int x) {
    int row = 2 + y, col = 4 + x * 4;
    if (y == cursorY && x == cursorX)
        attron(A_REVERSE); // destaca celda actual

    switch (state[y][x]) {
        case 0: mvprintw(row, col, "[·]"); break;
        case 1: mvprintw(row, col, " %d ", (y + x) % 9); break; // número simulado
        case 2: mvprintw(row, col, "[🚩]"); break;
    }

    if (y == cursorY && x == cursorX)
        attroff(A_REVERSE);
}

void drawBoard() {
    clear();
    box(stdscr, 0, 0);
    mvprintw(1, 2, "🕹️  MINESWEEPER RETRO DEMO");
    mvprintw(ROWS * 2 + 3, 2, "Usa flechas para moverte. [r] revelar, [f] bandera, [q] salir.");

    for (int y = 0; y < ROWS; ++y)
        for (int x = 0; x < BOARD_COLS; ++x)
            drawCell(y, x);
}

void handleInput() {
    int ch = getch();
    switch (ch) {
        case KEY_UP:    cursorY = (cursorY + ROWS - 1) % ROWS; break;
        case KEY_DOWN:  cursorY = (cursorY + 1) % ROWS; break;
        case KEY_LEFT:  cursorX = (cursorX + BOARD_COLS - 1) % BOARD_COLS; break;
        case KEY_RIGHT: cursorX = (cursorX + 1) % BOARD_COLS; break;
        case 'r':       state[cursorY][cursorX] = 1; break;
        case 'f':       state[cursorY][cursorX] = 2; break;
        case 'q':       running = false; break;
    }
}

int main() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    while (running) {
        drawBoard();
        refresh();
        handleInput();
    }

    endwin();
    return 0;
}
