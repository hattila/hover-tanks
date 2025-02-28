# HOVER TANKS

Showcase of a multiplayer game using UE5 and Steamworks.

Get it from itch.io:
https://attilah.itch.io/hover-tanks

### Features

Main menu system with:
- Host a server
  - Select a map
  - Select a game mode
- Join a server
  - Show a list of servers
  - Show the current map and game mode
- Basic Options
  - Resolution
  - Fullscreen
  - Overall quality level
- Quit

Hover Tank gameplay:
- Movement
  - Hovering
  - Adjustable turn/drift ratio based on speed
  - Surface normal alignment
  - Boost, jump, brake
  - Air control
- Turret rotation
  - Player camera directly controls aiming
- Weapon Fire and selection, with cooldown
- Health and Damage system
- Health Pickup and healing
- Death state
- Respawn
- Headlights

Weapons:
- Cannon, bouncing projectile
- Rocket, homing projectile with target lock

Fully replicated custom movement component with:
- Client side prediction
- Server reconciliation
- Server authoritative

Teams System:
- Team colors
- Team scoring, and scoreboard
- Team selection

Selectable Game Modes:
- DeathMatch
- Team DeathMatch

HUD and widget management:
- HUD class manages widgets
- HUD widget for the Player (Timer)
- HUD widget for the tank (Health, weapons, crosshair etc)
- Scoreboards
- Pause menu, with options

Some basic FX
- Tank burning
- Tank headlights
- Tank projectile impact, explosion
- Rocket smoke trail

### Gameplay Ability System (GAS)

As a learning exercise some parts of GAS were implemented:
 - Damage Calculation Execution, Damage Effects of the Cannon and Rockets
 - Attribute System, which contains Shield and Health, and a meta Damage attribute. Replaced the previous HealthComponents functionality
 - Shields are regenerative, but will not regenerate for 3 seconds after taking damage (cooldown)
 - Created a Smoke Screen ability, which throws smoke grenades that create niagara smoke FX. It has 10 second cooldown and a new UI widget to show for it.

Code usage examples
...

### Known Issues
- Custom collision mesh as complex collision does not work on the HoverTank.
- Start a session find by clicking on join, host a server before it finishes: crash.
- Burning FX is multicast only, late joiners cannot see dead tanks burning.
- Headlights are replicated, but late joiners cannot see the material change, only the light.
- Clients do not see the HealthPickups spawn in animation.
