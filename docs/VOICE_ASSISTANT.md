# Assistant vocal multilingue

## Langues supportées

| Langue | Code pipeline | Couverture |
|---|---|---|
| Français | `fr` | Complète (par défaut) |
| Baoulé | `baoule` | Corpus de phrases du musée + STT fine-tuné (progressif) |
| Dioula | `dyu` | Corpus de phrases du musée + STT fine-tuné (progressif) |
| Bété | `bete` | Banque de phrases pré-enregistrées (v1) |
| Sénoufo | `senoufo` | Banque de phrases pré-enregistrées (v1) |
| Yacouba (dan) | `yacouba` | Banque de phrases pré-enregistrées (v1) |

Stratégie réaliste : le **français** est complet de bout en bout ; les langues ivoiriennes démarrent
avec une **banque de phrases pré-enregistrées** (accueil, présentation d'artefact, réponses fréquentes)
pendant que des corpus STT/TTS spécifiques sont construits avec des locuteurs natifs (recording sessions
documentées, consentement et rémunération des locuteurs prévus).

## Pipeline

```
Micro (Quest) ──► UVoiceAssistantSubsystem
                    │ AudioCaptureCore : PCM float -> int16, 16 kHz mono
                    │ BuildWavHeader : WAV en mémoire
                    ▼
                STT (HTTP POST audio/wav, header X-Audio-Language)
                    │ {"text": "..."}
                    ▼
                LLM + RAG patrimoine (HTTP POST JSON)
                    │ contexte : fiches FArtifactInfo + base de connaissances validée
                    │ {"reply": "..."}   (garde-fous : uniquement patrimoine/musée)
                    ▼
                TTS (HTTP POST JSON, PCM 16 bits, header X-Audio-Sample-Rate)
                    │
                    ▼
                USoundWaveProcedural -> UAudioComponent (lecture dans le casque)
```

### Contrat des services

| Service | Requête | Réponse |
|---|---|---|
| STT | `POST {SttEndpointUrl}` — corps WAV, header `X-Audio-Language` | `{"text": "..."}` |
| LLM | `POST {LlmEndpointUrl}` — `{"language":"fr","question":"..."}` | `{"reply": "..."}` |
| TTS | `POST {TtsEndpointUrl}` — `{"language":"fr","text":"..."}` | PCM 16 bits brut + header `X-Audio-Sample-Rate` |

Les endpoints sont configurés dans `Config/DefaultGame.ini`, section
`[/Script/HeritageVR.VoiceAssistantSubsystem]` (actuellement vides tant que les services ne sont pas déployés).

## Recommandations services

- **STT** : Whisper (large-v3 pour le français ; variantes fine-tunées baoulé/dioula via corpus communautaires — Common Voice fr sert de base, corpus locaux à collecter). Alternative : endpoint local sur serveur (faster-whisper) pour la maîtrise des données.
- **LLM** : Mistral (API ou auto-hébergé) avec **RAG** sur une base de connaissances validée par des historiens/conservateurs ; garde-fou : refuser tout hors périmètre patrimoine/musée.
- **TTS** : français — voix neuronale standard ; langues locales — v1 = banque de phrases pré-enregistrées, v2 = voix fine-tunées avec locuteurs partenaires.

## Sécurité & vie privée

- Les **clés d'API ne sont jamais commitées** : `Config/UserVoiceSecrets.ini` est git-ignoré (voir `.gitignore`) et chargé par un sur-ensemble de config en local/dev ; sur serveur, variables d'environnement.
- L'audio n'est capté que pendant l'appui sur le trigger (`StartListening` → `StopListeningAndSubmit`) ; aucune capture de fond.
- Le consentement vocal s'affiche à la première utilisation (widget UMG) ; aucune donnée vocale stockée côté client.

## Intégration UE

- `UVoiceAssistantSubsystem` (GameInstance) : `SetLanguage`, `StartListening`, `StopListeningAndSubmit`, `AskQuestion`, `SpeakText`.
- Delegates Blueprint : `OnStateChanged`, `OnTranscription`, `OnReply` (sous-titres UMG).
- Déclencheur : mapping d'action `VoiceAssistant` (trigger droit), et option main levée via OpenXRHandTracking (phase 2).

## Feuille de route vocale

1. **v1** : français complet (STT cloud → LLM → TTS), banques de phrases en langues locales.
2. **v2** : STT fine-tuné baoulé/dioula ; reconnaissance de l'artefact pointé pour contextualiser la question.
3. **v3** : TTS neuronal langues locales avec locuteurs partenaires ; mode guide touristique narratif par village.
