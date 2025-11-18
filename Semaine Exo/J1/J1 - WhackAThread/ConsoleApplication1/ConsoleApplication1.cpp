#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <time.h>
#include "Game.h"
#include "utils.h"
#include "Message.h"

#define UI_HEADER_Y 0
#define UI_GAME_Y 3
#define UI_STATUS_Y 20
#define UI_HELP_Y 22
#define UI_EVENTS_Y 24

void RenderUI(Game* game) {
    PrintAt(0, UI_HEADER_Y, "====================================", COLOR_CYAN);
    PrintAt(0, UI_HEADER_Y + 1, "       WHACK-A-THREAD GAME", COLOR_YELLOW);
    PrintAt(0, UI_HEADER_Y + 2, "====================================", COLOR_CYAN);

    SetConsoleCursor(0, UI_GAME_Y);
    printf("Active Threads (press corresponding key):\n\n");

    int livingCount = 0;
    for (int i = 0; i < game->moleCount; i++) {
        Mole* mole = game->moles[i];
        if (mole != NULL && IsMoleAlive(mole)) {
            DWORD elapsed = GetTickCount() - mole->startTime;
            DWORD remaining = mole->lifetimeMs - elapsed;
            float progress = (float)elapsed / mole->lifetimeMs;

            WORD color = COLOR_GREEN;
            if (progress > 0.7f) color = COLOR_RED;
            else if (progress > 0.4f) color = COLOR_YELLOW;

            PrintAtf(2, UI_GAME_Y + 2 + livingCount, color,
                "Thread #%d: Press [%c] - %.1fs remaining",
                mole->id, mole->key, remaining / 1000.0f);

            livingCount++;
        }
    }

    for (int i = livingCount; i < 10; i++) {
        PrintAt(2, UI_GAME_Y + 2 + i, "                                                  ", COLOR_WHITE);
    }

    PrintAt(0, UI_STATUS_Y, "====================================", COLOR_CYAN);
    PrintAtf(0, UI_STATUS_Y + 1, COLOR_WHITE, "Score: %d | Living Threads: %d | Difficulty: %d",
        GetScore(game), livingCount, game->difficulty);

    PrintAt(0, UI_HELP_Y, "Press [ESC] to quit | Press [1-3] to change difficulty", COLOR_MAGENTA);
}

int main() {
    srand((unsigned int)time(NULL));

    InitConsole();

    printf("=== WHACK-A-THREAD ===\n\n");
    printf("Select difficulty:\n");
    printf("  [1] Easy   - Slow spawn, long lifetime\n");
    printf("  [2] Medium - Moderate spawn, medium lifetime\n");
    printf("  [3] Hard   - Fast spawn, short lifetime\n\n");
    printf("Choice: ");

    int difficulty = 1;
    if (_kbhit()) {
        int ch = _getch();
        if (ch >= '1' && ch <= '3') {
            difficulty = ch - '0';
        }
    }
    else {
        Sleep(2000);
        difficulty = 2;
    }

    system("cls");

    Game* game = CreateGame(difficulty);
    if (game == NULL) {
        printf("Failed to create game!\n");
        return 1;
    }

    if (!StartGame(game)) {
        printf("Failed to start game (thread creation failed)!\n");
        DestroyGame(game);
        return 1;
    }

    printf("Game starting...\n");
    Sleep(500);

    InitConsole(); 

    DWORD lastUpdate = GetTickCount();
    while (1) {
        if (_kbhit()) {
            int ch = _getch();

            if (ch == 27) {
                break;
            }

            if (ch >= '1' && ch <= '3') {
                int newDifficulty = ch - '0';
                if (newDifficulty != game->difficulty) {
                    StopGame(game);
                    DestroyGame(game);

                    game = CreateGame(newDifficulty);
                    if (!StartGame(game)) {
                        printf("Failed to restart game!\n");
                        break;
                    }

                    system("cls");
                    InitConsole();
                }
                continue;
            }

            char key = (char)ch;
            int moleId = -1;
            BOOL killed = TryKillMole(game, key, &moleId);

            if (killed) {
                PrintAtf(0, UI_STATUS_Y + 3, COLOR_GREEN,
                    "HIT! Thread #%d killed! +1 point              ", moleId);
            }
            else {
                PrintAtf(0, UI_STATUS_Y + 3, COLOR_RED,
                    "MISS! Wrong key! -1 point                    ");
            }
        }

        // Process message queue (pop events from threads)
        Message msg;
        while (TryPopMessage(game->messageQueue, &msg)) {
            switch (msg.type) {
            case MSG_MOLE_SPAWNED:
                // Optional: log spawn events
                break;
            case MSG_MOLE_KILLED:
                PrintAtf(0, UI_EVENTS_Y, COLOR_GREEN,
                    "[EVENT] Mole #%d KILLED! +1 point        ", msg.moleId);
                break;
            case MSG_MOLE_EXPIRED:
                PrintAtf(0, UI_EVENTS_Y, COLOR_RED,
                    "[EVENT] Mole #%d EXPIRED! -1 point       ", msg.moleId);
                break;
            }
        }

        DWORD now = GetTickCount();
        if (now - lastUpdate >= 100) {
            RenderUI(game);
            lastUpdate = now;
        }

        Sleep(20);
    }

    StopGame(game);

    system("cls");
    printf("\n\n");
    printf("====================================\n");
    printf("         GAME OVER\n");
    printf("====================================\n");
    printf("\nFinal Score: %d\n\n", GetScore(game));
    printf("Press any key to exit...\n");

    _getch();

    DestroyGame(game);

    return 0;
}
