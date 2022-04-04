## Semaine 28/04

Objectif: Tester l'outil intéractif de recalage sur les données de cerveau de l'IGF

- [x] Configurer le PC fixe
- [x] Compiler Kidpocket pour générer un maillage
- [x] Réparer le lecteur TIFF pour ouvrir le lightsheet du cerveau [ici](#Lecteur-TIFF)
- [x] Subsampler l'image pour l'ouvrir/convertir
- [x] Segmenter l'image pour générer le maillage 
- [x] Générer un maillage de cage avec Kidpocket (avec NOURA)
- [x] Générer un maillage tétraédrique avec Kidpocket (avec NOURA)
    - [x] Ouvrir les maillages tétrahédrique ".mesh" pour tester les données de Roberta plutôt que le cerveau souris
    - [x] Ouvrir une image avec un maillage tétraédrique qui n'est pas une grille
- [x] Réparer le bug d'offset lors de l'ouverture d'un maillage tétraédrique
- [x] Placer correctement la lumière pour les meshs surfaciques
- [x] Voir pour configurer Qt Designer (c'est compliqué il faut tout basculer en QMake)
- [x] Récupérer les indices des points caractéristiques 
- [x] Coder un outil de recalage simplifié
- [x] Tester le recalage manuel
- [x] Ajouter une feature pour assigner comme handles tous les points avant le slice
- [ ] Réparer l'écriture de la grille déformée
- [ ] Faire l'interface de l'ouverture de grille avec tetmesh
- [ ] Propager l'ouverture ".mesh" aux cages
- [ ] Ajouter sauvegarde ".mesh" aux cages (pour appliquer une cage)
---

Résultats:

### Lecteur TIFF

<details>
    <summary>- Ajustement automatique de la taille de l'image pour correspondre aux limites du GPU</summary>
    Les textures 3D alouables sur le GPU ont une taille limite.
    Cette taille est indépendante de la quantité de VRAM, en effet il est possible d'allouer une texture qui respecte ces limites mais qui dépasse la mémoire allouable sur le GPU.
    Exemple: sur la machine du LIRMM la limite de taille d'une texture est de 16 000 pour 24Go, l'image de lighsheet du cerveau fait environ 3000 pour 40Go.
</details>
<details>
    <summary>- Tentative d'ajustement automatique de la taille de l'image pour correspondre aux limites de VRAM du GPU</summary>
    Le GPU a une quantité limité de VRAM allouable qu'il ne faut pas dépasser pour ne pas faire crash l'affichage.
    Il n'est pas possible de récupérer automatiquement la quantité de VRAM de la machine sans installer des libs NVIDIA propriétaire compliqué, il faudra donc le rentrer manuellement.
    Pour l'instant il n'y a pas de correspondance entre mon calcul de la quantité de mémoire utilisé par l'image et son équivalent sur le GPU. 
    C'est probablement que mon calcul est faux, ou qu'OpenGL effectue une compression compliqué, affaire à suivre.
</details>
<details>
    <summary>- Correction bug signed/unsigned</summary>
    Quand les données sont signées, il faut inversé le premier bit pour effectuer une conversion vers des données non signées.
    Le cast seul qui est appliqué ne fait pas cette opération.
    Les données ne devrait en théorie jamais être signées car il n'y a pas d'intérêt à avoir des valeurs négatives.
    Pourtant c'est le cas de l'image de l'os issue du TP.
    Si le tag du fichier indique le mauvais signe, un warning s'affiche et la lecture va échouer.
    Pour être plus robuste un ajustement manuel devrait être proposé.
</details>
<details>
    <summary>- Ordre du parsing OME/TIFF</summary>
    Les fichiers OME/TIFF contiennent un fichier XML qui indique l'ordre dans lequel se trouve les images selon l'axe Z.
    Ce fichier est maintenant pris en compte.
</details>

---

# Tilted
- [ ] [FEATURE][FORMAT] Prendre en compte le dx/dy/dz du TIFF et OME/TIFF
- [ ] [FEATURE] Debug le placement de points dans la vue 3D
- [ ] [INTERFACE] Bloquage du manipulateur 
- [ ] [INTERFACE] Taille du manipulateur 
- [ ] [INTERFACE] Affichage du manipulateur durant la sélection (il ne devrait pas)
- [ ] [INTERFACE] Améliorer l'interface d'apairage de points 
- [ ] [INTERFACE] Activer/désactiver les plans
- [ ] [RENDU] Le scale affecte les performances

# Un jour 

- [ ] [INTERFACE] Ajouter un bouton pour choisir signed/unsigned [ici](#Lecteur-TIFF)
- [ ] [INTERFACE] Ajouter un bouton pour choisir 8/16/32/64 bits [ici](#Lecteur-TIFF)
- [x] [INTERFACE][SMALL] Changer la fenêtre de choix de fichier

- [ ] [FEATURE][TOOL][MAJEUR] Sélectionner les manipulateurs dans les plane viewers
- [ ] [FEATURE][TOOL][MAJEUR] Ajouter un mode offline pour les performance en cas de gros maillage tetra

- [ ] [FEATURE][FORMAT] Ajustement automatique de la taille de l'image pour correspondre a la limite de VRAM [ici](#Lecteur-TIFF)
- [ ] [FEATURE][FORMAT] Ajouter le format NIFTI
- [ ] [FEATURE][FORMAT] Ajouter le format DIM/IMA pour les mesh tetraedrique

- [ ] [RENDU] Améliorer la lumière
- [ ] [RENDU][MAJEUR] Debugger le rendu négatif 
- [ ] [RENDU][MAJEUR] Debugger le décalage de la grille

# Trier

- [ ] [FEATURE][FORMAT] Prendre en compte le dx/dy/dz du TIFF et OME/TIFF
- [ ] [FEATURE][FORMAT] Ajustement automatique de la taille de l'image pour correspondre a la limite de VRAM [ici](#Lecteur-TIFF)
- [ ] [INTERFACE] Debugger le lock random du manipulateur
- [ ] [RENDU][MAJEUR] Debugger le rendu négatif 
- [ ] [RENDU][MAJEUR] Debugger le décalage de la grille
