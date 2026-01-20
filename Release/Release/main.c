#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <direct.h>




#define MAX_OPTIONS 3
#define MAX_TEXT_LENGTH 256
#define MAX_QUESTIONS 100
#define MAX_CATEGORIES 5


// Structura pentru întrebări
typedef struct QuestionNode {
    char questionText[MAX_TEXT_LENGTH];
    char answers[MAX_OPTIONS][MAX_TEXT_LENGTH];
    int correctAnswer;
    int difficulty;  // 1 - ușor, 2 - mediu, 3 - greu
    bool visited;    // Dacă întrebarea a fost vizitată
} QuestionNode;



// Structura pentru graf (categorii de întrebări)
typedef struct QuestionGraph {
    QuestionNode questions[MAX_QUESTIONS];
    int totalQuestions;
} QuestionGraph;

// Graful pentru fiecare categorie
QuestionGraph graphs[MAX_CATEGORIES];
int selectedCategory = 0;
int currentQuestionIndex = 0;
int lives = 3;
float timerSeconds = 30.0f;
bool timerRunning = true;
bool gameJustStarted = true;
bool isMaximized = false;
int correctAnswers = 0;
Color culoareMBAPPE = { 222, 184, 135, 255 };  // r, g, b, a
Color transparentColor = { 255, 0, 0, 0 }; // Culoare roșie complet transparentă


// Categoriile și fișierele CSV corespunzătoare
const char* categories[] = {
    "Proiectarea algoritmilor",
    "Cultura generala",
    "Cinematografie",
    "Sport",
    "Geografie"
};

// Funcție pentru a obține calea corectă către folderul "src"
char* GetSrcDirectory() {
    static char srcPath[1024];
    if (_getcwd(srcPath, sizeof(srcPath)) != NULL) {
        strcat_s(srcPath, sizeof(srcPath), "\\src\\");
        printf("Folosim calea pentru CSV: %s\n", srcPath);
        return srcPath;
    }
    printf("Eroare: Nu s-a putut obține calea către src.\n");
    return NULL;
}

void LoadQuestionsFromCSV(const char* filename, QuestionGraph* graph) {
    char fullPath[1024];
    snprintf(fullPath, sizeof(fullPath), "%s%s", GetSrcDirectory(), filename);
    printf("Încerc să încarc fișierul: %s\n", fullPath);

    FILE* file = fopen(fullPath, "r");
    if (!file) {
        printf("Eroare la deschiderea fisierului: %s\n", fullPath);
        return;
    }

    char line[1024];
    graph->totalQuestions = 0;

    int i = 0;
    while (fgets(line, sizeof(line), file)) {
        char question[MAX_TEXT_LENGTH] = "";
        char answer1[MAX_TEXT_LENGTH] = "";
        char answer2[MAX_TEXT_LENGTH] = "";
        char answer3[MAX_TEXT_LENGTH] = "";
        int correctAnswer = 0;

        int matched = sscanf_s(line, "%255[^,],%255[^,],%255[^,],%255[^,],%d",
            question, (unsigned)_countof(question),
            answer1, (unsigned)_countof(answer1),
            answer2, (unsigned)_countof(answer2),
            answer3, (unsigned)_countof(answer3),
            &correctAnswer);

        if (matched == 5) {
            QuestionNode* q = &graph->questions[graph->totalQuestions];
            snprintf(q->questionText, MAX_TEXT_LENGTH, "%s", question);
            snprintf(q->answers[0], MAX_TEXT_LENGTH, "%s", answer1);
            snprintf(q->answers[1], MAX_TEXT_LENGTH, "%s", answer2);
            snprintf(q->answers[2], MAX_TEXT_LENGTH, "%s", answer3);
            q->correctAnswer = correctAnswer - 1;

            // Setăm dificultatea în funcție de poziția întrebării
            if (i < 10) {
                q->difficulty = 1;  // Ușor
            }
            else if (i < 20) {
                q->difficulty = 2;  // Mediu
            }
            else {
                q->difficulty = 3;  // Greu
            }

            q->visited = false;  // Marcam întrebarea ca nevizitată
            graph->totalQuestions++;
            i++;
        }
    }

    fclose(file);
    printf("Încărcate %d întrebări din %s\n", graph->totalQuestions, filename);
}




