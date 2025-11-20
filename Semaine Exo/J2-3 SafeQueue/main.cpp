#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "SafeQueue.h"
#include "TestItem.h"

#define QUEUE_SIZE 10
#define ITEMS_PER_PRODUCER 20
#define NUM_PRODUCERS 55
#define NUM_CONSUMERS 50

#define COLOR_RESET 7
#define COLOR_PRODUCER 10  // Vert
#define COLOR_CONSUMER 14  // Jaune
#define COLOR_INFO 11      // Cyan
#define COLOR_ERROR 12     // Rouge

SafeQueue<TestItem, QUEUE_SIZE> queue;

volatile LONG totalProduced = 0;
volatile LONG totalConsumed = 0;
volatile LONG producerBlocks = 0;
volatile LONG consumerBlocks = 0;

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

struct ThreadParams {
    int id;
    int itemCount;
};

DWORD WINAPI ProducerThread(LPVOID param) {
    ThreadParams* params = (ThreadParams*)param;
    int producerId = params->id;
    int itemsToCreate = params->itemCount;

    for (int i = 0; i < itemsToCreate; i++) {
        TestItem item(producerId, i);

        if (!queue.tryPush(item)) {
            InterlockedIncrement(&producerBlocks);
            PrintColored(COLOR_PRODUCER, "[PRODUCER %d] Queue pleine, attente...\n", producerId);

            queue.push(item);
        }

        InterlockedIncrement(&totalProduced);
        PrintColored(COLOR_PRODUCER, "[PRODUCER %d] Produced item #%d (Total: %ld, Queue size: %d)\n",
            producerId, i, totalProduced, queue.size());

        Sleep(50 + (rand() % 100));
    }

    PrintColored(COLOR_INFO, "[PRODUCER %d] TERMINE (%d items produits)\n", producerId, itemsToCreate);
    return 0;
}

DWORD WINAPI ConsumerThread(LPVOID param) {
    ThreadParams* params = (ThreadParams*)param;
    int consumerId = params->id;
    int itemsToConsume = params->itemCount;

    for (int i = 0; i < itemsToConsume; i++) {
        TestItem item;

        if (!queue.tryPop(item)) {
            InterlockedIncrement(&consumerBlocks);
            PrintColored(COLOR_CONSUMER, "[CONSUMER %d] Queue vide, attente...\n", consumerId);

            item = queue.pop();
        }

        InterlockedIncrement(&totalConsumed);
        PrintColored(COLOR_CONSUMER, "[CONSUMER %d] Consumed item from Producer %d, value=%d (Total: %ld, Queue size: %d)\n",
            consumerId, item.producerId, item.value, totalConsumed, queue.size());

        Sleep(50 + (rand() % 100));
    }

    PrintColored(COLOR_INFO, "[CONSUMER %d] TERMINE (%d items consommes)\n", consumerId, itemsToConsume);
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
    PrintColored(COLOR_INFO, "  - Total items a produire: %d\n\n", ITEMS_PER_PRODUCER * NUM_PRODUCERS);

    DWORD startTime = GetTickCount();

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
            PrintColored(COLOR_ERROR, "ERREUR: echec creation producer thread %d\n", i);
            return 1;
        }
    }

    HANDLE consumerThreads[NUM_CONSUMERS];
    ThreadParams consumerParams[NUM_CONSUMERS];
    int totalItems = ITEMS_PER_PRODUCER * NUM_PRODUCERS;
    int itemsPerConsumer = totalItems / NUM_CONSUMERS;
    int remainingItems = totalItems % NUM_CONSUMERS;

    for (int i = 0; i < NUM_CONSUMERS; i++) {
        consumerParams[i].id = i;
        consumerParams[i].itemCount = itemsPerConsumer;

        if (i == NUM_CONSUMERS - 1) {
            consumerParams[i].itemCount += remainingItems;
        }

        consumerThreads[i] = CreateThread(
            NULL,
            0,
            ConsumerThread,
            &consumerParams[i],
            0,
            NULL
        );

        if (consumerThreads[i] == NULL) {
            PrintColored(COLOR_ERROR, "ERREUR: echec creation consumer thread %d\n", i);
            return 1;
        }
    }

    PrintColored(COLOR_INFO, "\nAttente des producers...\n");
    WaitForMultipleObjects(NUM_PRODUCERS, producerThreads, TRUE, INFINITE);

    PrintColored(COLOR_INFO, "Attente des consumers...\n");
    WaitForMultipleObjects(NUM_CONSUMERS, consumerThreads, TRUE, INFINITE);

    DWORD endTime = GetTickCount();
    DWORD duration = endTime - startTime;

    for (int i = 0; i < NUM_PRODUCERS; i++) {
        CloseHandle(producerThreads[i]);
    }
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        CloseHandle(consumerThreads[i]);
    }

    PrintColored(COLOR_INFO, "\n=== STATISTIQUES FINALES ===\n");
    PrintColored(COLOR_INFO, "Total produit: %ld\n", totalProduced);
    PrintColored(COLOR_INFO, "Total consomme: %ld\n", totalConsumed);
    PrintColored(COLOR_INFO, "Duree totale: %lu ms (%.2f secondes)\n", duration, duration / 1000.0f);
    PrintColored(COLOR_INFO, "Producer bloques (queue pleine): %ld fois\n", producerBlocks);
    PrintColored(COLOR_INFO, "Consumer bloques (queue vide): %ld fois\n", consumerBlocks);
    PrintColored(COLOR_INFO, "Items restants dans la queue: %d\n", queue.size());

    if (totalProduced == totalConsumed && queue.isEmpty()) {
        PrintColored(COLOR_INFO, "\nTEST REUSSI: Tous les items ont ete produits et consommes!\n");
    } else {
        PrintColored(COLOR_ERROR, "\nTEST ECHOUE: Incoherence dans les compteurs!\n");
    }

    return 0;
}
