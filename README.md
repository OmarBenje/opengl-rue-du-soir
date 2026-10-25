# Rue du Soir

Une petite rue au crépuscule, réalisée en **C++17 / OpenGL 3.3 / GLSL** à partir de
l'archive IOGL du cours. Le programme compile et s'exécute nativement ; il charge
des modèles OBJ, leurs textures et les shaders au lancement.

[Voir la vidéo de démonstration sur YouTube](https://youtu.be/tx9xiZT7Xcw)

## Lancer

Après avoir cloné le dépôt :

```bash
git clone https://github.com/OmarBenje/opengl-rue-du-soir.git
cd opengl-rue-du-soir
```

Depuis ce dossier :

```bash
bash build.sh --run
```

Compilation seule : `bash build.sh`. Exécutable : `./build/dusk-street`.
Sur macOS, le script crée également `build/Rue du Soir.app`, un lanceur local
ouvrable par double-clic. Les ressources sont localisées indépendamment du
dossier depuis lequel le programme est lancé.

Prérequis macOS : `brew install cmake glfw glew`.
Sur Debian/Ubuntu : `sudo apt install build-essential cmake libglfw3-dev libglew-dev libgl1-mesa-dev`.
GLM et les en-têtes stb nécessaires sont inclus dans `third_party/`, avec leurs licences.
Le fonctionnement a été vérifié sur macOS / Apple M3 ; la configuration Linux
est fournie, mais n'a pas été exécutée sur Linux pendant cette livraison.

## Contenu de la scène

- 15 modèles OBJ distincts chargés : huit bâtiments, auvent, parasol,
  lampadaire, panneau stop, benne, caisse et tonneau.
- Commerces, enseignes, terrasses, bancs, arbres, trottoirs pavés, passages
  piétons et guirlandes ; les éléments complémentaires sont construits en code.
- Bâtiment **Polytech Marseille** au fond de la rue : façade blanche et bleue,
  entrée vitrée et éclairage sous la marquise. Cette architecture stylisée est
  créée pour la scène, sans reproduire un bâtiment réel du campus.
- Soleil directionnel avec ombres par shadow mapping et filtrage PCF.
- Douze lumières ponctuelles avec atténuation : huit lampadaires, deux guirlandes
  et deux éclairages à l'entrée de Polytech.
- Quatre spots de façade à cône progressif, plus une lampe torche optionnelle.
- Éclairage Blinn–Phong, matériaux texturés, surfaces émissives, ciel de coucher
  de soleil, brume, rendu HDR, léger bloom et correction gamma.
- Visite automatique de 60 secondes sur une courbe de Catmull–Rom fermée,
  avec une trajectoire indépendante pour le point regardé. Elle descend la rue,
  se tourne vers les commerces, présente Polytech, puis revient en hauteur
  après un demi-tour progressif vers les façades.

Les ombres sont calculées pour le soleil. Les lumières ponctuelles et les spots
éclairent sans produire d'ombres supplémentaires. Les enseignes émissives restent
visibles lorsque les trois groupes de lumières sont éteints. La caméra libre
est une caméra d'inspection sans collisions ; le trajet guidé reste dans le
couloir libre de la rue.

## Commandes

| Touche | Action |
|---|---|
| Espace | Mettre en pause / reprendre la visite |
| C | Basculer entre visite et caméra libre |
| R | Recommencer le parcours et enlever la pause |
| ZQSD ou WASD, flèches | Se déplacer en caméra libre |
| Souris | Orienter la caméra libre |
| E / X | Monter / descendre |
| Maj | Accélérer |
| Molette | Modifier le champ de vision |
| 1 / 2 / 3 | Soleil / lampadaires / spots |
| F | Activer la torche (groupe spots activé) |
| B | Activer / désactiver le bloom |
| H | Masquer / afficher l'interface |
| P | Enregistrer une image PNG dans `captures/` |
| Échap | Quitter |

## Vidéo du devoir

Exporter la visite complète en **1280 × 720, 30 images/s, 60 secondes** :

```bash
bash record.sh
```

Le rendu est enregistré à pas de temps fixe ; la durée reste exacte même si
l'export ne se fait pas en temps réel. FFmpeg est requis (`brew install ffmpeg`
ou `sudo apt install ffmpeg`). Résultat : `captures/virtual-visit.mp4`.

Pour ajouter un segment montrant une vraie reconstruction du programme :

```bash
python3 scripts/make_homework_video.py
```

Ce script nécessite Pillow (`python3 -m pip install Pillow`). Il exécute CMake
et le compilateur, conserve leur sortie horodatée et la présente au début de
`captures/homework-video.mp4`, suivie de la visite complète. Le premier segment
est **une visualisation du journal réel de compilation**, pas une capture du
bureau. Les preuves brutes sont dans `captures/compilation.log` et
`captures/compilation-events.json`.

Si une capture d'écran stricte est attendue par l'enseignant, enregistrer l'écran
pendant `bash build.sh --run`, puis laisser la visite se dérouler. La vidéo
d'export et les journaux restent disponibles en complément.

## Captures reproductibles et vérification

```bash
ctest --test-dir build --output-on-failure
./build/dusk-street --capture captures/street.png --time 8 --width 1440 --height 900 --no-hud --hidden
./build/dusk-street --capture captures/no-points.png --time 8 --no-points --no-hud --hidden
./build/dusk-street --frames 180 --hidden
```

Le test de caméra échantillonne une boucle entière à 60 Hz : coordonnées finies,
distance à la cible, couloir libre, déplacement entre images et fermeture.
Une erreur de fichier, de compilation GLSL, de framebuffer ou de rendu OpenGL
fait échouer le programme avec un message explicite.

## Organisation et origine du code

- `reference/IOGL-homework.zip` : archive originale, conservée sans modification.
- `reference/course/` : extraction de référence des sources et ressources du cours.
- `src/ShaderProgram.*`, `Texture2D.*`, `Camera.*` : classes du cours adaptées
  pour ce projet (initialisation, gestion des erreurs, textures sRGB/mipmaps).
- `src/Mesh.*` : pipeline de maillage du cours, chargeur OBJ étendu aux faces
  polygonales convexes, indices négatifs, normales ou UV absentes.
- `src/Scene.*` : modèles, composition de la rue, matériaux et lumières.
- `src/Tour.*` : animation de caméra et vérification du parcours.
- `src/main.cpp` : fenêtre, commandes, passes de rendu, captures et export.
- `shaders/` : shaders dérivés du principe d'éclairage multiple du cours.
- `assets/` : sélection des modèles Kenney et leurs licences.

Sources et attributions détaillées dans [SOURCES.md](SOURCES.md).
Le dossier `captures/` et les fichiers de compilation sont ignorés par Git.
