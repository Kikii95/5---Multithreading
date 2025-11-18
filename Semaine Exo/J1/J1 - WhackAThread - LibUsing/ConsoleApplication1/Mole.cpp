#include "Mole.h"
#include <chrono>
#include <iostream>

using namespace std::chrono;

Mole::Mole(int id, char key, ms lifetime,
    std::function<void(int)> onExpire,
    std::function<void(int)> onKill)
    : _id(id), _key(key), _lifetime(lifetime),
    _alive(false), _onExpire(onExpire), _onKill(onKill)
{
}

Mole::~Mole() {
    if (_thread.joinable()) _thread.join();
}

int Mole::getId() const { return _id; }
char Mole::getKey() const { return _key; }
bool Mole::isAlive() const { return _alive.load(); }

void Mole::start() {
    _alive = true;
    _thread = std::thread(&Mole::run, this);
}

bool Mole::kill() {
    bool expected = true;
    if (_alive.compare_exchange_strong(expected, false)) {
        if (_onKill) _onKill(_id);
        return true;
    }
    return false;
}

void Mole::run() {
    auto start = steady_clock::now();
    while (_alive) {
        auto now = steady_clock::now();
        if (duration_cast<milliseconds>(now - start) >= _lifetime) {
            bool expected = true;
            if (_alive.compare_exchange_strong(expected, false)) {
                if (_onExpire) _onExpire(_id);
                std::cout << "KILL: Thread #" << _id << "\n";
            }
            break;
        }
        std::this_thread::sleep_for(milliseconds(10));
    }
}

void Mole::join() {
    if (_thread.joinable()) _thread.join();
}
