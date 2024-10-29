#include "ConsoleDisplay.h"

NCWindow::NCWindow() {}

NCWindow::~NCWindow() {
    if(m_win != nullptr) {
        wborder(m_win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
        wrefresh(m_win);
        delwin(m_win);
    }
}

void NCWindow::refresh() {
    wborder(m_win, '|', '|', '-','-', '+', '+', '+', '+');
    wrefresh(m_win);
}

template<class... Types>
void NCWindow::mvprintw(int y, int x, const char* str, Types... args) {
    mvwprintw(m_win, y, x, str, args...);
}

void NCWindow::mvaddwchar(int y, int x, wchar_t ch) {
    wchar_t str[] = {ch, '\0'};
    mvwaddwstr(m_win, y, x, str);
}

void NCWindow::mvaddchar(int y, int x, char ch) {
    mvwaddch(m_win, y, x, ch);
    refresh();
}

void NCWindow::addwch(wchar_t ch) {
    wchar_t str[] = {ch, '\0'};
    waddwstr(m_win, str);
}

void NCWindow::addchar(char ch){
    waddch(m_win, ch);
}

void NCWindow::attribon(attr_t att) {
    wattron(m_win, att);
}

void NCWindow::attriboff(attr_t att) {
    wattroff(m_win, att);
}

void NCWindow::init(int height, int width, int starty, int startx) {
    m_win = newwin(height, width, starty, startx);
	nodelay(m_win, TRUE);
	wattron(m_win, DEF_COLOR_PAIR);
    refresh();
}

ConsoleDisplay::ConsoleDisplay() {
    initscr();
	raw();
	keypad(stdscr, TRUE);
	noecho();
	cbreak();
	nodelay(stdscr, TRUE);
	curs_set(0);

	start_color();
	init_pair(0, BG_COLOR, BG_COLOR);
	for(int i = 1; i <= 8; ++i) {
        init_pair(i, i - 1, BG_COLOR);
	}
	attron(DEF_COLOR_PAIR);

	for(int i = 0; i < LINES * COLS; ++i) {
        addch(' ');
	}
	refresh();
	drawScores();

	m_tetrisWindow.init(TETRIS_MATRIX_HEIGHT + 2, TETRIS_MATRIX_WIDTH + 2, Y1, X1);
	drawIncoming();

    refresh();
}

ConsoleDisplay::~ConsoleDisplay() {
    getch();
	endwin();
}

void ConsoleDisplay::tick() {
    char key = getch();
    if(m_keyToAct.contains(key))
        m_sim.act(m_keyToAct[key]);
    else {
        switch(key) {
        case ' ':
            while(getch() == -1);
            break;
        case 'q':
            exit(0);
            break;
        default:
            break;
        }
    }
    auto update = m_sim.tick();
    if(update & BV(TetrisUpdate::gameOver)) {
        m_tetrisWindow.mvprintw(1, 1, "GAME OVER!");
        m_tetrisWindow.refresh();
        while(getch() == -1) {}
        m_sim = TetrisSim(m_sim);
        mvprintw(Y1    , X1 - 4, "    ");
        mvprintw(Y1 + 1, X1 - 4, "    ");
        mvprintw(Y1 + 2, X1 - 4, "    ");
        mvprintw(Y1 + 3, X1 - 4, "    ");
        refresh();
    }
    if(update & BV(TetrisUpdate::swapped)) {
        mvprintw(Y1    , X1 - 4, "    ");
        mvprintw(Y1 + 1, X1 - 4, "    ");
        mvprintw(Y1 + 2, X1 - 4, "    ");
        mvprintw(Y1 + 3, X1 - 4, "    ");
        drawTetrimino(Y1, X1 - 4, m_sim.getHeld());
        refresh();
    }
    if(update & BV(TetrisUpdate::scoreChange)) {
        drawScores();
    }
    if(update & BV(TetrisUpdate::newBlockTaken)) {
        drawIncoming();
    }
    if(update & BV(TetrisUpdate::needRedraw)) {
        redraw();
    }
}

void ConsoleDisplay::redraw() {
    m_tetrisWindow.mvaddchar(1, 0, ' ');
    for(int i = 0; i < TETRIS_MATRIX_HEIGHT; ++i) {
        for(int j = 0; j < TETRIS_MATRIX_WIDTH; ++j) {
            auto ch = m_sim.cup(i,j);
            switch(ch) {
            case 0:
                m_tetrisWindow.addchar(' ');
                break;
            case 0xFF:
                m_tetrisWindow.attribon(COLOR_PAIR(m_sim.getColor()));
                m_tetrisWindow.addchar('#');
                break;
            default:
                m_tetrisWindow.attribon(COLOR_PAIR(m_sim.cup(i,j)));
                m_tetrisWindow.addwch(0x2588);
                //m_tetrisWindow.addwch(0x262D);
            }
        }
        m_tetrisWindow.addchar('\n');
        m_tetrisWindow.addchar(' ');
    }
    m_tetrisWindow.attribon(DEF_COLOR_PAIR);
    m_tetrisWindow.refresh();
}

void ConsoleDisplay::drawTetrimino(int y, int x, Tetrimino& piece) {
    attron(COLOR_PAIR(piece.m_color));
    for(size_t i = 0; i < piece.m_dim; ++i) {
        for(size_t j = 0; j < piece.m_dim; ++j) {
            if(piece.m_shape[0][i][j]) {
                mvaddwch(y + i, x + j, 0x2588);
            }
        }
    }
    attron(DEF_COLOR_PAIR);
}

void ConsoleDisplay::drawIncoming() {
    for(int i = Y1; i < LINES; ++i) {
        mvaddstr(i, X2 + 1, "              ");
    }

    int y = Y1;
    for(int i = 0; i < TETRIS_INCOMING_LOOK_AHEAD; ++i) {
        auto piece = m_sim.incoming(i);
        drawTetrimino(y, X2 + 1, piece);
        y += piece.m_dim + 1;
    }
    refresh();
}

void ConsoleDisplay::drawScores() {
    mvprintw(Y1,     0, "Score: %i", m_sim.getScore());
    mvprintw(Y1 + 1, 0, "Cleared Lines: %i", m_sim.getFinishedLines());
    mvprintw(Y1 + 2, 0, "Level: %i", m_sim.getLevel());
    auto combo = m_sim.getCombo();
    if(combo != 0)
        mvprintw(Y1 + 3, 0, "Combo: %i", combo);
    else
        mvprintw(Y1 + 3, 0, "           ");
    refresh();
}
