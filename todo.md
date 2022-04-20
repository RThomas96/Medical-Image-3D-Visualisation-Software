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
- [x] Préparer la réunion 
- [ ] Réparer l'écriture de la grille déformée
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

## Semaine 28/03

Objectif: Rendre l'interface parfaitement stable 

ARAP
- [x] Réparer le bloquage du manipulateur
- [x] Réparer la taille du manipulateur
- [x] Ne pas afficher le manipulateur durant la sélection 
- [x] Clean up handles ARAP
- [x] Réparer MAJ
- [x] Inverse sélection 
- [x] Ajouter shortcut ARAP add/remove handles
- [x] Remove shortcut S for stereo
- [x] Ajouter shortcut ARAP switch handles
- [x] Sync le boutton handles 

POSITION
- [x] Réparer le rotate étrange du manipulateur en mode position
- [x] Réparer taille 
- [x] Réparer even mode
- [x] Réparer le scale du manipulateur
- [x] Réparer bloquage aléatoire

INTERFACE
- [x] Refaire les slots de la scene 
- [x] Retirer le pannel avec 2D/3D/Pannel/etc
- [x] Retirer tous les boutons inutiles 
- [x] Ajouter une toolbar
- [x] Ajouter un frame autour du viewer
- [x] Ajouter le selector
- [x] Connecter le selector
- [x] Connecter la toolbar 
- [x] Ajouter un side pannel pour les tools
- [x] Connecter plane
- [x] Afficher le cut au bon endroit 
- [x] Afficher le plan au bon endroit 
- [x] Ajouter hide/show side pannel tools
- [x] Changer de couleur la sélection si handles 
- [x] Changer couleur mesh surfacique (pas vert)
- [x] Trier les faces pour le rendu
- [x] Changer curseur dans les planar viewers
- [x] Changer le mode de déplacement des planar viewers à clique droit
- [x] Cast un rayon dans planar viewer avec Q aussi
- [x] Preview
- [x] Changer la taille des manips en fonction de la taille de la scène
- [x] Connecter hide/show side pannel tools 
    - [x][ ] Display mesh 
    - [x][x] Size manipulators 
    - [x][x] Size kidmanipulator
    - [x][x] Info: [ index, position] 
    MOVE
    - [x][ ] Even mode
    DIRECT
    ARAP
    - [x][x] Mode Handle
    REGISTER
    - [x][x] Clear 
    - [x][x] Register

GENERAL
- [x] Réparer le ray casting
- [x] Ajouter le ray casting au registration
- [x] Ajouter le pannel des plans de KidPocket
- [x] Debugger le lock random du manipulateur
- [x] Debugger le décalage de la grille (texCoords)
- [x] Changer la fenêtre de choix de fichier en non native
- [x] Debug le placement de points dans la vue 3D
- [x] Bloquage du manipulateur 
- [x] Taille du manipulateur 
- [x] Affichage du manipulateur durant la sélection (il ne devrait pas)
- [x] Améliorer l'interface d'apairage de points 
- [x] Activer/désactiver les plans

FEATURES
- [x] Ajouter le none tool 
- [x] Réparer la sauvegarde

---
## Semaine 12/04

Objectif: Terminer le workflow complet 

MAIN
- [x] Envoyer le mail pour réunion
- [x] Générer un maillage depuis l'atlas
        - [x] Générer une cage
        - [x] Générer une cage hyper dilated
        - [x] Générer un transfert mesh
        - [x] Nettoyer le répertoire de maillage
- [ ] Tester et finir tous les outils pour faire le recalage
    - [x] Faire une demo visualisation atlas
    - [x] Faire une demo registration atlas
    - [x] Clarifier l'API open grid
    - [x] Ajouter shortcut pour display/hide mesh
    - [x] Créer helper pour QAction, shortcuts, toolbar et icons
    - [x] Créer helper pour exclusive groups
    - [x] Refaire l'interface et mettre que des shortcuts
    - [x] Refaire le layout avec des windows manipulables 
    - [x] Revoir l'initialisation
    - [x] Redimensionner les icons
    - [ ] Ajouter un undo
- [ ] Sauvegarder le maillage tétrahédrique déformé et la cage dans un format .deform
        - [ ] Terminer les interfaces IO
- [ ] Ouvrir un couple maillage tétrahédrique initial et déformé dans un format .deform
- [ ] Faire une interface pour lire une liste de point et les sortir déformés 
- [ ] Réparer la déformation de l'atlas avec le LRI
    - [ ] Vérifier que c'est bien la cage et le transfert qui cassent le LRI
    - [ ] Générer une cage et un transfert compatible avec le LRI

