# Messy Game: Arcane Ball

## Game Overview

"Messy Game: Arcane Ball" is a Zelda-like action-adventure game where the player controls a young mage who wields an enchanted ball as their primary weapon. The game combines dungeon exploration, combat, and puzzle solving as the player progresses through increasingly challenging dungeons.

![Game Screenshot](game_screenshot.png)

## Story

You play as a student mage who has discovered an ancient spell that allows them to control a magical orb. After your academy is attacked by dark forces, you must master this unconventional magic to defeat enemies, solve puzzles, and navigate through dungeons to save your fellow students and teachers.

## Key Features

- **Unique Weapon System**: Control an enchanted ball that bounces off walls and enemies
- **Multiple Ball Types**: Unlock different elemental balls with unique properties
- **Character Progression**: Level up your mage and unlock new abilities and spells
- **Dungeon Exploration**: Navigate through procedurally generated rooms with traps, puzzles, and enemies
- **Boss Battles**: Face challenging boss fights that require creative use of your abilities
- **Collectibles**: Find artifacts and spell components to enhance your powers

## Game Mechanics

### Player Character

- **Movement**: The player can move in 8 directions using WASD or arrow keys
- **Health System**: The player has a health bar that decreases when taking damage
- **Mana System**: Spells and special abilities consume mana, which regenerates over time
- **Leveling**: Gain experience by defeating enemies and completing quests

### The Arcane Ball

- **Basic Control**: The ball bounces off walls and obstacles
- **Player Interaction**: Touch the ball to send it flying in the opposite direction
- **Damage System**: The ball deals damage to enemies it contacts
- **Ball Types**:
  - **Fire Ball**: Sets enemies on fire, deals damage over time
  - **Ice Ball**: Slows enemies, creates temporary ice bridges over water
  - **Lightning Ball**: Chains damage to nearby enemies, travels faster
  - **Earth Ball**: Heavier but deals more damage, can break certain walls

### Rooms & Dungeons

- **Room Types**: Combat rooms, puzzle rooms, treasure rooms, boss rooms
- **Procedural Generation**: Dungeons are created with random layouts each time
- **Door Mechanics**: Some doors require keys, solving puzzles, or defeating all enemies
- **Environmental Hazards**: Lava, spikes, ice, moving platforms

### Enemies

- **Variety**: Different enemy types with unique behaviors and attack patterns
- **Elemental Properties**: Some enemies are vulnerable or resistant to certain ball types
- **Pathing**: Enemies use intelligent pathfinding to chase the player
- **Boss Enemies**: Special enemies with multiple phases and complex attack patterns

### Puzzles

- **Physics-Based**: Use the ball's properties to solve movement-based puzzles
- **Switch Puzzles**: Activate switches in the correct order
- **Element Puzzles**: Use the right ball type to solve element-based challenges
- **Time Challenges**: Complete objectives within a time limit

## Controls

### Keyboard & Mouse

| Input | Action |
|-------|--------|
| WASD / Arrow Keys | Move player |
| Space | Attack / Shoot ball |
| E | Interact with objects |
| Q | Switch ball type |
| Tab | Open inventory/menu |
| Shift | Special ability |
| R | Reset ball position |
| ESC | Pause game |

### Gamepad

| Input | Action |
|-------|--------|
| Left Stick | Move player |
| Right Stick | Aim ball (when applicable) |
| A / X | Attack / Shoot ball |
| B / Circle | Interact with objects |
| Y / Triangle | Switch ball type |
| X / Square | Special ability |
| Start | Pause game |
| LB/RB | Cycle through inventory |

## Technical Architecture

The game is built with a modular architecture that separates core systems:

### Core Systems

- **Game**: Central coordinator that manages all systems
- **Config**: Centralized game constants and settings

### Entity System

- **Base Entity**: Common properties for all game objects
- **Player**: Player character with stats and abilities
- **Ball**: Physics-based projectile with special effects
- **Enemy**: AI-controlled opponents with varied behaviors

### World System

- **World**: Manages the overall game environment
- **Room**: Individual areas with tiles, objects, and enemies
- **Tile**: Building blocks with different properties (solid, damaging, etc.)

### Graphics System

- **Renderer**: Handles drawing and visual effects
- **Camera**: Supports different camera behaviors for rooms and open areas
- **TextureManager**: Manages game assets and resources

### Input System

- **InputManager**: Handles controls across different platforms
- **Control Mapping**: Configurable control bindings

## Development Roadmap

### Phase 1: Core Mechanics
- Basic player movement and ball physics
- Simple room generation and collision
- Initial enemy AI
- Camera system

### Phase 2: Game Content
- Multiple ball types with special effects
- Enemy variety with different behaviors
- Room types and dungeon generation
- Basic UI and menus

### Phase 3: Progression & Polish
- Player leveling and upgrades
- Special abilities and spells
- Boss battles
- Sound effects and music
- Tutorial and story elements

### Phase 4: Expanded Features
- Additional dungeons and environments
- Side quests and NPCs
- Collectibles and achievements
- Advanced puzzle mechanics
- New enemy types

## Installation & Setup

### Windows
1. Download the latest release from the GitHub repository
2. Extract the ZIP file to your preferred location
3. Run `MessyGame.exe`

### Build from Source
1. Clone the repository
2. Install the required dependencies (raylib)
3. Build using CMake:
```
mkdir build
cd build
cmake ..
cmake --build .
```

## Contributing

We welcome contributions to the project! Please check our [CONTRIBUTING.md](CONTRIBUTING.md) file for guidelines on submitting code, art, or suggestions.

## Credits

- **Programming**: Your development team
- **Art**: Your art team
- **Music**: Your music team
- **Engine**: Built with raylib
- **Special Thanks**: To all the testers and contributors

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
