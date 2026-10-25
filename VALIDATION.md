# Vérification du 23 octobre 2026

- Compilation Release réussie, sans avertissement, AppleClang 17 / Apple M3.
- Contexte obtenu : OpenGL 4.1 Metal (shaders GLSL 330).
- 15 OBJ chargés, 434 objets assemblés, 12 lumières ponctuelles et 4 spots de façade.
- Test de boucle caméra : 3 601 poses contrôlées ; test CTest réussi.
- Session réelle dans la fenêtre : retour au début, pause, 1/2/3, caméra libre,
  déplacement W, retour à la visite, interface H et capture PNG P vérifiés.
- Captures à plusieurs points de la visite inspectées visuellement.
- Bancs retournés vers la rue, banc de terrasse dégagé des tables et tonneau
  déplacé hors de l'emplacement d'un banc.
- Deux arbres du fond déplacés sur le parvis : leurs enveloppes complètes
  ne croisent aucun des 16 bâtiments placés dans la scène (vérification numérique).
- Polytech Marseille ajouté et parcours adapté : vues à 7, 30, 33,3, 36, 39,
  42 et 46 secondes contrôlées ; le demi-tour conserve les façades dans le champ.
- Désactivation isolée de chaque type de lumière : images de sortie différentes
  à caméra identique (captures/qa/validation.json).
- Export final : 1 800 images, 1 280 × 720, 30 images/s, durée 60 secondes,
  aucune erreur OpenGL signalée. Décodage FFmpeg de la vidéo complète réussi.
- La vidéo finale de 89,33 secondes ajoute 29,33 secondes de journal réel de
  compilation animé (2 680 images au total). Son décodage complet est vérifié ;
  elle ne se présente pas comme une capture du bureau.
- Archive et extraction du cours comparées octet par octet : inchangées.
- Linux n'a pas été testé. Les ombres supplémentaires des points/spots et les
  collisions en caméra libre ne font pas partie de cette implémentation.

Preuves locales (ignorées par Git) : captures/export.log,
captures/compilation.log, captures/compilation-events.json,
captures/runtime.log, captures/qa/ et captures/qa/polytech/layout-check.json.

## Vérification du dépôt autonome — 25 octobre 2026

- Dépendances GLM et en-têtes stb inclus dans `third_party/`.
- Compilation Release depuis ce dossier autonome réussie sur macOS / Apple M3.
- Test CTest de la caméra réussi.
- Exécution du rendu pendant 180 images : aucune erreur OpenGL.
- Vidéo originale de 89,33 secondes décodée intégralement sans erreur.
