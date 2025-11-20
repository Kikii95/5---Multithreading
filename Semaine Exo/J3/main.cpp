#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "Sphere.h"
#include "CollisionDetector.h"

// Configuration du benchmark
#define NUM_SPHERES 5000
#define WORLD_SIZE 1000.0f
#define MIN_RADIUS 1.0f
#define MAX_RADIUS 5.0f
#define MAX_COLLISIONS 100000

// Couleurs console
#define COLOR_RESET 7
#define COLOR_INFO 11      // Cyan
#define COLOR_SUCCESS 10   // Vert
#define COLOR_WARNING 14   // Jaune
#define COLOR_ERROR 12     // Rouge

HANDLE hConsole;

void SetColor(int color) {
    SetConsoleTextAttribute(hConsole, (WORD)color);
}

void PrintColored(int color, const char* format, ...) {
    SetColor(color);
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    SetColor(COLOR_RESET);
}

// Genere des spheres aleatoires dans l'espace 3D
Sphere* GenerateRandomSpheres(int count) {
    Sphere* spheres = new Sphere[count];

    for (int i = 0; i < count; i++) {
        float x = ((float)rand() / RAND_MAX) * WORLD_SIZE - (WORLD_SIZE / 2);
        float y = ((float)rand() / RAND_MAX) * WORLD_SIZE - (WORLD_SIZE / 2);
        float z = ((float)rand() / RAND_MAX) * WORLD_SIZE - (WORLD_SIZE / 2);
        float radius = MIN_RADIUS + ((float)rand() / RAND_MAX) * (MAX_RADIUS - MIN_RADIUS);

        spheres[i] = Sphere(Point3D(x, y, z), radius, i);
    }

    return spheres;
}

// Fonction pour obtenir le nombre de threads optimal
int GetOptimalThreadCount() {
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
}

void PrintSeparator() {
    PrintColored(COLOR_INFO, "================================================================\n");
}

