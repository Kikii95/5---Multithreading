#ifndef MOLEFACTORY_H
#define MOLEFACTORY_H

#include "Mole.h"
#include <vector>
#include <memory>
#include <mutex>

class MoleFactory {
public:
    MoleFactory(int minLifeMs, int maxLifeMs);

    int spawn(char key);

    bool tryKillByKey(char key, int& outMoleId);

    void cleanup();

    int countAlive() const;

    ~MoleFactory();

private:
    mutable std::mutex _mutex;
    std::vector<std::shared_ptr<Mole>> _moles;
    int _nextId;
    int _minLifeMs, _maxLifeMs;

    char randomKey() const;
    int randomLife() const;
};

#endif // MOLEFACTORY_H
