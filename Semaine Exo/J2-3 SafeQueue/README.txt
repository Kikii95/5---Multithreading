=== TEST SAFEQUEUE - PRODUCER/CONSUMER ===

📦 DESCRIPTION
--------------
Projet de test isole pour la SafeQueue thread-safe.
Implemente le pattern Producer-Consumer classique avec :
  - 2 threads producers qui generent des items
  - 2 threads consumers qui consomment des items
  - Queue circulaire de taille limitee (10 items)
  - Synchronisation via CRITICAL_SECTION + CONDITION_VARIABLE


📂 FICHIERS
-----------
SafeQueue.h    - Template thread-safe avec CS + CV
TestItem.h     - Structure de donnees simple pour les tests
main.cpp       - Programme principal avec producers/consumers


🎯 OBJECTIFS PeDAGOGIQUES
--------------------------
1. Comprendre le fonctionnement d'une queue thread-safe
2. Observer les blocages quand la queue est pleine/vide
3. Voir l'utilite des CONDITION_VARIABLE (pas de busy-wait)
4. Tester les versions bloquantes (push/pop) vs non-bloquantes (tryPush/tryPop)


🔧 COMPILATION
--------------
Option 1 : Visual Studio
  - Ouvrir le dossier dans Visual Studio
  - Fichier > Nouveau > Projet depuis code existant
  - Selectionner "Console Application (C++)"
  - Ajouter les 3 fichiers (.h + .cpp)
  - Compiler et executer

Option 2 : Ligne de commande (cl.exe)
  cl.exe /EHsc main.cpp /Fe:TestSafeQueue.exe

Option 3 : g++ (MinGW/WSL)
  g++ -o TestSafeQueue main.cpp -std=c++11


🚀 EXeCUTION
------------
Lancer l'executable : TestSafeQueue.exe

Le programme affiche en temps reel :
  - [PRODUCER X] Production d'items (vert)
  - [CONSUMER X] Consommation d'items (jaune)
  - Messages de blocage quand queue pleine/vide
  - Statistiques finales


📊 PARAMeTRES MODIFIABLES (main.cpp)
-------------------------------------
QUEUE_SIZE           : Taille de la queue (actuellement 10)
ITEMS_PER_PRODUCER   : Nombre d'items par producer (actuellement 20)
NUM_PRODUCERS        : Nombre de threads producers (actuellement 2)
NUM_CONSUMERS        : Nombre de threads consumers (actuellement 2)

EXPeRIENCES a TESTER :
  - Queue tres petite (3 items) → Observe les blocages frequents
  - Beaucoup de producers (5) vs peu de consumers (1) → Queue toujours pleine
  - Peu de producers (1) vs beaucoup de consumers (5) → Queue toujours vide


🔍 POINTS CLeS a OBSERVER
--------------------------
1. Blocages producteurs (queue pleine)
   → tryPush echoue → push bloque → CONDITION_VARIABLE attend

2. Blocages consommateurs (queue vide)
   → tryPop echoue → pop bloque → CONDITION_VARIABLE attend

3. CPU usage
   → Doit rester faible (~0%) grâce aux CONDITION_VARIABLE
   → Pas de busy-wait (pas de boucle while avec Sleep)

4. Synchronisation parfaite
   → Total produit == Total consomme
   → Queue vide a la fin
   → Aucune data race (compteurs coherents)


❓ QUESTIONS POUR TON PROF
---------------------------
1. Pourquoi CRITICAL_SECTION seule ne suffit pas ?
   → Elle protege contre les data races, mais force le busy-wait

2. Quel est le rôle de CONDITION_VARIABLE ?
   → Signalisation efficace (0% CPU) au lieu de polling

3. Difference entre push() et tryPush() ?
   → push() = bloquant (attend si pleine)
   → tryPush() = non-bloquant (retourne false si pleine)

4. Pourquoi circular buffer ?
   → evite les deplacements memoire (O(1) au lieu de O(n))


📚 CONCEPTS TESTeS
------------------
✓ CRITICAL_SECTION (protection data races)
✓ CONDITION_VARIABLE (signalisation efficace)
✓ Producer-Consumer pattern
✓ Circular buffer
✓ Operations atomiques (InterlockedIncrement)
✓ Thread-safe queue
✓ Gestion blocage/deblocage threads


💡 VARIANTES a TESTER
---------------------
1. Changer QUEUE_SIZE a 3
   → Observe les blocages tres frequents

2. Ajouter 10 producers et 1 consumer
   → Queue reste pleine en permanence

3. Ajouter 1 producer et 10 consumers
   → Queue reste vide en permanence

4. Remplacer SleepConditionVariableCS par un while+Sleep
   → Compare l'usage CPU (devrait exploser a 100%+)
