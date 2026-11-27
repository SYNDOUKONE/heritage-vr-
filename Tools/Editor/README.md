# Scripts d'éditeur Unreal (Python)

À exécuter dans l'éditeur (Output Log → saisie `py` avec le chemin, ou
*Tools → Execute Python Script*, ou console Python de l'éditeur) :

```python
exec(open('/chemin/vers/Tools/Editor/bootstrap_welcome_hall.py').read())
```

## `bootstrap_welcome_hall.py`

Crée `/Game/Maps/WelcomeHall` (la map de démarrage définie dans `Config/DefaultEngine.ini`) :

- sol, lumière directionnelle + skylight, `PlayerStart` ;
- 2 artefacts de démonstration (`AArtifactActor` avec fiche `FArtifactInfo` : masque Goli baoulé, masque Dean Gle dan) posés sur leurs socles ;
- 1 `AVillageZone` de démonstration ;
- sauvegarde du niveau.

Le niveau est jouable immédiatement : **Play VR** (casque en Link) ou vue desktop.
Les artefacts de démonstration utilisent le cube de base du moteur — à remplacer par les
vrais meshes produits via `Tools/Blender/artifact_pipeline.py`.
