//#include <iostream>
#include <vector>
#include <string>
#include "raylib.h"
#include "button.hpp"

// TODO: Delete this line below, textButton.cpp, textButton.hpp if not used in the future
// #include "textButton.hpp"

// TODO: Add gameover screen, add new rules screen, add multiplyer screen

//using namespace std;

// Screen manager, based on an example from the raylib website
typedef enum GameScreen { MAIN_MENU = 0, SETTINGS, RULES, SINGLEPLAYER, MULTIPLAYER, PAUSE, GAMEOVER } GameScreen;

// Draws text and dynamically centers it horizontally 
void DrawTextHorizontal (Font font, const char* text, float fontSize, float fontSpacing, Color fontColor, float posY) {
    Vector2 textSize = MeasureTextEx(font, text, fontSize, fontSpacing);
    DrawTextEx(font, text, {(float)(GetScreenWidth() - textSize.x) / 2.0f, posY}, fontSize, fontSpacing, fontColor);
}

// Wrap text into multiple lines based on maxWidth
std::vector<std::string> WrapText(Font font, const char* text, int maxWidth, int fontSize) {
    std::vector<std::string> lines;
    std::string currentLine;
    std::string currentWord;

    const char* ptr = text;

    while (*ptr) {
        if (*ptr == ' ' || *ptr == '\n') {
            // Measure the current line with the new word appended
            std::string testLine = currentLine.empty() ? currentWord : currentLine + " " + currentWord;
            int testWidth = MeasureTextEx(font, testLine.c_str(), fontSize, 1).x;

            if (testWidth <= maxWidth && *ptr != '\n') {
                currentLine = testLine; // Append the word to the current line
            } else {
                // Push the current line and start a new one
                if (!currentLine.empty()) lines.push_back(currentLine);
                currentLine = currentWord;
            }

            // If it's a newline character, finalize the current line
            if (*ptr == '\n') {
                if (!currentLine.empty()) lines.push_back(currentLine);
                currentLine.clear();
            }

            currentWord.clear(); // Reset the word
        } else {
            // Build the current word
            currentWord += *ptr;
        }

        ++ptr;
    }

    // Add the last word and line
    if (!currentWord.empty()) {
        std::string testLine = currentLine.empty() ? currentWord : currentLine + " " + currentWord;
        if (MeasureTextEx(font, testLine.c_str(), fontSize, 1).x <= maxWidth) {
            currentLine = testLine;
        } else {
            if (!currentLine.empty()) lines.push_back(currentLine);
            currentLine = currentWord;
        }
    }
    if (!currentLine.empty()) lines.push_back(currentLine);

    return lines;
}

// Draw wrapped text centered both vertically and horizontally
void DrawQuestionText(Font font, const char* text, int maxWidth, int screenWidth, int screenHeight, int fontSize, Color color) {
    // Get wrapped lines
    std::vector<std::string> lines = WrapText(font, text, maxWidth, fontSize);

    // Calculate total height for vertical centering
    int totalHeight = lines.size() * fontSize;
    int posY = (screenHeight - totalHeight) / 2; // Vertically center

    for (const std::string& line : lines) {
        // Measure and center the line horizontally
        int lineWidth = MeasureTextEx(font, line.c_str(), fontSize, 1).x;
        int posX = (screenWidth - lineWidth) / 2;
        DrawTextEx(font, line.c_str(), (Vector2){(float)posX, (float)posY - 100}, fontSize, 1, color);
        posY += fontSize; // Move to the next line
    }
}

// Draw and center text for the answer buttons based on the buttons' dimensions
void DrawAnswerText(Font font, const char *text, float fontSize, float spacing, Color color, float buttonX, float buttonY, float buttonWidth, float buttonHeight) {
    // Get the text dimensions using MeasureTextEx
    Vector2 textSize = MeasureTextEx(font, text, fontSize, spacing); 
    int textWidth = textSize.x;
    int textHeight = textSize.y;

    // Calculate text position for centering
    float textX = buttonX + (buttonWidth - textWidth) / 2;
    float textY = buttonY + (buttonHeight - textHeight) / 2;

    // Draw the text centered in the button
    DrawTextEx(font, text, (Vector2){textX, textY + 10}, fontSize, spacing, color);
}


