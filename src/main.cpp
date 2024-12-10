/*****************************************************************************
*   TODO:
*   Add multiplayer (screen, mechanics, make user input two names, etc)
*   Add main menu background animation
*   Change timer font
*   Complete settings (UI, options for window size, audio)
*   Cosmetics (cursor skins when player reaches a highscore)
*   Audio (for buttons and etc.)
*   Implement loading screen
*   Clean code up and remove redundancy (make variables global)
*****************************************************************************/

#include <vector>
#include <string>
#include <fstream>
#include "raylib.h"
#include "button.hpp"
#include "questions.hpp"

#define DATA_FILE_PATH "data/data.bin" 

// Screen manager, based on an example from the raylib website
typedef enum GameScreen { MAIN_MENU = 0, STARTGAME, SETTINGS, RULES, RULES1, SINGLEPLAYER, MULTIPLAYER, READY, PAUSE, GAMEOVER, CONTROLS1, PLAYERNAME, EXIT } GameScreen;

// Draws text and dynamically centers it horizontally 
void DrawTextHorizontal (Font font, const char* text, float fontSize, float fontSpacing,
                         Color fontColor, float posY) {
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
void DrawQuestionText(Font font, const char* text, int maxWidth, int screenWidth, int screenHeight,
                      int fontSize, Color color, bool isMultiplayer) {
    // Get wrapped lines
    std::vector<std::string> lines = WrapText(font, text, maxWidth, fontSize);

    // Calculate total height for vertical centering
    int totalHeight = lines.size() * fontSize;
    float posY = (screenHeight - totalHeight) / 2; // Vertically center

    for (const std::string& line : lines) {
        // Measure and center the line horizontally
        int lineWidth = MeasureTextEx(font, line.c_str(), fontSize, 1).x;
        float posX = (screenWidth - lineWidth) / 2;
        DrawTextEx(font, line.c_str(), (Vector2){(float)posX, (isMultiplayer) ? posY - 120: posY - 100}, fontSize, 1, color);
        posY += fontSize; // Move to the next line
    }
}

//Re-center Player name input in a fixed x and y coordinates base on its with size and length
void DrawCenteredTextAtX(const char* text, Font font, float x, float y, int minFontSize, int maxFontSize, float maxWidth, Color color) {
    int fontSize = maxFontSize;
    Vector2 textSize = MeasureTextEx(font, text, fontSize, 1);

    while (textSize.x > maxWidth && fontSize > minFontSize) {
        fontSize--;
        textSize = MeasureTextEx(font, text, fontSize, 1);
    }

    DrawTextEx(font, text, (Vector2){x - textSize.x / 2, y - textSize.y / 2}, fontSize, 1, color);
}

// Draw and center text for the answer buttons based on the buttons' dimensions
void DrawAnswerText(Font font, const char *text, float fontSize, float spacing, Color color, 
                    float buttonX, float buttonY, float buttonWidth, float buttonHeight, int maxWidth, bool isMultiplayer) {
    // Wrap the text into multiple lines
    std::vector<std::string> lines = WrapText(font, text, maxWidth, fontSize);

    // Calculate total height of wrapped text for vertical centering
    float totalHeight = lines.size() * fontSize;

    // Starting Y position to vertically center the text within the button
    float startY = buttonY + (buttonHeight - totalHeight) / 2;

    // Loop through the wrapped lines and draw them
    for (const std::string &line : lines) {
        // Measure line width and calculate X position for horizontal centering
        float lineWidth = MeasureTextEx(font, line.c_str(), fontSize, spacing).x;
        float lineX = buttonX + (buttonWidth - lineWidth) / 2;

        // Draw the line at the calculated position
        DrawTextEx(font, line.c_str(), (Vector2){lineX, (isMultiplayer) ? startY :startY + 10}, fontSize, spacing, color);

        // Move to the next line's Y position
        startY += fontSize;
    }
}

// Checks for any key press
bool IsAnyKeyPressed(void) {
    for (int key = 32; key < 350; key++) { 
        if (IsKeyPressed(key)) return true;        
    }
    return false;
}

// Generates a random number ensuring it doesn't repeat within the last 'historySize' numbers
int GetUniqueRandomValue(int min, int max, std::vector<int>& history, size_t historySize) {
    while (true) {
        int newRandom = GetRandomValue(min, max);

        // Check if the number exists in the history
        bool isUnique = true;
        for (int num : history) {
            if (num == newRandom) {
                isUnique = false;
                break;
            }
        }

        // If unique, update history and return the value
        if (isUnique) {
            if (history.size() >= historySize) {
                history.erase(history.begin()); // Remove the oldest number
            }
            history.push_back(newRandom); // Add the new number to the history
            return newRandom;
        }
    }
}

std::vector<int> GetTwoWrongAnswersIndices(int correctAnswerIndex) {
    std::vector<int> wrongAnswers;

    while (wrongAnswers.size() < 2) {
        int randomIndex = GetRandomValue(0, 3);

        // Ensure the index is not the correct answer and is not already in the wrongAnswers vector
        bool isDuplicate = false;
        for (int answer : wrongAnswers) {
            if (answer == randomIndex) {
                isDuplicate = true;
                break;
            }
        }
        if (randomIndex != correctAnswerIndex && !isDuplicate) {
            wrongAnswers.push_back(randomIndex);
        }
    }

    return wrongAnswers;
}

int GetOneWrongAnswerIndex(int correctAnswerIndex) {
    int randomIndex;

    while (true) {
        randomIndex = GetRandomValue(0, 3);
        if (randomIndex != correctAnswerIndex) {
            return randomIndex;
            break;
        }
    }
}

// Save highscore to a binary file
void SaveHighScore(const char* filename, int value) {
    std::ofstream outFile(filename, std::ios::binary);
    if (outFile.is_open()) {
        outFile.write(reinterpret_cast<const char*>(&value), sizeof(value));
        outFile.close();
    }
}

// Load highscore from a binary file. Will create a new .bin file if not existing
int LoadHighScore(const char* filename) {
    int value = 0;
    std::ifstream inFile(filename, std::ios::binary);
    if (inFile.is_open()) {
        inFile.read(reinterpret_cast<char*>(&value), sizeof(value));
        inFile.close();
    }
    return value;
}


//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void) 
{
    // Initialization
    //--------------------------------------------------------------------------------------
    GameScreen currentScreen = MAIN_MENU;
    GameScreen previousScreen = MAIN_MENU;
    
    int screenWidth = 1920;
    int screenHeight = 1080;
    InitWindow(screenWidth, screenHeight, "BRAIN BLOOM");
    InitAudioDevice();

    // Game launches at fullscreen, can be changed in the games' settings, uncomment out when game is finished
    //ToggleFullscreen();                   
    SetExitKey(KEY_NULL);              
    SetTargetFPS(60);

    //For Multiplayer name input
    std::string player1Name = "";
    std::string player2Name = "";
    bool enteringPlayer1Name = false;
    bool enteringPlayer2Name = false;
    bool namesEntered = false;
    
    bool exitConfirmed = false;

    bool singlePLayerSelected = false; 

    bool isAnswerCorrect = false;
    bool enableInput = true;

    bool abilityA_Used = false;
    bool abilityS_Used = false;
    bool skipQuestion = false;
    bool abilityD_Used = false;
    bool addHealthPoint = false;
    bool abilityF_Used = false;

    bool isAnswerQ_Correct = false;
    bool isAnswerW_Correct = false;
    bool isAnswerE_Correct = false;
    bool isAnswerR_Correct = false;

    bool isAnswerQ_Wrong = false;
    bool isAnswerW_Wrong = false;
    bool isAnswerE_Wrong = false;
    bool isAnswerR_Wrong = false;
    bool answerSelected = false;

    bool exitFromGameover = false;

    bool muteMusic = false;
    bool muteUi = false;

    // Prevents mouse input when currentScreen transitions to RULES
    float inputCooldown = 0.2f;    // Cooldown time in seconds
    float timer = 0.0f;           // Reset to 0.0f ater each use

    std::vector<Question> questions = GetQuestionsVector();
    std::vector<int> history;    // To store last 'historySize' generated numbers
    size_t historySize = 10;    

    int countdownTime = 21;
    int seconds = 0;
    int currentQuestionIndex = GetUniqueRandomValue(0, questions.size()-1, history, historySize);
    int selectedAnswerIndex = -1;
    int score = 0;
    int highscore = LoadHighScore(DATA_FILE_PATH);
    int healthPoints = 10;
    int wrongAnswerIndex;

    std::vector<int> wrongAnswersIndices = {-1, -1};  // 2 wrong answers' indices, reset this variabe everytime after its value gets changed

    double startTime = GetTime();

    auto ResetGameVariables = [&]() {

        countdownTime = (currentScreen == MAIN_MENU || currentScreen == GAMEOVER) ? 21 : 20;    
        currentQuestionIndex = GetUniqueRandomValue(0, questions.size()-1, history, historySize);
        isAnswerCorrect = false; 
        timer = 0.0f;

        wrongAnswersIndices = {-1, -1};
        wrongAnswerIndex = -1;

        isAnswerQ_Correct = false;
        isAnswerW_Correct = false;
        isAnswerE_Correct = false;
        isAnswerR_Correct = false;

        isAnswerQ_Wrong = false;
        isAnswerW_Wrong = false;
        isAnswerE_Wrong = false;
        isAnswerR_Wrong = false;
        
        enableInput = true;
        answerSelected = false;

        if (currentScreen == MAIN_MENU || currentScreen == GAMEOVER) {
            score = 0;
            healthPoints = 10;

            abilityA_Used = false;
            abilityS_Used = false;
            skipQuestion = false;
            abilityD_Used = false;
            addHealthPoint = false;
            abilityF_Used = false;
        }
    };


    Font arcadeFont = LoadFont("assets/fonts/arcade.ttf");

    Music mainMenuMusic = LoadMusicStream("assets/sounds/Flim.mp3");
    Music singleplayerMusic = LoadMusicStream("assets/sounds/singleplayer-music.mp3");
    Music singleplayerLowHealthMusic = LoadMusicStream("assets/sounds/low-health.mp3");

    Sound menuButtonsSound = LoadSound("assets/sounds/button_click.mp3");
    Sound correctAnswerSound = LoadSound("assets/sounds/correct_answer.mp3");
    Sound wrongAnswerSound = LoadSound("assets/sounds/wrong_answer.mp3");
    Sound gameoverSound = LoadSound("assets/sounds/gameover.mp3");
    Sound timesUpSound = LoadSound("assets/sounds/no-time-left.mp3");
    SetSoundVolume(timesUpSound, 0.5f);
    Sound countdownSound = LoadSound("assets/sounds/3s-countdown.mp3");
    SetSoundVolume(countdownSound, 0.3f);

    auto SetMute = [&](bool muteMusic, bool muteUi) {
        float musicVolume = muteMusic ? 0.0f : 1.0f;
        float uiVolume = muteUi ? 0.0f : 1.0f;

        SetMusicVolume(singleplayerMusic, musicVolume);
        SetMusicVolume(singleplayerLowHealthMusic, musicVolume);
        SetMusicVolume(mainMenuMusic, musicVolume);

        SetSoundVolume(menuButtonsSound, uiVolume);
        SetSoundVolume(wrongAnswerSound, uiVolume);
        SetSoundVolume(correctAnswerSound, uiVolume);
        SetSoundVolume(gameoverSound, uiVolume);
        SetSoundVolume(timesUpSound, muteUi ? 0.0f : 0.5f);
        SetSoundVolume(countdownSound, muteUi ? 0.0f : 0.3f);
    };

    // Main Menu Textures
    Texture2D titleLogo = LoadTexture("assets/title-logo.png");
    Texture2D menuBackground = LoadTexture("assets/main-menu-bg.png");
    Texture2D settingsBackground = LoadTexture("assets/settings-bg.png");
    Texture2D rulesScreen = LoadTexture("assets/rules-screen.png");
    Texture2D rulesScreen1 = LoadTexture("assets/rules-screen1.png");
    Texture2D pausedTxt = LoadTexture("assets/game-paused-txt.png");
    Texture2D fiveHearts = LoadTexture("assets/five-hearts.png");
    Texture2D startGameBackground = LoadTexture("assets/start-game-bg.png");
    Texture2D exitBackground = LoadTexture("assets/exit-bg.png");
    Texture2D readyScreen = LoadTexture("assets/ready-screen.png");
    Texture2D soundIcon = LoadTexture("assets/sound-ic.png");

    // Singleplayer Textures
    Texture2D singleplayerBackground = LoadTexture("assets/singleplayer-bg.png");
    Texture2D questionBox = LoadTexture("assets/question-box.png");
    Texture2D health_1 = LoadTexture("assets/health/health_1.png");
    Texture2D health_2 = LoadTexture("assets/health/health_2.png");
    Texture2D health_3 = LoadTexture("assets/health/health_3.png");
    Texture2D health_4 = LoadTexture("assets/health/health_4.png");
    Texture2D health_5 = LoadTexture("assets/health/health_5.png");
    Texture2D health_6 = LoadTexture("assets/health/health_6.png");
    Texture2D health_7 = LoadTexture("assets/health/health_7.png");
    Texture2D health_8 = LoadTexture("assets/health/health_8.png");
    Texture2D health_9 = LoadTexture("assets/health/health_9.png");
    Texture2D health_10 = LoadTexture("assets/health/health_10.png");
    Texture2D health_11 = LoadTexture("assets/health/health_11.png");
    Texture2D abilityA_Used_Texture = LoadTexture("assets/ability-a-used.png");
    Texture2D abilityS_Used_Texture = LoadTexture("assets/ability-s-used.png");
    Texture2D abilityD_Used_Texture = LoadTexture("assets/ability-d-used.png");
    Texture2D abilityF_Used_Texture = LoadTexture("assets/ability-f-used.png");

    // Multiplayer Textures
    Texture2D multiplayerBackground = LoadTexture("assets/multiplayer-bg.png");
    Texture2D controlScreen2 = LoadTexture("assets/controlScreen2.png");
    Texture2D enterPlayerName = LoadTexture("assets/EnterPlayerName-screen.png");

    Texture2D gameoverBackground = LoadTexture("assets/gameover-bg.png");

    // Main Menu Buttons
    Button onePlayerBtn{"assets/one-player-btn.png", {0.0f, 500.0f}, 0.5f}; 
    Button twoPlayerBtn{"assets/two-players-btn.png", {0.0f, 650.0f}, 0.5f};
    Button yesBtn{"assets/exit-yes-btn.png", {0.0f, 600.0f}, 0.8f};
    Button noBtn{"assets/exit-no-btn.png", {0.0f, 700.0f}, 0.8f};
    Button settingsBtn{"assets/settings-btn.png", {0.0f, 620.0f}, 0.6f};
    Button startBtn{"assets/start-btn.png", {0.0f, 500.0f}, 0.6f};
    Button exitBtn{"assets/exit-btn.png", {0.0f, 730.0f}, 0.6f};

    //Settings Buttons
    Button muteUiFalse{"assets/mute-ui-false.png", {0, 300}, 1.0f};
    Button muteUiTrue{"assets/mute-ui-true.png", {0, 300}, 1.0f};
    Button muteMusicFalse{"assets/mute-music-false.png", {0, 450}, 1.0f};
    Button muteMusicTrue{"assets/mute-music-true.png", {0, 450}, 1.0f};

    // Pause & Gameover Buttons 
    Button pauseBtn{"assets/pause-btn.png", {10.0f, 10.0f}, 0.7f};
    Button resumeBtn{"assets/resume-btn.png", {0.0f, 400.0f}, 0.7f};
    Button restartBtn{"assets/restart-btn.png", {0, 500.0f}, 0.73f};
    Button mainMenuBtn{"assets/main-menu-btn.png", {0, 620.0f}, 0.6f};

    // Singlepayer Buttons
    Button answerQ_Btn{"assets/answer-q.png", {150.0f, (float) (GetScreenHeight() - 350.0f)}, 1.3f};
    Button answerW_Btn{"assets/answer-w.png", {(float) (GetScreenWidth() - 900.0f), (float) (GetScreenHeight() - 350.0f)}, 1.3f}; 
    Button answerE_Btn{"assets/answer-e.png", {150.0f, (float) (GetScreenHeight() - 200.0f)}, 1.3f};
    Button answerR_Btn{"assets/answer-r.png", {(float) (GetScreenWidth() - 900.0f), (float) (GetScreenHeight() - 200.0f)}, 1.3f};

    Button abilityA_Btn{"assets/ability-a.png", {(float) (GetScreenWidth() - 430.0f), 260.0f}, 0.5f};
    Button abilityS_Btn{"assets/ability-s.png", {(float) (GetScreenWidth() - 280.0f), 260.0f}, 0.5f};
    Button abilityD_Btn{"assets/ability-d.png", {(float) (GetScreenWidth() - 430.0f), 420.0f}, 0.53f}; 
    Button abilityF_Btn{"assets/ability-f.png", {(float) (GetScreenWidth() - 280.0f), 420.9f}, 0.538f};

    // Multiplayer Buttons
    Button answerQUBtn{"assets/answer-q-u.png", {150, (float) (GetScreenHeight() - 370)}, 0.85};
    Button answerWIBtn{"assets/answer-w-i.png", {(float) (GetScreenWidth() - 900), (float) (GetScreenHeight() - 373)}, 0.85};
    Button answerEOBtn{"assets/answer-e-o.png", {150, (float) (GetScreenHeight() - 200)}, 0.85};
    Button answerRPBtn{"assets/answer-r-p.png", {(float) (GetScreenWidth() - 900), (float) (GetScreenHeight() - 203)}, 0.85};
    Button playerNameBoxBtn{"assets/playerNameBox-btn.png", {750.0f, 415.0f}, 0.85};
    Button playerNameBox1Btn{"assets/playerNameBox-btn.png", {750.0f, 580.0f}, 0.85};

    Color pauseDark = {0,0,0, 100};

    //--------------------------------------------------------------------------------------
    // Main game loop
    while (!WindowShouldClose() && !exitConfirmed)
    {  
        float deltaTime = GetFrameTime();

        Vector2 mousePosition = GetMousePosition();
        bool mouseClicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT); 

        switch(currentScreen) {
            case MAIN_MENU:

                if (!IsMusicStreamPlaying(mainMenuMusic)) PlayMusicStream(mainMenuMusic); // Play main menu music
                if (IsMusicStreamPlaying(singleplayerMusic)) StopMusicStream(singleplayerMusic); // Stop singleplayer music
                UpdateMusicStream(mainMenuMusic);  // Update music stream to continue playing it
                
                // Reset variables
                ResetGameVariables();
                
                player1Name = "";
                player2Name = "";
                enteringPlayer1Name = false;
                enteringPlayer2Name = false;
                namesEntered = false;

                exitBtn.imgScale = 0.6f;
                exitBtn.position.y = 730.0f;

                mainMenuBtn.imgScale = 0.6f;
                mainMenuBtn.position.y = 620.0f;

                if (startBtn.isClicked(mousePosition, mouseClicked)) {
                    currentScreen = STARTGAME;
                    PlaySound(menuButtonsSound);
                }
                if (settingsBtn.isClicked(mousePosition, mouseClicked)) {
                    currentScreen = SETTINGS;
                    PlaySound(menuButtonsSound);
                }
                if (exitBtn.isClicked(mousePosition, mouseClicked)) {
                    currentScreen = EXIT;
                    PlaySound(menuButtonsSound);
                }   
            break;
            case STARTGAME:
                UpdateMusicStream(mainMenuMusic);  // Update music stream to continue playing it
                if (onePlayerBtn.isClicked(mousePosition, mouseClicked)) {
                    currentScreen = RULES;
                    singlePLayerSelected = true;
                    PlaySound(menuButtonsSound);
                }
                if (twoPlayerBtn.isClicked(mousePosition, mouseClicked)) {
                    currentScreen = RULES1;
                    singlePLayerSelected = false;
                    PlaySound(menuButtonsSound);
                }
                break;
            case SETTINGS:
                UpdateMusicStream(mainMenuMusic);  // Update music stream to continue playing it
                if (mainMenuBtn.isClicked(mousePosition, mouseClicked) || IsKeyPressed(KEY_ESCAPE)) {
                    currentScreen = MAIN_MENU;
                    PlaySound(menuButtonsSound);
                }
                if (muteMusicFalse.isClicked(mousePosition, mouseClicked)) {
                    muteMusic = true;
                    SetMute(muteMusic, muteUi);
                    PlaySound(menuButtonsSound);
                }
                if (muteUiFalse.isClicked(mousePosition, mouseClicked)) {
                    muteUi = true;
                    SetMute(muteMusic, muteUi);
                    PlaySound(menuButtonsSound);
                }
                if (muteMusicTrue.isClicked(mousePosition, mouseClicked)) {
                    muteMusic = false;
                    SetMute(muteMusic, muteUi);
                    PlaySound(menuButtonsSound);
                }
                if (muteUiTrue.isClicked(mousePosition, mouseClicked)) {
                    muteUi = false;
                    SetMute(muteMusic, muteUi);
                    PlaySound(menuButtonsSound);
                }
                break;
            case RULES:
                UpdateMusicStream(mainMenuMusic);  // Update music stream to continue playing it
                timer += deltaTime;
                countdownTime = 4;
                if (timer > inputCooldown) {
                    if (IsKeyPressed(KEY_ESCAPE)) currentScreen = MAIN_MENU;
                    else if (IsAnyKeyPressed() || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)|| IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
                        timer = 0;
                        currentScreen = READY;
                    } 
                }
                break;
            case RULES1:
                timer += deltaTime;
                countdownTime = 4;
                if (timer > inputCooldown) {
                    if (IsKeyPressed(KEY_ESCAPE)) currentScreen = MAIN_MENU;
                    else if (IsAnyKeyPressed() || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)|| IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
                        timer = 0;
                        currentScreen = CONTROLS1;
                    } 
                }
                break;
            case CONTROLS1:
                UpdateMusicStream(mainMenuMusic);  // Update music stream to continue playing it
                timer += deltaTime;
                countdownTime = 4;
                if (timer > inputCooldown) {
                    if (IsKeyPressed(KEY_ESCAPE)) currentScreen = MAIN_MENU;
                    else if (IsAnyKeyPressed() || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)|| IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
                        timer = 0;
                        currentScreen = PLAYERNAME;
                    } 
                }
                break;
            case PLAYERNAME:
                countdownTime = 4;
                if (!namesEntered) {
                    if (playerNameBoxBtn.isClicked(mousePosition, mouseClicked)) {
                        enteringPlayer1Name = true;
                        enteringPlayer2Name = false;
                    }
                    if (playerNameBox1Btn.isClicked(mousePosition, mouseClicked)) {
                        enteringPlayer1Name = false;
                        enteringPlayer2Name = true;
                    }
                    int key = GetCharPressed();
                    while (key > 0) {
                        if (enteringPlayer1Name && player1Name.size() < 11) {
                            player1Name += static_cast<char>(key);
                        }
                        if (enteringPlayer2Name && player2Name.size() < 11) {
                            player2Name += static_cast<char>(key);
                        }
                        key = GetCharPressed();
                    }
                    if (IsKeyPressed(KEY_BACKSPACE)) {
                        if (enteringPlayer1Name && !player1Name.empty()) {
                            player1Name.pop_back();
                        }
                        if (enteringPlayer2Name && !player2Name.empty()) {
                            player2Name.pop_back();
                        }
                    }
                    if (IsKeyPressed(KEY_ENTER) && !player1Name.empty() && !player2Name.empty()) {
                        namesEntered = true;
                        currentScreen = READY;
                        singlePLayerSelected = false;
                    }
                }
                break;
            case READY:

                if (IsMusicStreamPlaying(mainMenuMusic)) {
                    StopMusicStream(mainMenuMusic);
                    PlaySound(countdownSound);
                } 

                if (GetTime() - startTime >= 1.0) {
                    countdownTime--;
                    startTime = GetTime();
                }

                if (countdownTime < 0) countdownTime = 0;
        
                seconds = countdownTime % 60;

                timer += deltaTime;

                if (timer >= 3.0f) {
                    currentScreen = (singlePLayerSelected) ? SINGLEPLAYER : MULTIPLAYER;
                    countdownTime = 20;
                    timer = 0.f;
                }
                break;
            case SINGLEPLAYER:
                if (!IsMusicStreamPlaying(singleplayerMusic)) {
                    PlayMusicStream(singleplayerMusic);
                }
                 
                UpdateMusicStream(singleplayerMusic);  // Update music stream to continue playing it

                if (GetTime() - startTime >= 1.0) {
                    countdownTime--;
                    startTime = GetTime();
                }

                if (countdownTime < 0) countdownTime = 0;
        
                seconds = countdownTime % 60;

                // Answers
                if ((enableInput && answerQ_Btn.isClicked(mousePosition, mouseClicked)) || (enableInput && IsKeyPressed(KEY_Q))){
                    selectedAnswerIndex = 0;
                    answerSelected = true;
                    if (selectedAnswerIndex == questions[currentQuestionIndex].correctAnswerIndex) isAnswerQ_Correct = true;
                    if (selectedAnswerIndex != questions[currentQuestionIndex].correctAnswerIndex) isAnswerQ_Wrong = true;
                } 
                else if ((enableInput && answerW_Btn.isClicked(mousePosition, mouseClicked)) || (enableInput && IsKeyPressed(KEY_W))) {
                    selectedAnswerIndex = 1;
                    answerSelected = true;
                    if (selectedAnswerIndex == questions[currentQuestionIndex].correctAnswerIndex) isAnswerW_Correct = true;
                    if (selectedAnswerIndex != questions[currentQuestionIndex].correctAnswerIndex) isAnswerW_Wrong = true;
                }
                else if ((enableInput && answerE_Btn.isClicked(mousePosition, mouseClicked)) || (enableInput && IsKeyPressed(KEY_E))) {
                    selectedAnswerIndex = 2;
                    answerSelected = true;
                    if (selectedAnswerIndex == questions[currentQuestionIndex].correctAnswerIndex) isAnswerE_Correct = true;
                    if (selectedAnswerIndex != questions[currentQuestionIndex].correctAnswerIndex) isAnswerE_Wrong = true;
                }
                else if ((enableInput && answerR_Btn.isClicked(mousePosition, mouseClicked)) || (enableInput && IsKeyPressed(KEY_R))) {
                    selectedAnswerIndex = 3;
                    answerSelected = true;
                    if (selectedAnswerIndex == questions[currentQuestionIndex].correctAnswerIndex) isAnswerR_Correct = true;
                    if (selectedAnswerIndex != questions[currentQuestionIndex].correctAnswerIndex) isAnswerR_Wrong = true;
                }
                
                if (selectedAnswerIndex != -1) {
                    if (selectedAnswerIndex == questions[currentQuestionIndex].correctAnswerIndex) {
                        isAnswerCorrect = true;
                        score++;
                        selectedAnswerIndex = -1;
                        PlaySound(correctAnswerSound);
                    } else {
                        isAnswerCorrect = false;
                        healthPoints--;
                        selectedAnswerIndex = -1;
                        PlaySound(wrongAnswerSound);
                    }
                }

                if (seconds == 1) PlaySound(timesUpSound);

                if (seconds == 0) {    // If time runs out:
                    timer += deltaTime;
                    selectedAnswerIndex = -1;
                    enableInput = false;
                    addHealthPoint = false;

                    // Gives time to draw and show "Times Up!" text, dissapears after 1.5 seconds and draws the timer again
                    if (timer > 1.5f) {
                        healthPoints--;
                        ResetGameVariables();
                    }
                }

                else if (isAnswerCorrect || skipQuestion) {    // If answer is correct or skib ability used, resets variables
                    //Reset timer, incase player corectly answers in the last second, since there is a 1.5s delay to reset variables
                    countdownTime = 20;  
                    timer += deltaTime;
                    enableInput = false;
                    
                    // Gives time to draw and show "Correct!" text, dissapears after 1.5 seconds and draws the timer again
                    if (timer > 1.5f) {
                        if (addHealthPoint) healthPoints++;
                        addHealthPoint = false;
    
                        if (skipQuestion) {
                            score++;
                            abilityS_Used = true;                       
                        }

                        skipQuestion = false;

                        ResetGameVariables();
                    }
                } else if (!isAnswerCorrect && answerSelected) { // If answer is incorrect and player has selected an answer, resets variables
                    //Reset timer, incase player corectly answers in the last second, since there is a 1.5s delay to reset variables
                    countdownTime = 20;  
                    timer += deltaTime;
                    enableInput = false;
                    
                    // Gives time to draw  "Incorrect!" text, dissapears after 1.5 seconds and draws the timer again
                    if (timer > 1.5f) {
                        addHealthPoint = false;
                        ResetGameVariables();
                    }
                }

                if (healthPoints == 1) {
                    StopMusicStream(singleplayerMusic);
                    PlayMusicStream(singleplayerLowHealthMusic);
                    UpdateMusicStream(singleplayerLowHealthMusic);
                }

                if (healthPoints <= 0) currentScreen = GAMEOVER;

                // Pause
                if (pauseBtn.isClicked(mousePosition, mouseClicked) || IsKeyPressed(KEY_ESCAPE)) {
                    PlaySound(menuButtonsSound);
                    previousScreen = SINGLEPLAYER;
                    currentScreen = PAUSE;
                }

                // Remove 2 wrong answers
                if ((enableInput && !abilityA_Used && abilityA_Btn.isClicked(mousePosition, mouseClicked)) || (enableInput && !abilityA_Used && IsKeyPressed(KEY_A))) {
                    wrongAnswersIndices = GetTwoWrongAnswersIndices(questions[currentQuestionIndex].correctAnswerIndex);
                    abilityA_Used = true;
                }
                // Skip question
                if ((enableInput && !abilityS_Used && abilityS_Btn.isClicked(mousePosition, mouseClicked)) || (enableInput && !abilityS_Used && IsKeyPressed(KEY_S))) {
                    skipQuestion = true; 
                    abilityS_Used = true;
                }
                // Gain 1 health point if question is answered correctly
                if ((enableInput && !abilityD_Used && abilityD_Btn.isClicked(mousePosition, mouseClicked)) || (enableInput && !abilityD_Used && IsKeyPressed(KEY_D))) {
                    addHealthPoint = true;
                    abilityD_Used = true;
                }
                // Remove 1 wrong option
                if ((enableInput && !abilityF_Used && abilityF_Btn.isClicked(mousePosition, mouseClicked)) || (enableInput && !abilityF_Used && IsKeyPressed(KEY_F))) {
                    wrongAnswerIndex = GetOneWrongAnswerIndex(questions[currentQuestionIndex].correctAnswerIndex);
                    abilityF_Used = true;
                }
                break;
            case MULTIPLAYER:
                if (IsMusicStreamPlaying(mainMenuMusic)) StopMusicStream(mainMenuMusic);

                if (pauseBtn.isClicked(mousePosition, mouseClicked) || IsKeyPressed(KEY_ESCAPE)) {
                    PlaySound(menuButtonsSound);
                    currentScreen = PAUSE;
                    previousScreen = MULTIPLAYER;
                }
            break;
            case PAUSE:
                if (mainMenuBtn.isClicked(mousePosition, mouseClicked)) {
                    currentScreen = MAIN_MENU;
                    PlaySound(menuButtonsSound);
                }
                if (resumeBtn.isClicked(mousePosition, mouseClicked)) {
                    currentScreen = previousScreen; 
                    PlaySound(menuButtonsSound);
                }
                else if (IsKeyPressed(KEY_ESCAPE)) {
                    currentScreen = previousScreen; 
                }
                break;
            case EXIT:
                if (!exitFromGameover) UpdateMusicStream(mainMenuMusic);
                if (yesBtn.isClicked(mousePosition, mouseClicked)) {
                    PlaySound(menuButtonsSound);
                    exitConfirmed = true;
                }
                if (noBtn.isClicked(mousePosition, mouseClicked)) {
                    currentScreen = (exitFromGameover) ? GAMEOVER:MAIN_MENU;  // Returns to GAMEOVER screen when player clicks no, if in the GAMEOVER screen
                    exitFromGameover = false;
                    PlaySound(menuButtonsSound);
                }
                break;
            case GAMEOVER:
                if (IsMusicStreamPlaying(singleplayerMusic) || IsMusicStreamPlaying(singleplayerLowHealthMusic)) {
                    StopMusicStream(singleplayerMusic); // Stop singleplayer music
                    StopMusicStream(singleplayerLowHealthMusic); // Stop singleplayer music
                    PlaySound(gameoverSound);
                }

                if (!IsMusicStreamPlaying(mainMenuMusic)) PlayMusicStream(mainMenuMusic);

                if (score > highscore) {
                    highscore = score;
                    SaveHighScore(DATA_FILE_PATH, highscore);
                }
                if (mainMenuBtn.isClicked(mousePosition, mouseClicked)) {
                    PlaySound(menuButtonsSound);
                    currentScreen = MAIN_MENU;
                }
                if (exitBtn.isClicked(mousePosition, mouseClicked)) {
                    PlaySound(menuButtonsSound);
                    currentScreen = EXIT;
                    exitFromGameover = true;
                }
                if (restartBtn.isClicked(mousePosition, mouseClicked)) {    // Reset variables and return to RULES GameScreen
                    ResetGameVariables();
                    currentScreen = RULES;   
                    PlaySound(menuButtonsSound);
                }
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
            DrawTextureEx(titleLogo, {(float)(GetScreenWidth() - titleLogo.width * 1.1) / 2, 260}, 0, 1.1, WHITE);
            DrawTextureEx(fiveHearts, (Vector2){755, 70}, 0.0f, 0.3, WHITE);
            startBtn.DrawButtonHorizontal();
            settingsBtn.DrawButtonHorizontal();
            exitBtn.DrawButtonHorizontal();        
            break;
        case STARTGAME:
            DrawTexture(startGameBackground, 0,0, WHITE);
            onePlayerBtn.DrawButtonHorizontal();
            twoPlayerBtn.DrawButtonHorizontal();
            break;
        case SINGLEPLAYER:
            DrawTexture(singleplayerBackground, 0, 0, WHITE);
            DrawTextureEx(questionBox, {(float)(GetScreenWidth() - questionBox.width * 1.8) / 2.0f, 200}, 0, 1.8, WHITE);
            
            DrawQuestionText(arcadeFont, questions[currentQuestionIndex].questionText.c_str(), 800, GetScreenWidth(), GetScreenHeight(), 30, BLACK, false);

            answerQ_Btn.DrawButton();
            answerW_Btn.DrawButton();
            answerE_Btn.DrawButton();
            answerR_Btn.DrawButton();
            
            // Draw Answers/Choices.
            if (isAnswerQ_Correct) DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[0].c_str(), 25.0f, 1.0f, GREEN, answerQ_Btn.position.x, answerQ_Btn.position.y, answerQ_Btn.width, answerQ_Btn.height, 600, false);
            else DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[0].c_str(), 25.0f, 1.0f, (wrongAnswersIndices[0] != 0 && wrongAnswersIndices[1] != 0 && wrongAnswerIndex != 0 && !isAnswerQ_Wrong) ? BLACK : RED, answerQ_Btn.position.x, answerQ_Btn.position.y, answerQ_Btn.width, answerQ_Btn.height, 600, false); 
            
            if (isAnswerW_Correct) DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[1].c_str(), 25.0f, 1.0f, GREEN, answerW_Btn.position.x, answerW_Btn.position.y, answerW_Btn.width, answerW_Btn.height, 600, false);
            else DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[1].c_str(), 25.0f, 1.0f, (wrongAnswersIndices[0] != 1 && wrongAnswersIndices[1] != 1 && wrongAnswerIndex != 1 && !isAnswerW_Wrong) ? BLACK : RED, answerW_Btn.position.x, answerW_Btn.position.y, answerW_Btn.width, answerW_Btn.height, 600, false);

            if (isAnswerE_Correct) DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[2].c_str(), 25.0f, 1.0f, GREEN, answerE_Btn.position.x, answerE_Btn.position.y, answerE_Btn.width, answerE_Btn.height, 600, false);
            else DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[2].c_str(), 25.0f, 1.0f, (wrongAnswersIndices[0] != 2 && wrongAnswersIndices[1] != 2 && wrongAnswerIndex != 2 && !isAnswerE_Wrong) ? BLACK : RED, answerE_Btn.position.x, answerE_Btn.position.y, answerE_Btn.width, answerE_Btn.height, 600, false);

            if (isAnswerR_Correct) DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[3].c_str(), 25.0f, 1.0f, GREEN, answerR_Btn.position.x, answerR_Btn.position.y, answerR_Btn.width, answerR_Btn.height, 600, false);
            else DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[3].c_str(), 25.0f, 1.0f, (wrongAnswersIndices[0] != 3 && wrongAnswersIndices[1] != 3 && wrongAnswerIndex != 3 && !isAnswerR_Wrong) ? BLACK : RED, answerR_Btn.position.x, answerR_Btn.position.y, answerR_Btn.width, answerR_Btn.height, 600, false);

            // Draw Timer
            if (seconds == 0) {
                DrawTextHorizontal(arcadeFont, "Times Up!", 50.0f, 1.0f, RED, 100.0f);  
            }
            else if (skipQuestion) {
                DrawTextHorizontal(arcadeFont, "Skip!", 50.0f, 1.0f, ORANGE, 100.0f);  
            }
            else if (!isAnswerCorrect && answerSelected) {
                DrawTextHorizontal(arcadeFont, "Wrong!", 50.0f, 1.0f, RED, 100.0f);
            }
            else if (!isAnswerCorrect && seconds != 0) {
                DrawTextHorizontal(arcadeFont, TextFormat("Timer: %i", seconds), 50.0f, 1.0f, BLACK, 100.0f);
            } 
            else {
                DrawTextHorizontal(arcadeFont, "Correct!", 50.0f, 1.0f, LIME, 100.0f);
            }

            if (addHealthPoint) {
                DrawTextHorizontal(arcadeFont, "Answer correctly to gain health!", 20.0f, 1.0f, BLACK, 170.0f);
            }
            
            // Draw Score
            DrawTextEx(arcadeFont, TextFormat("Score: %i", score), {100.0f, 350.0f}, 30.0f, 1.0f, BLACK);

            // Draw Health
            DrawTextEx(arcadeFont, "Health: ", {100.0f, 400.0f}, 30.0f, 1.0f, (healthPoints == 1) ? RED:BLACK);
            if (healthPoints == 11) DrawTextureEx(health_11, {100.0f, 450.0f}, 0.0f, 0.5f, WHITE);
            if (healthPoints == 10) DrawTextureEx(health_10, {100.0f, 450.0f}, 0.0f, 0.15f, WHITE);
            if (healthPoints == 9) DrawTextureEx(health_9, {100.0f, 450.0f}, 0.0f, 0.15f, WHITE);
            if (healthPoints == 8) DrawTextureEx(health_8, {100.0f, 450.0f}, 0.0f, 0.15f, WHITE);
            if (healthPoints == 7) DrawTextureEx(health_7, {100.0f, 450.0f}, 0.0f, 0.15f, WHITE);
            if (healthPoints == 6) DrawTextureEx(health_6, {100.0f, 450.0f}, 0.0f, 0.15f, WHITE);
            if (healthPoints == 5) DrawTextureEx(health_5, {100.0f, 450.0f}, 0.0f, 0.18f, WHITE);
            if (healthPoints == 4) DrawTextureEx(health_4, {100.0f, 450.0f}, 0.0f, 0.15f, WHITE);
            if (healthPoints == 3) DrawTextureEx(health_3, {100.0f, 450.0f}, 0.0f, 0.15f, WHITE);
            if (healthPoints == 2) DrawTextureEx(health_2, {100.0f, 450.0f}, 0.0f, 0.15f, WHITE);
            if (healthPoints == 1) DrawTextureEx(health_1, {100.0f, 450.0f}, 0.0f, 0.15f, WHITE);

            // Draw Abilities
            if (abilityA_Used) DrawTextureEx(abilityA_Used_Texture, {abilityA_Btn.position.x, abilityA_Btn.position.y}, 0, abilityA_Btn.imgScale, WHITE);
            else abilityA_Btn.DrawButton();

            if (abilityS_Used) DrawTextureEx(abilityS_Used_Texture, {abilityS_Btn.position.x, abilityS_Btn.position.y}, 0, abilityS_Btn.imgScale, WHITE);
            else abilityS_Btn.DrawButton();

            if (abilityD_Used) DrawTextureEx(abilityD_Used_Texture, {abilityD_Btn.position.x, abilityD_Btn.position.y}, 0, abilityD_Btn.imgScale, WHITE);
            else abilityD_Btn.DrawButton();

            if (abilityF_Used) DrawTextureEx(abilityF_Used_Texture, {abilityF_Btn.position.x, abilityF_Btn.position.y}, 0, abilityF_Btn.imgScale, WHITE);
            else abilityF_Btn.DrawButton();


            pauseBtn.DrawButton();
            break;
        case MULTIPLAYER:
            DrawTexture(multiplayerBackground, 0,0, WHITE);
            DrawTextureEx(questionBox, {(float)(GetScreenWidth() - questionBox.width * 1.9) / 2.0f, 150}, 0, 1.9, WHITE);
            DrawQuestionText(arcadeFont, questions[currentQuestionIndex].questionText.c_str(), 800, GetScreenWidth(), GetScreenHeight(), 30, BLACK, true);
            // Player 1 name outline effect
            for (int x = -2; x <= 2; x++) {
            for (int y = -2; y <= 2; y++) {
            if (x != 0 || y != 0) {
            DrawTextEx(arcadeFont, "Player 1", (Vector2){100 + (float)x, 240 + (float)y}, 20, 0.50, ORANGE);}}}
            DrawTextEx(arcadeFont, "Player 1", (Vector2){100, 240}, 20, 0.50, BLACK);
            // Player 2 name outline effect
            for (int x = -2; x <= 2; x++) {
            for (int y = -2; y <= 2; y++) {if (x != 0 || y != 0) {
            DrawTextEx(arcadeFont, "Player 2", (Vector2){1590 + (float)x, 240 + (float)y}, 20, 0.50, PURPLE);}}}
            DrawTextEx(arcadeFont, "Player 2", (Vector2){1590, 240}, 20, 0.50, BLACK);
            //Player1 input name
            for (int x = -2; x <= 2; x++) {
            for (int y = -2; y <= 2; y++) {
            if (x != 0 || y != 0) {
            DrawTextEx(arcadeFont, player1Name.c_str(), (Vector2){100 + (float)x, 170 + (float)y}, (player1Name.length() > 6) ? 30:50, 1, ORANGE);}}}
            DrawTextEx(arcadeFont, player1Name.c_str(), (Vector2){100, 170}, (player1Name.length() > 6) ? 30:50, 1, BLACK);
            //Player2 input name
            for (int x = -2; x <= 2; x++) {
            for (int y = -2; y <= 2; y++) {
            if (x != 0 || y != 0) {
            DrawTextEx(arcadeFont, player2Name.c_str(), (Vector2){1590 + (float)x, 170 + (float)y}, (player2Name.length() > 6) ? 30:50, 1, PURPLE);}}}
            DrawTextEx(arcadeFont, player2Name.c_str(), (Vector2){1590, 170}, (player2Name.length() > 6) ? 30:50, 1, BLACK);
        
            // Draw Score
            for (int x = -2; x <= 2; x++) {
            for (int y = -2; y <= 2; y++) {
            if (x != 0 || y != 0) {
            DrawTextEx(arcadeFont, "Score: ", (Vector2){100 + (float)x, 350 + (float)y}, 30, 1.0f, ORANGE);}}}
            DrawTextEx(arcadeFont, "Score: ", (Vector2){100, 350}, 30, 1.0f, BLACK);
            for (int x = -2; x <= 2; x++) {
            for (int y = -2; y <= 2; y++) {
            if (x != 0 || y != 0) {
            DrawTextEx(arcadeFont, "Score: ", (Vector2){1590 + (float)x, 350 + (float)y}, 30, 1.0f, PURPLE);}}}
            DrawTextEx(arcadeFont, "Score: ", (Vector2){1590, 350}, 30, 1.0f, BLACK);

            // Draw Health
            for (int x = -2; x <= 2; x++) {
            for (int y = -2; y <= 2; y++) {
            if (x != 0 || y != 0) {
            DrawTextEx(arcadeFont, "Health: ", (Vector2){100 + (float)x, 400 + (float)y}, 30, 1.0f, ORANGE);}}}
            DrawTextEx(arcadeFont, "Health: ", (Vector2){100, 400}, 30, 1.0f, BLACK);
            for (int x = -2; x <= 2; x++) {
            for (int y = -2; y <= 2; y++) {
            if (x != 0 || y != 0) {
            DrawTextEx(arcadeFont, "Health: ", (Vector2){1590 + (float)x, 400 + (float)y}, 30, 1.0f, PURPLE);}}}
            DrawTextEx(arcadeFont, "Health: ", (Vector2){1590, 400}, 30, 1.0f, BLACK);
            //DrawTextEx(arcadeFont, "Health: ", {100.0f, 400.0f}, 30.0f, 1.0f, ORANGE);
            //DrawTextEx(arcadeFont, "Health: ", {1590.0f, 400.0f}, 30.0f, 1.0f, PURPLE);

            answerQUBtn.DrawButton();
            answerWIBtn.DrawButton();
            answerEOBtn.DrawButton();
            answerRPBtn.DrawButton();

            // Draw Answers/Choices.
            if (isAnswerQ_Correct) DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[0].c_str(), 25.0f, 1.0f, GREEN, answerQ_Btn.position.x, answerQ_Btn.position.y, answerQ_Btn.width, answerQ_Btn.height, 600, true);
            else DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[0].c_str(), 25.0f, 1.0f, (wrongAnswersIndices[0] != 0 && wrongAnswersIndices[1] != 0 && wrongAnswerIndex != 0 && !isAnswerQ_Wrong) ? BLACK : RED, answerQ_Btn.position.x, answerQ_Btn.position.y, answerQ_Btn.width, answerQ_Btn.height, 600, true); 
            
            if (isAnswerW_Correct) DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[1].c_str(), 25.0f, 1.0f, GREEN, answerW_Btn.position.x, answerW_Btn.position.y, answerW_Btn.width, answerW_Btn.height, 600, true);
            else DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[1].c_str(), 25.0f, 1.0f, (wrongAnswersIndices[0] != 1 && wrongAnswersIndices[1] != 1 && wrongAnswerIndex != 1 && !isAnswerW_Wrong) ? BLACK : RED, answerW_Btn.position.x, answerW_Btn.position.y, answerW_Btn.width, answerW_Btn.height, 600, true);

            if (isAnswerE_Correct) DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[2].c_str(), 25.0f, 1.0f, GREEN, answerE_Btn.position.x, answerE_Btn.position.y, answerE_Btn.width, answerE_Btn.height, 600, true);
            else DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[2].c_str(), 25.0f, 1.0f, (wrongAnswersIndices[0] != 2 && wrongAnswersIndices[1] != 2 && wrongAnswerIndex != 2 && !isAnswerE_Wrong) ? BLACK : RED, answerE_Btn.position.x, answerE_Btn.position.y, answerE_Btn.width, answerE_Btn.height, 600, true);

            if (isAnswerR_Correct) DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[3].c_str(), 25.0f, 1.0f, GREEN, answerR_Btn.position.x, answerR_Btn.position.y, answerR_Btn.width, answerR_Btn.height, 600, true);
            else DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[3].c_str(), 25.0f, 1.0f, (wrongAnswersIndices[0] != 3 && wrongAnswersIndices[1] != 3 && wrongAnswerIndex != 3 && !isAnswerR_Wrong) ? BLACK : RED, answerR_Btn.position.x, answerR_Btn.position.y, answerR_Btn.width, answerR_Btn.height, 600, true);

            pauseBtn.DrawButton();
            break;
        case SETTINGS:
            DrawTexture(settingsBackground, 0, 0 , WHITE);

            mainMenuBtn.imgScale = 0.9f;
            mainMenuBtn.position.y = 700.0f;
            mainMenuBtn.DrawButtonHorizontal();

            DrawTextHorizontal(arcadeFont, "Main Menu Music: Flim - Aphex Twin", 30.0f, 0.5f, BLACK, 100.0f);
            DrawTextHorizontal(arcadeFont, "Copyright Sounds and Music From: https://www.zapsplat.com", 30.0f, 0.5f, BLACK, 200.0f);

           if (muteUi) {
                muteUiFalse.position.y = 0;
                muteUiTrue.position.y = 300;
                muteUiTrue.DrawButtonHorizontal();
            } 
            else {
                muteUiTrue.position.y = 0;
                muteUiFalse.position.y = 300;
                muteUiFalse.DrawButtonHorizontal();
            }

            if (muteMusic) {
                muteMusicFalse.position.y = 0;
                muteMusicTrue.position.y = 450;
                muteMusicTrue.DrawButtonHorizontal();
            } 
            else {
                muteMusicTrue.position.y = 0;
                muteMusicFalse.position.y = 450;
                muteMusicFalse.DrawButtonHorizontal();
            }

            break;
        case READY:
            DrawTexture(readyScreen, 0, 0, WHITE);
            if (seconds > 0) DrawTextHorizontal(arcadeFont, TextFormat("in %i", seconds), 70.0f, 1.0f, WHITE, GetScreenHeight() - 200.0f);
            else DrawTextHorizontal(arcadeFont, "Go!", 80.0f, 1.0f, GREEN, GetScreenHeight() - 200.0f);
            break;
        case RULES:
            DrawTexture(rulesScreen, 0, 0, WHITE);
            DrawTextHorizontal(arcadeFont, "Press any button to start", 30, 1, WHITE, GetScreenHeight() - 200);
            break;
        case RULES1:
            DrawTexture(rulesScreen1, 0, 0, WHITE);
            DrawTextHorizontal(arcadeFont, "Press any button to proceed", 30, 1, WHITE, GetScreenHeight() - 200);
            break;
        case CONTROLS1:
            DrawTexture(controlScreen2, 0,0,WHITE);
             DrawTextHorizontal(arcadeFont, "Press any button to proceed", 30, 1, WHITE, GetScreenHeight() - 200);
            if (timer > inputCooldown) {
                if (IsAnyKeyPressed() || IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
                    currentScreen = PLAYERNAME;
                }
            }
            break;
        case PLAYERNAME:
            DrawTexture(enterPlayerName, 0, 0, WHITE);
            playerNameBoxBtn.DrawButton();
            playerNameBox1Btn.DrawButton();
                //Player 1 name input
                for (int x = -2; x <= 2; x++) {
                for (int y = -2; y <= 2; y++) {
                if (x != 0 || y != 0) {
                DrawTextEx(arcadeFont, player1Name.c_str(), (Vector2){840 + (float)x, 455 + (float)y}, 30, 1, ORANGE);}}}
                DrawTextEx(arcadeFont, player1Name.c_str(), (Vector2){840, 455}, 30, 1, BLACK);
                if (enteringPlayer1Name){
                        for (int x = -2; x <= 2; x++) {
                        for (int y = -2; y <= 2; y++) {
                        if (x != 0 || y != 0) {
                        DrawTextEx(arcadeFont, "Typing... ", (Vector2){840 + (float)x, 540 + (float)y}, 20, 1.0f, ORANGE);}}}
                        DrawTextEx(arcadeFont, "Typing... ", (Vector2){840, 540}, 20, 1.0f, BLACK);}
                //Player 2 name input        
                for (int x = -2; x <= 2; x++) {
                for (int y = -2; y <= 2; y++) {
                if (x != 0 || y != 0) {
                DrawTextEx(arcadeFont, player2Name.c_str(), (Vector2){840 + (float)x, 620 + (float)y}, 30, 1, PURPLE);}}}
                DrawTextEx(arcadeFont, player2Name.c_str(), (Vector2){840, 620}, 30, 1, BLACK);
                if (enteringPlayer2Name){
                        for (int x = -2; x <= 2; x++) {
                        for (int y = -2; y <= 2; y++) {
                        if (x != 0 || y != 0) {
                        DrawTextEx(arcadeFont, "Typing... ", (Vector2){840 + (float)x, 705 + (float)y}, 20, 1.0f, PURPLE);}}}
                        DrawTextEx(arcadeFont, "Typing... ", (Vector2){840, 705}, 20, 1.0f, BLACK);}

                DrawTextHorizontal(arcadeFont, "Press ENTER to start", 30, 1, WHITE, GetScreenHeight() - 200);
            break;
        case PAUSE:
            DrawTexture(singleplayerBackground, 0, 0,WHITE);
            DrawRectangle(0,0,GetScreenWidth(),GetScreenHeight(), pauseDark);
            DrawTexture(pausedTxt, ((GetScreenWidth() - pausedTxt.width) / 2), 150, WHITE); 
            resumeBtn.DrawButtonHorizontal();
            mainMenuBtn.DrawButtonHorizontal();
            mainMenuBtn.imgScale = 0.84f;
            mainMenuBtn.position.y = 530.0f;
            break;
        case EXIT:
            DrawTexture(exitBackground, 0, 0, WHITE);
            yesBtn.DrawButtonHorizontal();
            noBtn.DrawButtonHorizontal();
            break;
        case GAMEOVER:
            DrawTexture(gameoverBackground, 0, 0, WHITE);

            DrawTextHorizontal(arcadeFont, TextFormat("Score: %i", score), 50.0f, 1.0f, BLACK, 300.0f);
            DrawTextHorizontal(arcadeFont, TextFormat("High Score: %i", highscore), 50.0f, 1.0f, ORANGE, 400.0f);

            restartBtn.DrawButtonHorizontal();

            exitBtn.DrawButtonHorizontal();
            exitBtn.imgScale = 0.53f;
            exitBtn.position.y = 700.0f;

            mainMenuBtn.DrawButtonHorizontal();
            mainMenuBtn.imgScale = 0.80f;
            mainMenuBtn.position.y = 595.0f;

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
    UnloadMusicStream(mainMenuMusic);
    UnloadMusicStream(singleplayerMusic);
    UnloadMusicStream(singleplayerLowHealthMusic);
    UnloadSound(menuButtonsSound);
    UnloadSound(wrongAnswerSound);
    UnloadSound(correctAnswerSound);
    UnloadSound(gameoverSound);
    UnloadSound(timesUpSound);
    UnloadSound(countdownSound);
    UnloadTexture(menuBackground);
    UnloadTexture(titleLogo);
    UnloadTexture(pausedTxt);
    UnloadTexture(rulesScreen);
    UnloadTexture(singleplayerBackground);
    UnloadTexture(settingsBackground);
    UnloadTexture(questionBox);
    UnloadTexture(exitBackground);
    UnloadTexture(readyScreen);
    UnloadTexture(gameoverBackground);
    UnloadTexture(soundIcon);
    UnloadTexture(health_1);
    UnloadTexture(health_2);
    UnloadTexture(health_3);
    UnloadTexture(health_4);
    UnloadTexture(health_5);
    UnloadTexture(health_6);
    UnloadTexture(health_7);
    UnloadTexture(health_8);
    UnloadTexture(health_9);
    UnloadTexture(health_10);
    UnloadTexture(abilityA_Used_Texture);
    UnloadTexture(abilityS_Used_Texture);
    UnloadTexture(abilityD_Used_Texture);
    UnloadTexture(abilityF_Used_Texture);

    CloseAudioDevice();
    CloseWindow();  // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}