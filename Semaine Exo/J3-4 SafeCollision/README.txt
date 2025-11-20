================================================================
  SAFEQUEUE COLLISION DETECTION BENCHMARK
  Pattern Producer-Consumer avec SafeQueue
================================================================

DESCRIPTION
-----------
Ce projet combine la detection de collision (J3) avec le pattern
Producer-Consumer utilisant SafeQueue (J2-3).

ARCHITECTURE PRODUCER-CONSUMER
-------------------------------
1. PRODUCERS (Threads de detection)
   - Chaque producer detecte les collisions dans une tranche de spheres
   - Les collisions detectees sont envoyees dans la SafeQueue
   - Si la queue est pleine, le producer attend (blocking push)

2. CONSUMERS (Threads de stockage)
   - Chaque consumer recupere les collisions depuis la SafeQueue
   - Les collisions sont stockees dans un buffer de resultats
   - Le consumer attend que les producers terminent + queue vide

3. SAFEQUEUE (Buffer intermediaire)
   - Taille fixe: 100,000 elements
   - Synchronisation: CRITICAL_SECTION + CONDITION_VARIABLE
   - Operations: push() (bloquant), tryPop() (non-bloquant)

FICHIERS
--------
main.cpp                     - Programme principal avec 4 benchmarks
SafeCollisionDetector.h      - Interface Producer-Consumer
SafeCollisionDetector.cpp    - Implementation threads + synchronisation
SafeQueue.h                  - Template SafeQueue (de J2-3)
Sphere.h                     - Structures 3D (Point3D, Sphere)

BENCHMARKS
----------
Le programme effectue 4 tests :

1. TEST 1: SINGLE-THREADED (baseline)
   - Detection naive sans threads
   - Reference pour calculer le speedup

2. TEST 2: SAFEQUEUE (2 Producers + 2 Consumers)
   - 2 threads detectent collisions (producers)
   - 2 threads stockent resultats (consumers)

3. TEST 3: SAFEQUEUE (4 Producers + 2 Consumers)
   - 4 threads producers
   - 2 threads consumers

4. TEST 4: SAFEQUEUE (optimal)
   - Nombre de threads auto-detecte (CPU cores)
   - Split optimal entre producers et consumers

CONFIGURATION
-------------
Dans main.cpp :

#define NUM_SPHERES 5000          // Nombre de spheres a tester
#define WORLD_SIZE 1000.0f        // Taille du monde 3D
#define MIN_RADIUS 1.0f           // Rayon minimum sphere
#define MAX_RADIUS 5.0f           // Rayon maximum sphere
#define MAX_COLLISIONS 1000000    // Limite buffer resultats

Dans SafeCollisionDetector.h :

typedef SafeQueue<CollisionPair, 100000> CollisionQueue;
                                  // Taille de la queue

COMPILATION
-----------
1. Ouvrir SafeCollisionBenchmark.sln dans Visual Studio
2. Selectionner Release | x64 (recommande pour benchmark)
3. Build > Build Solution (F7)
4. Run (F5)

METRIQUES AFFICHEES
-------------------
Pour chaque test :
- Collisions detectees
- Temps d'execution (ms + secondes)
- Speedup par rapport au baseline
- Barre de progression en temps reel

Resume final :
- Tableau comparatif complet
- Meilleure methode identifiee
- Statistiques globales

SYNCHRONISATION UTILISEE
------------------------
1. SafeQueue (Producer-Consumer)
   - CRITICAL_SECTION pour proteger buffer circulaire
   - CONDITION_VARIABLE pour signaler items disponibles
   - push() bloquant, tryPop() non-bloquant

2. Result Buffer (Consumers)
   - CRITICAL_SECTION pour proteger ecriture resultats
   - volatile LONG producersRunning pour signalement

3. Progress Monitor
   - volatile LONG comparisonsCompleted (atomic)
   - InterlockedExchangeAdd pour batch updates
   - Thread monitoring separe

AVANTAGES PATTERN PRODUCER-CONSUMER
------------------------------------
+ Separation travail detection/stockage
+ Load balancing automatique via queue
+ Consumers peuvent traiter resultats (tri, filtrage, etc.)
+ Moins de contention que partage direct buffer

INCONVENIENTS
-------------
- Overhead de la queue (allocation, synchronisation)
- Latence entre detection et stockage
- Queue peut devenir bottleneck si producers >> consumers
- Memoire additionnelle (queue = 100k * sizeof(CollisionPair))

COMPARAISON AVEC J3
-------------------
J3 (Direct):
  Threads -> CRITICAL_SECTION -> Buffer resultats
  + Simple, rapide pour ce cas d'usage
  - Pas flexible, contention sur buffer

J3-4 (Producer-Consumer):
  Producers -> SafeQueue -> Consumers -> Buffer
  + Flexible, separation responsabilites
  - Overhead queue, peut etre plus lent

QUAND UTILISER PRODUCER-CONSUMER ?
-----------------------------------
Utiliser si :
- Traitement complexe des resultats (tri, filtrage, I/O)
- Vitesse producers != vitesse consumers
- Besoin de buffering (absorption pics de charge)
- Plusieurs etapes de traitement

Ne PAS utiliser si :
- Stockage simple (comme ici)
- Performance critique absolue
- Memoire limitee

RESULTATS ATTENDUS
------------------
Le pattern Producer-Consumer peut etre PLUS LENT que la version
directe (J3) pour ce cas d'usage car :
- Overhead SafeQueue (locks, signals, copies)
- Latence entre detection et stockage
- Memoire cache moins efficace

Mais c'est un EXCELLENT exercice pedagogique pour comprendre :
- Pattern Producer-Consumer
- Utilisation SafeQueue en contexte reel
- Trade-offs architecture vs performance

CONTRAINTES TECHNIQUES
----------------------
- Win32 API uniquement (CreateThread, CRITICAL_SECTION, etc.)
- INTERDIT: std::thread, std::mutex, std::atomic
- C++17 minimum pour templates
- Console Windows uniquement (couleurs, cursor)

NOTES
-----
- Complexite: O(n^2) - toutes les paires testees
- Pour NUM_SPHERES=5000 : 12,497,500 comparaisons
- Pour NUM_SPHERES=50000 : 1,249,975,000 comparaisons (!!)
- Batch progress updates (10,000) pour performance
- GetTickCount() precision ~15ms

AUTEUR
------
Exercice multithreading - Semaine J3-4
Pattern Producer-Consumer avec SafeQueue

================================================================
