# Scripts Blender — production des artefacts

Blender 4.x requis. Tous les scripts fonctionnent en mode `--background` (automatisation CI possible).

## `artifact_pipeline.py`

Chaîne de production d'un artefact (masque, statuette, tissu, instrument) :

```bash
blender --background models/MasqueGoli.blend --python artifact_pipeline.py -- \
    --out ../Export/Artifacts/Baoule --collection MasqueGoli
```

Prérequis dans le fichier `.blend` :
- une collection nommée d'après l'objet (ex. `MasqueGoli`) contenant les meshes ;
- propriétés personnalisées de la collection : `culture` (`Baoule`, `Senoufo`, `DanYacouba`, `GrandBassam`)
  et `artifact_id` (ex. `MasqueGoli`) ;
- meshes préfixés `SM_`.

Le script applique les transformations, centre l'origine, génère LOD0/LOD1/LOD2 (100 % / 50 % / 25 %)
et exporte un FBX orienté Unreal (forward X, up Z, échelle cm).

## `optimize_for_quest.py`

Contrôle qualité avant intégration UE :

```bash
blender --background models/MasqueGoli.blend --python optimize_for_quest.py -- \
    --collection MasqueGoli --export-gltf ../Export/glTF/MasqueGoli.glb
```

Vérifie : budget triangles par artefact (10 000 en LOD0) et par scène (150 000), nombre de matériaux (≤ 2),
taille des textures (≤ 2048 px), espace colorimétrique des textures ORM. Échoue avec un code non nul
si un budget est dépassé — brancher ce script dans la CI Blender si souhaité.
