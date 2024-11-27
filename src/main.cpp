//#include <iostream>
#include "raylib.h"
#include "button.hpp"

// TODO: Delete this line below, textButton.cpp, textButton.hpp if not used in the future
// #include "textButton.hpp"

//using namespace std;

// Screen manager, based on an example from the raylib website
typedef enum GameScreen { MAIN_MENU = 0, SETTINGS, RULES, SINGLEPLAYER, MULTIPLAYER, PAUSE } GameScreen;

// Draws text and dynamically centers it horizontally 
void DrawTextHorizontal (Font font, const char* text, float fontSize, float fontSpacing, Color fontColor, float posY) {
    Vector2 textSize = MeasureTextEx(font, text, fontSize, fontSpacing);
    DrawTextEx(font, text, {(float)(GetScreenWidth() - textSize.x) / 2.0f, posY}, fontSize, fontSpacing, fontColor);
}

// Checks for any key press
bool IsAnyKeyPressed() {
    for (int key = 32; key < 350; key++) { 
        if (IsKeyPressed(key)) return true;        
    }
    return false;
}

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

    // Game launches at fullscreen, can be changed in the settings
    //ToggleFullscreen();                   
    SetExitKey(KEY_NULL);              
    SetTargetFPS(60);
    
    bool exitPressed = false;

    // Prevents mouse input when currentScreen transitions to RULES
    float inputCooldown = 0.2f; // Cooldown time in seconds
    float timer = 0.0f; 

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
    Button answerQBtn{"assets/answer-q.png", {150, (float) (GetScreenHeight() - 350)}, 1.3};
    Button answerWBtn{"assets/answer-w.png", {150, (float) (GetScreenHeight() - 200)}, 1.3};
    Button answerEBtn{"assets/answer-e.png", {(float) (GetScreenWidth() - 900), (float) (GetScreenHeight() - 350)}, 1.3};
    Button answerRBtn{"assets/answer-r.png", {(float) (GetScreenWidth() - 900), (float) (GetScreenHeight() - 200)}, 1.3};
    Button skillABtn{"assets/skill-a.png", {0, 0}, 1};
    Button skillSBtn{"assets/skill-s.png", {0, 0}, 1};
    Button skillDBtn{"assets/skill-d.png", {0, 0}, 1};
    Button skillFBtn{"assets/skill-f.png", {0, 0}, 1};

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
                if (onePlayerBtn.isDrawn && onePlayerBtn.isClicked(mousePosition, mouseClicked)) {
                    currentScreen = RULES;
                    timer = 0.0f;
                }
                if (twoPlayerBtn.isDrawn && twoPlayerBtn.isClicked(mousePosition, mouseClicked)) {
                    //currentScreen = RULES;            
                    //timer = 0.0f;
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
                if (pauseBtn.isDrawn && pauseBtn.isClicked(mousePosition, mouseClicked) ) {
                    currentScreen = PAUSE;
                }
                if (IsKeyPressed(KEY_ESCAPE)) {
                    currentScreen = PAUSE;
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
            DrawTexture(singleplayerBackground,0,0,WHITE);
            DrawTextureEx(questionBox, {(float)(GetScreenWidth() - questionBox.width * 1.9) / 2.0f, 150}, 0, 1.9, WHITE);
            answerQBtn.DrawButton();
            answerWBtn.DrawButton();
            answerEBtn.DrawButton();
            answerRBtn.DrawButton();
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
                    currentScreen = SINGLEPLAYER;
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

    CloseWindow();  // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}