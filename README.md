# Base Project — Unreal Engine 5.8.3

Base C++ minimale e pronta da estendere per un progetto desktop in Unreal Engine 5.8.3.

## Contenuto

- modulo runtime `BaseProject`;
- target Game ed Editor configurati per UE 5.8 (`BuildSettingsVersion.V7`);
- `ABaseProjectGameMode` come Game Mode predefinita;
- personaggio C++ in prima persona con movimento, visuale e salto;
- input per tastiera/mouse e gamepad;
- rendering desktop con Lumen e Virtual Shadow Maps; su Windows il progetto seleziona DirectX 12 e compila gli shader D3D12 SM6 richiesti;
- configurazione iniziale di packaging;
- generatore Python per creare nell'Editor la mappa modificabile `Content/Maps/StarterMap.umap`;
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
6. Nell'Editor, esegui `Content/Python/CreateStarterMap.py` da **Tools > Execute Python Script** (oppure dal comando Python nell'Output Log).
7. L'Editor salva `Content/Maps/StarterMap.umap` e la imposta come mappa di avvio e di gioco; quindi premi **Play**.

Il `.umap` è un asset binario che deve essere serializzato da Unreal Editor: non è affidabile crearne uno scrivendo un file a mano. Per questo il repository include il generatore, ma non un `.umap` fittizio. La mappa generata contiene `ABaseProjectMapBase` (terreno e porta), `PlayerStart`, luce direzionale, luce ambientale e atmosfera. Se la mappa non viene impostata automaticamente, seleziona `StarterMap` in **Project Settings > Maps & Modes**. Fino alla generazione, il progetto continua a usare la mappa template dell'engine e `ABaseProjectGameMode` aggiunge in **Play** la base e il punto di partenza mancanti.

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

## Base di mappa e porta

- `ABaseProjectMapBase` (`Source/BaseProject/Public/BaseProjectMapBase.h`): lastra di terreno quadrata con collisione. Il lato si imposta con `MapSizeMeters` (default 2000 m). Include la porta come attore figlio.
- `ABaseProjectDoor` (`Source/BaseProject/Public/BaseProjectDoor.h`): portale in pietra largo 9 m e alto 12 m, con stipiti a blocchi, chiave di volta e anta incernierata composta da grandi conci. Si apre quando il personaggio entra nell'area di attivazione e si richiude quando esce.

Il generatore `Content/Python/CreateStarterMap.py` crea una mappa persistente con la base, la porta, il punto di partenza e l'illuminazione. Dopo averla generata, puoi regolare `MapSizeMeters`, `DoorLocation` e `DoorRotation` selezionando `ABaseProjectMapBase` nei dettagli. La porta può anche essere piazzata da sola come `ABaseProjectDoor`. In **Play**, il Game Mode aggiunge la base o il `PlayerStart` solo se mancano, evitando duplicati.

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
  Python/
    CreateStarterMap.py
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

1. Esegui `Content/Python/CreateStarterMap.py` dall'Editor per generare `StarterMap.umap`.
2. Crea un Blueprint derivato da `ABaseProjectCharacter` per aggiungere mesh, animazioni e audio.
3. Crea un Blueprint derivato da `ABaseProjectGameMode` e assegnalo alla mappa.
4. Per un progetto più complesso, migra gli input a **Enhanced Input** con `Input Action` e `Input Mapping Context` come asset.
5. Configura Git LFS prima di aggiungere file `.uasset` e `.umap` di grandi dimensioni.
