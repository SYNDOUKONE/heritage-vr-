# Architecture technique — Heritage VR

## Vue d'ensemble

```
┌─────────────────────────── Meta Quest 3 (Android, OpenXR) ────────────────────────────┐
│                                                                                       │
│  UE 5.4 — module C++ "HeritageVR"                                                     │
│  ┌──────────────────┐ ┌──────────────────────┐ ┌─────────────────────────────────┐   │
│  │ AHeritageVRPawn  │ │ Museum (gameplay)    │ │ Voice (assistant multilingue)   │   │
│  │ - teleportation  │─│ - AArtifactActor     │ │ - UVoiceAssistantSubsystem      │   │
│  │ - snap turn      │ │ - AVillageZone       │ │   micro -> STT -> LLM -> TTS    │   │
│  │ - grab (interf.) │ │ - UMuseumState...    │ │ - lecture SoundWaveProcedural   │   │
│  └──────────────────┘ └──────────────────────┘ └─────────────────────────────────┘   │
└───────────────────────────────────────────────────────────────────────────────────────┘
                                  │ HTTP (Wi-Fi)
                    ┌─────────────▼──────────────┐
                    │  Services vocaux distants  │
                    │  STT (Whisper fine-tuné)   │
                    │  LLM + RAG patrimoine      │
                    │  TTS fr + langues locales  │
                    └────────────────────────────┘
```

## Décisions clés

| Sujet | Choix | Justification |
|---|---|---|
| Plugin XR | **OpenXR** natif UE (+ OpenXRHandTracking) | Cible Quest sans dépendre du plugin MetaXR propriétaire ; portage PCVR/Pico possible |
| Interaction | Interface C++ `IInteractable` + Blueprint enfant `BP_Artifact` | Logique de saisie en C++ (fiable, testable), habillage visuel/sonore en Blueprint |
| Locomotion | Téléportation (confort musée) + snap turn 30° | Standard confort VR ; déplacement fluide optionnel (axes `MoveForward/Backward`) |
| Suivi de visite | `UMuseumStateSubsystem` (GameInstance) | Survit aux changements de niveau, exposé aux widgets UMG et à un futur backend |
| Voix | Sous-système HTTP découplé, endpoints en `.ini` | Le Quest n'héberge pas de modèle lourd ; pipeline serveur évolutif sans retoucher le C++ |
| Contenu 3D | FBX/Blender + budgets (`Tools/Blender`) | Production simple et pérenne ; glTF en secours pour revues web |

## Niveaux (maps)

1. **WelcomeHall** — hub : hall du musée, table holographique de navigation, tutoriel téléportation/saisie, sélection de langue de l'assistant.
2. **Villages** (une map par village) :
   - `Baoule_Village` — cases en terre, toits de chaume, aire du masque Goli ;
   - `Senoufo_Village` — case sacrée du Poro, atelier du forgeron, greniers ;
   - `Dan_Yacouba_Village` — clairière des masques Dean Gle, sentier de forêt sacrée ;
   - `GrandBassam_Quartier` — maisons à véranda coloniales (site UNESCO).

Chaque map contient des `AVillageZone` (déclencheurs narratifs + ambiance) et des `BP_Artifact`.

## Flux d'interaction

1. Le visiteur saisit un artefact (grip) → `IInteractable::Grab` attache le mesh à la main ; inspection à 360°.
2. L'inspection prolongée (> 1,5 s) diffuse `OnArtifactExamined` → `UMuseumStateSubsystem::MarkArtifactExamined` → fiche culturelle UMG + narration (`FArtifactInfo.NarrationAudio*`).
3. L'entrée dans une `AVillageZone` diffuse `OnVillageEntered` → ambiance sonore, marqueur de visite, toast UMG avec le nom du village.
4. Le mapping `VoiceAssistant` (trigger droit) ouvre le micro ; la question suit le pipeline vocal (voir `docs/VOICE_ASSISTANT.md`) ; la réponse est lue en synthèse dans la langue choisie.

## Performance Quest 3

- Rendu **Mobile / Forward Shading**, MSAA 4×, pas de bloom/motion blur (déjà dans `Config/DefaultEngine.ini`).
- Budgets Blender : 10 000 tris/artefact (LOD0), 150 000 tris/scène, textures ≤ 2048 px.
- Objectif : **72 FPS sur Quest 3**, 60 FPS sur Quest 2 (profils de qualité à ajouter en production).
- LODs générés par `artifact_pipeline.py` ; éclairage statique cuit (Lightmass), 1 directionnelle + 2 ponctuelles max par zone.

## Points ouverts (phase de production)

- Visites guidées à distance / multijoueur léger (réplication, serveur dédié).
- Persistance de la progression sur backend (les delegates du sous-système y sont prêts).
- Photogrammétrie des pièces réelles avec autorisation écrite des institutions (musées d'Abidjan, Grand-Bassam).
