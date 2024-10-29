#include "TetrisSim.h"

Tetrimino::Tetrimino(std::vector<std::vector<bool>>&& shape, unsigned char color = 1) :
    m_color(color)
{
    m_dim = shape.size();
    m_shape[0] = shape;
    for(int i = 1; i < 4; ++i) {
        m_shape[i].resize(m_dim, std::vector<bool>(m_dim, false));
    }

    auto dim1 = m_dim - 1;
    for(size_t i = 0; i < m_dim; ++i) {
        for(size_t j = 0; j < m_dim; ++j) {
            m_shape[1][j       ][dim1 - i] = m_shape[0][i][j];
            m_shape[2][dim1 - i][dim1 - j] = m_shape[0][i][j];
            m_shape[3][dim1 - j][i       ] = m_shape[0][i][j];
        }
    }

//    for(int k = 0; k < 4; ++k) {
//        for(int i = 0; i < m_dim; ++i) {
//            for(int j = 0; j < m_dim; ++j) {
//                std::cout << m_shape[k][i][j];
//            }
//            std::cout << std::endl;
//        }
//        std::cout << std::endl;
//    }
}

TetrisSim::TetrisSim(std::vector<Tetrimino>&& blocks) :
    m_blocks(blocks)
{
    std::random_device seed;
    m_rand.seed(seed());
    for(int i = 0; i < TETRIS_INCOMING_LOOK_AHEAD; ++i) {
        m_incoming[i] = m_rand() % m_blocks.size();
    }
    newBlock();
    updateGhost();
}

TetrisSim::TetrisSim(TetrisSim& in) :
    TetrisSim(std::move(in.m_blocks)) {}

bool TetrisSim::act(TetrisAct a) {
    if(m_state >= BV(TetrisState::noActAfter))
        return false;
    bool res = true;
    switch(a) {
    case TetrisAct::clockwise:
        res = tryPutting(m_tType, m_tX, m_tY, (m_tRot + 1)%4, true);
        break;
    case TetrisAct::counterClockwise:
        res = tryPutting(m_tType, m_tX, m_tY, (m_tRot + 3)%4, true);
        break;
    case TetrisAct::left:
        res = tryPutting(m_tType, m_tX - 1, m_tY, m_tRot);
        break;
    case TetrisAct::right:
        res = tryPutting(m_tType, m_tX + 1, m_tY, m_tRot);
        break;
    case TetrisAct::down:
        res = tryPutting(m_tType, m_tX, m_tY + 1, m_tRot);
        m_prevTime = std::chrono::steady_clock::now();
        break;
    case TetrisAct::drop:
        m_state |= BV(TetrisState::blockFalling);
        break;
    case TetrisAct::hold:
        if(!(m_state & BV(TetrisState::swapped))) {
            std::swap(m_hold, m_tType);
            newBlock(m_tType);
            m_state |= BV(TetrisState::swapped);
            m_update |= BV(TetrisUpdate::swapped);
        }
        break;
    default:
        break;
    }
    if(!check(m_tType, m_tX, m_tY + 1, m_tRot))
        m_state |= BV(TetrisState::atTheBottom);
    else
        m_state &= ~BV(TetrisState::atTheBottom);
    if((a == TetrisAct::down && !res))
        finalize();

    updateGhost();
    return res;
}

uint8_t TetrisSim::cup(int y, int x) {
    if(x >= m_tX && x < m_tX + int(m_blocks[m_tType].m_dim) &&
       y >= m_tY && y < m_tY + int(m_blocks[m_tType].m_dim)) {
        if(m_blocks[m_tType].m_shape[m_tRot][y-m_tY][x-m_tX] != 0)
            return m_blocks[m_tType].m_color;
    }
    if(x >= m_tX && x < m_tX + int(m_blocks[m_tType].m_dim) &&
       y >= m_ghostY && y < m_ghostY + int(m_blocks[m_tType].m_dim)) {
        if(m_blocks[m_tType].m_shape[m_tRot][y-m_ghostY][x-m_tX] != 0)
            return 0xFF;
    }
    return m_cup[4 + y][x];
}

uint8_t TetrisSim::getColor() {
    return m_blocks[m_tType].m_color;
}

Tetrimino& TetrisSim::incoming(int N) {
    return m_blocks[m_incoming[(m_incomingN + N) % TETRIS_INCOMING_LOOK_AHEAD]];
}