void ChangeQuestion(bool correctAnswer) {
    // Obținem întrebarea curentă
    QuestionNode* currentQuestion = &graphs[selectedCategory].questions[currentQuestionIndex];

    // Dacă răspunsul este corect, căutăm o întrebare mai dificilă
    if (correctAnswer) {
        // Căutăm întrebări mai grele
        for (int i = currentQuestionIndex + 1; i < graphs[selectedCategory].totalQuestions; i++) {
            if (graphs[selectedCategory].questions[i].difficulty > currentQuestion->difficulty && !graphs[selectedCategory].questions[i].visited) {
                currentQuestionIndex = i;
                break;
            }
        }
    }
    else {
        // Dacă răspunsul este greșit, căutăm întrebări mai ușoare
        for (int i = currentQuestionIndex + 1; i < graphs[selectedCategory].totalQuestions; i++) {
            if (graphs[selectedCategory].questions[i].difficulty < currentQuestion->difficulty && !graphs[selectedCategory].questions[i].visited) {
                currentQuestionIndex = i;
                break;
            }
        }
    }

    // Marcați întrebarea ca vizitată
    graphs[selectedCategory].questions[currentQuestionIndex].visited = true;
}
void HandleAnswer(bool correctAnswer) {
    QuestionNode* currentQuestion = &graphs[selectedCategory].questions[currentQuestionIndex];

    // Dacă răspunsul este corect, căutăm o întrebare mai dificilă
    if (correctAnswer) {
        bool found = false;
        // Căutăm o întrebare mai dificilă
        for (int i = currentQuestionIndex + 1; i < graphs[selectedCategory].totalQuestions; i++) {
            if (!graphs[selectedCategory].questions[i].visited && graphs[selectedCategory].questions[i].difficulty > currentQuestion->difficulty) {
                currentQuestionIndex = i;  // Avansăm la întrebarea următoare mai dificilă
                found = true;
                break;
            }
        }

        // Dacă nu am găsit o întrebare mai dificilă, căutăm o întrebare de dificultate medie
        if (!found) {
            for (int i = 0; i < graphs[selectedCategory].totalQuestions; i++) {
                if (!graphs[selectedCategory].questions[i].visited && graphs[selectedCategory].questions[i].difficulty == 2) {
                    currentQuestionIndex = i;  // Avansăm la întrebarea de dificultate medie
                    break;
                }
            }
        }

        // Dacă nici o întrebare medie nu există, trecem la una ușoară
        if (!found) {
            for (int i = 0; i < graphs[selectedCategory].totalQuestions; i++) {
                if (!graphs[selectedCategory].questions[i].visited && graphs[selectedCategory].questions[i].difficulty == 1) {
                    currentQuestionIndex = i;  // Avansăm la întrebarea ușoară
                    break;
                }
            }
        }

    }
    else {
        // Dacă răspunsul este greșit, căutăm o întrebare mai ușoară
        for (int i = currentQuestionIndex + 1; i < graphs[selectedCategory].totalQuestions; i++) {
            if (!graphs[selectedCategory].questions[i].visited && graphs[selectedCategory].questions[i].difficulty < currentQuestion->difficulty) {
                currentQuestionIndex = i;  // Avansăm la întrebarea mai ușoară
                break;
            }
        }
    }

    // Marcați întrebarea ca vizitată, indiferent dacă răspunsul a fost corect sau greșit
    graphs[selectedCategory].questions[currentQuestionIndex].visited = true;
}



// Funcție pentru Fisher-Yates Shuffle (amestecare îmbunătățită)
void FisherYatesShuffle(QuestionGraph* graph) {
    srand((unsigned int)time(NULL));
    for (int i = graph->totalQuestions - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        QuestionNode temp = graph->questions[i];
        graph->questions[i] = graph->questions[j];
        graph->questions[j] = temp;
    }
}

void ResetGameState() {
    // Resetăm toate variabilele jocului
    currentQuestionIndex = 0;
    lives = 3;
    timerSeconds = 0;
    correctAnswers = 0;

    // Resetăm întrebările vizitate
    for (int i = 0; i < graphs[selectedCategory].totalQuestions; i++) {
        graphs[selectedCategory].questions[i].visited = false;  // Marcați toate întrebările ca nevizitate
    }

    // Aplicați shuffle pentru întrebările din categoria selectată
    FisherYatesShuffle(&graphs[selectedCategory]);

    // Poți adăuga și resetarea altor variabile relevante jocului
}








// Încărcare întrebări pentru toate categoriile
void LoadAllQuestions() {
    const char* csvFiles[] = {
        "Intrebari_proiectarea_algoritmilor.csv",
        "Intrebari_cultura_generala.csv",
        "Intrebari_cinematografie.csv",
        "Intrebari_sport.csv",
        "Intrebari_geografie.csv"
    };

    for (int i = 0; i < MAX_CATEGORIES; i++) {
        graphs[i].totalQuestions = 0;
        LoadQuestionsFromCSV(csvFiles[i], &graphs[i]);
        FisherYatesShuffle(&graphs[i]);
        printf("Încărcate %d întrebări pentru categoria %s\n", graphs[i].totalQuestions, categories[i]);
    }
}



bool DrawHoverButton(Rectangle baseRect, const char* text, int fontSize, Color baseColor, Color hoverColor, float scaleFactor) {
    Vector2 mouse = GetMousePosition();
    bool isHovered = CheckCollisionPointRec(mouse, baseRect);

    Rectangle drawRect = baseRect;

    if (isHovered) {
        // Mărim butonul proporțional față de centru
        drawRect.x -= (baseRect.width * (scaleFactor - 1.0f)) / 2;
        drawRect.y -= (baseRect.height * (scaleFactor - 1.0f)) / 2;
        drawRect.width *= scaleFactor;
        drawRect.height *= scaleFactor;
    }

    DrawRectangleRec(drawRect, isHovered ? hoverColor : baseColor);

    // Centrare text
    int textWidth = MeasureText(text, fontSize);
    Vector2 textPos = {
        drawRect.x + (drawRect.width - textWidth) / 2,
        drawRect.y + (drawRect.height - fontSize) / 2
    };

    DrawText(text, (int)textPos.x, (int)textPos.y, fontSize, BLACK);

    return isHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}



//--------------------------------------------


typedef enum {
    MENU_MAIN,
    MENU_CATEGORIES,
    GAME_RUNNING,
    MENU_RATING,
    MENU_MUSIC,
    GAME_LOST,
    GAME_WIN,
    MENU_PAUSE,
    MENU_THEME,
    MENU_OPTIONS
} GameScreen;

