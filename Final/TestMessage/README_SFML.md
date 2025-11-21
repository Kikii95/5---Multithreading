# Version SFML - Visualisation Graphique

## 🎨 Installation SFML

### Étape 1 : Télécharger SFML

1. Aller sur : https://www.sfml-dev.org/download/sfml/2.6.1/
2. Télécharger : **SFML-2.6.1-windows-vc17-64-bit.zip**
   (ou vc16 si Visual Studio 2019)
3. Extraire dans `C:\SFML\`

Résultat :
```
C:\SFML\
├── bin\       (DLL)
├── include\   (headers)
└── lib\       (bibliothèques)
```

### Étape 2 : Copier les DLL

Copier depuis `C:\SFML\bin\` vers le dossier de l'exe :

**Pour Debug** :
- `sfml-graphics-d-2.dll`
- `sfml-window-d-2.dll`
- `sfml-system-d-2.dll`

**Pour Release** :
- `sfml-graphics-2.dll`
- `sfml-window-2.dll`
- `sfml-system-2.dll`

### Étape 3 : Ouvrir et compiler

1. **Double-cliquer** sur `TestMessage_SFML.sln`
2. **F7** (Build)
3. **Ctrl+F5** (Run)

---

## 🎮 Contrôles

- **ESC** ou **Fermer fenêtre** : Quitter

---

## 🎨 Visualisation

### Couleurs des boules = Thread

- 🔴 **Rouge** → Thread 0
- 🟢 **Vert** → Thread 1
- 🔵 **Bleu** → Thread 2
- 🟡 **Jaune** → Thread 3

**Les boules changent de couleur quand elles changent de zone !**

### Grille

La grille grise montre les **zones** (4x3 cellules) :
- Chaque zone = 1 thread
- Quand boule traverse ligne → change de thread → change de couleur

### Direction

Petite ligne blanche = direction de déplacement

### Transparence

Boule semi-transparente = en **transition** (immunité temporaire)

---

## 🐛 Problèmes courants

### Erreur : "SFML/Graphics.hpp not found"

→ Vérifier que SFML est installé dans `C:\SFML\`
→ OU modifier le chemin dans `TestMessage_SFML.vcxproj` ligne 64 :
```xml
<AdditionalIncludeDirectories>$(ProjectDir)src;C:\SFML\include;...
```

### Erreur : "sfml-graphics-d.lib not found"

→ Vérifier que `C:\SFML\lib\` existe
→ OU modifier le chemin dans `TestMessage_SFML.vcxproj` ligne 70 :
```xml
<AdditionalLibraryDirectories>C:\SFML\lib;...
```

### Exe ne se lance pas : "sfml-graphics-2.dll missing"

→ Copier les DLL depuis `C:\SFML\bin\` vers :
- `bin\Debug\` (versions -d.dll)
- `bin\Release\` (versions sans -d)

### Erreur : "v143 toolset not found"

→ Clic droit sur projet → **Retargeter la solution**
→ Choisir version SDK disponible

---

## 📊 Ce que vous verrez

### Fenêtre SFML

- **800x600 pixels**
- Fond sombre (gris-bleu)
- Grille grise (zones des threads)
- **Boules colorées** qui bougent et rebondissent
- Changements de couleur en temps réel (transitions)

### Console (toujours active)

- Logs des collisions locales/remote
- Messages inter-threads
- Transitions
- Kills
- Stats toutes les 5 secondes

---

## 🎯 Avantages SFML

✅ **Voir en temps réel** les collisions
✅ **Observer les transitions** (changements de couleur)
✅ **Comprendre visuellement** le multithreading
✅ **Démonstration impactante** pour les collègues

---

## 🔧 Personnalisation

### Modifier le nombre de boules

`src/Core/Utils.h` ligne 11 :
```cpp
const int NB_BALLS = 12;  // Changer ici
```

### Modifier la vitesse

`src/Core/Utils.h` ligne 14-15 (dans Ball.h) :
```cpp
mStats.speed = Utils::RandomFloat(60.0f, 100.0f);  // Ajuster
```

### Changer les couleurs

`src/Rendering/Renderer.h` lignes 11-18 :
```cpp
sf::Color mThreadColors[8] = {
    sf::Color(255, 100, 100),  // Rouge → Changer RGB
    ...
};
```

---

## 📚 Fichiers ajoutés

- `src/main_sfml.cpp` → Main avec SFML
- `src/Rendering/Renderer.h` → Classe de rendu
- `TestMessage_SFML.sln` → Solution Visual Studio
- `TestMessage_SFML.vcxproj` → Projet avec SFML

**La version console reste disponible** : `TestMessage.sln`

---

**Prêt à voir les boules en action !** 🎨🚀