uint64_t TetrisSim::tick() {
    if(m_state < BV(TetrisState::noActAfter)) {
        if(std::chrono::duration_cast<std::chrono::milliseconds>(
           std::chrono::steady_clock::now() - m_prevTime).count() >=
           ((m_state & BV(TetrisState::atTheBottom)) ? 1000 : LEVEL_TO_SPEED(m_level))) {
            m_prevTime = std::chrono::steady_clock::now();
            act(TetrisAct::down);
        }
    }
    if(m_state & BV(TetrisState::blockFalling)) {
        if(m_tY < m_ghostY) {
            m_tY++;
            m_update |= BV(TetrisUpdate::needRedraw);
        }
        else {
            m_state &= ~BV(TetrisState::blockFalling);
            act(TetrisAct::down);
        }
    }
    if(m_state & BV(TetrisState::lineClearing)) {
        if(m_finishProgress > 0) {
            for(int i = 0; i < m_finishNum; ++i) {
                m_cup[4 + m_finishLines[i]][m_finishProgress - 1] = 0;
            }
            m_finishProgress--;
        }
        else {
            for(int i = 0; i < m_finishNum; ++i) {
                m_cup.erase(m_cup.begin() + 4 + m_finishLines[i]);
                m_cup.insert(m_cup.begin(), std::vector<uint8_t>(TETRIS_MATRIX_WIDTH, 0));
            }

            m_finishedLines += m_finishNum;
            m_combo += m_finishNum;
            m_score += m_combo * (m_level + 1);
            if(m_level < MAX_LEVEL)
                m_level += std::min(MAX_LEVEL, m_score/(10 * m_level));

            m_finishNum = 0;
            m_state &= ~BV(TetrisState::lineClearing);
            m_update |= BV(TetrisUpdate::scoreChange);
        }
        m_update |= BV(TetrisUpdate::needRedraw);
    }

    auto out = m_update;
    m_update = 0;
    return out;
}

bool TetrisSim::check(uint8_t type, int x, int y, uint8_t rot) {
    auto siz = m_blocks[type].m_dim;
    for(uint8_t i = 0; i < siz; ++i) {
        for(uint8_t j = 0; j < siz; ++j) {
            if(m_blocks[type].m_shape[rot][i][j]) {
                if(y + i >= TETRIS_MATRIX_HEIGHT ||
                   x + j >= TETRIS_MATRIX_WIDTH ||
                   x + j < 0 ||
                   y + i < -2)
                    return false;
                if(m_cup[4 + y + i][x + j])
                    return false;
            }
        }
    }
    return true;
}

bool TetrisSim::tryPutting(uint8_t type, int x, int y, uint8_t rot, bool fit) {
    const int ORDER[] = {0, -1, 1};
    bool flag = check(type, x, y, rot);

    if(fit) {
        for(int i = 0; i < 3 && !flag; ++i) {
            for(int j = 0; j < 3; ++j) {
                if(check(type, x + ORDER[j], y + ORDER[i], rot)) {
                    flag = true;
                    x += ORDER[j];
                    y += ORDER[i];
                    break;
                }
            }
        }
    }

    if(flag) {
        m_tType = type;
        m_tX = x;
        m_tY = y;
        m_tRot = rot;
        m_update |= BV(TetrisUpdate::needRedraw);
        return true;
    }
    return false;
}

void TetrisSim::updateGhost() {
    for(int i = m_tY + 1; i < TETRIS_MATRIX_HEIGHT; ++i) {
        if(!check(m_tType, m_tX, i, m_tRot)) {
            m_ghostY = i - 1;
            return;
        }
    }
}

void TetrisSim::finalize() {
    m_state &= ~BV(TetrisState::swapped);

    auto siz = m_blocks[m_tType].m_dim;
    for(uint8_t i = 0; i < siz; ++i) {
        for(uint8_t j = 0; j < siz; ++j) {
            if(m_blocks[m_tType].m_shape[m_tRot][i][j])
                m_cup[4 + m_tY + i][m_tX + j] = m_blocks[m_tType].m_color;
        }
    }

    for(int i = 0; i < TETRIS_MATRIX_HEIGHT; ++i) {
        int j = 0;
        for(; j < TETRIS_MATRIX_WIDTH && m_cup[4 + i][j]; ++j) {}
        if(j >= TETRIS_MATRIX_WIDTH)
            m_finishLines[m_finishNum++] = i;
    }
    if(m_finishNum) {
        m_state |= BV(TetrisState::lineClearing);
        m_update |= BV(TetrisUpdate::scoreChange);
        m_finishProgress = TETRIS_MATRIX_WIDTH;
    }
    else {
        m_combo = 0;
        m_update |= BV(TetrisUpdate::scoreChange);
    }

    newBlock();
}

void TetrisSim::newBlock(int type) {
    m_state &= ~BV(TetrisState::atTheBottom);
    m_update |= BV(TetrisUpdate::newBlockTaken);
    m_update |= BV(TetrisUpdate::needRedraw);

    if(type == -1) {
        m_tType = m_incoming[m_incomingN];
        m_incoming[m_incomingN] = m_rand() % m_blocks.size();
        m_incomingN = (m_incomingN + 1) % TETRIS_INCOMING_LOOK_AHEAD;
    }
    else {
        m_tType = type;
    }

    m_tY = -m_blocks[m_tType].m_dim / 2;
    m_tX = (TETRIS_MATRIX_WIDTH - m_blocks[m_tType].m_dim)/2;
    m_tRot = 0;

    if(!check(m_tType, m_tX, m_tY, m_tRot))
        m_update |= BV(TetrisUpdate::gameOver);
}

Tetrimino& TetrisSim::getHeld() {
    return m_blocks[m_hold];
}

int TetrisSim::getFinishedLines() {
    return m_finishedLines;
}

int TetrisSim::getCombo() {
    return m_combo;
}

int TetrisSim::getLevel() {
    return m_level;
}

int TetrisSim::getScore() {
    return m_score;
}
