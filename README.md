# Heritage VR — Patrimoine culturel ivoirien

Musée virtuel consacré aux cultures et patrimoines de Côte d'Ivoire, conçu pour **Meta Quest** avec **Unreal Engine 5** et une chaîne de production **Blender**.

## Fonctionnalités

- **Villages historiques reconstitués** — halls d'exposition et zones de villages (Baoulé, Sénoufo, Dan/Yacouba, Grand-Bassam) navigables en téléportation VR.
- **Objets culturels en 3D** — masques, statuettes, tissus et instruments saisissables, avec fiches culturelles bilingues.
- **Traditions et architectures** — chaque village est documenté : habitat, matériaux, usages, contexte historique.
- **Assistant vocal multilingue** — pipeline STT → LLM → TTS supportant le français et les langues ivoiriennes (baoulé, dioula, bété, sénoufo, yacouba).

## Pile technique

| Domaine | Technologie |
|---|---|
| Moteur | Unreal Engine 5.4 (OpenXR, C++) |
| Modélisation | Blender 4.x (scripts Python de production) |
| Cible matérielle | Meta Quest 3 / Quest 2 (Android, OpenXR) |
| Voix | HTTP → STT (Whisper) → LLM → TTS, audio procédural UE |

## Démarrage rapide

1. Installer Unreal Engine 5.4, puis cloner le dépôt et ouvrir `HeritageVR.uproject`.
2. Compiler le module C++ `HeritageVR` avec la toolchain de la plateforme.
3. Créer les maps listées dans `docs/CONTENT_PIPELINE.md` (la logique C++ s'y greffe par actors de `Source/HeritageVR/Museum`).

Pour le setup détaillé, les commandes de compilation et le test sur casque, voir [le guide macOS](docs/MACOS_SETUP.md). Sur Mac, le test VR se fait directement sur le Quest avec un APK sideloadé : Quest Link n'est pas disponible et la prévisualisation OpenXR HMD dans l'éditeur n'est pas prise en charge.

## Structure du dépôt

```
HeritageVR.uproject          Point d'entrée du projet UE5
Config/                      Réglages moteur, inputs VR, assistant vocal
Source/HeritageVR/           Module C++ (pion VR, artefacts, villages, voix)
Content/README.md            Conventions de contenu (maps, artefacts, audio)
Tools/Blender/               Scripts Python de production et d'optimisation
Tools/Editor/                Script Python d'editeur : generation du WelcomeHall
docs/                        Architecture, pipeline de contenu, assistant vocal
```

## Documentation

- [Architecture technique](docs/ARCHITECTURE.md)
- [Pipeline de contenu (Blender → UE)](docs/CONTENT_PIPELINE.md)
- [Assistant vocal multilingue](docs/VOICE_ASSISTANT.md)
- [Conventions de contenu UE](Content/README.md)

## État du projet

Scaffold initial : code C++ prêt à compiler, configurations Quest/OpenXR, scripts Blender, design détaillé de l'assistant vocal. Les assets binaires (modèles, textures, sons) suivent le plan de production décrit dans `docs/CONTENT_PIPELINE.md`.
