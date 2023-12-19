# HOVER TANKS

Showcase of a multiplayer game using UE5 and Steamworks.




Make response time 1000 ms = 1s

```
NetEmulation.PktLag 1000
```


+- value to introduce variation into lag

```
NetEmulation.PktLagVariance 10
```


https://www.reddit.com/r/unrealengine/comments/quh7m7/properly_reload_blueprint_actors_components_after/

This has been a long-standing bug in UE4. I'm not sure if it's fixed in 4.27 or 5 or in the latest UE4 source updates, but in the latest 4.26 release it's still bugged.

Here's the best workaround I know of. If someone knows of a better one, please share:

- Comment the UPROPERTY macro above the component declaration.
- Compile.
- Uncomment the UPROPERTY macro.
- Compile.

### Know How

- GameInstance: Create a new GameInstance class. Close Unreal, Generate project files, build


```
Start a Server on the Portotype map
C:\UnrealEngineVersions\UE_5.1\Engine\Binaries\Win64\UnrealEditor.exe "H:\UnrealProjects\HoverTanks\HoverTanks.uproject" /Game/HoverTanks/Maps/PrototypeMap?listen -server -log

Start a stand alone client in the Main Menu map
C:\UnrealEngineVersions\UE_5.1\Engine\Binaries\Win64\UnrealEditor.exe "H:\UnrealProjects\HoverTanks\HoverTanks.uproject" /Game/HoverTanks/Maps/MainMenuMap?listen -game -log
```

Manual LAN Test
```
C:\UnrealEngineVersions\UE_5.1\Engine\Binaries\Win64\UnrealEditor.exe "H:\UnrealProjects\HoverTanks\HoverTanks.uproject" /Game/HoverTanks/Maps/DesertRampsMap?listen -game -log
C:\UnrealEngineVersions\UE_5.1\Engine\Binaries\Win64\UnrealEditor.exe "H:\UnrealProjects\HoverTanks\HoverTanks.uproject" 127.0.0.1 -game -log
```

Tail -f logs
```
PS H:\UnrealProjects\BUILD\HoverTanks\Windows\HoverTanks\Saved\Logs> Get-Content .\HoverTanks.log -wait
```


### Known Issues
- Custom collision mesh as complex collision does not work on the HoverTank.
- &Game parameter cannot be used during ServerTravel, because Steam Session is not joinable that way.
- Start a session find by clicking on join, host a server before it finishes: crash.