#define MAX_RATING 5
#define NUM_SONGS 3

bool gameFinishedOnce = false;
bool showRatingPrompt = false;

const char* songFiles[NUM_SONGS] = { // DIN FOLDERUL MUZICA SE IAU CELE 3 PIESE
    "muzica/pirate.ogg",
    "muzica/pirate2.mp3",
    "muzica/pirate3.mp3" };

int currentSongIndex = 0;
Music backgroundMusic;
bool gameRestarted = false;
int userRating = 0;
float musicVolume = 0.5f;
bool musicOn = true;


const int SMALL_WIDTH = 800;
const int SMALL_HEIGHT = 600;

void MaximizeWindowProperly() {
    MaximizeWindow();
    isMaximized = true;
}

void RestoreWindowProperly() {
    RestoreWindow();
    SetWindowSize(SMALL_WIDTH, SMALL_HEIGHT);

    int monitor = GetCurrentMonitor();
    int monitorWidth = GetMonitorWidth(monitor);
    int monitorHeight = GetMonitorHeight(monitor);
    Vector2 monitorPos = GetMonitorPosition(monitor);

    int centerX = monitorPos.x + (monitorWidth - SMALL_WIDTH) / 2;
    int centerY = monitorPos.y + (monitorHeight - SMALL_HEIGHT) / 2;
    SetWindowPosition(centerX, centerY);

    isMaximized = false;
}

void UpdateMusic() {
    if (musicOn) {
        // Actualizează fluxul audio pentru a continua redarea muzicii
        UpdateMusicStream(backgroundMusic);
    }
}

// Funcții pentru a obține următoarea întrebare în funcție de dificultate:
int GetNextHardQuestion(int currentIndex) {
    // Găsește o întrebare mai greu de la indexul curent
    return (currentIndex + 1) % graphs[selectedCategory].totalQuestions;  // Exemplu simplu
}

int GetNextEasyQuestion(int currentIndex) {
    // Găsește o întrebare mai ușoară de la indexul curent
    return (currentIndex + 1) % graphs[selectedCategory].totalQuestions;  // Exemplu simplu
}




bool isFalling = false;
float fallTimer = 0.0f;        // FALLING
float prisonerFallOffset = 0.0f;

Texture2D background_categ; // Fundalul cu categoria
Texture2D background_pirate; // Fundalul cu pirati
int themeIndex = 0;


