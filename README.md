# Base Project — Unreal Engine 5.8.3

Base C++ minimale e pronta da estendere per un progetto desktop in Unreal Engine 5.8.3.

## Contenuto

- modulo runtime `BaseProject`;
- target Game ed Editor configurati per UE 5.8 (`BuildSettingsVersion.V7`);
- `ABaseProjectGameMode` come Game Mode predefinita;
- personaggio C++ in prima persona con movimento, visuale e salto;
- input per tastiera/mouse e gamepad;
- rendering desktop con Lumen e Virtual Shadow Maps;
- configurazione iniziale di packaging;
- cartelle `Content/Maps` e `Content/Blueprints` pronte per gli asset;
- `.gitignore` adatto ai file generati da Unreal Engine.

## Requisiti

- Unreal Engine **5.8.3**;
- toolchain C++ supportata dalla propria installazione di UE 5.8.3;
- su Windows, Visual Studio con il workload **Sviluppo di giochi con C++**; su Linux/macOS, la toolchain indicata dalla documentazione della relativa build dell'engine.

> Nel file `.uproject` l'associazione è `5.8`: Unreal registra normalmente l'associazione con versione major/minor, mentre la patch effettiva (`5.8.3`) dipende dall'installazione selezionata. I target C++ sono esplicitamente configurati con `EngineIncludeOrderVersion.Unreal5_8`.

## Avvio

1. Installa o compila Unreal Engine 5.8.3.
2. Apri `BaseProject.uproject` con quella versione.
3. Se richiesto, seleziona **Switch Unreal Engine version...** e scegli l'installazione 5.8.3.
4. Genera i file del progetto IDE dal menu contestuale del `.uproject`.
5. Compila il target `BaseProjectEditor` in configurazione `Development Editor`.
6. Avvia l'Editor e premi **Play**.

Il progetto usa inizialmente la mappa template inclusa nell'engine, così può partire senza asset binari versionati. Prima di modificarla, salvala come nuova mappa in `Content/Maps` (per esempio `StarterMap`) e impostala in **Project Settings > Maps & Modes**.

### Build da riga di comando

Windows (PowerShell/CMD):

```bat
"%UE_ROOT%\Engine\Build\BatchFiles\Build.bat" BaseProjectEditor Win64 Development "%CD%\BaseProject.uproject" -WaitMutex
```

Linux:

```bash
"$UE_ROOT/Engine/Build/BatchFiles/Linux/Build.sh" BaseProjectEditor Linux Development "$PWD/BaseProject.uproject" -WaitMutex
```

`UE_ROOT` deve puntare alla cartella principale di Unreal Engine 5.8.3.

## Comandi

| Azione | Tastiera/mouse | Gamepad |
|---|---|---|
| Movimento | `W A S D` | Stick sinistro |
| Visuale | Mouse | Stick destro |
| Salto | `Spazio` | Pulsante inferiore |

## Struttura

```text
BaseProject.uproject
Config/
  DefaultEngine.ini
  DefaultGame.ini
  DefaultInput.ini
Content/
  Blueprints/
  Maps/
Source/
  BaseProject.Target.cs
  BaseProjectEditor.Target.cs
  BaseProject/
    BaseProject.Build.cs
    Public/
    Private/
```

Le directory `Binaries`, `DerivedDataCache`, `Intermediate` e `Saved` vengono create localmente dall'engine e sono escluse da Git.

## Prossimi passi consigliati

1. Crea e salva una mappa del progetto in `Content/Maps`.
2. Crea un Blueprint derivato da `ABaseProjectCharacter` per aggiungere mesh, animazioni e audio.
3. Crea un Blueprint derivato da `ABaseProjectGameMode` e assegnalo alla mappa.
4. Per un progetto più complesso, migra gli input a **Enhanced Input** con `Input Action` e `Input Mapping Context` come asset.
5. Configura Git LFS prima di aggiungere file `.uasset` e `.umap` di grandi dimensioni.
