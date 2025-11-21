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
    printf("  MULTITHREADING COLLISION TEST - SFML\n");
    printf("  Boules Rebondissantes - Battle Royale\n");
    printf("========================================\n");
    printf("\n");
    printf("CONTROLES:\n");
    printf("  ESC ou fermer fenetre - Quitter\n");
    printf("\n");
    printf("COULEURS DES BOULES = THREAD:\n");
    printf("  Rouge   - Thread 0\n");
    printf("  Vert    - Thread 1\n");
    printf("  Bleu    - Thread 2\n");
    printf("  Jaune   - Thread 3\n");
    printf("\n");
    printf("Les boules changent de couleur quand\n");
    printf("elles changent de zone (thread)!\n");
    printf("\n");
    printf("========================================\n");
    printf("\n");
}

int main() {
    // Initialiser random
    Utils::InitRandom();

    // Afficher instructions
    PrintInstructions();

    // Configuration
    const int NB_THREADS = Config::NB_THREADS;
    const int NB_BALLS = Config::NB_BALLS;
    const int GRID_WIDTH = (int)(Config::MAP_WIDTH / Config::CELL_SIZE);
    const int GRID_HEIGHT = (int)(Config::MAP_HEIGHT / Config::CELL_SIZE);

    // Créer renderer SFML
    Renderer renderer;

    // Créer manager
    ThreadManager manager(NB_THREADS, GRID_WIDTH, GRID_HEIGHT, Config::CELL_SIZE);

    // Spawner balls
    manager.SpawnBalls(NB_BALLS);

    // Démarrer threads
    manager.StartAll();

    printf("[MAIN] Game started! Watch the window!\n\n");

    // Boucle principale de rendu
    int frameCount = 0;
    bool running = true;

    while (running && renderer.IsOpen()) {
        // Traiter événements fenêtre
        renderer.ProcessEvents();

        // Vérifier game over toutes les 60 frames (~1 sec)
        if (++frameCount % 60 == 0) {
            manager.CheckGameOver();

            if (manager.IsGameOver()) {
                printf("\n");
                printf("========================================\n");
                printf("           GAME OVER!\n");
                printf("========================================\n");

                if (manager.GetWinnerID() >= 0) {
                    printf("  WINNER: Ball %d\n", manager.GetWinnerID());
                } else {
                    printf("  DRAW (no survivors)\n");
                }

                printf("========================================\n");

                manager.PrintStats();

                // Laisser fenêtre ouverte 5 secondes
                Sleep(5000);
                running = false;
            }
        }

        // Afficher stats toutes les 5 secondes
        if (frameCount % 300 == 0) {
            manager.PrintStats();
        }

        // RENDERING
        renderer.Clear();
        renderer.DrawGrid();

        // Dessiner toutes les boules (lecture seule, thread-safe)
        std::vector<Entity*> allEntities = manager.GetAllEntities();
        for (Entity* entity : allEntities) {
            renderer.DrawBall(entity);
        }

        // Afficher HUD
        int aliveCount = manager.GetAliveCount();
        renderer.DrawHUD(aliveCount, 0, 0);

        renderer.Display();

        // Pas besoin de Sleep, SFML limite à 60 FPS
    }

    // Arrêter threads
    printf("\n[MAIN] Stopping threads...\n");
    manager.StopAll();
    manager.JoinAll();

    // Stats finales
    printf("\n=== FINAL STATS ===\n");
    manager.PrintStats();

    printf("\n=== TEST COMPLETED ===\n");

    return 0;
}