FIX
- [ ] Bug sync de la cage (move point avec le manip)
- [ ] Ajouter interface display maillage tetra 
- [x] Perfs du move manip

---
## Semaine 19/04

### Mardi

Objectif: préparer la réunion

- [x] Fichiers démo
- [x] Modifier les points du registration tool
- [x] Faire une archive avec les fichiers démos
- [x] Présentation du logiciel
- [x] Build sur l'ordi perso
- [x] Plusieurs builds pour chaque demo
- [ ] Ajouter clear de la scene
- [ ] Undo

Objectif: Terminer le workflow complet 

MAIN
- [x] Préparer la réunion avec l'IGF
- [ ] Tester et finir tous les outils pour faire le recalage
    - [ ] Ajouter un undo
- [ ] Sauvegarder le maillage tétrahédrique déformé et la cage dans un format .deform
        - [ ] Terminer les interfaces IO
- [ ] Ouvrir un couple maillage tétrahédrique initial et déformé dans un format .deform
- [ ] Faire une interface pour lire une liste de point et les sortir déformés 
- [ ] Réparer la déformation de l'atlas avec le LRI
    - [ ] Vérifier que c'est bien la cage et le transfert qui cassent le LRI
    - [ ] Générer une cage et un transfert compatible avec le LRI

FIX
- [ ] Bug sync de la cage (move point avec le manip)
- [x] Ajouter interface display maillage tetra 

---

# INBOX

MANDATORY
    UNSORTED
    - [ ] Debug LRI
    - [ ] Change the move manip and use move points to debug the link features
    - [ ] Rendu des plans de coupes est bugé, avec tetmesh coupé
- [ ] Le scale affecte les performances
- [ ] Ajouter un undo
- [x] Ajouter des icônes
- [ ] Sync plane et header 
- [ ] Voir le mesh dans planar view
- [ ] Ajustement automatique de la taille de l'image pour correspondre a la limite de VRAM
- [ ] glSubdata pour les perfs
- [ ] Prendre en compte le dx/dy/dz du TIFF et OME/TIFF
- [x] Ajouter le link to mesh for cage
- [ ] Couleurs du kid manipulateur 
- [ ] Ajouter un bouton center camera et retirer le center auto
- [ ] Faire l'interface de l'ouverture de grille avec tetmesh
- [x] Connecter even mode 
- [x] Connecter display mesh
- [x] Ajouter shortcut pour hide/show mesh
- [ ] Debugger le rendu négatif 
- [ ] Ajouter le format NIFTI
- [ ] Ajouter le format DIM/IMA pour les mesh tetraedrique
- [ ] Propager l'ouverture ".mesh" aux cages
- [ ] Ajouter sauvegarde ".mesh" aux maillages tetra

OPTIONNAL
- [ ] Rendre le radius du preview plus petit
- [ ] Remplacer le ray casting par le read du framebuffer comme dans le PlaneViewer
- [ ] Réparer sélection trigger (faire en sorte que les shorcut passe dans le viewer même sans cliquer)
- [ ] Persistent position 
- [ ] Plusieurs radius de manipulateurs 
- [ ] Placer correctement le selector
- [ ] Ajouter object pannel
- [ ] Ajouter la lumière sur les manipulateurs 
- [ ] Ajouter le free manipulator 
- [ ] Draw le wireframe avec les bary coord
- [ ] Interpolation pour dimensions asymetriques
- [ ] Améliorer la lumière pour la grille
- [ ] Ajouter un mode offline pour les performance en cas de gros maillage tetra
- [ ] Sélectionner les manipulateurs dans les plane viewers
- [ ] Ajouter un bouton pour choisir signed/unsigned
- [ ] Ajouter un bouton pour choisir 8/16/32/64 bits
- [ ] Faire une dual sauvegarde pour faire des correspondance 

CODE DEBT
- [ ] Retirer tous les signaux de manipulation de radius 
- [ ] Supprimer l'ancien registration tool et renommer le "fixed"
- [ ] Supprimer l'ancien registration et tout ce qui s'en rapporte (undo etc) 
- [ ] Supprimer le weighted mesh deformer
- [ ] Refont du deformer, retirer la sélection par exemple car suppression du weighted de toute façon

BUG
- [ ] You cannot change the voxel size when you load a custom transfert mesh
