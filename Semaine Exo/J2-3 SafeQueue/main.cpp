#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "SafeQueue.h"
#include "TestItem.h"

// Configuration du test
#define QUEUE_SIZE 10
#define ITEMS_PER_PRODUCER 20
#define NUM_PRODUCERS 2
#define NUM_CONSUMERS 2

// Couleurs console
#define COLOR_RESET 7
#define COLOR_PRODUCER 10  // Vert
#define COLOR_CONSUMER 14  // Jaune
#define COLOR_INFO 11      // Cyan
#define COLOR_ERROR 12     // Rouge

// Queue globale
SafeQueue<TestItem, QUEUE_SIZE> queue;

// Statistiques globales (protégées par Interlocked)
volatile LONG totalProduced = 0;
volatile LONG totalConsumed = 0;
volatile LONG producerBlocks = 0;
volatile LONG consumerBlocks = 0;

// Handle console pour les couleurs
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

// Structure pour les paramètres des threads
struct ThreadParams {
    int id;
    int itemCount;
};

// Thread Producer
DWORD WINAPI ProducerThread(LPVOID param) {
    ThreadParams* params = (ThreadParams*)param;
    int producerId = params->id;
    int itemsToCreate = params->itemCount;

    for (int i = 0; i < itemsToCreate; i++) {
        TestItem item(producerId, i);

        // Tenter d'abord un tryPush pour voir si la queue est pleine
        if (!queue.tryPush(item)) {
            InterlockedIncrement(&producerBlocks);
            PrintColored(COLOR_PRODUCER, "[PRODUCER %d] Queue pleine, attente...\n", producerId);

            // Push bloquant (attend qu'il y ait de la place)
            queue.push(item);
        }

        InterlockedIncrement(&totalProduced);
        PrintColored(COLOR_PRODUCER, "[PRODUCER %d] Produced item #%d (Total: %ld, Queue size: %d)\n",
            producerId, i, totalProduced, queue.size());

        // Simuler un temps de production variable
        Sleep(50 + (rand() % 100));
    }

    PrintColored(COLOR_INFO, "[PRODUCER %d] TERMINÉ (%d items produits)\n", producerId, itemsToCreate);
    return 0;
}

// Thread Consumer
DWORD WINAPI ConsumerThread(LPVOID param) {
    ThreadParams* params = (ThreadParams*)param;
    int consumerId = params->id;
    int itemsToConsume = params->itemCount;

    for (int i = 0; i < itemsToConsume; i++) {
        TestItem item;

        // Tenter d'abord un tryPop pour voir si la queue est vide
        if (!queue.tryPop(item)) {
            InterlockedIncrement(&consumerBlocks);
            PrintColored(COLOR_CONSUMER, "[CONSUMER %d] Queue vide, attente...\n", consumerId);

            // Pop bloquant (attend qu'il y ait un élément)
            item = queue.pop();
        }

        InterlockedIncrement(&totalConsumed);
        PrintColored(COLOR_CONSUMER, "[CONSUMER %d] Consumed item from Producer %d, value=%d (Total: %ld, Queue size: %d)\n",
            consumerId, item.producerId, item.value, totalConsumed, queue.size());

        // Simuler un temps de consommation variable
        Sleep(50 + (rand() % 100));
    }

    PrintColored(COLOR_INFO, "[CONSUMER %d] TERMINÉ (%d items consommés)\n", consumerId, itemsToConsume);
    return 0;
}

int main() {
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    srand((unsigned int)time(NULL));

    PrintColored(COLOR_INFO, "=== TEST SAFEQUEUE ===\n");
    PrintColored(COLOR_INFO, "Configuration:\n");
    PrintColored(COLOR_INFO, "  - Taille queue: %d\n", QUEUE_SIZE);
    PrintColored(COLOR_INFO, "  - Nombre de producers: %d\n", NUM_PRODUCERS);
    PrintColored(COLOR_INFO, "  - Nombre de consumers: %d\n", NUM_CONSUMERS);
    PrintColored(COLOR_INFO, "  - Items par producer: %d\n", ITEMS_PER_PRODUCER);
    PrintColored(COLOR_INFO, "  - Total items à produire: %d\n\n", ITEMS_PER_PRODUCER * NUM_PRODUCERS);

    DWORD startTime = GetTickCount();

    // Créer les threads producers
    HANDLE producerThreads[NUM_PRODUCERS];
    ThreadParams producerParams[NUM_PRODUCERS];

    for (int i = 0; i < NUM_PRODUCERS; i++) {
        producerParams[i].id = i;
        producerParams[i].itemCount = ITEMS_PER_PRODUCER;

        producerThreads[i] = CreateThread(
            NULL,
            0,
            ProducerThread,
            &producerParams[i],
            0,
            NULL
        );

        if (producerThreads[i] == NULL) {
            PrintColored(COLOR_ERROR, "ERREUR: Échec création producer thread %d\n", i);
            return 1;
        }
    }

    // Créer les threads consumers
    HANDLE consumerThreads[NUM_CONSUMERS];
    ThreadParams consumerParams[NUM_CONSUMERS];
    int itemsPerConsumer = (ITEMS_PER_PRODUCER * NUM_PRODUCERS) / NUM_CONSUMERS;

    for (int i = 0; i < NUM_CONSUMERS; i++) {
        consumerParams[i].id = i;
        consumerParams[i].itemCount = itemsPerConsumer;

        consumerThreads[i] = CreateThread(
            NULL,
            0,
            ConsumerThread,
            &consumerParams[i],
            0,
            NULL
        );

        if (consumerThreads[i] == NULL) {
            PrintColored(COLOR_ERROR, "ERREUR: Échec création consumer thread %d\n", i);
            return 1;
        }
    }

    // Attendre que tous les producers terminent
    PrintColored(COLOR_INFO, "\nAttente des producers...\n");
    WaitForMultipleObjects(NUM_PRODUCERS, producerThreads, TRUE, INFINITE);

    // Attendre que tous les consumers terminent
    PrintColored(COLOR_INFO, "Attente des consumers...\n");
    WaitForMultipleObjects(NUM_CONSUMERS, consumerThreads, TRUE, INFINITE);

    DWORD endTime = GetTickCount();
    DWORD duration = endTime - startTime;

    // Fermer les handles
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        CloseHandle(producerThreads[i]);
    }
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        CloseHandle(consumerThreads[i]);
    }

    // Afficher les statistiques finales
    PrintColored(COLOR_INFO, "\n=== STATISTIQUES FINALES ===\n");
    PrintColored(COLOR_INFO, "Total produit: %ld\n", totalProduced);
    PrintColored(COLOR_INFO, "Total consommé: %ld\n", totalConsumed);
    PrintColored(COLOR_INFO, "Durée totale: %lu ms (%.2f secondes)\n", duration, duration / 1000.0f);
    PrintColored(COLOR_INFO, "Producer bloqués (queue pleine): %ld fois\n", producerBlocks);
    PrintColored(COLOR_INFO, "Consumer bloqués (queue vide): %ld fois\n", consumerBlocks);
    PrintColored(COLOR_INFO, "Items restants dans la queue: %d\n", queue.size());

    if (totalProduced == totalConsumed && queue.isEmpty()) {
        PrintColored(COLOR_INFO, "\n✓ TEST RÉUSSI: Tous les items ont été produits et consommés!\n");
    } else {
        PrintColored(COLOR_ERROR, "\n✗ TEST ÉCHOUÉ: Incohérence dans les compteurs!\n");
    }

    return 0;
}