int main() {
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    srand((unsigned int)time(NULL));

    PrintSeparator();
    PrintColored(COLOR_INFO, "        BENCHMARK COLLISION DETECTION 3D\n");
    PrintSeparator();

    printf("\n");
    PrintColored(COLOR_INFO, "Configuration:\n");
    printf("  - Nombre de spheres: %d\n", NUM_SPHERES);
    printf("  - Taille du monde: %.0f x %.0f x %.0f\n", WORLD_SIZE, WORLD_SIZE, WORLD_SIZE);
    printf("  - Rayon min/max: %.1f / %.1f\n", MIN_RADIUS, MAX_RADIUS);
    printf("  - Collisions max trackees: %d\n", MAX_COLLISIONS);

    int optimalThreads = GetOptimalThreadCount();
    printf("  - CPU cores detectes: %d\n", optimalThreads);

    printf("\n");
    PrintColored(COLOR_WARNING, "Generation des spheres...\n");
    Sphere* spheres = GenerateRandomSpheres(NUM_SPHERES);
    PrintColored(COLOR_SUCCESS, "Spheres generees!\n");

    printf("\n");
    PrintSeparator();

    // ============================================
    // BENCHMARK 1 : SINGLE-THREADED
    // ============================================
    PrintColored(COLOR_INFO, "TEST 1: SINGLE-THREADED\n");
    PrintSeparator();

    CollisionResult* resultSingle = new CollisionResult(MAX_COLLISIONS);

    // Calcul du nombre total de comparaisons
    long long totalComparisons = ((long long)NUM_SPHERES * (NUM_SPHERES - 1)) / 2;

    // Creer le moniteur de progression
    ProgressMonitor progressSingle(totalComparisons);
    progressSingle.Start();

    printf("\n");  // Ligne pour la barre de progression

    DWORD startSingle = GetTickCount();
    int collisionsSingle = DetectCollisionsSingleThreaded(spheres, NUM_SPHERES, resultSingle, &progressSingle);
    DWORD endSingle = GetTickCount();
    DWORD durationSingle = endSingle - startSingle;

    progressSingle.Stop();

    PrintColored(COLOR_SUCCESS, "Collisions detectees: %d\n", collisionsSingle);
    PrintColored(COLOR_SUCCESS, "Temps d'execution: %lu ms (%.3f secondes)\n",
        durationSingle, durationSingle / 1000.0f);

    printf("\n");
    PrintSeparator();

    // ============================================
    // BENCHMARK 2 : MULTI-THREADED (2 threads)
    // ============================================
    PrintColored(COLOR_INFO, "TEST 2: MULTI-THREADED (2 threads)\n");
    PrintSeparator();

    CollisionResult* result2Threads = new CollisionResult(MAX_COLLISIONS);

    ProgressMonitor progress2(totalComparisons);
    progress2.Start();

    printf("\n");  // Ligne pour la barre de progression

    DWORD start2 = GetTickCount();
    int collisions2 = DetectCollisionsMultiThreaded(spheres, NUM_SPHERES, result2Threads, 2, &progress2);
    DWORD end2 = GetTickCount();
    DWORD duration2 = end2 - start2;

    progress2.Stop();

    PrintColored(COLOR_SUCCESS, "Collisions detectees: %d\n", collisions2);
    PrintColored(COLOR_SUCCESS, "Temps d'execution: %lu ms (%.3f secondes)\n",
        duration2, duration2 / 1000.0f);

    float speedup2 = (float)durationSingle / duration2;
    if (speedup2 > 1.0f) {
        PrintColored(COLOR_SUCCESS, "Speedup: %.2fx plus rapide!\n", speedup2);
    } else {
        PrintColored(COLOR_WARNING, "Speedup: %.2fx (plus lent)\n", speedup2);
    }

    printf("\n");
    PrintSeparator();

    // ============================================
    // BENCHMARK 3 : MULTI-THREADED (4 threads)
    // ============================================
    PrintColored(COLOR_INFO, "TEST 3: MULTI-THREADED (4 threads)\n");
    PrintSeparator();

    CollisionResult* result4Threads = new CollisionResult(MAX_COLLISIONS);

    ProgressMonitor progress4(totalComparisons);
    progress4.Start();

    printf("\n");  // Ligne pour la barre de progression

    DWORD start4 = GetTickCount();
    int collisions4 = DetectCollisionsMultiThreaded(spheres, NUM_SPHERES, result4Threads, 4, &progress4);
    DWORD end4 = GetTickCount();
    DWORD duration4 = end4 - start4;

    progress4.Stop();

    PrintColored(COLOR_SUCCESS, "Collisions detectees: %d\n", collisions4);
    PrintColored(COLOR_SUCCESS, "Temps d'execution: %lu ms (%.3f secondes)\n",
        duration4, duration4 / 1000.0f);

    float speedup4 = (float)durationSingle / duration4;
    if (speedup4 > 1.0f) {
        PrintColored(COLOR_SUCCESS, "Speedup: %.2fx plus rapide!\n", speedup4);
    } else {
        PrintColored(COLOR_WARNING, "Speedup: %.2fx (plus lent)\n", speedup4);
    }

    printf("\n");
    PrintSeparator();

    // ============================================
    // BENCHMARK 4 : MULTI-THREADED (optimal threads)
    // ============================================
    PrintColored(COLOR_INFO, "TEST 4: MULTI-THREADED (%d threads - optimal)\n", optimalThreads);
    PrintSeparator();

    CollisionResult* resultOptimal = new CollisionResult(MAX_COLLISIONS);

    ProgressMonitor progressOpt(totalComparisons);
    progressOpt.Start();

    printf("\n");  // Ligne pour la barre de progression

    DWORD startOpt = GetTickCount();
    int collisionsOpt = DetectCollisionsMultiThreaded(spheres, NUM_SPHERES, resultOptimal, optimalThreads, &progressOpt);
    DWORD endOpt = GetTickCount();
    DWORD durationOpt = endOpt - startOpt;

    progressOpt.Stop();

    PrintColored(COLOR_SUCCESS, "Collisions detectees: %d\n", collisionsOpt);
    PrintColored(COLOR_SUCCESS, "Temps d'execution: %lu ms (%.3f secondes)\n",
        durationOpt, durationOpt / 1000.0f);

    float speedupOpt = (float)durationSingle / durationOpt;
    if (speedupOpt > 1.0f) {
        PrintColored(COLOR_SUCCESS, "Speedup: %.2fx plus rapide!\n", speedupOpt);
    } else {
        PrintColored(COLOR_WARNING, "Speedup: %.2fx (plus lent)\n", speedupOpt);
    }

    printf("\n");
    PrintSeparator();

    // ============================================
    // RESUME FINAL
    // ============================================
    PrintColored(COLOR_INFO, "RESUME COMPARATIF\n");
    PrintSeparator();

    printf("\n");
    printf("%-25s | %10s | %10s | %10s\n", "Methode", "Temps (ms)", "Speedup", "Collisions");
    printf("------------------------- | ---------- | ---------- | ----------\n");
    printf("%-25s | %10lu | %10s | %10d\n",
        "Single-threaded", durationSingle, "1.00x", collisionsSingle);
    printf("%-25s | %10lu | %9.2fx | %10d\n",
        "Multi-threaded (2)", duration2, speedup2, collisions2);
    printf("%-25s | %10lu | %9.2fx | %10d\n",
        "Multi-threaded (4)", duration4, speedup4, collisions4);
    printf("%-25s | %10lu | %9.2fx | %10d\n",
        "Multi-threaded (optimal)", durationOpt, speedupOpt, collisionsOpt);

    printf("\n");

    // Determiner la meilleure methode
    DWORD bestTime = durationSingle;
    const char* bestMethod = "Single-threaded";

    if (duration2 < bestTime) { bestTime = duration2; bestMethod = "Multi-threaded (2)"; }
    if (duration4 < bestTime) { bestTime = duration4; bestMethod = "Multi-threaded (4)"; }
    if (durationOpt < bestTime) { bestTime = durationOpt; bestMethod = "Multi-threaded (optimal)"; }

    PrintColored(COLOR_SUCCESS, "Meilleure methode: %s (%lu ms)\n", bestMethod, bestTime);

    printf("\n");
    PrintSeparator();

    // Calcul de la complexite
    long long totalComparisons = ((long long)NUM_SPHERES * (NUM_SPHERES - 1)) / 2;
    PrintColored(COLOR_INFO, "Complexite: O(n^2) - %lld comparaisons effectuees\n", totalComparisons);

    printf("\n");
    PrintColored(COLOR_WARNING, "Note: Le speedup depend fortement de la charge CPU et du nombre de cores.\n");
    PrintColored(COLOR_WARNING, "      L'overhead de creation/synchronisation des threads peut reduire le gain.\n");

    printf("\n");

    // Nettoyage
    delete resultSingle;
    delete result2Threads;
    delete result4Threads;
    delete resultOptimal;
    delete[] spheres;

    return 0;
}
