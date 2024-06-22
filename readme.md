# Projet OpenGL avec MSYS2

## Installation

### Étape 1 : Télécharger MSYS2
Téléchargez MSYS2 depuis le site officiel : [https://www.msys2.org/](https://www.msys2.org/)

### Étape 2 : Lancer MSYS2
Lancez MSYS2 MinGW 64-bit Shell.

### Étape 3 : Mise à jour des paquets
Assurez-vous que les paquets sont à jour en exécutant les commandes suivantes :
```sh
pacman -Syu
```

### Étape 4 : Installer les outils de développement
Installez les outils de développement de base et GCC :
```sh
pacman -S base-devel gcc
```

### Étape 5 : Installer la toolchain MinGW
Installez la toolchain MinGW :
```sh
pacman -S mingw-w64-x86_64-toolchain
```

### Étape 6 : Installer FreeGLUT, GLEW, GLFW et GLM
Installez FreeGLUT :
```sh
pacman -S mingw-w64-x86_64-freeglut
```

Installez GLEW, GLFW et GLM :
```sh
pacman -S mingw-w64-x86_64-glew mingw-w64-x86_64-glfw mingw-w64-x86_64-glm
```

### Étape 7 : Cloner le dépôt
Clonez ce dépôt et placez-le dans un dossier de votre choix dans MSYS2, par exemple dans `C:\msys64\home\WILLIAM`.

```sh
git clone <url_du_depot>
cd <nom_du_dossier_du_projet>
```

## Description des projets

### Projet 1 : Rendu en Raymarching

Ce projet rend une scène en utilisant la technique de raymarching. Il inclut 5 shaders d'illumination différents et 5 post-traitements, ainsi qu'un calcul des ombres portées.

#### Compilation et exécution
Pour compiler et exécuter le projet, utilisez les commandes suivantes dans le dossier `build` :

```sh
./build.sh
./main_scene.exe
```

### Projet 2 : Visualisation de fichiers .obj

Ce projet permet de visualiser des fichiers .obj avec leurs fichiers .mtl correspondants.

#### Compilation et exécution
Pour compiler et exécuter le projet, utilisez les commandes suivantes dans le dossier `build` :

```sh
./build2.sh
./tinyobj_loader.exe nom_du_fichier_a_charger
```

Vous pouvez charger des fichiers depuis le répertoire `src/ressources/obj`. Par exemple, pour commencer, essayez :

```sh
./tinyobj_loader.exe flat_vase.obj
```

## Utilisation

### Contrôles de la scène (Projet 1)
- **Espace** : Mettre en pause/reprendre la scène
- **Souris** : Déplacer la souris pour interagir avec la scène

### Contrôles de la visualisation (Projet 2)
- **Souris** : Déplacer la souris pour interagir avec l'objet .obj
- **echap** : Fermer la fenêtre

### ImGui Interface (Projet 1)
- Utilisez l'interface ImGui pour ajuster le champ de vision (FOV) et la position de l'objet, ainsi que pour activer/désactiver les post-traitements.

## Dépendances

- GLEW
- GLFW
- GLM
- FreeGLUT
- ImGui
- stb_image

Ces dépendances sont installées via MSYS2 comme décrit dans les étapes ci-dessus.

## Fonctionnement interne du fragment shader

### Introduction

Le fragment shader de ce projet est conçu pour rendre une scène 3D complexe en utilisant la technique de raymarching. Cette technique permet de créer des effets visuels détaillés et dynamiques en parcourant chaque pixel de l'image pour déterminer la distance à la surface la plus proche.

### Structure du Shader

Le fragment shader utilise plusieurs étapes clés pour rendre la scène :

1. **Initialisation des Uniformes** :
    - `iResolution` : Résolution de l'écran.
    - `iTime` : Temps écoulé depuis le début du programme.
    - `iMouse` : Position de la souris.
    - `fov` : Champ de vision de la caméra.
    - `objectPosition`, `objectRotationX`, `objectRotationY`, `objectRotationZ` : Transformations appliquées aux objets dans la scène.
    - `vignetteEnabled`, `gammaCorrectionEnabled`, `sepiaEnabled`, `hueShiftEnabled` : Options de post-traitement.

2. **Fonctions de Transformation** :
    - Les fonctions `translate`, `rotateX`, `rotateY`, `rotateZ` sont utilisées pour appliquer des transformations spatiales aux objets de la scène.

3. **Définition des Objets** :
    - Les fonctions `dPlane`, `dSphere`, `dTorus`, `dCylinder`, `dBox` définissent les géométries de base utilisées dans la scène.

4. **Calcul des Distances** :
    - La fonction `scene` combine les objets et applique les transformations pour calculer la distance de chaque point à l'objet le plus proche.

5. **Raymarching** :
    - La fonction `march` parcourt l'espace de chaque pixel pour déterminer la distance à la surface la plus proche en utilisant les fonctions de distance.

6. **Calcul des Normales et de l'Éclairage** :
    - Les fonctions `normal`, `basicLighting`, `phongLighting`, `blinnPhongLighting`, `toonLighting` calculent les normales et appliquent différents modèles d'éclairage pour rendre la scène plus réaliste.

7. **Post-Traitements** :
    - Des effets de post-traitement tels que le sépia, le changement de teinte, le vignettage et la correction gamma sont appliqués pour améliorer l'esthétique de l'image finale.

### Détails du Rendu des Objets

Le rendu des objets dans le fragment shader repose sur plusieurs étapes :

1. **Transformation des Objets** :
    - Les objets sont positionnés dans l'espace 3D en utilisant les fonctions de transformation `translate`, `rotateX`, `rotateY`, `rotateZ`. Par exemple, un objet peut être déplacé à une position spécifique, puis tourné autour des axes X, Y, et Z.

2. **Définition des Géométries** :
    - Chaque objet de la scène est défini par une fonction de distance qui calcule la distance entre un point donné et la surface de l'objet. Les fonctions de distance incluent `dPlane` pour les plans, `dSphere` pour les sphères, `dTorus` pour les tores, `dCylinder` pour les cylindres, et `dBox` pour les boîtes.

3. **Combinaison des Objets** :
    - La fonction `scene` combine toutes les géométries définies et applique les transformations nécessaires pour calculer la distance minimale à un objet pour un point donné dans l'espace.

4. **Raymarching** :
    - Le raymarching est une technique qui consiste à avancer le long d'un rayon partant de la caméra jusqu'à ce qu'il rencontre la surface d'un objet ou atteigne une distance maximale. La fonction `march` implémente cette technique en parcourant l'espace de chaque pixel de l'image pour déterminer la distance à la surface la plus proche.

### Fonctionnement de la Caméra

La caméra dans le fragment shader est définie par plusieurs paramètres et transformations :

1. **Position et Orientation** :
    - La position de la caméra est déterminée par la variable uniforme `iMouse`, qui contrôle la direction de la caméra en fonction des mouvements de la souris. La caméra est placée dans l'espace 3D et dirigée vers un point cible défini.

2. **Calcul des Rayons** :
    - Pour chaque pixel de l'image, un rayon est calculé en fonction de la position de la caméra, du champ de vision (`fov`), et des coordonnées du pixel. Le rayon est ensuite normalisé pour parcourir l'espace 3D.

3. **Transformation des Rayons** :
    - Les rayons sont transformés en appliquant les rotations et translations nécessaires pour aligner la vue de la caméra avec la scène. Les transformations `rotateX`, `rotateY`, `rotateZ` sont utilisées pour ajuster l'orientation du rayon en fonction des mouvements de la caméra.

4. **Calcul de la Direction des Rayons** :
    - La direction des rayons est déterminée en combinant les vecteurs avant, latéral et vertical de la caméra. Ces vecteurs sont calculés en utilisant les fonctions de croix et de normalisation pour assurer une orientation correcte dans l'espace 3D.

### Justification de la Suppression des UBO

Les Uniform Buffer Objects (UBO) ont été initialement introduits pour regrouper certaines variables uniformes et optimiser les performances en réduisant le nombre de mises à jour d'uniformes. Cependant, nous avons constaté que l'utilisation des UBO introduisait des problèmes de compatibilité et des erreurs de rendu dans notre scène.

Après plusieurs tests et tentatives de débogage, nous avons décidé de revenir à l'utilisation des uniformes individuels pour assurer la stabilité et la fiabilité du projet. Les UBO peuvent être bénéfiques dans certains contextes, mais dans notre implémentation actuelle, les uniformes individuels offrent une solution plus simple et plus robuste.

Nous continuerons d'explorer les UBO dans les futures versions du projet pour améliorer les performances et l'optimisation.

---