#ifndef CONSOLEDISPLAY_H
#define CONSOLEDISPLAY_H

#include <unordered_map>
#include <ncursesw/curses.h>

#include "TetrisSim.h"

// Defined color pairs
// 1     2   3     4      5    6       7    8
// BLACK RED GREEN YELLOW BLUE MAGENTA CYAN WHITE
#define BG_COLOR COLOR_BLUE
#define DEF_COLOR_PAIR COLOR_PAIR(8)

#define mvaddwch(y, x, ch) { wchar_t str[] = {(ch), '\0'}; mvaddwstr((y), (x), str); }

/*
      0                        X1                         X2                        COLS
     ┌─────────────────────────┬──────────────────────────┬─────────────────────────┐
    0│                         │                          │                         │
     │                         │                          │                         │
     │                         │                          │                         │
     │                         │                          │                         │
   Y1├─────────────────────────┼──────────────────────────┼─────────────────────────┤
     │Stats                Hold│                          │Incoming                 │
     │                         │                          │                         │
     │                         │                          │                         │
     │                         │                          │                         │
     │                         │                          │                         │
     │                         │                          │                         │
     │                         │                          │                         │
     │                         │                          │                         │
     │                         │          Tetris          │                         │
     │                         │                          │                         │
     │                         │                          │                         │
     │                         │                          │                         │
     │                         │                          │                         │
     │                         │                          │                         │
     │                         │                          │                         │
     │                         │                          │                         │
     │                         │                          │                         │
     │                         │                          │                         │
   Y2├─────────────────────────┼──────────────────────────┼─────────────────────────┤
     │                         │                          │                         │
     │                         │                          │                         │
     │                         │                          │                         │
     │                         │                          │                         │
LINES└─────────────────────────┴──────────────────────────┴─────────────────────────┘
 */

#define Y1 (LINES - TETRIS_MATRIX_HEIGHT - 2)/2
#define X1 (COLS - TETRIS_MATRIX_WIDTH - 2)/2
#define Y2 ((LINES + TETRIS_MATRIX_HEIGHT + 2)/2 - 1)
#define X2 ((COLS + TETRIS_MATRIX_WIDTH + 2)/2 - 1)

class NCWindow {
public:
    WINDOW* m_win = nullptr;

    NCWindow();
    ~NCWindow();
    void init(int height, int width, int starty, int startx);
    void refresh();
    template<class... Types>
    void mvprintw(int y, int x, const char* str, Types... args);
    void mvaddwchar(int y, int x, wchar_t ch);
    void mvaddchar(int y, int x, char ch);
    void addwch(wchar_t ch);
    void addchar(char ch);
    void attribon(attr_t att);
    void attriboff(attr_t att);

protected:
};

class ConsoleDisplay
{
public:
    ConsoleDisplay();
    ~ConsoleDisplay();

    void tick();
    void redraw();
    void drawTetrimino(int y, int x, Tetrimino& piece);
    void drawIncoming();
    void drawScores();

protected:
    NCWindow m_tetrisWindow;
    TetrisSim m_sim{{Tetrimino({{0, 0, 0},
                                {1, 1, 1},
                                {0, 0, 1}}, 6),
                     Tetrimino({{0, 0, 0},
                                {1, 1, 1},
                                {1, 0, 0}}, 7),
                     Tetrimino({{0, 0, 0},
                                {1, 1, 0},
                                {0, 1, 1}}, 1),
                     Tetrimino({{0, 0, 0},
                                {0, 1, 1},
                                {1, 1, 0}}, 8),
                     Tetrimino({{0, 0, 0},
                                {1, 1, 1},
                                {0, 1, 0}}, 3),
                     Tetrimino({{0, 0, 0, 0},
                                {1, 1, 1, 1},
                                {0, 0, 0, 0},
                                {0, 0, 0, 0}}, 2),
                     Tetrimino({{1, 1},
                                {1, 1}}, 4)}};
    std::unordered_map<char, TetrisAct> m_keyToAct = {{KEY_LEFT, TetrisAct::left},
                                                      {KEY_RIGHT, TetrisAct::right},
                                                      {KEY_DOWN, TetrisAct::down},
                                                      {KEY_UP, TetrisAct::drop},
                                                      {'z', TetrisAct::counterClockwise},
                                                      {'x', TetrisAct::clockwise},
                                                      {'a', TetrisAct::hold}};
};

#endif // CONSOLEDISPLAY_H
