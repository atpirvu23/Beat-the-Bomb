# Beat the Bomb

Pirate-themed trivia game written in **C** with a graphical interface built on **raylib**. Answer questions against the clock, avoid mistakes and save the prisoner before he falls off the plank!

## How to play

1. Pick one of the **5 question categories**.
2. Choose a **difficulty level**, which sets the time limit:

| Difficulty | Time limit |
|---|---|
| Easy | 300 seconds |
| Medium | 180 seconds |
| Hard | 120 seconds |

3. Answer by clicking one of the three options.
   - **Correct answer:** you move on to a harder question.
   - **Wrong answer:** you lose a life and **5 seconds** of time, and get an easier question.
4. **Win** by giving **12 correct answers**.
5. **Lose** if you run out of lives (3) or time. Every life lost pushes the prisoner closer to the edge, until he falls.

### Categories

- Algorithm Design
- General Knowledge
- Cinema
- Sports
- Geography

### Controls

| Key / Action | Effect |
|---|---|
| Mouse click | Select buttons and answers |
| `Space` | Pause the game |
| `Tab` | Return to the main menu |
| `Esc` | Go back from the Music / Theme menus |
| `R` | Restart after winning or losing |

## Features

- Graphical interface with textures, animated sprites and hover effects on buttons
- Countdown timer with time penalties for wrong answers
- Lives system with an animated prisoner that moves and falls as you lose
- Adaptive question selection based on whether the previous answer was correct
- Questions shuffled with the **Fisher–Yates** algorithm for a different order every game
- Questions loaded from **CSV files**, one per category, so new questions are easy to add
- Multi-screen **state machine**: main menu, category and difficulty selection, game, pause, options, music, theme, win and game over screens
- Background music with on/off toggle, volume slider and 3 selectable songs
- Switchable visual themes

## Technologies

- C
- [raylib](https://www.raylib.com/) – graphics, input, window and audio
- Visual Studio (MSVC) on Windows

## Project structure

```
Beat-the-Bomb/
├── main.c
├── src/                 # Question files (CSV)
│   ├── Intrebari_proiectarea_algoritmilor.csv
│   ├── Intrebari_cultura_generala.csv
│   ├── Intrebari_cinematografie.csv
│   ├── Intrebari_sport.csv
│   └── Intrebari_geografie.csv
├── muzica/              # Background music
│   ├── pirate.ogg
│   ├── pirate2.mp3
│   └── pirate3.mp3
└── *.png / *.jpg        # Backgrounds and sprites
```

### Question file format

Each line in a CSV file holds one question, three answers and the number of the correct answer (1–3):

```
Question text,Answer 1,Answer 2,Answer 3,2
```

## Build and run

1. Install [raylib](https://www.raylib.com/) for Windows.
2. Open the project in **Visual Studio** and link raylib.
3. Build and run. Make sure the `src` and `muzica` folders and the image files are in the working directory, since the game loads them at startup.
