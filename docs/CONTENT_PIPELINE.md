# Pipeline de contenu — Blender → Unreal Engine

## Vue d'ensemble

```
Photos / photogrammétrie ─► Blender (retopo, UV, bake) ─► Tools/Blender/artifact_pipeline.py
                                                            │  LOD0/1/2, FBX Unreal (cm, X-forward)
                                                            ▼
                                                  Export/Artifacts/<Culture>/
                                                            │  import UE (règles Content/README.md)
                                                            ▼
                                    Content/Artifacts/<Culture>/ + BP_Artifact (FArtifactInfo)
```

## Étapes par artefact

1. **Acquisition** — photogrammétrie (48–120 vues) ou modélisation d'après références documentaires ; accord écrit de l'institution détentrice obligatoire.
2. **Retopologie** — cible 10 000 tris LOD0 pour un masque/statuette ; tissus en planches avec cloth bake.
3. **UV + textures** — 1 UV set, atlas unique, textures ≤ 2048 px : `T_<objet>_D` (albedo, sRGB), `_N` (normal), `_ORM` (linéaire).
4. **Vérification Quest** — `blender --background <fichier> --python optimize_for_quest.py -- --collection <Collection>` (échoue si budgets dépassés).
5. **Export** — `blender --background <fichier> --python artifact_pipeline.py -- --out <dossier> --collection <Collection>`.
6. **Import UE** — FBX dans `Content/Artifacts/<Culture>/`, échelle 1 uu = 1 cm, matériaux assignés, puis `BP_Artifact` (enfant de `AArtifactActor`) avec sa fiche `FArtifactInfo`.
7. **Narration** — audios `SND_Narr_<objet>_<lang>` ; renseigner `NarrationAudioFr` / `NarrationAudioLocal`.

## Villages (architecture)

Approche **kit modulaire** par culture : murs, poteaux, toits, portes, clôtures, mobiliers.

| Village | Kit de modules | Repères documentaires |
|---|---|---|
| Baoulé (Bouaké) | murs terre-pisée, toit chaume, porte sculptée, siège royal | architecture akan/baoulé, aire du masque Goli |
| Sénoufo (Korhogo) | case sacrée Poro, atelier de forge, greniers à mil | architecture sénoufo du nord ivoirien |
| Dan/Yacouba (Man) | habitat de forêt, clairière de masques, sentiers sous canopée | pays dan, région des montagnes |
| Grand-Bassam | maisons coloniales à véranda, comptoirs commerciaux | site UNESCO, fin XIXe siècle |

Chaque kit passe la validation `optimize_for_quest.py` (150 000 tris max/scène) puis est assemblé dans la map du village, couverte par une `AVillageZone`.

## Audio

- Narration : WAV 48 kHz mono (fr + langues locales) ; compression UE ADPCM.
- Ambiances par village : boucles 60–90 s, branchées sur `AVillageZone::AmbienceSound`.
- Voix de l'assistant : PCM 16 kHz renvoyé par le service TTS (voir `docs/VOICE_ASSISTANT.md`).

## Contrôle qualité

- Script Blender `optimize_for_quest.py` en rejet de CI (budgets triangles/textures).
- Revue UE : échelle au sol (repère 1,80 m), lisibilité des fiches, distance d'interaction (0,5–2 m).
- Test matériel hebdomadaire sur Quest 3 : 72 FPS, thermique après 20 min de session.
