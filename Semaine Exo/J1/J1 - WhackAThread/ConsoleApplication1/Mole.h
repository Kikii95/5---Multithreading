#ifndef MOLE_H
#define MOLE_H

#include <thread>
#include <atomic>
#include <functional>
#include <chrono>

class Mole {
public:
    using Clock = std::chrono::steady_clock;
    using ms = std::chrono::milliseconds;

    Mole(int id, char key, ms lifetime,
        std::function<void(int)> onExpire,
        std::function<void(int)> onKill);
    ~Mole();

    int getId() const;
    char getKey() const;
    bool isAlive() const;

    void start();

    bool kill();

    void join();

private:
    int _id;
    char _key;
    ms _lifetime;
    std::atomic<bool> _alive;
    std::thread _thread;
    std::function<void(int)> _onExpire;
    std::function<void(int)> _onKill;

    void run();
};

#endif // MOLE_H
