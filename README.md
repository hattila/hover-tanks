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

