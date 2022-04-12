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

## Semaine 28/04

Objectif: Rendre l'interface parfaitement stable 

LUNDI:

- [ ] Envoyer le mail pour réunion
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
- [x] Ajouter le none tool 
- [x] Réparer la sauvegarde
- [ ] Bug sync de la cage (move point avec le manip)
- [ ] Ajouter un bouton center camera et retirer le center auto
- [ ] Ajouter le link to mesh for cage
- [ ] Ajouter un undo
- [ ] Terminer les interfaces d'ouverture

- [ ] Sync plane et header 
- [ ] Ajouter des icônes
- [ ] Ajouter shortcut pour hide/show mesh
- [ ] Voir le mesh dans planar view
- [ ] Connecter display mesh
- [ ] Connecter even mode 


ARAP
- [x] Réparer le bloquage du manipulateur
- [x] Réparer la taille du manipulateur
- [x] Ne pas afficher le manipulateur durant la sélection 
- [x] Clean up handles ARAP
- [x] Réparer MAJ
- [x] Inverse sélection 
- [ ] Réparer sélection trigger (faire en sorte que les shorcut passe dans le viewer même sans cliquer)
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
- [ ] Persistent position 

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
- [ ] Ajouter le free manipulator 
- [ ] Couleurs du kid manipulateur 
- [ ] Ajouter la lumière sur les manipulateurs 
- [ ] Ajouter object pannel
- [ ] Placer correctement le selector
- [x] Changer curseur dans les planar viewers
- [x] Changer le mode de déplacement des planar viewers à clique droit
- [x] Cast un rayon dans planar viewer avec Q aussi
- [x] Preview
- [ ] Plusieurs radius de manipulateurs 
- [ ] Rendre le radius du preview plus petit
- [ ] Remplacer le ray casting par le read du framebuffer comme dans le PlaneViewer
- [ ] Supprimer l'ancien registration et tout ce qui s'en rapporte (undo etc) 

GENERAL
- [x] Réparer le ray casting
- [x] Ajouter le ray casting au registration
- [x] Ajouter le pannel des plans de KidPocket
- [ ] Sélectionner dans planar view
- [ ] Améliorer la lumière de la grille
- [ ] Ajouter un mode offline pour les performances (en fait pas important)
- [ ] glSubdata pour les perfs
- [ ] Draw le wireframe avec les bary coord
- [ ] Supprimer l'ancien registration tool et renommer le "fixed"
- [ ] Faire une dual sauvegarde pour faire des correspondance 

---

# Tilted
- [ ] [FEATURE][FORMAT] Prendre en compte le dx/dy/dz du TIFF et OME/TIFF
- [x] [FEATURE] Debug le placement de points dans la vue 3D
- [x] [INTERFACE] Bloquage du manipulateur 
- [x] [INTERFACE] Taille du manipulateur 
- [x] [INTERFACE] Affichage du manipulateur durant la sélection (il ne devrait pas)
- [x] [INTERFACE] Améliorer l'interface d'apairage de points 
- [x] [INTERFACE] Activer/désactiver les plans

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
- [x] [RENDU][MAJEUR] Debugger le décalage de la grille

- [ ] Faire l'interface de l'ouverture de grille avec tetmesh
- [ ] Propager l'ouverture ".mesh" aux cages
- [ ] Ajouter sauvegarde ".mesh" aux cages (pour appliquer une cage)

- [ ] Retirer tous les signaux de manipulation de radius 

# Trier

- [ ] [RENDU] Le scale affecte les performances
- [ ] [FEATURE][FORMAT] Prendre en compte le dx/dy/dz du TIFF et OME/TIFF
- [ ] [FEATURE][FORMAT] Ajustement automatique de la taille de l'image pour correspondre a la limite de VRAM [ici](#Lecteur-TIFF)
- [x] [INTERFACE] Debugger le lock random du manipulateur
- [ ] [RENDU][MAJEUR] Debugger le rendu négatif 
- [x] [RENDU][MAJEUR] Debugger le décalage de la grille

# Idée

- [ ] Interpolation pour dimensions asymetriques
