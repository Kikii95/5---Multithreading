#include "MoleFactory.h"
#include <random>
#include <algorithm>
#include <chrono>
#include <iostream>

using namespace std::chrono;

MoleFactory::MoleFactory(int minLifeMs, int maxLifeMs)
    : _nextId(1), _minLifeMs(minLifeMs), _maxLifeMs(maxLifeMs)
{
}

MoleFactory::~MoleFactory() {
    std::lock_guard<std::mutex> lk(_mutex);
    for (auto& m : _moles) {
        if (m) m->join();
    }
}

static int rndInt(int a, int b) {
    static thread_local std::mt19937 rng((unsigned)steady_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<int> dist(a, b);
    return dist(rng);
}

char MoleFactory::randomKey() const {
    return char('a' + rndInt(0, 25));
}

int MoleFactory::randomLife() const {
    return rndInt(_minLifeMs, _maxLifeMs);
}

int MoleFactory::spawn(char key) {
    auto onExpire = [this](int id) {
       (void)id;
        };
    auto onKill = [this](int id) {
        (void)id;
        };

    std::lock_guard<std::mutex> lk(_mutex);
    int id = _nextId++;
    char k = (key == 0) ? randomKey() : key;
    int life = randomLife();
    auto m = std::make_shared<Mole>(id, k, Mole::ms(life), onExpire, onKill);
    m->start();
    _moles.push_back(m);
    std::cout << "Mole #" << id
        << " | Touche : '" << k
        << "' | Durée : " << life << " ms\n";

    return id;

}

bool MoleFactory::tryKillByKey(char key, int& outMoleId) {
    std::lock_guard<std::mutex> lk(_mutex);
    for (auto it = _moles.rbegin(); it != _moles.rend(); ++it) {
        auto& m = *it;
        if (m && m->isAlive() && m->getKey() == key) {
            bool killed = m->kill();
            outMoleId = m->getId();
            return killed;
        }
    }
    return false;
}

void MoleFactory::cleanup() {
    std::lock_guard<std::mutex> lk(_mutex);
    for (auto& m : _moles) {
        if (m && !m->isAlive()) {
            m->join();
        }
    }
    _moles.erase(std::remove_if(_moles.begin(), _moles.end(),
        [](const std::shared_ptr<Mole>& m) {
            return !m || !m->isAlive();
        }), _moles.end());
}

int MoleFactory::countAlive() const {
    std::lock_guard<std::mutex> lk(_mutex);
    int c = 0;
    for (auto& m : _moles) if (m && m->isAlive()) ++c;
    return c;
}
