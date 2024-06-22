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

---