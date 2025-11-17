#include <iostream>
#include <conio.h> 
#include <cctype>
#include <thread>
#include <chrono>

#include "Game.h"

using namespace std::chrono;

int main() {
    std::cout << "Whack-a-thread (modulaire). Appuie ESC pour quitter.\n";
    int difficulty = 2; // 1 facile, 2 normal, 3 difficile
    Game game(difficulty);
    game.start();

    auto lastStatus = steady_clock::now();
    while (true) {
        if (_kbhit()) {
            int ch = _getch();
            if (ch == 27) { // ESC
                break;
            }
            char pressed = static_cast<char>(std::tolower(ch));
            int moleId = -1;
            bool ok = game.tryKill(pressed, moleId);
            if (ok) {
                std::cout << "TUE: Mole #" << moleId << " +1 point\n";
            }
            else {
                std::cout << "RIEN (mauvaise touche) -1 point\n";
            }
        }

        auto now = steady_clock::now();
        if (duration_cast<milliseconds>(now - lastStatus).count() >= 400) {
            std::cout << "SCORE: " << game.getScore() << " | Moles vivantes: " << game.livingMoles() << "\n";
            lastStatus = now;
        }

        std::this_thread::sleep_for(milliseconds(20));
    }

    game.stop();
    std::cout << "Jeu termine. Score final: " << game.getScore() << "\n";
    return 0;
}
