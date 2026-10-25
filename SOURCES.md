# Sources

## Code de départ du cours

- Archive : https://mavromatis.org/dl/IOGL-homework.zip
- Récupérée le 23 octobre 2026.
- SHA-256 : `ba7856573603ee4702f5859a545310f3e4c76bcfceed77e5ed94a6a9c2a0f273`.
- Original conservé dans `reference/IOGL-homework.zip` et extrait dans
  `reference/course/`. Aucun fichier de référence n'a été modifié.
- Le point de départ fonctionnel est `Lighting_Multiple.cpp`, les classes
  `Mesh`, `Camera`, `Texture2D`, `ShaderProgram` et les shaders d'éclairage multiple.
- Les modèles `woodcrate.obj`, `barrel.obj` et leurs textures proviennent de cette archive.
- L'archive ne comporte pas de licence générale explicite. Elle est conservée
  comme matériel pédagogique fourni pour ce devoir ; elle n'est pas présentée
  comme une création originale.

## Modèles additionnels : Kenney, CC0

- [City Kit (Commercial) 2.1](https://kenney.nl/assets/city-kit-commercial) :
  bâtiments A à H, auvent, parasol, palette de textures.
- [City Kit (Roads)](https://kenney.nl/assets/city-kit-roads) : lampadaire courbé,
  panneau stop, benne et palette de textures.
- Auteur : Kenney, https://kenney.nl.
- Licence : Creative Commons Zero, copies incluses dans chaque dossier d'assets.
- Les OBJ et MTL sélectionnés sont conservés tels que téléchargés ; le programme
  associe explicitement à chaque modèle sa palette `Textures/colormap.png`.

## Références de conception

- [Stanford CS248 — Real-time shading](https://github.com/stanford-cs248/shading) :
  référence pédagogique pour l'organisation des passes et le shadow mapping.
- [USC — Roller Coaster](https://github.com/MarioFromShrek/CSCI-420-Rollercoaster) :
  référence de principe pour une visite suivant une courbe de Catmull–Rom.
- [Manhattan Project](https://cmartin27.github.io/Portfolio/projects/manhattan.html) :
  référence visuelle de ville avec différents éclairages.

Le code de ces projets étudiants n'a pas été copié dans cette scène.
Le placement, la visite, les décors complémentaires, l'interface et les outils
de capture ont été réalisés pour ce devoir.
Le bâtiment Polytech Marseille est une composition originale de primitives 3D
avec une enseigne en lettres vectorielles ; son architecture est fictive.

GLFW, GLEW, GLM et stb conservent leurs licences respectives.
