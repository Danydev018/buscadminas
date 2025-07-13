#include "common/Board.h"
#include <queue>
#include <cassert>

Board::Board(int rows, int cols, int mines, unsigned seed)
    : R(rows), C(cols), M(mines), grid(rows*cols)
{
    placeMines(seed);
    computeAdjacents();
}

void Board::placeMines(unsigned seed) {
    std::mt19937 gen(seed);
    std::uniform_int_distribution<> dist(0, R*C - 1);
    int placed = 0;
    while (placed < M) {
        int idx = dist(gen);
        if (!grid[idx].hasMine) {
            grid[idx].hasMine = true;
            ++placed;
        }
    }
}

void Board::computeAdjacents() {
    for (int r = 0; r < R; ++r) {
        for (int c = 0; c < C; ++c) {
            if (grid[index(r,c)].hasMine) continue;
            int cnt = 0;
            for (int dr=-1; dr<=1; dr++)
            for (int dc=-1; dc<=1; dc++) {
                int nr=r+dr, nc=c+dc;
                if (nr>=0 && nr<R && nc>=0 && nc<C
                    && grid[index(nr,nc)].hasMine) ++cnt;
            }
            grid[index(r,c)].adjacentMines = cnt;
        }
    }
}

void Board::reveal(int r, int c) {
    assert(r>=0 && r<R && c>=0 && c<C);
    auto& cell = grid[index(r,c)];
    if (cell.state != Hidden) return;
    cell.state = Revealed;
    if (cell.adjacentMines == 0 && !cell.hasMine)
        floodFill(r,c);
}

void Board::toggleFlag(int r, int c) {
    assert(r>=0 && r<R && c>=0 && c<C);
    auto& cell = grid[index(r,c)];
    if (cell.state == Hidden)
        cell.state = Flagged;
    else if (cell.state == Flagged)
        cell.state = Hidden;
}

void Board::floodFill(int r, int c) {
    std::queue<std::pair<int,int>> q;
    q.push({r,c});
    while (!q.empty()) {
        auto [x,y] = q.front(); q.pop();
        for (int dr=-1; dr<=1; dr++)
        for (int dc=-1; dc<=1; dc++) {
            int nr = x+dr, nc = y+dc;
            if (nr<0||nr>=R||nc<0||nc>=C) continue;
            auto& ncell = grid[index(nr,nc)];
            if (ncell.state==Hidden && !ncell.hasMine) {
                ncell.state = Revealed;
                if (ncell.adjacentMines==0)
                    q.push({nr,nc});
            }
        }
    }
}

bool Board::isMine(int r, int c) const {
    return grid[index(r,c)].hasMine;
}

bool Board::allSafeRevealed() const {
    for (auto& c : grid)
        if (!c.hasMine && c.state!=Revealed)
            return false;
    return true;
}

const Cell& Board::at(int r, int c) const {
    return grid[index(r,c)];
}

int Board::rows() const { return R; }
int Board::cols() const { return C; }
int Board::index(int r, int c) const { return r*C + c; }

void Board::print() const {
    std::cout << "   ";
    for (int c = 0; c < C; ++c)
        std::cout << ' ' << c;
    std::cout << "\n  +";
    for (int c = 0; c < C; ++c) std::cout << "--";
    std::cout << "+\n";
    for (int r = 0; r < R; ++r) {
        std::cout << (r < 10 ? " " : "") << r << "|";
        for (int c = 0; c < C; ++c) {
            const Cell& cell = at(r,c);
            std::string ch;

            if (cell.state == Hidden) {
                ch = "\033[90m·\033[0m"; // celda oculta: gris
            }
            else if (cell.state == Flagged) {
                ch = "\033[33m🚩\033[0m"; // bandera: amarillo
            }
            else if (cell.hasMine) {
                ch = "\033[31m💣\033[0m"; // bomba revelada: rojo
            }
            else if (cell.adjacentMines > 0) {
                const char* colors[] = {
                    "\033[34m", // azul
                    "\033[32m", // verde
                    "\033[36m", // cyan
                    "\033[35m", // magenta
                    "\033[31m", // rojo
                    "\033[33m", // amarillo
                    "\033[95m", // rosa
                    "\033[91m"  // rojo claro
                };
                int idx = cell.adjacentMines - 1;
                ch = colors[idx] + std::to_string(cell.adjacentMines) + "\033[0m";
            }
            else {
                ch = " ";
            }

            std::cout << ' ' << ch;


        }
        std::cout << " |\n";
    }
    std::cout << "  +";
    for (int c = 0; c < C; ++c) std::cout << "--";
    std::cout << "+\n";
}
