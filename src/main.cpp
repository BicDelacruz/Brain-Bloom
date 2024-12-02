/*****************************************************************************
*   TODO:
*   Add multiplayer (screen, mechanics, make user input two names, etc)
*   Add main menu background animation
*   Change timer font
*   Complete settings (UI, options for window size, audio)
*   Implemet highscore (only for singleplayer?) and/or leaderboards (only in multiplayer?)
*   Cosmetics (cursor skins when player reaches a highscore)
*   Audio (for buttons and etc.)
*   Implement loading screen
*   Clean code up and remove redundancy (make variables global)
*****************************************************************************/

#include <vector>
#include <string>
#include "raylib.h"
#include "button.hpp"
#include "questions.hpp"

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
                      int fontSize, Color color) {
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
void DrawAnswerText(Font font, const char *text, float fontSize, float spacing, Color color, 
                    float buttonX, float buttonY, float buttonWidth, float buttonHeight, int maxWidth) {
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
        DrawTextEx(font, line.c_str(), (Vector2){lineX, startY + 10}, fontSize, spacing, color);

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

//For Multiplayer name input
std::string player1Name = "";
std::string player2Name = "";
bool enteringPlayer1Name = false;
bool enteringPlayer2Name = false;
bool namesEntered = false;


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

    // Game launches at fullscreen, can be changed in the games' settings, uncomment out when game is finished
    //ToggleFullscreen();                   
    SetExitKey(KEY_NULL);              
    SetTargetFPS(60);
    
    bool exitConfirmed = false;
    bool isAnswerCorrect = false;
    bool singlePLayerSelected = false; 
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
    int healthPoints = 10;
    int wrongAnswerIndex;

    std::vector<int> wrongAnswersIndices = {-1, -1};  // 2 wrong answers' indices, reset this variabe everytime after its value gets changed

    double startTime = GetTime();

    Font arcadeFont = LoadFont("fonts/arcade.ttf");

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
    Button settingsBtn{"assets/settings-btn.png", {0, 620.0f}, 0.6f};
    Button startBtn{"assets/start-btn.png", {0, 500.0f}, 0.6f};
    Button exitBtn{"assets/exit-btn.png", {0, 730.0f}, 0.6f};



    // Pause & Gameover Buttons 
    Button pauseBtn{"assets/pause-btn.png", {10.0f, 10.0f}, 0.7f};
    Button resumeBtn{"assets/resume-btn.png", {0.0f, 400.0f}, 0.7f};
    Button restartBtn{"assets/restart-btn.png", {0, 500.0f}, 0.6f};
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

        if (GetTime() - startTime >= 1.0) {
            countdownTime--;
            startTime = GetTime();
        }

        if (countdownTime < 0) countdownTime = 0;
        
        seconds = countdownTime % 60;

        Vector2 mousePosition = GetMousePosition();
        bool mouseClicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT); 
        
        switch(currentScreen) {
            case MAIN_MENU:
                
                // Reset variables
                countdownTime = 21;    
                currentQuestionIndex = GetUniqueRandomValue(0, questions.size()-1, history, historySize);
                isAnswerCorrect = false; 
                timer = 0.0f;
                score = 0;
                healthPoints = 10;

                wrongAnswersIndices = {-1, -1};
                wrongAnswerIndex = -1;
                abilityA_Used = false;
                abilityS_Used = false;
                skipQuestion = false;
                abilityD_Used = false;
                addHealthPoint = false;
                abilityF_Used = false;

                isAnswerQ_Correct = false;
                isAnswerW_Correct = false;
                isAnswerE_Correct = false;
                isAnswerR_Correct = false;
                isAnswerQ_Wrong = false;
                isAnswerW_Wrong = false;
                isAnswerE_Wrong = false;
                isAnswerR_Wrong = false;
                
                player1Name = "";
                player2Name = "";
                enteringPlayer1Name = false;
                enteringPlayer2Name = false;
                namesEntered = false;


                exitBtn.imgScale = 0.6f;
                
                if (startBtn.isClicked(mousePosition, mouseClicked)) {
                    currentScreen = STARTGAME;
                }
                if (settingsBtn.isClicked(mousePosition, mouseClicked)) currentScreen = SETTINGS;
                if (exitBtn.isClicked(mousePosition, mouseClicked)) currentScreen = EXIT;   
            break;
            case STARTGAME:
                if (onePlayerBtn.isClicked(mousePosition, mouseClicked)) {
                    currentScreen = RULES;
                    singlePLayerSelected = true;
                }
                if (twoPlayerBtn.isClicked(mousePosition, mouseClicked)) {
                    currentScreen = RULES1;
                    singlePLayerSelected = false;
                }
                break;
            case SETTINGS:
                if (IsKeyPressed(KEY_ESCAPE)) {
                    currentScreen = MAIN_MENU;
                }
                break;
            case RULES:
                timer += deltaTime;
                countdownTime = 3;
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
                countdownTime = 3;
                if (timer > inputCooldown) {
                    if (IsKeyPressed(KEY_ESCAPE)) currentScreen = MAIN_MENU;
                    else if (IsAnyKeyPressed() || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)|| IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
                        timer = 0;
                        currentScreen = CONTROLS1;
                    } 
                }
                break;
            case CONTROLS1:
                timer += deltaTime;
                countdownTime = 3;
                if (timer > inputCooldown) {
                    if (IsKeyPressed(KEY_ESCAPE)) currentScreen = MAIN_MENU;
                    else if (IsAnyKeyPressed() || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)|| IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
                        timer = 0;
                        currentScreen = PLAYERNAME;
                    } 
                }
                break;
            case PLAYERNAME:
                countdownTime = 3;
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
                        if (enteringPlayer1Name && player1Name.size() < 20) {
                            player1Name += static_cast<char>(key);
                        }
                        if (enteringPlayer2Name && player2Name.size() < 20) {
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
                timer += deltaTime;
                if (timer >= 3.0f) {
                    currentScreen = (singlePLayerSelected) ? SINGLEPLAYER : MULTIPLAYER;
                    countdownTime = 20;
                    timer = 0.f;
                }
                break;
            case SINGLEPLAYER:
                
                // Answers
                if ((enableInput && answerQ_Btn.isClicked(mousePosition, mouseClicked)) || (enableInput && IsKeyPressed(KEY_Q))){
                    selectedAnswerIndex = 0;
                    if (selectedAnswerIndex == questions[currentQuestionIndex].correctAnswerIndex) isAnswerQ_Correct = true;
                    if (selectedAnswerIndex != questions[currentQuestionIndex].correctAnswerIndex) isAnswerQ_Wrong = true;
                } 
                else if ((enableInput && answerW_Btn.isClicked(mousePosition, mouseClicked)) || (enableInput && IsKeyPressed(KEY_W))) {
                    selectedAnswerIndex = 1;
                    if (selectedAnswerIndex == questions[currentQuestionIndex].correctAnswerIndex) isAnswerW_Correct = true;
                    if (selectedAnswerIndex != questions[currentQuestionIndex].correctAnswerIndex) isAnswerW_Wrong = true;
                }
                else if ((enableInput && answerE_Btn.isClicked(mousePosition, mouseClicked)) || (enableInput && IsKeyPressed(KEY_E))) {
                    selectedAnswerIndex = 2;
                    if (selectedAnswerIndex == questions[currentQuestionIndex].correctAnswerIndex) isAnswerE_Correct = true;
                    if (selectedAnswerIndex != questions[currentQuestionIndex].correctAnswerIndex) isAnswerE_Wrong = true;
                }
                else if ((enableInput && answerR_Btn.isClicked(mousePosition, mouseClicked)) || (enableInput && IsKeyPressed(KEY_R))) {
                    selectedAnswerIndex = 3;
                    if (selectedAnswerIndex == questions[currentQuestionIndex].correctAnswerIndex) isAnswerR_Correct = true;
                    if (selectedAnswerIndex != questions[currentQuestionIndex].correctAnswerIndex) isAnswerR_Wrong = true;
                }
                
                if (selectedAnswerIndex != -1) {
                    if (selectedAnswerIndex == questions[currentQuestionIndex].correctAnswerIndex) {
                        isAnswerCorrect = true;
                        score++;
                        selectedAnswerIndex = -1;
                    } else {
                        isAnswerCorrect = false;
                        healthPoints--;
                        selectedAnswerIndex = -1;
                    }
                }

                if (seconds == 0) {    // If time runs out:
                    timer += deltaTime;
                    selectedAnswerIndex = -1;
                    enableInput = false;

                    // Gives time to draw and show "Times Up!" text, dissapears after 1.5 seconds and draws the timer again
                    if (timer > 1.5f) {
                        healthPoints--;
                        countdownTime = 20;    
                        currentQuestionIndex = GetUniqueRandomValue(0, questions.size()-1, history, historySize);
                        isAnswerCorrect = false; 
                        timer = 0.0f;
                        wrongAnswersIndices = {-1, -1};

                        isAnswerQ_Correct = false;
                        isAnswerW_Correct = false;
                        isAnswerE_Correct = false;
                        isAnswerR_Correct = false;

                        isAnswerQ_Wrong = false;
                        isAnswerW_Wrong = false;
                        isAnswerE_Wrong = false;
                        isAnswerR_Wrong = false;
                        
                        enableInput = true;
                    }
                }

                else if (isAnswerCorrect || skipQuestion) {    // If answer is correct, resets variables
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

                        countdownTime = 20;    
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
                    }
                }

                if (healthPoints <= 0) currentScreen = GAMEOVER;

                // Pause
                if (pauseBtn.isClicked(mousePosition, mouseClicked) ) {
                    previousScreen = SINGLEPLAYER;
                    currentScreen = PAUSE;
                }
                if (IsKeyPressed(KEY_ESCAPE)) {
                    previousScreen = SINGLEPLAYER;
                    currentScreen = PAUSE;
                }
                // Remove 2 wrong answers
                if ((!abilityA_Used && abilityA_Btn.isClicked(mousePosition, mouseClicked)) || (!abilityA_Used && IsKeyPressed(KEY_A))) {
                    wrongAnswersIndices = GetTwoWrongAnswersIndices(questions[currentQuestionIndex].correctAnswerIndex);
                    abilityA_Used = true;
                }
                // Skip question
                if ((!abilityS_Used && abilityS_Btn.isClicked(mousePosition, mouseClicked)) || (!abilityS_Used && IsKeyPressed(KEY_S))) {
                    skipQuestion = true; 
                    abilityS_Used = true;
                }
                // Gain 1 health point if question is answered correctly
                if ((!abilityD_Used && abilityD_Btn.isClicked(mousePosition, mouseClicked)) || (!abilityD_Used && IsKeyPressed(KEY_D))) {
                    addHealthPoint = true;
                    abilityD_Used = true;
                }
                // Remove 1 wrong option
                if ((!abilityF_Used && abilityF_Btn.isClicked(mousePosition, mouseClicked)) || (!abilityF_Used && IsKeyPressed(KEY_F))) {
                    wrongAnswerIndex = GetOneWrongAnswerIndex(questions[currentQuestionIndex].correctAnswerIndex);
                    abilityF_Used = true;
                }
                break;
            case MULTIPLAYER:
                if (pauseBtn.isClicked(mousePosition, mouseClicked) ) {
                    currentScreen = PAUSE;
                    previousScreen = MULTIPLAYER;
                    
                }
                if (IsKeyPressed(KEY_ESCAPE)) {
                    currentScreen = PAUSE;
                    previousScreen = MULTIPLAYER;
                    
                }
            break;
            case PAUSE:
                if (mainMenuBtn.isClicked(mousePosition, mouseClicked)) {
                    currentScreen = MAIN_MENU;
                }
                if (resumeBtn.isClicked(mousePosition, mouseClicked)) {
                    currentScreen = previousScreen; 
                }
                else if (IsKeyPressed(KEY_ESCAPE)) {
                    currentScreen = previousScreen; 
                }
                break;
            case EXIT:
                if (yesBtn.isClicked(mousePosition, mouseClicked)) exitConfirmed = true;
                if (noBtn.isClicked(mousePosition, mouseClicked)) currentScreen = MAIN_MENU;
                break;
            case GAMEOVER:
                if (mainMenuBtn.isClicked(mousePosition, mouseClicked)) currentScreen = MAIN_MENU;
                if (exitBtn.isClicked(mousePosition, mouseClicked)) currentScreen = EXIT;
                if (restartBtn.isClicked(mousePosition, mouseClicked)) {    // Reset variables and return to RULES GameScreen
                    countdownTime = 21;    
                    currentQuestionIndex = GetUniqueRandomValue(0, questions.size()-1, history, historySize);
                    isAnswerCorrect = false; 
                    timer = 0.0f;
                    score = 0;
                    healthPoints = 10;

                    wrongAnswersIndices = {-1, -1};
                    wrongAnswerIndex = -1;
                    abilityA_Used = false;
                    abilityS_Used = false;
                    skipQuestion = false;
                    abilityD_Used = false;
                    addHealthPoint = false;
                    abilityF_Used = false;

                    isAnswerQ_Correct = false;
                    isAnswerW_Correct = false;
                    isAnswerE_Correct = false;
                    isAnswerR_Correct = false;
                    isAnswerQ_Wrong = false;
                    isAnswerW_Wrong = false;
                    isAnswerE_Wrong = false;
                    isAnswerR_Wrong = false;

                    currentScreen = RULES;   
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
            
            DrawQuestionText(arcadeFont, questions[currentQuestionIndex].questionText.c_str(), 800, GetScreenWidth(), GetScreenHeight(), 30, BLACK);

            answerQ_Btn.DrawButton();
            answerW_Btn.DrawButton();
            answerE_Btn.DrawButton();
            answerR_Btn.DrawButton();
            
            // Draw Answers/Choices.
            if (isAnswerQ_Correct) DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[0].c_str(), 25.0f, 1.0f, GREEN, answerQ_Btn.position.x, answerQ_Btn.position.y, answerQ_Btn.width, answerQ_Btn.height, 600);
            else DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[0].c_str(), 25.0f, 1.0f, (wrongAnswersIndices[0] != 0 && wrongAnswersIndices[1] != 0 && wrongAnswerIndex != 0 && !isAnswerQ_Wrong) ? BLACK : RED, answerQ_Btn.position.x, answerQ_Btn.position.y, answerQ_Btn.width, answerQ_Btn.height, 600); 
            
            if (isAnswerW_Correct) DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[1].c_str(), 25.0f, 1.0f, GREEN, answerW_Btn.position.x, answerW_Btn.position.y, answerW_Btn.width, answerW_Btn.height, 600);
            else DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[1].c_str(), 25.0f, 1.0f, (wrongAnswersIndices[0] != 1 && wrongAnswersIndices[1] != 1 && wrongAnswerIndex != 1 && !isAnswerW_Wrong) ? BLACK : RED, answerW_Btn.position.x, answerW_Btn.position.y, answerW_Btn.width, answerW_Btn.height, 600);

            if (isAnswerE_Correct) DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[2].c_str(), 25.0f, 1.0f, GREEN, answerE_Btn.position.x, answerE_Btn.position.y, answerE_Btn.width, answerE_Btn.height, 600);
            else DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[2].c_str(), 25.0f, 1.0f, (wrongAnswersIndices[0] != 2 && wrongAnswersIndices[1] != 2 && wrongAnswerIndex != 2 && !isAnswerE_Wrong) ? BLACK : RED, answerE_Btn.position.x, answerE_Btn.position.y, answerE_Btn.width, answerE_Btn.height, 600);

            if (isAnswerR_Correct) DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[3].c_str(), 25.0f, 1.0f, GREEN, answerR_Btn.position.x, answerR_Btn.position.y, answerR_Btn.width, answerR_Btn.height, 600);
            else DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[3].c_str(), 25.0f, 1.0f, (wrongAnswersIndices[0] != 3 && wrongAnswersIndices[1] != 3 && wrongAnswerIndex != 3 && !isAnswerR_Wrong) ? BLACK : RED, answerR_Btn.position.x, answerR_Btn.position.y, answerR_Btn.width, answerR_Btn.height, 600);

            // Draw Timer
            if (seconds == 0) {
                DrawTextHorizontal(arcadeFont, "Times Up!", 50.0f, 1.0f, RED, 100.0f);  
            }
            else if (skipQuestion) {
                DrawTextHorizontal(arcadeFont, "Skip!", 50.0f, 1.0f, ORANGE, 100.0f);  
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
            DrawTextEx(arcadeFont, "Health: ", {100.0f, 400.0f}, 30.0f, 1.0f, BLACK);
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
            // Player 1 name outline effect
            for (int x = -2; x <= 2; x++) {
            for (int y = -2; y <= 2; y++) {
            if (x != 0 || y != 0) {
            DrawTextEx(arcadeFont, "Player 1", (Vector2){140 + (float)x, 240 + (float)y}, 20, 0.50, ORANGE);}}}
            DrawTextEx(arcadeFont, "Player 1", (Vector2){140, 240}, 20, 0.50, BLACK);
            // Player 2 name outline effect
            for (int x = -2; x <= 2; x++) {
            for (int y = -2; y <= 2; y++) {if (x != 0 || y != 0) {
            DrawTextEx(arcadeFont, "Player 2", (Vector2){1610 + (float)x, 240 + (float)y}, 20, 0.50, PURPLE);}}}
            DrawTextEx(arcadeFont, "Player 2", (Vector2){1610, 240}, 20, 0.50, BLACK);
             //Player1 input name
            for (int x = -2; x <= 2; x++) {
            for (int y = -2; y <= 2; y++) {
            if (x != 0 || y != 0) {
            DrawTextEx(arcadeFont, player1Name.c_str(), (Vector2){140 + (float)x, 170 + (float)y}, 50, 1, ORANGE);}}}
            DrawTextEx(arcadeFont, player1Name.c_str(), (Vector2){140, 170}, 50, 1, BLACK);
            //Player2 input name
            for (int x = -2; x <= 2; x++) {
            for (int y = -2; y <= 2; y++) {
            if (x != 0 || y != 0) {
            DrawTextEx(arcadeFont, player2Name.c_str(), (Vector2){1610 + (float)x, 170 + (float)y}, 50, 1, PURPLE);}}}
            DrawTextEx(arcadeFont, player2Name.c_str(), (Vector2){1610, 170}, 50, 1, BLACK);
        
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
            pauseBtn.DrawButton();
            break;
        case SETTINGS:
            DrawTexture(settingsBackground, 0, 0 , WHITE);
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
            restartBtn.DrawButtonHorizontal();
            restartBtn.imgScale = 0.90f;
            restartBtn.position.y = 500.0f;
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

    CloseWindow();  // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}