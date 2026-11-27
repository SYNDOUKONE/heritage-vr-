# Développement macOS et test sur Meta Quest

Ce guide décrit le workflow sur iMac ou Mac : Unreal Engine compile avec Xcode/Clang, puis l'application est testée directement sur le casque à partir d'un APK. Meta Quest Link n'est pas disponible sur macOS.

## Prérequis

| Outil | Installation et notes |
|---|---|
| Unreal Engine 5.4 | Installer la version macOS depuis l'onglet Unreal Engine de l'Epic Games Launcher. Compatible Apple Silicon et Intel. |
| Xcode et Command Line Tools | Installer Xcode depuis l'App Store, puis lancer `xcode-select --install` si les Command Line Tools ne sont pas présents. UnrealBuildTool utilise la toolchain Xcode/Clang. Le premier build peut télécharger des dépendances supplémentaires. |
| Android SDK et NDK | Dans Unreal Editor, utiliser `Platforms → Android → Install SDK`, ou configurer les outils avec Android Studio. Ils sont nécessaires pour empaqueter l'APK Quest. |
| `adb` | Installer avec `brew install --cask android-platform-tools` ou via Android Studio. |
| Blender 4.x | Installer avec `brew install --cask blender`. Les scripts de `Tools/Blender/` fonctionnent aussi sur Mac. |

## Cloner et compiler

```bash
git clone https://github.com/SYNDOUKONE/heritage-vr-.git
cd heritage-vr-
git checkout vibe/heritage-vr-scaffold-0a43ce
```

Le chemin d'installation Unreal Engine utilisé ci-dessous est celui par défaut. Ajuster le chemin si UE est installé ailleurs.

```bash
"/Users/Shared/Epic Games/UE_5.4/Engine/Build/BatchFiles/Build.sh" \
  HeritageVREditor Mac Development \
  -project="$PWD/HeritageVR.uproject" -WaitMutex
```

Il est aussi possible d'ouvrir `HeritageVR.uproject` dans l'éditeur : Unreal peut proposer de générer les fichiers de projet Xcode et de compiler. Aucune modification du `.uproject` n'est nécessaire pour sélectionner la toolchain Mac.

### Tâches VS Code / Antigravity facultatives

Après avoir ouvert le dossier du dépôt, vous pouvez ajouter ces tâches dans `.vscode/tasks.json` pour lancer les builds depuis l'éditeur :

```json
{
  "version": "2.0.0",
  "tasks": [
    {
      "label": "Build Editor (Mac)",
      "type": "shell",
      "command": "\"/Users/Shared/Epic Games/UE_5.4/Engine/Build/BatchFiles/Build.sh\" HeritageVREditor Mac Development -project=\"${workspaceFolder}/HeritageVR.uproject\" -WaitMutex",
      "problemMatcher": []
    },
    {
      "label": "Build Quest APK",
      "type": "shell",
      "command": "\"/Users/Shared/Epic Games/UE_5.4/Engine/Build/BatchFiles/Build.sh\" HeritageVR Android Shipping -project=\"${workspaceFolder}/HeritageVR.uproject\" -WaitMutex",
      "problemMatcher": []
    }
  ]
}
```

Les tâches utilisent le chemin Unreal Engine par défaut indiqué plus haut. L'agent de développement peut analyser les erreurs Clang du module C++ et aider à les corriger.

## Empaqueter et tester sur le Quest

1. Activer le mode développeur pour le casque dans l'application Meta, et créer une organisation sur [developer.oculus.com](https://developer.oculus.com/).
2. Dans Unreal Editor, choisir `Platforms → Android → Package Project` pour générer l'APK. On peut aussi lancer la tâche **Build Quest APK** ci-dessus.
3. Brancher le Quest en USB et accepter la demande de débogage USB dans le casque.
4. Installer l'APK avec `adb` en indiquant le chemin réel du fichier produit :

   ```bash
   adb install -r "/chemin/vers/HeritageVR-Android-Shipping-arm64.apk"
   ```

   Le nom et le sous-dossier exacts peuvent varier selon les réglages de packaging ; chercher l'APK généré sous `Saved/StagedBuilds/`.

[Meta Quest Developer Hub](https://developer.oculus.com/meta-quest-developer-hub/) pour macOS est une alternative à la ligne de commande pour déployer l'application, consulter les logs (`adb logcat`) et suivre les performances.

## Limites et cycle de test

- La prévisualisation VR dans l'éditeur sur Mac n'est pas disponible pour ce workflow : OpenXR HMD n'est pas pris en charge sur macOS. Itérer sur la géométrie et l'UX dans l'éditeur en vue desktop, puis vérifier le rendu sur le casque.
- Pour filtrer les logs Unreal du casque : `adb logcat -s UE`.
- Tester le microphone et l'assistant vocal sur desktop quand c'est possible avant de générer un APK.
- Vérifier les performances et la chauffe directement sur le Quest 3 ; le Mac est la machine d'authoring, pas la référence de performance VR.