// Checks for any key press
bool IsAnyKeyPressed(void) {
    for (int key = 32; key < 350; key++) { 
        if (IsKeyPressed(key)) return true;        
    }
    return false;
}

struct Question {
    std::string questionText;
    std::vector<std::string> answers;
    int correctAnswerIndex;
};

std::vector<Question> questions = {
    {"What is the capital of France?", {"Paris", "London", "Berlin", "Madrid"}, 0},
    {"What is 2 + 2?", {"3", "4", "5", "6"}, 1},
    {"Which planet is known as the Red Planet?", {"Venus", "Saturn", "Mars", "Mercury"}, 2},
    {"Which element is known as the building block of life?", {"Oxygen", "Carbon", "Hydrogen", "Nitrogen"}, 1},
    {"Which is the fastest bird in the world?",  {"Bald Eagle", "Peregrine Falcon", "Hummingbird", "Raven"}, 1}
};

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void) 
{
    // Initialization
    //--------------------------------------------------------------------------------------
    GameScreen currentScreen = MAIN_MENU;
    

    int screenWidth = 1920;
    int screenHeight = 1080;
    InitWindow(screenWidth, screenHeight, "BRAIN BLOOM");

    // Game launches at fullscreen, can be changed in the games' settings
    //ToggleFullscreen();                   
    SetExitKey(KEY_NULL);              
    SetTargetFPS(60);
    
    bool exitPressed = false;
    bool showCorrectAnswer = false;
    bool singlePLayerSelected = false; 

    // Prevents mouse input when currentScreen transitions to RULES
    float inputCooldown = 0.2f; // Cooldown time in seconds
    float timer = 0.0f;

    int countdownTime = 21;
    int seconds = 0;
    int currentQuestionIndex = 0;
    int selectedAnswerIndex = 0;
    int score = 0;
    int healthPoints = 5;

    double startTime = GetTime();


    Font arcadeFont = LoadFont("fonts/arcade.ttf");

    // Main Menu Textures
    Texture2D titleLogo = LoadTexture("assets/title-logo.png");
    Texture2D menuBackground = LoadTexture("assets/main-menu-bg.png");
    Texture2D settingsBackground = LoadTexture("assets/settings-bg.png");
    Texture2D rulesScreen = LoadTexture("assets/rules-screen.png");
    Texture2D pausedTxt = LoadTexture("assets/game-paused-txt.png");

    // Singleplayer Textures
    Texture2D singleplayerBackground = LoadTexture("assets/singleplayer-bg.png");
    Texture2D questionBox = LoadTexture("assets/question-box.png");

    // Main Menu Buttons
    Button onePlayerBtn{"assets/one-player-btn.png", {0, 500}, 0.5}; 
    Button twoPlayerBtn{"assets/two-players-btn.png", {0, 600}, 0.5};
    Button settingsBtn{"assets/settings-btn.png", {0, 700}, 0.6};
    Button exitBtn{"assets/exit-btn.png", {0, 800}, 0.6};

    // Pause Buttons
    Button pauseBtn{"assets/pause-btn.png", {10, 10}, 0.7};
    Button resumeBtn{"assets/resume-btn.png", {0, 400}, 0.7};
    Button mainMenuBtn{"assets/main-menu-btn.png", {0, 500}, 0.7};

    // Singlepayer Buttons
    Button answerQ_Btn{"assets/answer-q.png", {150.0f, (float) (GetScreenHeight() - 350.0f)}, 1.3f};
    Button answerW_Btn{"assets/answer-w.png", {(float) (GetScreenWidth() - 900.0f), (float) (GetScreenHeight() - 350.0f)}, 1.3f}; 
    Button answerE_Btn{"assets/answer-e.png", {150.0f, (float) (GetScreenHeight() - 200.0f)}, 1.3f};
    Button answerR_Btn{"assets/answer-r.png", {(float) (GetScreenWidth() - 900.0f), (float) (GetScreenHeight() - 200.0f)}, 1.3f};

    Button abilityA_Btn{"assets/skill-a.png", {(float) (GetScreenWidth() - 430.0f), 260.0f}, 0.5f};
    Button abilityS_Btn{"assets/skill-s.png", {(float) (GetScreenWidth() - 280.0f), 260.0f}, 0.5f};
    Button abilityD_Btn{"assets/skill-d.png", {(float) (GetScreenWidth() - 430.0f), 420.0f}, 0.53f}; 
    Button abilityF_Btn{"assets/skill-f.png", {(float) (GetScreenWidth() - 280.0f), 420.9f}, 0.538f};

    Color pauseDark = {0,0,0, 100};

    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose() && !exitPressed)
    {  
        float deltaTime = GetFrameTime();
        timer += deltaTime;

        Vector2 mousePosition = GetMousePosition();
        bool mouseClicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT); 
        
        switch(currentScreen) {
            case MAIN_MENU:
                countdownTime = 21;    // Reset timer

                if (onePlayerBtn.isDrawn && onePlayerBtn.isClicked(mousePosition, mouseClicked)) {
                    currentScreen = RULES;
                    singlePLayerSelected = true;
                    timer = 0.0f;
                }
                if (twoPlayerBtn.isDrawn && twoPlayerBtn.isClicked(mousePosition, mouseClicked)) {
                    currentScreen = RULES;
                    singlePLayerSelected = false;
                    timer = 0.0f;  
                }
                if (settingsBtn.isDrawn && settingsBtn.isClicked(mousePosition, mouseClicked)) {
                    currentScreen = SETTINGS;
                }   
                if (exitBtn.isDrawn && exitBtn.isClicked(mousePosition, mouseClicked)) {
                    exitPressed = true;
                }   
            break;
            case SETTINGS:
                if (IsKeyPressed(KEY_ESCAPE)) {
                    currentScreen = MAIN_MENU;
                }
                break;
            case SINGLEPLAYER:
                // Timer
                if (GetTime() - startTime >= 1.0) {
                    countdownTime--;
                    startTime = GetTime();
                }

                if (countdownTime < 0) {
                    countdownTime = 0;
                }

                seconds = countdownTime % 60;

                // Answers
                if (answerQ_Btn.isDrawn && answerQ_Btn.isClicked(mousePosition, mouseClicked)) selectedAnswerIndex = 0;
                if (answerW_Btn.isDrawn && answerW_Btn.isClicked(mousePosition, mouseClicked)) selectedAnswerIndex = 1;
                if (answerE_Btn.isDrawn && answerE_Btn.isClicked(mousePosition, mouseClicked)) selectedAnswerIndex = 2;
                if (answerR_Btn.isDrawn && answerR_Btn.isClicked(mousePosition, mouseClicked)) selectedAnswerIndex = 3;

                if (selectedAnswerIndex == questions[currentQuestionIndex].correctAnswerIndex) score++;
                if (healthPoints <= 0) currentScreen = GAMEOVER;
                else healthPoints--;
                
                // Pause
                if (pauseBtn.isDrawn && pauseBtn.isClicked(mousePosition, mouseClicked) || IsKeyPressed(KEY_ESCAPE) ) currentScreen = PAUSE;

                // TODO: Add logic for the skills
                if (abilityA_Btn.isDrawn && abilityA_Btn.isClicked(mousePosition, mouseClicked)) {
                    //
                }
                if (abilityS_Btn.isDrawn && abilityS_Btn.isClicked(mousePosition, mouseClicked)) {
                    //
                }
                if (abilityD_Btn.isDrawn && abilityD_Btn.isClicked(mousePosition, mouseClicked)) {
                    //
                }
                if (abilityF_Btn.isDrawn && abilityF_Btn.isClicked(mousePosition, mouseClicked)) {
                    //
                }

            break;
            case PAUSE:
                if (resumeBtn.isDrawn && resumeBtn.isClicked(mousePosition, mouseClicked) ) {
                    currentScreen = SINGLEPLAYER;
                }
                if (mainMenuBtn.isDrawn && mainMenuBtn.isClicked(mousePosition, mouseClicked) ) {
                    currentScreen = MAIN_MENU;
                }
                if (IsKeyPressed(KEY_ESCAPE)) {
                    currentScreen = SINGLEPLAYER;
                }
            break;
            default:
                break;
        }

        //----------------------------------------------------------------------------------
        // Draw
        
        BeginDrawing();
        ClearBackground(GRAY);

        switch (currentScreen)
        {
        case MAIN_MENU:
            DrawTexture(menuBackground, 0,0, WHITE);
            DrawTexture(titleLogo, ((GetScreenWidth() - titleLogo.width) / 2.0), 200, WHITE); 
            onePlayerBtn.DrawButtonHorizontal();
            twoPlayerBtn.DrawButtonHorizontal();
            settingsBtn.DrawButtonHorizontal();
            exitBtn.DrawButtonHorizontal();
            break;
        case SINGLEPLAYER:
            DrawTexture(singleplayerBackground, 0, 0, WHITE);
            DrawTextureEx(questionBox, {(float)(GetScreenWidth() - questionBox.width * 1.8) / 2.0f, 200}, 0, 1.8, WHITE);
            
            DrawQuestionText(arcadeFont, questions[currentQuestionIndex].questionText.c_str(), 800, GetScreenWidth(), GetScreenHeight(), 30, BLACK);

            answerQ_Btn.DrawButton();
            answerW_Btn.DrawButton();
            answerE_Btn.DrawButton();
            answerR_Btn.DrawButton();

            DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[0].c_str(), 25, 1, BLACK, answerQ_Btn.position.x, answerQ_Btn.position.y, answerQ_Btn.width, answerQ_Btn.height);
            DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[1].c_str(), 25, 1, BLACK, answerW_Btn.position.x, answerW_Btn.position.y, answerW_Btn.width, answerW_Btn.height);
            DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[2].c_str(), 25, 1, BLACK, answerE_Btn.position.x, answerE_Btn.position.y, answerE_Btn.width, answerE_Btn.height);
            DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[3].c_str(), 25, 1, BLACK, answerR_Btn.position.x, answerR_Btn.position.y, answerR_Btn.width, answerR_Btn.height);

            // Draw Timer
            DrawTextHorizontal(arcadeFont, TextFormat("Timer: %i", seconds), 50, 1, BLACK, 100);

            // Draw Skill
            abilityA_Btn.DrawButton();
            abilityS_Btn.DrawButton();
            abilityD_Btn.DrawButton();
            abilityF_Btn.DrawButton();
            pauseBtn.DrawButton();
            break;
        case SETTINGS:
            DrawTexture(settingsBackground, 0, 0 , WHITE);
            break;
        case RULES:
            DrawTexture(rulesScreen, 0, 0, WHITE);
            DrawTextHorizontal(arcadeFont, "Press any button to start", 30, 1, WHITE, GetScreenHeight() - 200);

            if (timer > inputCooldown) {
                if (IsAnyKeyPressed() || IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))  {
                    currentScreen = (singlePLayerSelected) ? SINGLEPLAYER : MULTIPLAYER;
                }
            }
            break;
        case PAUSE:
            DrawTexture(singleplayerBackground,0,0,WHITE);
            DrawRectangle(0,0,GetScreenWidth(),GetScreenHeight(), pauseDark);
            DrawTexture(pausedTxt, ((GetScreenWidth() - pausedTxt.width) / 2), 150, WHITE); 
            resumeBtn.DrawButtonHorizontal();
            mainMenuBtn.DrawButtonHorizontal();
            break;
        default:
            break;
        }

        EndDrawing();
        
        //----------------------------------------------------------------------------------
    }

    //--------------------------------------------------------------------------------------
    // De-Initialization
    // TODO: Unload all loaded data (textures, fonts, audio) here!
    
    UnloadFont(arcadeFont);
    UnloadTexture(menuBackground);
    UnloadTexture(titleLogo);
    UnloadTexture(pausedTxt);
    UnloadTexture(rulesScreen);
    UnloadTexture(singleplayerBackground);
    UnloadTexture(settingsBackground);
    UnloadTexture(questionBox);


    CloseWindow();  // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}