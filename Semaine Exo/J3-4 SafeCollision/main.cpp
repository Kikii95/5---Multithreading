#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "Sphere.h"
#include "SafeCollisionDetector.h"

// Configuration du benchmark
#define NUM_SPHERES 10000
#define WORLD_SIZE 500.0f
#define MIN_RADIUS 1.0f
#define MAX_RADIUS 50.0f
#define MAX_COLLISIONS 1000000

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
    PrintColored(COLOR_INFO, "    BENCHMARK COLLISION DETECTION avec SAFEQUEUE\n");
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

    // Calcul du nombre total de comparaisons
    long long totalComparisons = ((long long)NUM_SPHERES * (NUM_SPHERES - 1)) / 2;

    // Buffer pour les resultats
    CollisionPair* resultBuffer = new CollisionPair[MAX_COLLISIONS];

    // ============================================
    // BENCHMARK 1 : SINGLE-THREADED (baseline)
    // ============================================
    PrintColored(COLOR_INFO, "TEST 1: SINGLE-THREADED (baseline)\n");
    PrintSeparator();

    ProgressMonitor progressSingle(totalComparisons);
    progressSingle.Start();

    printf("\n");

    DWORD startSingle = GetTickCount();
    int collisionsSingle = DetectCollisionsSingleThreaded(spheres, NUM_SPHERES, resultBuffer, MAX_COLLISIONS, &progressSingle);
    DWORD endSingle = GetTickCount();
    DWORD durationSingle = endSingle - startSingle;

    progressSingle.Stop();

    PrintColored(COLOR_SUCCESS, "Collisions detectees: %d\n", collisionsSingle);
    PrintColored(COLOR_SUCCESS, "Temps d'execution: %lu ms (%.3f secondes)\n",
        durationSingle, durationSingle / 1000.0f);

    printf("\n");
    PrintSeparator();

    // ============================================
    // BENCHMARK 2 : SafeQueue avec 2 Producers + 2 Consumers
    // ============================================
    PrintColored(COLOR_INFO, "TEST 2: SAFEQUEUE (2 Producers + 2 Consumers)\n");
    PrintSeparator();

    ProgressMonitor progress2(totalComparisons);
    progress2.Start();

    printf("\n");

    DWORD start2 = GetTickCount();
    int collisions2 = DetectCollisionsWithSafeQueue(spheres, NUM_SPHERES, resultBuffer, MAX_COLLISIONS, 2, 2, &progress2);
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
    // BENCHMARK 3 : SafeQueue avec 4 Producers + 2 Consumers
    // ============================================
    PrintColored(COLOR_INFO, "TEST 3: SAFEQUEUE (4 Producers + 2 Consumers)\n");
    PrintSeparator();

    ProgressMonitor progress3(totalComparisons);
    progress3.Start();

    printf("\n");

    DWORD start3 = GetTickCount();
    int collisions3 = DetectCollisionsWithSafeQueue(spheres, NUM_SPHERES, resultBuffer, MAX_COLLISIONS, 4, 2, &progress3);
    DWORD end3 = GetTickCount();
    DWORD duration3 = end3 - start3;

    progress3.Stop();

    PrintColored(COLOR_SUCCESS, "Collisions detectees: %d\n", collisions3);
    PrintColored(COLOR_SUCCESS, "Temps d'execution: %lu ms (%.3f secondes)\n",
        duration3, duration3 / 1000.0f);

    float speedup3 = (float)durationSingle / duration3;
    if (speedup3 > 1.0f) {
        PrintColored(COLOR_SUCCESS, "Speedup: %.2fx plus rapide!\n", speedup3);
    } else {
        PrintColored(COLOR_WARNING, "Speedup: %.2fx (plus lent)\n", speedup3);
    }

    printf("\n");
    PrintSeparator();

    // ============================================
    // BENCHMARK 4 : SafeQueue avec optimal Producers + optimal Consumers
    // ============================================
    int numProducers = optimalThreads / 2;
    int numConsumers = optimalThreads - numProducers;

    PrintColored(COLOR_INFO, "TEST 4: SAFEQUEUE (%d Producers + %d Consumers)\n", numProducers, numConsumers);
    PrintSeparator();

    ProgressMonitor progress4(totalComparisons);
    progress4.Start();

    printf("\n");

    DWORD start4 = GetTickCount();
    int collisions4 = DetectCollisionsWithSafeQueue(spheres, NUM_SPHERES, resultBuffer, MAX_COLLISIONS, numProducers, numConsumers, &progress4);
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
    // BENCHMARK 5 : SafeQueue + ACCUMULATION LOCALE (optimisé)
    // ============================================
    PrintColored(COLOR_INFO, "TEST 5: SAFEQUEUE OPTIMISE (%d Producers + %d Consumers + Accumulation Locale)\n", numProducers, numConsumers);
    PrintSeparator();

    ProgressMonitor progress5(totalComparisons);
    progress5.Start();

    printf("\n");

    DWORD start5 = GetTickCount();
    int collisions5 = DetectCollisionsWithSafeQueueOptimized(spheres, NUM_SPHERES, resultBuffer, MAX_COLLISIONS, numProducers, numConsumers, &progress5);
    DWORD end5 = GetTickCount();
    DWORD duration5 = end5 - start5;

    progress5.Stop();

    PrintColored(COLOR_SUCCESS, "Collisions detectees: %d\n", collisions5);
    PrintColored(COLOR_SUCCESS, "Temps d'execution: %lu ms (%.3f secondes)\n",
        duration5, duration5 / 1000.0f);

    float speedup5 = (float)durationSingle / duration5;
    if (speedup5 > 1.0f) {
        PrintColored(COLOR_SUCCESS, "Speedup: %.2fx plus rapide!\n", speedup5);
    } else {
        PrintColored(COLOR_WARNING, "Speedup: %.2fx (plus lent)\n", speedup5);
    }

    // Comparaison avec version non-optimisée
    float gainVsNonOptimized = (float)duration4 / duration5;
    if (gainVsNonOptimized > 1.0f) {
        PrintColored(COLOR_SUCCESS, "Gain vs SafeQueue standard: %.2fx plus rapide!\n", gainVsNonOptimized);
    } else {
        PrintColored(COLOR_WARNING, "Gain vs SafeQueue standard: %.2fx\n", gainVsNonOptimized);
    }

    printf("\n");
    PrintSeparator();

    // ============================================
    // RESUME FINAL
    // ============================================
    PrintColored(COLOR_INFO, "RESUME COMPARATIF\n");
    PrintSeparator();

    printf("\n");
    printf("%-45s | %10s | %10s | %10s\n", "Methode", "Temps (ms)", "Speedup", "Collisions");
    printf("--------------------------------------------- | ---------- | ---------- | ----------\n");
    printf("%-45s | %10lu | %10s | %10d\n",
        "Single-threaded (baseline)", durationSingle, "1.00x", collisionsSingle);
    printf("%-45s | %10lu | %9.2fx | %10d\n",
        "SafeQueue (2P + 2C)", duration2, speedup2, collisions2);
    printf("%-45s | %10lu | %9.2fx | %10d\n",
        "SafeQueue (4P + 2C)", duration3, speedup3, collisions3);
    printf("%-45s | %10lu | %9.2fx | %10d\n",
        "SafeQueue (optimal)", duration4, speedup4, collisions4);
    printf("%-45s | %10lu | %9.2fx | %10d\n",
        "SafeQueue (optimal) + Accumulation Locale", duration5, speedup5, collisions5);

    printf("\n");

    // Determiner la meilleure methode
    DWORD bestTime = durationSingle;
    const char* bestMethod = "Single-threaded";

    if (duration2 < bestTime) { bestTime = duration2; bestMethod = "SafeQueue (2P + 2C)"; }
    if (duration3 < bestTime) { bestTime = duration3; bestMethod = "SafeQueue (4P + 2C)"; }
    if (duration4 < bestTime) { bestTime = duration4; bestMethod = "SafeQueue (optimal)"; }
    if (duration5 < bestTime) { bestTime = duration5; bestMethod = "SafeQueue (optimal) + Accumulation Locale"; }

    PrintColored(COLOR_SUCCESS, "Meilleure methode: %s (%lu ms)\n", bestMethod, bestTime);

    printf("\n");
    PrintSeparator();

    PrintColored(COLOR_INFO, "Pattern: Producer-Consumer avec SafeQueue\n");
    printf("  - Producers: Detectent collisions et les envoient dans la queue\n");
    printf("  - Consumers: Recuperent collisions de la queue et les stockent\n");
    printf("  - SafeQueue: Synchronisation CRITICAL_SECTION + CONDITION_VARIABLE\n");

    printf("\n");
    PrintColored(COLOR_INFO, "Optimisation: Accumulation Locale (TEST 5)\n");
    printf("  - Consumers accumulent dans buffer LOCAL (10k items)\n");
    printf("  - Flush vers result buffer seulement quand plein ou fin\n");
    printf("  - Reduction locks result buffer: ~1000x -> ~1-2x\n");
    printf("  - memcpy en bloc au lieu de copies individuelles\n");

    printf("\n");
    PrintColored(COLOR_INFO, "Complexite: O(n^2) - %lld comparaisons effectuees\n", totalComparisons);

    printf("\n");
    PrintColored(COLOR_WARNING, "Note: Le pattern Producer-Consumer peut ajouter de l'overhead\n");
    PrintColored(COLOR_WARNING, "      si les consumers ne sont pas assez rapides (queue pleine).\n");
    PrintColored(COLOR_WARNING, "      L'accumulation locale reduit la contention sur result buffer.\n");

    printf("\n");

    // Nettoyage
    delete[] resultBuffer;
    delete[] spheres;

    return 0;
}
