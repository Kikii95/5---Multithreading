#ifndef GAME_H
#define GAME_H

#include "MoleFactory.h"
#include <thread>
#include <atomic>
#include <functional>

class Game {
public:
    Game(int difficulty); 
    ~Game();

    void start();
    void stop();

    void spawnOne();
    bool tryKill(char key, int& outMoleId);

    int getScore() const;
    int livingMoles() const;

private:
    std::unique_ptr<MoleFactory> _factory;
    std::thread _spawner;
    std::atomic<bool> _running;
    std::atomic<int> _score;

    int _spawnIntervalMs;
    int _minLifeMs;
    int _maxLifeMs;

    void spawnerLoop();
};

#endif // GAME_H
