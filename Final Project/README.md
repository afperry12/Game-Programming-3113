# Final Project: The Maze Trials

Welcome to the **The Maze Trials**, a challenging and action-packed adventure where you must navigate randomly generated mazes, outwit enemies, and survive until the end! With (kind of) infinite unique levels, this game tests your strategy, reflexes, and shooting skills. Can you make it to the end?

---

## 🎮 **Gameplay Overview**

- **Lives:** You start with **5 lives**. Losing all lives ends the game.
- **Levels:** Progress through **infinite unique maze levels** to win the game. (Set to 15 by default)
- **Objective:** Navigate the maze, avoid or eliminate enemies, and escape!

### Key Features:
- **Randomly Generated Levels:** Each maze layout is uniquely generated for an unpredictable challenge.
- **Scrolling Mazes:** Some levels scroll horizontally to the right, adding a dynamic twist.
- **Enemies:**
  - **Teleporting Enemy:** This enemy teleports around the maze. If it teleports **3 times**, you lose a life.
  - **Fireball and Fish Enemies:** These enemies must escape the maze by reaching the bottom exit. Prevent them from escaping!
- **Shooting Mechanics:** Use your projectile skills to eliminate enemies before they become a threat.
- **Action Controls:** Move using **arrow keys** (`↑`, `↓`, `←`, `→`) and shoot with the **`P` key**.

---

## 🕹️ **Controls**

| Action             | Key     |
|--------------------|---------|
| Move Up           | `↑`     |
| Move Down         | `↓`     |
| Move Left         | `←`     |
| Move Right        | `→`     |
| Shoot Projectile  | `P`     |

---

## 📖 **Rules & Objectives**

1. **Survive 15 Levels:** Navigate through randomly generated mazes to reach the final level.
2. **Defeat Enemies:**
   - Eliminate enemies by shooting projectiles.
   - Be cautious of the **teleporting enemy**, which costs you a life if it teleports **3 times**.
   - Stop the **Fireball and Fish enemies** from escaping at the bottom of the maze.
3. **Lives:**
   - You start with **5 lives**.
   - Losing all lives ends the game.
4. **Random Mazes:** Each maze is procedurally generated, making each playthrough unique.

---

1. **Start the Adventure:**
   - Use the arrow keys to navigate through the maze.
   - Press `P` to shoot enemies.

2. **Survive and Strategize:**
   - Pay attention to enemy patterns and adapt your strategy.
   - Watch out for the teleporting enemy!

---

## 🎨 **Assets**

The game features:
- **High-quality textures and animations** for a visually engaging experience.
- **Background music and sound effects** for an immersive atmosphere.

---

## 🛠️ **Development Details**

- **Language:** C++
- **Libraries:** SDL2, OpenGL
- **Key Files:**
  - `main.cpp`: Main game loop and logic.
  - `MazeGenerator.cpp/h`: Random maze generation.
  - `Entity.cpp/h`: Enemy and player behavior.
  - `LevelA.cpp/h`, `LevelB.cpp/h`: Level progression and management.

---

## 🚀 **Future Enhancements**

- Additional enemy types and AI.
- Power-ups and collectible items.
- Multiplayer mode.

---

## 📜 **License**

This project is open-source and available under the [MIT License](LICENSE).

---

**Enjoy the game and good luck escaping the maze!**
