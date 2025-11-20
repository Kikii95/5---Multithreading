=== TEST SAFEQUEUE - PRODUCER/CONSUMER ===

📦 DESCRIPTION
--------------
Projet de test isolé pour la SafeQueue thread-safe.
Implémente le pattern Producer-Consumer classique avec :
  - 2 threads producers qui génèrent des items
  - 2 threads consumers qui consomment des items
  - Queue circulaire de taille limitée (10 items)
  - Synchronisation via CRITICAL_SECTION + CONDITION_VARIABLE


📂 FICHIERS
-----------
SafeQueue.h    - Template thread-safe avec CS + CV
TestItem.h     - Structure de données simple pour les tests
main.cpp       - Programme principal avec producers/consumers


🎯 OBJECTIFS PÉDAGOGIQUES
--------------------------
1. Comprendre le fonctionnement d'une queue thread-safe
2. Observer les blocages quand la queue est pleine/vide
3. Voir l'utilité des CONDITION_VARIABLE (pas de busy-wait)
4. Tester les versions bloquantes (push/pop) vs non-bloquantes (tryPush/tryPop)


🔧 COMPILATION
--------------
Option 1 : Visual Studio
  - Ouvrir le dossier dans Visual Studio
  - Fichier > Nouveau > Projet depuis code existant
  - Sélectionner "Console Application (C++)"
  - Ajouter les 3 fichiers (.h + .cpp)
  - Compiler et exécuter

Option 2 : Ligne de commande (cl.exe)
  cl.exe /EHsc main.cpp /Fe:TestSafeQueue.exe

Option 3 : g++ (MinGW/WSL)
  g++ -o TestSafeQueue main.cpp -std=c++11


🚀 EXÉCUTION
------------
Lancer l'exécutable : TestSafeQueue.exe

Le programme affiche en temps réel :
  - [PRODUCER X] Production d'items (vert)
  - [CONSUMER X] Consommation d'items (jaune)
  - Messages de blocage quand queue pleine/vide
  - Statistiques finales


📊 PARAMÈTRES MODIFIABLES (main.cpp)
-------------------------------------
QUEUE_SIZE           : Taille de la queue (actuellement 10)
ITEMS_PER_PRODUCER   : Nombre d'items par producer (actuellement 20)
NUM_PRODUCERS        : Nombre de threads producers (actuellement 2)
NUM_CONSUMERS        : Nombre de threads consumers (actuellement 2)

EXPÉRIENCES À TESTER :
  - Queue très petite (3 items) → Observe les blocages fréquents
  - Beaucoup de producers (5) vs peu de consumers (1) → Queue toujours pleine
  - Peu de producers (1) vs beaucoup de consumers (5) → Queue toujours vide


🔍 POINTS CLÉS À OBSERVER
--------------------------
1. Blocages producteurs (queue pleine)
   → tryPush échoue → push bloque → CONDITION_VARIABLE attend

2. Blocages consommateurs (queue vide)
   → tryPop échoue → pop bloque → CONDITION_VARIABLE attend

3. CPU usage
   → Doit rester faible (~0%) grâce aux CONDITION_VARIABLE
   → Pas de busy-wait (pas de boucle while avec Sleep)

4. Synchronisation parfaite
   → Total produit == Total consommé
   → Queue vide à la fin
   → Aucune data race (compteurs cohérents)


❓ QUESTIONS POUR TON PROF
---------------------------
1. Pourquoi CRITICAL_SECTION seule ne suffit pas ?
   → Elle protège contre les data races, mais force le busy-wait

2. Quel est le rôle de CONDITION_VARIABLE ?
   → Signalisation efficace (0% CPU) au lieu de polling

3. Différence entre push() et tryPush() ?
   → push() = bloquant (attend si pleine)
   → tryPush() = non-bloquant (retourne false si pleine)

4. Pourquoi circular buffer ?
   → Évite les déplacements mémoire (O(1) au lieu de O(n))


📚 CONCEPTS TESTÉS
------------------
✓ CRITICAL_SECTION (protection data races)
✓ CONDITION_VARIABLE (signalisation efficace)
✓ Producer-Consumer pattern
✓ Circular buffer
✓ Opérations atomiques (InterlockedIncrement)
✓ Thread-safe queue
✓ Gestion blocage/déblocage threads


💡 VARIANTES À TESTER
---------------------
1. Changer QUEUE_SIZE à 3
   → Observe les blocages très fréquents

2. Ajouter 10 producers et 1 consumer
   → Queue reste pleine en permanence

3. Ajouter 1 producer et 10 consumers
   → Queue reste vide en permanence

4. Remplacer SleepConditionVariableCS par un while+Sleep
   → Compare l'usage CPU (devrait exploser à 100%+)
