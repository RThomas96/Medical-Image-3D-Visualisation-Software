# Visualisation d'images médicales

*N.B.* : Ce projet est le résultat du travail effectué pendant le stage de fin de master de l'année scolaire 2019-2020. Vous pourrez trouver le sujet initial
[sur ce lien](http://www.lirmm.fr/~nfaraj/files/positions/sujet_stage-prostate3D.pdf).

====

### Installation

Pour cloner le dépôt, il faut effectuer un clone du dépôt avec :

```sh
$ git clone git@gitlab.com:thibaulltt/visualisation.git visualisation # Pour cloner via SSH
$ git clone https://gitlab.com/thibaulltt/visualisation.git visualisation # Pour cloner via HTTPS
```

Ensuite, une fois dans le dossier `visualisation/`, il suffit de mettre à jour et de cloner les dépôts que l'on a besoin avec :

```
$ git submodule init && git submodule update
```

Afin de compiler, ce programme nécessite d'avoir `cmake`, `Qt 5`, `libQGLViewer`, la bibliothèque `glm` ainsi qu'une implémentation d'OpenGL (généralement empaquetée avec le pilote de votre carte
graphique, ou via la bibliothèque `mesa` pour les cartes graphiques dédiées sur les processeurs Intel par exemple). Une fois ces prérequis installés sur votre système, il suffit de lancer
la commande suivante :

```sh
$ cmake -S ./ -B buildDir
$ cmake --build buildDir --parallel
```

Ensuite, il suffit d'exécuter l'application en la lançant grâce à :

```sh
$ ./buildDir/visualisation
```

====

### Fonctionnalités

[Todo : transférer les infos du Trello ici]
