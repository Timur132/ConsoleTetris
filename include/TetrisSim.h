#ifndef TETRISSIM_H
#define TETRISSIM_H

#include <chrono>
#include <vector>
#include <random>

#define TETRIS_MATRIX_WIDTH 10
#define TETRIS_MATRIX_HEIGHT 20
#define TETRIS_INCOMING_LOOK_AHEAD 5

#define MAX_LEVEL 100
#define LEVEL_TO_SPEED(level) (1100 - 10*(level))

#define BV(a) (1 << int(a))

enum class TetrisAct {
    clockwise = 0,
    counterClockwise,
    left,
    right,
    down,
    drop,
    hold
};

enum class TetrisUpdate {
    needRedraw = 0,
    scoreChange,
    newBlockTaken,
    gameOver,
    swapped
};

enum class TetrisState {
    atTheBottom = 0,
    swapped,
    noActAfter,
    blockFalling,
    lineClearing
};

class Tetrimino {
public:
    Tetrimino(std::vector<std::vector<bool>>&& shape, unsigned char color);

    size_t m_dim;
    unsigned char m_color = 1;
    std::vector<std::vector<std::vector<bool>>> m_shape{4};
protected:
};

class TetrisSim {
public:
    TetrisSim(std::vector<Tetrimino>&& blocks);
    TetrisSim(TetrisSim& in);

    bool act(TetrisAct a);
    uint8_t cup(int y, int x);
    uint8_t getColor();
    Tetrimino& incoming(int N);
    uint64_t tick();
    Tetrimino& getHeld();
    int getFinishedLines();
    int getCombo();
    int getLevel();
    int getScore();

protected:
    std::vector<std::vector<uint8_t>> m_cup =
        std::vector<std::vector<uint8_t>>
        (TETRIS_MATRIX_HEIGHT + 4, std::vector<uint8_t>(TETRIS_MATRIX_WIDTH, 0));
    std::vector<Tetrimino> m_blocks;

    int m_tType = 0;
    int m_tX = 0;
    int m_tY = -1;
    int m_tRot = 0;
    int m_ghostY = 0;

    std::mt19937 m_rand;

    std::chrono::steady_clock::time_point m_prevTime = std::chrono::steady_clock::now();
    uint64_t m_update = 0;
    uint64_t m_state = 0;

    int m_finishLines[4];
    int m_finishNum = 0;
    int m_finishProgress = 0;

    int m_hold = -1;

    int m_incoming[TETRIS_INCOMING_LOOK_AHEAD];
    int m_incomingN = 0;

    int m_finishedLines = 0;
    int m_combo = 0;
    int m_level = 1;
    int m_score = 0;

    bool check(uint8_t type, int x, int y, uint8_t rot);
    bool tryPutting(uint8_t type, int x, int y, uint8_t rot, bool fit = false);
    void updateGhost();
    void finalize();
    void newBlock(int type = -1);
};

#endif // TETRISSIM_H
