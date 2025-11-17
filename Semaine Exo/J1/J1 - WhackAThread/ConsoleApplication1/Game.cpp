#include "Game.h"
#include <chrono>
#include <iostream>

using namespace std::chrono;

Game::Game(int difficulty)
    : _running(false), _score(0)
{
    _spawnIntervalMs = 1200 - (difficulty * 300);
    if (_spawnIntervalMs < 250) _spawnIntervalMs = 250;

    _minLifeMs = 500 - (difficulty * 120);
    if (_minLifeMs < 120) _minLifeMs = 120;
    _maxLifeMs = 1500 - (difficulty * 250);
    if (_maxLifeMs < 300) _maxLifeMs = 300;

    _factory = std::make_unique<MoleFactory>(_minLifeMs, _maxLifeMs);
}

Game::~Game() {
    stop();
}

void Game::start() {
    _running = true;
    _spawner = std::thread(&Game::spawnerLoop, this);
}

void Game::stop() {
    _running = false;
    if (_spawner.joinable()) _spawner.join();
    _factory->cleanup();
}

void Game::spawnerLoop() {
    while (_running) {
        int id = _factory->spawn(0);
        std::cout << "Mole #" << id << " spawn (appuie la bonne touche)\n";
        std::this_thread::sleep_for(milliseconds(_spawnIntervalMs));
        _factory->cleanup(); 
    }
}

void Game::spawnOne() {
    _factory->spawn(0);
}

bool Game::tryKill(char key, int& outMoleId) {
    bool killed = _factory->tryKillByKey(key, outMoleId);
    if (killed) {
        _score.fetch_add(1);
    }
    else {
        _score.fetch_sub(1);
    }
    return killed;
}

int Game::getScore() const {
    return _score.load();
}

int Game::livingMoles() const {
    return _factory->countAlive();
}
