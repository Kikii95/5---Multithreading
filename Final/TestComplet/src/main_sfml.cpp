#include "Threading/ThreadManager.h"
#include "Rendering/Renderer.h"
#include "Core/Utils.h"
#include <stdio.h>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
void PrintInstructions() {
    printf("\n");
    printf("========================================\n");
    printf("     TEST COMPLET - MULTITHREADING\n");
    printf("  Players + Bonus - Battle Royale\n");
    printf("========================================\n");
    printf("\n");
    printf("CONTROLES:\n");
    printf("  ESC ou fermer fenetre - Quitter\n");
    printf("\n");
    printf("COULEURS PLAYERS = THREAD:\n");
    printf("  Rouge   - Thread 0\n");
    printf("  Vert    - Thread 1\n");
    printf("  Bleu    - Thread 2\n");
    printf("  Jaune   - Thread 3\n");
    printf("\n");
    printf("BONUS (Hexagones):\n");
    printf("  Rouge vif   - HP Boost (+20 HP)\n");
    printf("  Bleu vif    - Speed Boost (+20%%)\n");
    printf("  Jaune dore  - Shield (1 coup)\n");
    printf("  Violet vif  - Damage Boost (+50%%)\n");
    printf("\n");
    printf("Les players changent de couleur quand\n");
    printf("ils changent de zone (thread)!\n");
    printf("\n");
    printf("Le shield apparait comme un cercle dore!\n");
    printf("\n");
    printf("========================================\n");
    printf("\n");
}
int main() {
    Utils::InitRandom();
    PrintInstructions();
    const int NB_THREADS = Config::NB_THREADS;
    const int NB_PLAYERS = Config::NB_PLAYERS;
    const int NB_BONUS = Config::NB_BONUS;
    const int GRID_WIDTH = (int)(Config::MAP_WIDTH / Config::CELL_SIZE);
    const int GRID_HEIGHT = (int)(Config::MAP_HEIGHT / Config::CELL_SIZE);
    Renderer renderer;
    ThreadManager manager(NB_THREADS, GRID_WIDTH, GRID_HEIGHT, Config::CELL_SIZE);
    manager.SpawnBalls(NB_PLAYERS);  
    manager.SpawnBonus(NB_BONUS);
    manager.StartAll();
    printf("[MAIN] Game started! Watch the window!\n\n");
    int frameCount = 0;
    bool running = true;
    while (running && renderer.IsOpen()) {
        renderer.ProcessEvents();
        if (++frameCount % 60 == 0) {
            manager.CheckGameOver();
            if (manager.IsGameOver()) {
                printf("\n");
                printf("========================================\n");
                printf("           GAME OVER!\n");
                printf("========================================\n");
                if (manager.GetWinnerID() >= 0) {
                    printf("  WINNER: Player %d\n", manager.GetWinnerID());
                } else {
                    printf("  DRAW (no survivors)\n");
                }
                printf("========================================\n");
                manager.PrintStats();
                Sleep(5000);
                running = false;
            }
        }
        if (frameCount % 300 == 0) {
            manager.PrintStats();
        }
        renderer.Clear();
        renderer.DrawGrid();
        std::vector<Entity*> allEntities = manager.GetAllEntities();
        for (Entity* entity : allEntities) {
            renderer.DrawEntity(entity);  
        }
        int aliveCount = manager.GetAliveCount();
        renderer.DrawHUD(aliveCount, 0, 0, 0);
        renderer.Display();
    }
    printf("\n[MAIN] Stopping threads...\n");
    manager.StopAll();
    manager.JoinAll();
    printf("\n=== FINAL STATS ===\n");
    manager.PrintStats();
    printf("\n=== TEST COMPLETED ===\n");
    return 0;
}