int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(SMALL_WIDTH, SMALL_HEIGHT, "Beat the Bomb - Meniu");

    InitAudioDevice();
    SetAudioStreamBufferSizeDefault(262144);

    backgroundMusic = LoadMusicStream(songFiles[currentSongIndex]);
    SetMusicVolume(backgroundMusic, 0.3f);

    PlayMusicStream(backgroundMusic);

    Texture2D background_game = LoadTexture("BEAT-the-bomb.png"); // imagine 
    Texture2D pirateSprite = LoadTexture("pirat_fara_bg.png"); // pirat cu sabie
    Texture2D prizonierSprite = LoadTexture("prizonier_fara_bg.png");
    background_categ = LoadTexture("background_categ.png");  // Fundalul actual (poți înlocui cu altă imagine)
    background_pirate = LoadTexture("background_pirate.png");

    float scaleX = GetScreenWidth() / (float)SMALL_WIDTH;
    float scaleY = GetScreenHeight() / (float)SMALL_HEIGHT;


    int selected_index_cat = -1;
    int selected_index_diff = -1;

    RestoreWindowProperly();  // Start centered

    GameScreen currentScreen = MENU_MAIN;
    GameScreen previousScreen = MENU_MAIN;

    SetTargetFPS(60);


    int themeIndex = 0;
    Color themes[] = { RAYWHITE, LIGHTGRAY, SKYBLUE, DARKGRAY, BEIGE };
    int themeCount = sizeof(themes) / sizeof(themes[0]);
    Color backgroundColor = RAYWHITE;


    //PrintWorkingDirectory();
    LoadAllQuestions();


    Texture2D background = LoadTexture("start_menu_background.jpg");
    Texture2D background_categ = LoadTexture("pirates.jpeg");

    while (!WindowShouldClose()) {

        UpdateMusic();

        if (currentScreen != previousScreen) {
            if ((currentScreen == MENU_CATEGORIES || currentScreen == GAME_RUNNING) && !isMaximized) {
                MaximizeWindowProperly();
            }
            else if ((currentScreen == MENU_MAIN || currentScreen == MENU_RATING || currentScreen == MENU_OPTIONS) && isMaximized) {
                RestoreWindowProperly();
            }
            previousScreen = currentScreen;
        }

        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();

        BeginDrawing();
        ClearBackground(RAYWHITE);

        //------------------------------------START MENU------------------------------------------

        if (currentScreen == MENU_MAIN) {
            // Fundalul
            DrawTexturePro(
                background,
                (Rectangle) {
                0, 0, background.width, background.height
            },
                (Rectangle) {
                0, 0, screenWidth, screenHeight
            },
                (Vector2) {
                0, 0
            },
                0.0f,
                WHITE
            );

            DrawText("BEAT THE BOMB", screenWidth / 2 - 120, screenHeight * 0.01f, 30, WHITE);

            Rectangle startBtn = { screenWidth / 2 - 228, screenHeight * 0.225f, 452, 85 };
            Rectangle ratingBtn = { screenWidth / 2 - 228, screenHeight * 0.411f, 452, 82 };
            Rectangle optionsBtn = { screenWidth / 2 - 228, screenHeight * 0.595f, 452, 75 };
            Rectangle termsBtn = { screenWidth / 2 - 228, screenHeight * 0.76f, 453, 75 };

            if (DrawHoverButton(startBtn, "Start Game", 20, BEIGE, culoareMBAPPE, 1.1f)) {
                currentScreen = MENU_CATEGORIES;
            }

            if (DrawHoverButton(ratingBtn, "Rate the game", 20, BEIGE, culoareMBAPPE, 1.1f)) {
                currentScreen = MENU_RATING;
            }

            if (DrawHoverButton(optionsBtn, "Options", 20, BEIGE, culoareMBAPPE, 1.1f)) {
                currentScreen = MENU_OPTIONS;
            }

            if (DrawHoverButton(termsBtn, "Terms", 20, BEIGE, culoareMBAPPE, 1.1f)) {
                OpenURL("https://termeniconditiibeatthebomb.tiiny.site");
            }
        }

        //--------------------------------CATEGORIES-------------------------------------------

        else if (currentScreen == MENU_CATEGORIES) {


            DrawTexturePro(background_categ,
                (Rectangle) {
                0, 0, (float)background_categ.width, (float)background_categ.height
            },
                (Rectangle) {
                0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()
            },
                (Vector2) {
                0, 0
            }, 0.0f, WHITE);






            DrawText("Pick a category:", screenWidth / 2 - 120, screenHeight * 0.05f, 30, RAYWHITE);

            


            // Butoane categorii
            Rectangle catBtn_0 = { screenWidth * 0.15f * 1, screenHeight * 0.2f, 250 , 75 };
            Rectangle catBtn_1 = { screenWidth * 0.15f * 2.14 , screenHeight * 0.2f, 200 , 75 };
            Rectangle catBtn_2 = { screenWidth * 0.15f * 3.1, screenHeight * 0.2f, 200 , 75 };
            Rectangle catBtn_3 = { screenWidth * 0.15f * 4.05, screenHeight * 0.2f, 200 , 75 };
            Rectangle catBtn_4 = { screenWidth * 0.15f * 5, screenHeight * 0.2f, 200 , 75 };

            // Culoare de bază și hover pentru categorie
            Color baseColor = BEIGE;
            Color hoverColor = DARKGRAY;
            float scaleSelected = 1.1f;

            // Verificare hover și selecție pentru categorii
            if (DrawHoverButton(catBtn_0, categories[0], 20, baseColor, hoverColor, 1.1f)) {
                selectedCategory = 0;
            }

            if (DrawHoverButton(catBtn_1, categories[1], 20, baseColor, hoverColor, 1.1f)) {
                selectedCategory = 1;
            }
            if (DrawHoverButton(catBtn_2, categories[2], 20, baseColor, hoverColor, 1.1f)) {
                selectedCategory = 2;
            }
            if (DrawHoverButton(catBtn_3, categories[3], 20, baseColor, hoverColor, 1.1f)) {
                selectedCategory = 3;
            }
            if (DrawHoverButton(catBtn_4, categories[4], 20, baseColor, hoverColor, 1.1f)) {
                selectedCategory = 4;
            }

            // Efect de selecție pentru butonul de categorie
            Rectangle* selectedCatBtn = NULL;
            if (selectedCategory == 0) selectedCatBtn = &catBtn_0;
            if (selectedCategory == 1) selectedCatBtn = &catBtn_1;
            if (selectedCategory == 2) selectedCatBtn = &catBtn_2;
            if (selectedCategory == 3) selectedCatBtn = &catBtn_3;
            if (selectedCategory == 4) selectedCatBtn = &catBtn_4;

            if (selectedCatBtn) {
                Rectangle scaledBtn = *selectedCatBtn;
                scaledBtn.x -= (scaledBtn.width * (scaleSelected - 1.0f)) / 2;
                scaledBtn.y -= (scaledBtn.height * (scaleSelected - 1.0f)) / 2;
                scaledBtn.width *= scaleSelected;
                scaledBtn.height *= scaleSelected;
                DrawRectangleRec(scaledBtn, hoverColor);
                DrawText(categories[selectedCategory], scaledBtn.x + (scaledBtn.width - MeasureText(categories[selectedCategory], 20)) / 2,
                    scaledBtn.y + (scaledBtn.height - 20) / 2, 20, BLACK);
            }

            // Text static pentru dificultate
            DrawText("Choose a difficulty:", screenWidth / 2 - 120, screenHeight * 0.38f, 30, RAYWHITE);

            // Difficulty Buttons
            // Butoane de dificultate
            Rectangle diffBtn_0 = { screenWidth * 0.15f * 2, screenHeight * 0.5f, 200 , 75 };
            Rectangle diffBtn_1 = { screenWidth * 0.15f * 3, screenHeight * 0.5f, 200 , 75 };
            Rectangle diffBtn_2 = { screenWidth * 0.15f * 4, screenHeight * 0.5f, 200 , 75 };

            // Verificare hover și selecție pentru dificultăți
            if (DrawHoverButton(diffBtn_0, "Easy", 20, LIME, DARKGREEN, 1.1f)) {
                selected_index_diff = 0;
            }
            if (DrawHoverButton(diffBtn_1, "Medium", 20, ORANGE, GOLD, 1.1f)) {
                selected_index_diff = 1;
            }
            if (DrawHoverButton(diffBtn_2, "Hard", 20, RED, MAROON, 1.1f)) {
                selected_index_diff = 2;
            }

            // Aplicăm efect de selecție și scalare pentru dificultatea selectată
            Rectangle* selectedDiffBtn = NULL;
            Color selectedColor = WHITE;

            if (selected_index_diff == 0) {
                selectedDiffBtn = &diffBtn_0;
                selectedColor = DARKGREEN;
            }
            else if (selected_index_diff == 1) {
                selectedDiffBtn = &diffBtn_1;
                selectedColor = GOLD;
            }
            else if (selected_index_diff == 2) {
                selectedDiffBtn = &diffBtn_2;
                selectedColor = MAROON;
            }

            // Dacă există o dificultate selectată, o evidențiem
            if (selectedDiffBtn) {
                Rectangle scaledDiffBtn = *selectedDiffBtn;
                scaledDiffBtn.x -= (scaledDiffBtn.width * (scaleSelected - 1.0f)) / 2;
                scaledDiffBtn.y -= (scaledDiffBtn.height * (scaleSelected - 1.0f)) / 2;
                scaledDiffBtn.width *= scaleSelected;
                scaledDiffBtn.height *= scaleSelected;

                DrawRectangleRec(scaledDiffBtn, selectedColor);

                const char* text = (selected_index_diff == 0) ? "Easy" :
                    (selected_index_diff == 1) ? "Medium" : "Hard";

                DrawText(text, scaledDiffBtn.x + (scaledDiffBtn.width - MeasureText(text, 20)) / 2,
                    scaledDiffBtn.y + (scaledDiffBtn.height - 20) / 2, 20, BLACK);
            }


            // Buton CONTINUE-START GAME
            Rectangle continueBtn = { screenWidth * 0.15f * 2.65f, screenHeight * 0.8f, 400 , 75 };
            if (DrawHoverButton(continueBtn, "CONTINUE", 20, culoareMBAPPE, LIGHTGRAY, 1.1f)) {
                if (selectedCategory >= 0 && selected_index_diff >= 0) {
                    currentScreen = GAME_RUNNING;
                    gameJustStarted = true;
                    FisherYatesShuffle(&graphs[selectedCategory]); // Shuffle doar pentru graful categoriei selectate
                }
            }

            Rectangle backbtn_game = { screenWidth / 2 + 880 , screenHeight / 2 - 450, 60 , 24 };
            DrawHoverButton(backbtn_game, "BACK", 20, culoareMBAPPE, LIGHTGRAY, 1.1f);

            if (CheckCollisionPointRec(GetMousePosition(), backbtn_game) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                currentScreen = MENU_MAIN;
            }

            if (IsKeyPressed(KEY_TAB)) {
                currentScreen = MENU_MAIN;
            }
        }


        //-------------------------------------GAME-----------------------------------------

  else if (currentScreen == GAME_RUNNING) {
      // Desenăm fundalul
      DrawTexturePro(background_game,
          (Rectangle) {
          0, 0, (float)background_game.width - 20, (float)background_game.height
      },
          (Rectangle) {
          0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()
      },
          (Vector2) {
          0, 0
      }, 0.0f, WHITE);

      int pirateX = screenWidth / 2 - pirateSprite.width / 2 + 840;  // PIRAT CU SABIE
      int pirateY = screenHeight - pirateSprite.height - 210;

      float pirateScale = 0.4f;
      Vector2 piratePosition = { pirateX, pirateY };
      DrawTextureEx(pirateSprite, piratePosition, 0.0f, pirateScale, WHITE);

      int prizonierX = screenWidth / 2 - prizonierSprite.width / 2 + 700;
      int basePrizonierY = screenHeight - prizonierSprite.height - 235;
      float prizonierScale = 0.4f;

      // Adjustăm poziția prizonierului pe baza numărului de vieți rămase
      if (lives == 2) prizonierX = screenWidth / 2 - prizonierSprite.width / 2 + 600;
      if (lives == 1) prizonierX = screenWidth / 2 - prizonierSprite.width / 2 + 500;
      if (lives == 0) prizonierX = screenWidth / 2 - prizonierSprite.width / 2 + 360;

      // Dacă prizonierul cade
      if (isFalling) {
          fallTimer += GetFrameTime();
          if (fallTimer < 3.0f) {
              prisonerFallOffset = (fallTimer / 3.0f) * 650;  // Interpolare liniară: 0 -> 300px în 3s
          }
          else {
              currentScreen = GAME_LOST;
              isFalling = false;
              prisonerFallOffset = 0.0f;
          }
      }

      int prizonierY = basePrizonierY + (int)prisonerFallOffset;
      Vector2 prizonierPosition = { prizonierX, prizonierY };
      DrawTextureEx(prizonierSprite, prizonierPosition, 0.0f, prizonierScale, WHITE);

      // Logica de joc
      if (gameJustStarted) {
          if (graphs[selectedCategory].totalQuestions == 0) {
              DrawText("Eroare: Nu sunt întrebări încărcate!", screenWidth / 2 - 200, screenHeight / 2, 30, RED);
              return;
          }

          // Setare timp în funcție de dificultate
          if (selected_index_diff == 0) timerSeconds = 300;
          if (selected_index_diff == 1) timerSeconds = 180;
          if (selected_index_diff == 2) timerSeconds = 120;

          // Amestecăm întrebările din categoria selectată
          FisherYatesShuffle(&graphs[selectedCategory]);
          timerRunning = true;
          gameJustStarted = false;
          currentQuestionIndex = 0;
          lives = 3;
          correctAnswers = 0;
      }

      // Verificăm dacă jocul este terminat
      if (correctAnswers >= 12) {  // Dacă jucătorul a dat 12 răspunsuri corecte
          currentScreen = GAME_WIN;
      }

      if ((lives <= 0 || timerSeconds <= 0) && !isFalling) {
          isFalling = true;
          fallTimer = 0.0f;
      }

      if (isFalling) {
          fallTimer += GetFrameTime();
          if (fallTimer < 3.0f) {
              prisonerFallOffset = (fallTimer / 3.0f) * 650.0f;  // Cădere de 300 pixeli în 3 secunde
          }
          else {
              isFalling = false;
              currentScreen = GAME_LOST;
              prisonerFallOffset = 0.0f;
          }
      }

      // Desenăm panoul întrebării și răspunsurilor
      Rectangle question_board = { screenWidth * 0.1f - 70, screenHeight * 0.3f, 1000, 300 };
      Rectangle answer_board = { screenWidth * 0.1f - 70, screenHeight * 0.6f + 40, 1000, 200 };
      DrawRectangleRec(question_board, GRAY);
      DrawRectangleRec(answer_board, LIGHTGRAY);

      // Afișăm viețile și timpul rămas
      DrawText(TextFormat("Lives: %d", lives), 50, 50, 30, BLACK);
      DrawText(TextFormat("Time: %.1f sec", timerSeconds), screenWidth / 2 - 100, 50, 30, BLACK);

      // Logica pentru afișarea întrebărilor și răspunsurilor
      if (!isFalling && timerRunning && timerSeconds > 0 && lives > 0) {
          timerSeconds -= GetFrameTime();
          QuestionNode* currentQuestion = &graphs[selectedCategory].questions[currentQuestionIndex];
          DrawText(currentQuestion->questionText, question_board.x + 20, question_board.y + 20, 25, BLACK);

          // Afișăm răspunsurile și gestionăm interacțiunea
          for (int i = 0; i < MAX_OPTIONS; i++) {
              Rectangle answerBtn = { answer_board.x + 20, answer_board.y + 20 + i * 60, 955, 50 };
              DrawRectangleRec(answerBtn, SKYBLUE);
              DrawText(currentQuestion->answers[i], answerBtn.x + 10, answerBtn.y + 15, 20, BLACK);

              // Verificăm dacă jucătorul a ales un răspuns corect sau greșit
              if (CheckCollisionPointRec(GetMousePosition(), answerBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                  if (i == currentQuestion->correctAnswer) {
                      correctAnswers++;
                      HandleAnswer(true);  // Răspuns corect
                      // Atribuim o întrebare mai dificilă
                      currentQuestionIndex = GetNextHardQuestion(currentQuestionIndex);
                  }
                  else {
                      lives--;
                      timerSeconds -= 5.0f;
                      if (timerSeconds < 0) timerSeconds = 0;
                      HandleAnswer(false);  // Răspuns greșit
                      // Trece la următoarea întrebare, dar nu schimbă dificultatea
                      currentQuestionIndex = GetNextEasyQuestion(currentQuestionIndex);
                  }

                  // Verificăm dacă s-a ajuns la finalul jocului
                  if (correctAnswers >= 12) {
                      currentScreen = GAME_WIN;
                  }

                  if ((lives <= 0 || timerSeconds <= 0) && !isFalling) {
                      isFalling = true;
                      fallTimer = 0.0f;
                  }
              }
          }
      }

      // Butonul de back
      Rectangle backbtn_game = { screenWidth / 2 + 880 , screenHeight / 2 - 450, 60 , 24 };
      DrawHoverButton(backbtn_game, "BACK", 20,transparentColor, BLUE, 1.1f);

      if (CheckCollisionPointRec(GetMousePosition(), backbtn_game) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
          currentScreen = MENU_CATEGORIES;
      }

      if (IsKeyPressed(KEY_SPACE))
      {
          currentScreen = MENU_PAUSE;
      }



      }

      


       //--------------------------------------------PAUSE-------------------------------------------------------


        else if (currentScreen == MENU_PAUSE)
        {
            DrawText("PAUSE MENU", screenWidth / 2 - 100, screenHeight * 0.2f, 30, BLACK);//aici sunt centrate prost astea cand dam pe fullscreen dupa ce unim codurile le rezolv eu dar trebuie sa vad cum arata tot(am incercat sa fac un buton de fullscreen dar era destul de bugat si i am dat scrap putem sal bagam la final daca e)

            Rectangle resumeBtn = { screenWidth / 2 - 100, screenHeight * 0.35f, 200, 50 };
            DrawRectangleRec(resumeBtn, CheckCollisionPointRec(GetMousePosition(), resumeBtn) ? RED : LIGHTGRAY);
            DrawText("Resume", resumeBtn.x + 60, resumeBtn.y + 15, 20, BLACK);

            Rectangle restartBtn = { screenWidth / 2 - 100, screenHeight * 0.45f, 200, 50 };
            DrawRectangleRec(restartBtn, CheckCollisionPointRec(GetMousePosition(), restartBtn) ? RED : LIGHTGRAY);
            DrawText("Restart", restartBtn.x + 60, restartBtn.y + 15, 20, BLACK);

            Rectangle mainMenuBtn = { screenWidth / 2 - 100, screenHeight * 0.55f, 200, 50 };
            DrawRectangleRec(mainMenuBtn, CheckCollisionPointRec(GetMousePosition(), mainMenuBtn) ? RED : LIGHTGRAY);
            DrawText("Main Menu", mainMenuBtn.x + 45, mainMenuBtn.y + 15, 20, BLACK);

            Rectangle quitBtn = { screenWidth / 2 - 100, screenHeight * 0.65f, 200, 50 };
            DrawRectangleRec(quitBtn, CheckCollisionPointRec(GetMousePosition(), quitBtn) ? RED : LIGHTGRAY);
            DrawText("Quit", quitBtn.x + 75, quitBtn.y + 15, 20, BLACK);

            if (CheckCollisionPointRec(GetMousePosition(), resumeBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                currentScreen = GAME_RUNNING;
            }

            if (CheckCollisionPointRec(GetMousePosition(), mainMenuBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                currentScreen = MENU_MAIN;
            }

            if (CheckCollisionPointRec(GetMousePosition(), restartBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                gameRestarted = true;
                currentScreen = MENU_CATEGORIES;
            }

            if (CheckCollisionPointRec(GetMousePosition(), quitBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                UnloadMusicStream(backgroundMusic);
                CloseAudioDevice();
                CloseWindow();
                return 0;
            }
        }


        //------------------------------------RATING----------------------------------------

        else if (currentScreen == MENU_RATING)
        {
            const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "Beat the Bomb - Help Button Example");

    GameScreen currentScreen = MENU;

    Rectangle buttonBounds = { screenWidth / 2.0f - 100, screenHeight / 2.0f - 25, 200, 50 };
    bool helpImagesLoaded = false;
    Texture2D helpImage1, helpImage2;

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        // LOGIC
        Vector2 mousePoint = GetMousePosition();

        if (currentScreen == MENU)
        {
            if (CheckCollisionPointRec(mousePoint, buttonBounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                currentScreen = HELP;

                // Încarcă imaginile odată când intri în HELP
                if (!helpImagesLoaded)
                {
                    helpImage1 = LoadTexture("help1.png");
                    helpImage2 = LoadTexture("help2.png");
                    helpImagesLoaded = true;
                }
            }
        }

        // DRAW
        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (currentScreen == MENU)
        {
            DrawText("Main Menu", screenWidth / 2 - MeasureText("Main Menu", 30) / 2, 100, 30, DARKGRAY);

            DrawRectangleRec(buttonBounds, LIGHTGRAY);
            DrawText("Help", buttonBounds.x + 70, buttonBounds.y + 15, 20, DARKBLUE);
        }
        else if (currentScreen == HELP)
        {
            DrawText("Help Screen", 20, 20, 30, DARKGRAY);
            DrawTexture(helpImage1, 100, 100, WHITE);
            DrawTexture(helpImage2, 400, 100, WHITE);
        }

        EndDrawing();
    }

    // Clean up
    if (helpImagesLoaded)
    {
        UnloadTexture(helpImage1);
        UnloadTexture(helpImage2);
    }

        }

        //--------------------------------OPTIONS-----------------------------------------

        else if (currentScreen == MENU_OPTIONS)
        {
            DrawText("OPTIONS", screenWidth / 2 - 70, 40, 30, BLACK);

            Rectangle musicMenuBtn = { screenWidth / 2 - 100, 100, 200, 50 };
            DrawRectangleRec(musicMenuBtn, CheckCollisionPointRec(GetMousePosition(), musicMenuBtn) ? RED : LIGHTGRAY);
            DrawText("Music", musicMenuBtn.x + 70, musicMenuBtn.y + 15, 20, BLACK);

            if (CheckCollisionPointRec(GetMousePosition(), musicMenuBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                currentScreen = MENU_MUSIC;
            }

            // Culoare de background
            Rectangle bgBtn = { screenWidth / 2 - 100, 160, 200, 50 };
            DrawRectangleRec(bgBtn, CheckCollisionPointRec(GetMousePosition(), bgBtn) ? RED : LIGHTGRAY);
            DrawText("Theme", bgBtn.x + 65, bgBtn.y + 15, 20, BLACK);

            if (CheckCollisionPointRec(GetMousePosition(), bgBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                currentScreen = MENU_THEME;
            }

            Rectangle musicMenuBackBtn = { screenWidth / 2 - 100, 400, 200, 50 };
            DrawRectangleRec(musicMenuBackBtn, CheckCollisionPointRec(GetMousePosition(), musicMenuBackBtn) ? LIGHTGRAY : RED);
            DrawText("Back", musicMenuBackBtn.x + 70, musicMenuBackBtn.y + 15, 20, BLACK);

            if (CheckCollisionPointRec(GetMousePosition(), musicMenuBackBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                currentScreen = MENU_MAIN;
            }


            if (IsKeyPressed(KEY_TAB))
            {
                currentScreen = MENU_MAIN;
            }
        }
        //--------------------------------MUZICA-----------------------------------------
        else if (currentScreen == MENU_MUSIC)
        {
            DrawText("MUSIC", screenWidth / 2 - 50, 40, 30, BLACK);
            // Pornire/orpire muzica
            Rectangle musicBtn = { screenWidth / 2 - 100, 100, 200, 50 };
            DrawRectangleRec(musicBtn, CheckCollisionPointRec(GetMousePosition(), musicBtn) ? RED : LIGHTGRAY);

            DrawText(musicOn ? "Music: ON" : "Music: OFF", musicBtn.x + 50, musicBtn.y + 15, 20, BLACK);

            if (CheckCollisionPointRec(GetMousePosition(), musicBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                musicOn = !musicOn;
            }

            // Bara de volum
            DrawText("Volume", screenWidth / 2 - 100, 180, 20, BLACK);
            Rectangle sliderBar = { screenWidth / 2 - 100, 210, 200, 20 };
            DrawRectangleRec(sliderBar, DARKGRAY);
            Rectangle sliderKnob = { sliderBar.x + musicVolume * sliderBar.width - 10, sliderBar.y - 5, 20, 30 };
            DrawRectangleRec(sliderKnob, RED);

            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), sliderBar))
            {
                musicVolume = (GetMousePosition().x - sliderBar.x) / sliderBar.width;
                if (musicVolume < 0.0f)
                    musicVolume = 0.0f;
                if (musicVolume > 1.0f)
                    musicVolume = 1.0f;
            }

            SetMusicVolume(backgroundMusic, musicOn ? musicVolume : 0.0f);

            Rectangle changeSongBtn = { screenWidth / 2 - 100, 270, 200, 50 };
            DrawRectangleRec(changeSongBtn, CheckCollisionPointRec(GetMousePosition(), changeSongBtn) ? RED : LIGHTGRAY);
            DrawText("Change Song", changeSongBtn.x + 35, changeSongBtn.y + 15, 20, BLACK);

            if (CheckCollisionPointRec(GetMousePosition(), changeSongBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                StopMusicStream(backgroundMusic);
                UnloadMusicStream(backgroundMusic);

                currentSongIndex = (currentSongIndex + 1) % NUM_SONGS;
                backgroundMusic = LoadMusicStream(songFiles[currentSongIndex]);
                PlayMusicStream(backgroundMusic);
                SetMusicVolume(backgroundMusic, musicOn ? musicVolume : 0.0f);
            }

            if (IsKeyPressed(KEY_ESCAPE))
            {
                currentScreen = MENU_OPTIONS;
            }
        
        
        Rectangle musicBackBtn = { screenWidth / 2 - 100, 400, 200, 50 };
        DrawRectangleRec(musicBackBtn, CheckCollisionPointRec(GetMousePosition(), musicBackBtn) ? LIGHTGRAY : RED);
        DrawText("Back", musicBackBtn.x + 70, musicBackBtn.y + 15, 20, BLACK);

        if (CheckCollisionPointRec(GetMousePosition(), musicBackBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            currentScreen = MENU_OPTIONS;
        }



        if (IsKeyPressed(KEY_TAB))
        {
            currentScreen = MENU_MAIN;
        }
}
    
            //--------------------------------THEMES----------------------------------------

  
        else if (currentScreen == MENU_THEME) {
            DrawText("THEME", screenWidth / 2 - 55, 40, 30, BLACK);

            // Butonul de schimbare a temei
            Rectangle themeBtn = { screenWidth / 2 - 100, 100, 200, 50 };
            DrawRectangleRec(themeBtn, CheckCollisionPointRec(GetMousePosition(), themeBtn) ? RED : LIGHTGRAY);
            DrawText("Change Theme", themeBtn.x + 30, themeBtn.y + 15, 20, BLACK);

            // La apăsarea butonului schimbăm tema
            if (CheckCollisionPointRec(GetMousePosition(), themeBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                themeIndex = (themeIndex + 1) % 2;  // Schimbăm între 0 și 1
                if (themeIndex == 0) {
                   background_categ = LoadTexture("pirates.jpeg"); // Reîncarcă fundalul cu pirat
                }
                else {
                     background_categ = LoadTexture("categ_background_ship.jpeg"); // Reîncarcă fundalul cu categoria
                }
            }

            // Butonul de întoarcere
            Rectangle themeBackBtn = { screenWidth / 2 - 100, 400, 200, 50 };
            DrawRectangleRec(themeBackBtn, CheckCollisionPointRec(GetMousePosition(), themeBackBtn) ? LIGHTGRAY : RED);
            DrawText("Back", themeBackBtn.x + 70, themeBackBtn.y + 15, 20, BLACK);

            if (CheckCollisionPointRec(GetMousePosition(), themeBackBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                currentScreen = MENU_OPTIONS;
            }

            // Butonul ESC
            if (IsKeyPressed(KEY_ESCAPE)) {
                currentScreen = MENU_OPTIONS;
            }
            }


       //-------------------------------------GAME LOST--------------------------------------------------------

        else if (currentScreen == GAME_LOST) {
            DrawText("Game Over! Apasă R pentru a reîncepe.", screenWidth / 2 - 200, screenHeight / 2, 30, RED);
            if (IsKeyPressed(KEY_R)) {
                gameJustStarted = true;
                currentScreen = MENU_CATEGORIES;  // Revenim la selectarea categoriei
            }
        }

        //------------------------------------------------------------GAME WON--------------------------------------------
        else if (currentScreen == GAME_WIN) {
            DrawText("Winner of the game,Congratiulations!.", screenWidth / 2 - 200, screenHeight / 2, 30, RED);
            if (IsKeyPressed(KEY_R)) {
                gameJustStarted = true;
                currentScreen = MENU_CATEGORIES;  // Revenim la selectarea categoriei
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

