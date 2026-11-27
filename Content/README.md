# Contenu Unreal Engine — conventions

Le contenu binaire (`.uasset` / `.umap`) n'est pas versionné dans ce scaffold initial ; il est produit
pendant la phase de production décrite dans `docs/CONTENT_PIPELINE.md`. Les dossiers sont créés dans
l'Unreal Editor en respectant l'arborescence ci-dessous.

## Arborescence

```
Content/
├── Maps/
│   ├── WelcomeHall.umap              Hub central du musée (accueil, table de navigation, tutoriel VR)
│   └── Villages/
│       ├── Baoule_Village.umap       Village baoulé (région de Bouaké) — cases, aire du Goli
│       ├── Senoufo_Village.umap      Village sénoufo (région de Korhogo) — case sacrée, atelier du forgeron
│       ├── Dan_Yacouba_Village.umap  Village dan/yacouba (région de Man) — masques Dean Gle, forêt sacrée
│       └── GrandBassam_Quartier.umap Quartier colonial de Grand-Bassam (UNESCO) — maisons à véranda
├── Artifacts/
│   ├── Baoule/                       Masques, pagnes ikat, statuettes asiè usu
│   ├── Senoufo/                      Masques kponiugo, poro, trompes, bancs
│   ├── Dan_Yacouba/                  Masques Dean Gle, pilons, tissus
│   └── GrandBassam/                  Meubles, photographies, objets de traite
├── Architecture/
│   ├── Baoule/                       Kits de modules : murs terre-pisée, toits chaume, portes sculptées
│   ├── Senoufo/
│   ├── Dan_Yacouba/
│   └── GrandBassam/
├── Audio/
│   ├── Narration/FR/                 Narrations françaises (un USoundBase par artefact)
│   ├── Narration/Local/              Narrations en langues locales (baoulé, dioula...)
│   ├── Ambience/                     Ambiances par village
│   └── UI/                           Retours sonores d'interface
├── UI/                               Widgets UMG : fiche artefact, minimap, sous-titres
└── Materials/                        M_Outfocus (outline), M_Wood, M_EarthenWall, M_Fabric...
```

## Règles de nommage

- Maps : `PascalCase` sans accents (`Baoule_Village`).
- Meshes artefacts : `SM_<culture>_<objet>_<lod>` (ex. `SM_Baoule_MasqueGoli_LOD0`).
- Textures : `T_<objet>_<usage>` (`T_MasqueGoli_D`, `_N`, `_ORM`).
- Materials : `M_` ou `MI_` (instances).
- Sounds : `SND_Narr_<artifact>_<lang>` (`SND_Narr_BaouleMasqueGoli_FR`).
- Actors C++ : ne pas renommer les classes de `Source/HeritageVR` (les maps les référencent).

## Actors du module C++ à poser dans les niveaux

| Actor | Rôle |
|---|---|
| `AArtifactActor` (Blueprint enfant `BP_Artifact`) | Objet 3D saisissable + fiche culturelle (`FArtifactInfo`) |
| `AVillageZone` | Volume déclencheur d'une zone villageoise (ambiance, stats de visite) |
| `UMuseumStateSubsystem` | Suivi global de la progression, câblé aux widgets UMG |
| `UVoiceAssistantSubsystem` | Assistant vocal multilingue, déclenché par le mapping `VoiceAssistant` |

## Import Blender

Exporter les artefacts en FBX (script `Tools/Blender/artifact_pipeline.py`) vers
`Artifacts/<Culture>/FBX/`, puis importer dans l'Unreal Editor avec les réglages :
unité cm, axe forward X, `Combine Meshes = false`, `Import Textures = true`, LODs déjà nommés.
