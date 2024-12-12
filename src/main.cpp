/*****************************************************************************
*   
*   https://github.com/BicDelacruz/Brain-Bloom
*
*****************************************************************************/

#include <vector>
#include <string>
#include <fstream>
#include "raylib.h"
#include "button.hpp"
#include "questions.hpp"
#include <ctime>
#include <thread>
#include <chrono>
#include <algorithm>
#include <stdio.h>
#include <string.h>

#define SINGLEPLAYER_DATA_FILE_PATH "data/singleplayer-data.bin" 
#define MULTIPLAYER_DATA_FILE_PATH "data/multiplayer-data.bin" 


// Screen manager, based on an example from the raylib website
typedef enum GameScreen { MAIN_MENU = 0, STARTGAME, SETTINGS, SINGLEPLAYER_RULES, MULTIPLAYER_RULES, SINGLEPLAYER, MULTIPLAYER, READY, PAUSE, SINGLEPLAYER_GAMEOVER, MULTIPLAYER_GAMEOVER, MULTIPLAYER_CONTROLS, PLAYERNAME, LEADERBOARDS, EXIT } GameScreen;

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
        DrawTextEx(font, line.c_str(), {posX, (isMultiplayer) ? posY - 120: posY - 100}, fontSize, 1, color);
        posY += fontSize; // Move to the next line
    }
}

//Re-center Player name input in a fixed x and y coordinates base on its width size and length
void DrawCenteredTextAtX(const char* text, Font font, float x, float y, int minFontSize, int maxFontSize, float maxWidth, Color color) {
    int fontSize = maxFontSize;
    Vector2 textSize = MeasureTextEx(font, text, fontSize, 1);

    while (textSize.x > maxWidth && fontSize > minFontSize) {
        fontSize--;
        textSize = MeasureTextEx(font, text, fontSize, 1);
    }

    DrawTextEx(font, text, {x - textSize.x / 2, y - textSize.y / 2}, fontSize, 1, color);
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
        DrawTextEx(font, line.c_str(), {lineX, (isMultiplayer) ? startY :startY + 10}, fontSize, spacing, color);

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

void DrawTextHighlight(Font font, const char* text, float posX, float posY, 
                        float fontSize, float fontSpacing, Color highlightColor) {
    for (int x = -2; x <= 2; x++) {
        for (int y = -2; y <= 2; y++) {
            if (x != 0 || y != 0) DrawTextEx(font, text, {posX + (float) x, posY + (float)y}, fontSize, fontSpacing, highlightColor);
            }
        }
    DrawTextEx(font, text, {posX, posY}, fontSize, fontSpacing, BLACK);
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

//Stores player names and scores used for leader board
struct LeaderboardEntry {
    std::string playerName;
    int score;

    bool operator>(const LeaderboardEntry& other) const {
        return score > other.score; // Compare scores for sorting
    }
};

// Global leaderboard vector
std::vector<LeaderboardEntry> leaderboard;

void SaveLeaderboard() {
    std::ofstream file(MULTIPLAYER_DATA_FILE_PATH, std::ios::binary);
    if (file.is_open()) {
        // Save the leaderboard size first
        int size = leaderboard.size();
        file.write(reinterpret_cast<char*>(&size), sizeof(size));

        // Save each player's name and score
        for (const auto& entry : leaderboard) {
            int nameLength = entry.playerName.length();
            file.write(reinterpret_cast<char*>(&nameLength), sizeof(nameLength)); // Save name length
            file.write(entry.playerName.c_str(), nameLength); // Save name
            file.write(reinterpret_cast<const char*>(&entry.score), sizeof(entry.score)); // Save score
        }
        file.close();
    }
}

// Load leaderboard from a binary file. Will create a new .bin file if not existing
void LoadLeaderboard() {
    std::ifstream file(MULTIPLAYER_DATA_FILE_PATH, std::ios::binary);
    if (file.is_open()) {
        leaderboard.clear(); // Clear existing leaderboard

        // Read leaderboard size
        int size;
        file.read(reinterpret_cast<char*>(&size), sizeof(size));

        // Read each player's name and score
        for (int i = 0; i < size; i++) {
            int nameLength;
            file.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength)); // Read name length
            char* nameBuffer = new char[nameLength + 1]; // Allocate memory for name
            file.read(nameBuffer, nameLength); // Read name
            nameBuffer[nameLength] = '\0'; // Null-terminate the string

            int score;
            file.read(reinterpret_cast<char*>(&score), sizeof(score)); // Read score

            // Add to leaderboard
            leaderboard.push_back(LeaderboardEntry{nameBuffer, score});

            delete[] nameBuffer; // Clean up memory
        }

        file.close();
    }
}

void UpdateLeaderboard(const std::string& player1Name, int player1Score, const std::string& player2Name, int player2Score) {
    // Add new scores to the leaderboard
    leaderboard.push_back(LeaderboardEntry{player1Name, player1Score});
    leaderboard.push_back(LeaderboardEntry{player2Name, player2Score});

    // Sort leaderboard by score (highest to lowest)
    std::sort(leaderboard.begin(), leaderboard.end(), std::greater<LeaderboardEntry>());

    // Keep only top 10 players
    if (leaderboard.size() > 10) {
        leaderboard.resize(10);
    }

    // Save the updated leaderboard
    SaveLeaderboard();
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
    ToggleFullscreen();                   
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
    int highscore = LoadHighScore(SINGLEPLAYER_DATA_FILE_PATH);
    LoadLeaderboard();
    int healthPoints = 10;
    int wrongAnswerIndex;

    //Multiplayer variables
    int player1Score = 0;
    int player2Score = 0;
    int player1Healthpoints = 10;
    int player2Healthpoints = 10;
    bool player1Selected = false, player2Selected = false;
    bool gameInProgress = false;
    int player1Answer = -1, player2Answer = -1;
    float player1AnswerTime = 0, player2AnswerTime = 0;
    bool correctAnswer = false;
    bool messageDisplayed = false;
    std::string gameMessage;
    float gameOverDelayTimer = 0.0f;
    bool isGameOverTriggered = false;
    std::string gameMessage1;
    std::string gameMessage2;

    // Predefined Y-offsets for leaderboard entries (top 10)
    float yOffsets[] = {470.0f, 510.0f, 550.0f, 590.0f, 630.0f, 670.0f, 710.0f, 750.0f, 790.0f, 830.0f};

    std::vector<int> wrongAnswersIndices = {-1, -1};  // 2 wrong answers' indices, reset this variabe everytime after its value gets changed

    double startTime = GetTime();

    auto ResetGameVariables = [&]() {

        countdownTime = (currentScreen == MAIN_MENU || currentScreen == SINGLEPLAYER_GAMEOVER) ? 21 : 20;    
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
        
        player1Selected = false, player2Selected = false;
        player1Answer = -1, player2Answer = -1;
        player1AnswerTime = 0, player2AnswerTime = 0;
        correctAnswer = false;
        gameMessage = "";
        messageDisplayed = false;
        gameMessage1 = "";
        gameMessage2 = "";

        if (currentScreen == MAIN_MENU || currentScreen == SINGLEPLAYER_GAMEOVER || currentScreen == MULTIPLAYER_GAMEOVER) {
            score = 0;
            healthPoints = 10;

            abilityA_Used = false;
            abilityS_Used = false;
            skipQuestion = false;
            abilityD_Used = false;
            addHealthPoint = false;
            abilityF_Used = false;

            //Multiplayer variables
            player1Name = "";
            player2Name = "";
            enteringPlayer1Name = false;
            enteringPlayer2Name = false;
            namesEntered = false;
            player1Score = 0;
            player2Score = 0;
            player1Healthpoints = 10;
            player2Healthpoints = 10;
            player1Selected = false, player2Selected = false;
            player1Answer = -1, player2Answer = -1;
            player1AnswerTime = 0, player2AnswerTime = 0;
            correctAnswer = false;
            gameMessage = "";
            gameMessage1 = "";
            gameMessage2 = "";
            gameOverDelayTimer = 0.0f;

        }
    };


    Font arcadeFont = LoadFont("assets/fonts/arcade.ttf");

    Music mainMenuMusic = LoadMusicStream("assets/sounds/Flim.mp3");
    Music singleplayerMusic = LoadMusicStream("assets/sounds/singleplayer-music.mp3");
    Music multiplayerMusic = LoadMusicStream("assets/sounds/multiplayer-music.mp3");
    Music singleplayerLowHealthMusic = LoadMusicStream("assets/sounds/low-health.mp3");

    Sound menuButtonsSound = LoadSound("assets/sounds/button_click.mp3");
    Sound correctAnswerSound = LoadSound("assets/sounds/correct_answer.mp3");
    Sound wrongAnswerSound = LoadSound("assets/sounds/wrong_answer.mp3");
    Sound gameoverSound = LoadSound("assets/sounds/gameover.mp3");
    Sound playercorrect = LoadSound("assets/sounds/gotcorrect.mp3");
    Sound bothWrong = LoadSound("assets/sounds/Both-wrong.mp3");
    Sound playerWins = LoadSound("assets/sounds/playerWins.mp3");
    SetSoundVolume(playerWins, 0.5f);
    Sound buttonClicked = LoadSound("assets/sounds/correct_answer.mp3");

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
        SetMusicVolume(multiplayerMusic, musicVolume);

        SetSoundVolume(menuButtonsSound, uiVolume);
        SetSoundVolume(playerWins, uiVolume);
        SetSoundVolume(bothWrong, uiVolume);
        SetSoundVolume(playercorrect, uiVolume);
        SetSoundVolume(buttonClicked, uiVolume);
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
    Texture2D leaderBoardBackground = LoadTexture("assets/Leaderboards-screen.png");

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
    Button leaderboardsBtn{"assets/leaderboards-btn.png", {0, 620.0f}, 0.93f};

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
                    currentScreen = SINGLEPLAYER_RULES;
                    singlePLayerSelected = true;
                    PlaySound(menuButtonsSound);
                }
                if (twoPlayerBtn.isClicked(mousePosition, mouseClicked)) {
                    currentScreen = MULTIPLAYER_RULES;
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
            case SINGLEPLAYER_RULES:
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
            case MULTIPLAYER_RULES:
                UpdateMusicStream(mainMenuMusic);  // Update music stream to continue playing it
                timer += deltaTime;
                countdownTime = 4;
                if (timer > inputCooldown) {
                    if (IsKeyPressed(KEY_ESCAPE)) currentScreen = MAIN_MENU;
                    else if (IsAnyKeyPressed() || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)|| IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
                        timer = 0;
                        currentScreen = MULTIPLAYER_CONTROLS;
                    } 
                }
                break;
            case MULTIPLAYER_CONTROLS:
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
                UpdateMusicStream(mainMenuMusic);  // Update music stream to continue playing it
                countdownTime = 4;
                if (!namesEntered) {
                    if (playerNameBoxBtn.isClicked(mousePosition, mouseClicked)) {
                        PlaySound(menuButtonsSound);
                        enteringPlayer1Name = true;
                        enteringPlayer2Name = false;
                    }
                    if (playerNameBox1Btn.isClicked(mousePosition, mouseClicked)) {
                        PlaySound(menuButtonsSound);
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

                if (GetTime() - startTime >= 1.0) {
                    countdownTime--;
                    startTime = GetTime();
                }

                if (countdownTime < 0) countdownTime = 0;

                if (!IsSoundPlaying(countdownSound)) PlaySound(countdownSound);

                if (IsMusicStreamPlaying(mainMenuMusic)) StopMusicStream(mainMenuMusic);

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

                if (GetTime() - startTime >= 1.0) {
                countdownTime--;
                startTime = GetTime();}

                if (countdownTime < 0) countdownTime = 0;
                
                seconds = countdownTime % 60;

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
                    if (!IsMusicStreamPlaying(singleplayerLowHealthMusic)) {
                    PlayMusicStream(singleplayerLowHealthMusic);
                    }
                    UpdateMusicStream(singleplayerLowHealthMusic);  // Update music stream to continue playing it
                    }

                if (healthPoints <= 0) currentScreen = SINGLEPLAYER_GAMEOVER;

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
                if (GetTime() - startTime >= 1.0) {
                    countdownTime--;
                    startTime = GetTime();}
                if (countdownTime < 0) countdownTime = 0;
                seconds = countdownTime % 60;

                if (!IsMusicStreamPlaying(multiplayerMusic)) {
                    PlayMusicStream(multiplayerMusic);
                }
                 
                UpdateMusicStream(multiplayerMusic);  // Update music stream to continue playing it

                // Start the timer for a new question
                if (!gameInProgress) { // Reset the game when a new question starts
                    timer = 0;  // Reset timer
                    enableInput = true;  // Enable player input
                    gameInProgress = true; // Indicate the game is in progress for the new question
                }

                // Handle Player 1 input (Q, W, E, R keys)
                if (enableInput && !player1Selected) { // Check if input is enabled and Player 1 hasn't selected
                    if (IsKeyPressed(KEY_Q)) {
                        PlaySound(buttonClicked);
                        player1Answer = 0;
                        player1Selected = true; // Lock Player 1's choice
                        player1AnswerTime = (int)(GetTime() * 1000);
                    }
                    else if (IsKeyPressed(KEY_W)) {
                        PlaySound(buttonClicked);
                        player1Answer = 1;
                        player1Selected = true; // Lock Player 1's choice
                        player1AnswerTime = (int)(GetTime() * 1000);
                    }
                    else if (IsKeyPressed(KEY_E)) {
                        PlaySound(buttonClicked);
                        player1Answer = 2;
                        player1Selected = true; // Lock Player 1's choice
                        player1AnswerTime = (int)(GetTime() * 1000);
                    }
                    else if (IsKeyPressed(KEY_R)) {
                        PlaySound(buttonClicked);
                        player1Answer = 3;
                        player1Selected = true; // Lock Player 1's choice
                        player1AnswerTime = (int)(GetTime() * 1000);
                    }
                }

                // Handle Player 2 input (U, I, O, P keys)
                if (enableInput && !player2Selected) { // Check if input is enabled and Player 2 hasn't selected
                    if (IsKeyPressed(KEY_U)) {
                        PlaySound(buttonClicked);
                        player2Answer = 0;
                        player2Selected = true; // Lock Player 2's choice
                        player2AnswerTime = (int)(GetTime() * 1000);
                    }
                    else if (IsKeyPressed(KEY_I)) {
                        PlaySound(buttonClicked);
                        player2Answer = 1;
                        player2Selected = true; // Lock Player 2's choice
                        player2AnswerTime = (int)(GetTime() * 1000);
                    }
                    else if (IsKeyPressed(KEY_O)) {
                        PlaySound(buttonClicked);
                        player2Answer = 2;
                        player2Selected = true; // Lock Player 2's choice
                        player2AnswerTime = (int)(GetTime() * 1000);
                    }
                    else if (IsKeyPressed(KEY_P)) {
                        PlaySound(buttonClicked);
                        player2Answer = 3;
                        player2Selected = true; // Lock Player 2's choice
                        player2AnswerTime = (int)(GetTime() * 1000);
                    }
                }

                // After both players have selected their answers, proceed with evaluation
                if (player1Selected && player2Selected) {
                    // Stop the timer after both players select
                    countdownTime = 20;
                    timer += deltaTime;
                    enableInput = false;
                if (timer > 0.50f && !messageDisplayed) {
                    // Evaluate answers
                    if (player1Answer == questions[currentQuestionIndex].correctAnswerIndex && 
                        player2Answer != questions[currentQuestionIndex].correctAnswerIndex) {
                        gameMessage = "Player 1 got the correct answer!";
                        PlaySound(playercorrect);
                        player1Score++;
                        player2Healthpoints--;
                    }
                    else if (player2Answer == questions[currentQuestionIndex].correctAnswerIndex && 
                            player1Answer != questions[currentQuestionIndex].correctAnswerIndex) {
                        gameMessage = "Player 2 got the correct answer!";
                        PlaySound(playercorrect);
                        player2Score++;
                        player1Healthpoints--;
                    }
                    else if (player1Answer == questions[currentQuestionIndex].correctAnswerIndex && 
                            player2Answer == questions[currentQuestionIndex].correctAnswerIndex) {
                        // Both players got it correct, check who answered first
                        if (player1AnswerTime < player2AnswerTime) {
                            gameMessage = "Both players are correct, but Player 1 was faster!";
                            PlaySound(playercorrect);
                            player1Score++;
                        } else if (player2AnswerTime < player1AnswerTime) {
                            gameMessage = "Both players are correct, but Player 2 was faster!";
                            PlaySound(playercorrect);
                            player2Score++;
                        }
                    }
                    else {
                        // Both players got it wrong
                        gameMessage = "Both players got the wrong answer!";
                        PlaySound(bothWrong);
                        player1Healthpoints--;
                        player2Healthpoints--;
                        correctAnswer = true;;
                    }

                        messageDisplayed = true; // Flag to indicate message is displayed
                        timer = 0; // Reset timer for next delay
                        
                    }

                    // Delay before resetting game variables
                    if (messageDisplayed && timer > 2.5f) { // Adjust delay as needed
                        ResetGameVariables(); // Reset variables after delay
                        messageDisplayed = false; // Reset the flag
                    }
                }

                // Handle timer countdown logic (time out handling)
                if (seconds == 1) PlaySound(timesUpSound);

                if (seconds == 0) {  // If time runs out:
                    timer += deltaTime;
                    player1Answer = -1;
                    player2Answer = -1;
                    enableInput = false;
                    correctAnswer = true;
                    gameMessage = "You both ran out of time!";

                    // Both players lose health points and reset game variables
                    if (timer > 2.0f) {
                        player1Healthpoints--;
                        player2Healthpoints--;
                        ResetGameVariables();
                    }
                }

                if (player1Healthpoints == 1 || player2Healthpoints == 1) {
                    StopMusicStream(multiplayerMusic);
                    if (!IsMusicStreamPlaying(singleplayerLowHealthMusic)) {
                        PlayMusicStream(singleplayerLowHealthMusic);
                    }
                    UpdateMusicStream(singleplayerLowHealthMusic);  // Update music stream to continue playing it
                    }


                // Add a delay before going to game over screen
                if (player1Healthpoints <= 0 || player2Healthpoints <= 0) {
                    if (!isGameOverTriggered) {
                        isGameOverTriggered = true;  // Flag to indicate game-over condition
                        gameOverDelayTimer = 0.0f; 
                        // After Player 1 and Player 2 finish the round, update the leaderboard
                        UpdateLeaderboard(player1Name, player1Score, player2Name, player2Score);  // Reset the timer
                    }
                }
                // If the game-over condition has been triggered, increment the timer
                if (isGameOverTriggered) {
                    gameOverDelayTimer += deltaTime;  // Update timer with the elapsed time

                    if (gameOverDelayTimer >= 2.5f) { // Add a 2.5-second delay
                        currentScreen = MULTIPLAYER_GAMEOVER;
                        PlaySound(playerWins);    // Switch to the Game Over screen
                    }
                }

                // Pause
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
                    PlaySound(menuButtonsSound);
                }
                if (resumeBtn.isClicked(mousePosition, mouseClicked)) {

                    currentScreen = previousScreen;

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
                    currentScreen = (exitFromGameover) ? SINGLEPLAYER_GAMEOVER:MAIN_MENU;  // Returns to GAMEOVER screen when player clicks no, if in the GAMEOVER screen
                    exitFromGameover = false;
                    PlaySound(menuButtonsSound);
                }
                break;
            case SINGLEPLAYER_GAMEOVER:
                if (IsMusicStreamPlaying(singleplayerMusic) || IsMusicStreamPlaying(singleplayerLowHealthMusic)) {
                    StopMusicStream(singleplayerMusic); // Stop multiplayer music
                    StopMusicStream(singleplayerLowHealthMusic); // Stop singleplayer music
                    PlaySound(gameoverSound);
                }
                

                if (!IsMusicStreamPlaying(mainMenuMusic)) {
                    PlayMusicStream(mainMenuMusic);
                }
                 
                UpdateMusicStream(mainMenuMusic);  // Update music stream to continue playing it

                if (score > highscore) {
                    highscore = score;
                    SaveHighScore(SINGLEPLAYER_DATA_FILE_PATH, highscore);
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
                    
                    currentScreen = SINGLEPLAYER_RULES; 
                } 
                break;
            case MULTIPLAYER_GAMEOVER:
                if (!IsMusicStreamPlaying(multiplayerMusic)) {
                    PlayMusicStream(multiplayerMusic);
                }
                UpdateMusicStream(multiplayerMusic);  // Update music stream to continue playing it
                // Determine the winner based on scores
                if (player1Score > player2Score) {
                    gameMessage = player1Name + " wins!";
                    gameMessage1 = "Score: " + std::to_string(player1Score);           
                    gameMessage2 = "Remaining Health: " + std::to_string(player1Healthpoints);
                } else if (player2Score > player1Score) {
                    gameMessage = player2Name + " wins! ";
                    gameMessage1 = "Score: " + std::to_string(player2Score);           
                    gameMessage2 = "Remaining Health: " + std::to_string(player2Healthpoints);
                } else {
                    gameMessage = "It's a draw!";
                    gameMessage1 = "Score: " + std::to_string(player1Score);
                }

                if (leaderboardsBtn.isClicked(mousePosition, mouseClicked)) currentScreen = LEADERBOARDS;
                if (mainMenuBtn.isClicked(mousePosition, mouseClicked)) currentScreen = MAIN_MENU;
                if (exitBtn.isClicked(mousePosition, mouseClicked)) currentScreen = EXIT;
                if (restartBtn.isClicked(mousePosition, mouseClicked)) {    // Reset variables and return to RULES GameScreen
                    isGameOverTriggered = false;
                    ResetGameVariables();
                    currentScreen = MULTIPLAYER_RULES; 
                } 
                break;
            case LEADERBOARDS:
                if (!IsMusicStreamPlaying(multiplayerMusic)) {
                    PlayMusicStream(multiplayerMusic);
                }
                UpdateMusicStream(multiplayerMusic);  // Update music stream to continue playing it
                // Pause
                if (pauseBtn.isClicked(mousePosition, mouseClicked) || IsKeyPressed(KEY_ESCAPE)) {
                    previousScreen = LEADERBOARDS;
                    currentScreen = PAUSE;
                    PlaySound(menuButtonsSound);
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
            DrawTextureEx(titleLogo, {(float)(GetScreenWidth() - titleLogo.width * 1.1) / 2, 260}, 0, 1.1, WHITE);
            DrawTextureEx(fiveHearts, {755, 70}, 0.0f, 0.3, WHITE);
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
            
            DrawTextHighlight(arcadeFont, "Player 1", 140.0f, 240.0f, 20.0f, 0.5f, ORANGE);

            DrawTextHighlight(arcadeFont, "Player 2", 1610.0f, 240.0f, 20.0f, 0.5f, PURPLE);

            for (int x = -2; x <= 2; x++) {
                for (int y = -2; y <= 2; y++) {
                    if (x != 0 || y != 0) {
                        DrawCenteredTextAtX(player1Name.c_str(), arcadeFont, 212 + x, 190 + y, 10, 40, screenWidth * 0.8f, ORANGE);
                    }
                }
            }

            DrawCenteredTextAtX(player1Name.c_str(), arcadeFont, 212, 190, 10, 40, screenWidth * 0.8f, BLACK);

            for (int x = -2; x <= 2; x++) {
                for (int y = -2; y <= 2; y++) {
                    if (x != 0 || y != 0) {
                        DrawCenteredTextAtX(player2Name.c_str(), arcadeFont, 1690 + x, 190 + y, 10, 40, screenWidth * 0.8f, PURPLE);
                    }
                }
            }

            DrawCenteredTextAtX(player2Name.c_str(), arcadeFont, 1690, 190, 10, 40, screenWidth * 0.8f, BLACK);

            // Draw Player 1 score
            DrawTextHighlight(arcadeFont, TextFormat("Score: %i", player1Score), 100, 350, 30, 1.0f, ORANGE);
            // Draw Player 2 score
            DrawTextHighlight(arcadeFont, TextFormat("Score: %i", player2Score), 1590, 350, 30, 1.0f, PURPLE);


            // Draw Player 1 health
            DrawTextHighlight(arcadeFont, "Health: ", 100, 400, 30, 1.0f, ORANGE);
            // Draw Player 2 health
            DrawTextHighlight(arcadeFont, "Health: ", 1590, 400, 30, 1.0f, PURPLE);

            answerQUBtn.DrawButton();
            answerWIBtn.DrawButton();
            answerEOBtn.DrawButton();
            answerRPBtn.DrawButton();

            // Player 1's answers
            for (int i = 0; i < 4; i++) {
                // If Player 1 has selected this answer, highlight it with an orange color
                if (player1Answer == i) {
                    // Draw the highlight first (outline effect) using a small offset
                    for (int x = -2; x <= 2; x++) {
                        for (int y = -2; y <= 2; y++) {
                            if (x != 0 || y != 0) {  // Skip the center to avoid overlapping
                                if (i == 0) DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[i].c_str(), 25.0f, 1.0f, ORANGE, answerQUBtn.position.x + (float)x, answerQUBtn.position.y + (float)y, answerQUBtn.width, answerQUBtn.height, 600, false);
                                else if (i == 1) DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[i].c_str(), 25.0f, 1.0f, ORANGE, answerWIBtn.position.x + (float)x, answerWIBtn.position.y + (float)y, answerWIBtn.width, answerWIBtn.height, 600, false);
                                else if (i == 2) DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[i].c_str(), 25.0f, 1.0f, ORANGE, answerEOBtn.position.x + (float)x, answerEOBtn.position.y + (float)y, answerEOBtn.width, answerEOBtn.height, 600, false);
                                else if (i == 3) DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[i].c_str(),25.0f, 1.0f, ORANGE, answerRPBtn.position.x + (float)x, answerRPBtn.position.y + (float)y, answerRPBtn.width, answerRPBtn.height, 600, false);
                            }
                        }
                    }
                }

                // Draw the actual answer in black for Player 1
                if (i == 0) DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[i].c_str(), 25.0f, 1.0f, BLACK, answerQUBtn.position.x, answerQUBtn.position.y, answerQUBtn.width, answerQUBtn.height, 600, false);
                else if (i == 1) DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[i].c_str(), 25.0f, 1.0f, BLACK, answerWIBtn.position.x, answerWIBtn.position.y, answerWIBtn.width, answerWIBtn.height, 600, false);
                else if (i == 2) DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[i].c_str(), 25.0f, 1.0f, BLACK, answerEOBtn.position.x, answerEOBtn.position.y, answerEOBtn.width, answerEOBtn.height, 600, false);
                else if (i == 3) DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[i].c_str(), 25.0f, 1.0f, BLACK, answerRPBtn.position.x, answerRPBtn.position.y, answerRPBtn.width, answerRPBtn.height, 600, false);
            }

            // Player 2's answers
            for (int i = 0; i < 4; i++) {
                // If Player 2 has selected this answer, highlight it with a PURPLE color
                if (player2Answer == i) {
                    // Draw the highlight first (outline effect) using a small offset
                    for (int x = -2; x <= 2; x++) {
                        for (int y = -2; y <= 2; y++) {
                            if (x != 0 || y != 0) {  // Skip the center to avoid overlapping
                                if (i == 0) DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[i].c_str(), 25.0f, 1.0f, PURPLE, answerQUBtn.position.x + (float)x, answerQUBtn.position.y + (float)y, answerQUBtn.width, answerQUBtn.height, 600, false);
                                else if (i == 1) DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[i].c_str(), 25.0f, 1.0f, PURPLE, answerWIBtn.position.x + (float)x, answerWIBtn.position.y + (float)y, answerWIBtn.width, answerWIBtn.height, 600, false);
                                else if (i == 2) DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[i].c_str(), 25.0f, 1.0f, PURPLE, answerEOBtn.position.x + (float)x, answerEOBtn.position.y + (float)y, answerEOBtn.width, answerEOBtn.height, 600, false);
                                else if (i == 3) DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[i].c_str(), 25.0f, 1.0f, PURPLE, answerRPBtn.position.x + (float)x, answerRPBtn.position.y + (float)y, answerRPBtn.width, answerRPBtn.height, 600, false);
                            }
                        }
                    }
                }
            
                // Draw the actual answer in black for Player 2
                if (i == 0) DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[i].c_str(), 25.0f, 1.0f, BLACK, answerQUBtn.position.x, answerQUBtn.position.y, answerQUBtn.width, answerQUBtn.height, 600, false); 
                else if (i == 1) DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[i].c_str(), 25.0f, 1.0f, BLACK, answerWIBtn.position.x, answerWIBtn.position.y, answerWIBtn.width, answerWIBtn.height, 600, false);
                else if (i == 2) DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[i].c_str(), 25.0f, 1.0f, BLACK, answerEOBtn.position.x, answerEOBtn.position.y, answerEOBtn.width, answerEOBtn.height, 600, false);
                else if (i == 3) DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[i].c_str(), 25.0f, 1.0f, BLACK, answerRPBtn.position.x, answerRPBtn.position.y, answerRPBtn.width, answerRPBtn.height, 600, false);
            
            }
            
            //If both player got the wrong answer then it reveals the correct one
            if (correctAnswer) {
                // Draw the correct answer with green highlight
                for (int x = -2; x <= 2; x++) { 
                    for (int y = -2; y <= 2; y++) {
                        if (x != 0 || y != 0) {
                            DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[0].c_str(), 25.0f, 1.0f, (questions[currentQuestionIndex].correctAnswerIndex == 0) ? GREEN : RED, answerQUBtn.position.x + (float)x, answerQUBtn.position.y + (float)y, answerQUBtn.width, answerQUBtn.height, 600, false);
                            DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[1].c_str(), 25.0f, 1.0f, (questions[currentQuestionIndex].correctAnswerIndex == 1) ? GREEN : RED, answerWIBtn.position.x + (float)x, answerWIBtn.position.y + (float)y, answerWIBtn.width, answerWIBtn.height, 600, false);
                            DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[2].c_str(), 25.0f, 1.0f, (questions[currentQuestionIndex].correctAnswerIndex == 2) ? GREEN : RED, answerEOBtn.position.x + (float)x, answerEOBtn.position.y + (float)y, answerEOBtn.width, answerEOBtn.height, 600, false);
                            DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[3].c_str(), 25.0f, 1.0f, (questions[currentQuestionIndex].correctAnswerIndex == 3) ? GREEN : RED, answerRPBtn.position.x + (float)x, answerRPBtn.position.y + (float)y, answerRPBtn.width, answerRPBtn.height, 600, false);
                        }
                    }
                }

                DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[0].c_str(), 25.0f, 1.0f, BLACK, answerQUBtn.position.x, answerQUBtn.position.y, answerQUBtn.width,answerQUBtn.height, 600, false);
                DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[1].c_str(), 25.0f, 1.0f, BLACK, answerWIBtn.position.x, answerWIBtn.position.y, answerWIBtn.width, answerWIBtn.height, 600, false);
                DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[2].c_str(), 25.0f, 1.0f, BLACK, answerEOBtn.position.x, answerEOBtn.position.y, answerEOBtn.width, answerEOBtn.height, 600, false);
                DrawAnswerText(arcadeFont, questions[currentQuestionIndex].answers[3].c_str(), 25.0f, 1.0f, BLACK, answerRPBtn.position.x, answerRPBtn.position.y, answerRPBtn.width, answerRPBtn.height, 600, false);
                    
            }

            // Display the message after answers are evaluated
            if (!gameMessage.empty()) {
                // Calculate the width of the text to center it
                Vector2 textSize = MeasureTextEx(arcadeFont, gameMessage.c_str(), 30.0f, 1.0f);
                DrawTextHighlight(arcadeFont, gameMessage.c_str(), (float) (GetScreenWidth() - textSize.x) / 2.0f, (float) (GetScreenHeight() - 400), 30.0f, 1.0f, GREEN);
            }

            // Draw Timer at the start of the question
            if (seconds > 0) DrawTextHorizontal(arcadeFont, TextFormat("Timer: %i", seconds), 50.0f, 1.0f, BLACK, 100.0f);
            else DrawTextHorizontal(arcadeFont, "Times Up!", 50.0f, 1.0f, RED, 100.0f);  // Display "Times Up!"

            // Draw Health for Player 1
            if (player1Healthpoints == 11) DrawTextureEx(health_11, {90.0f, 450.0f}, 0.0f, 0.5f, WHITE);
            if (player1Healthpoints == 10) DrawTextureEx(health_10, {90.0f, 450.0f}, 0.0f, 0.15f, WHITE);
            if (player1Healthpoints == 9) DrawTextureEx(health_9, {90.0f, 450.0f}, 0.0f, 0.15f, WHITE);
            if (player1Healthpoints == 8) DrawTextureEx(health_8, {90.0f, 450.0f}, 0.0f, 0.15f, WHITE);
            if (player1Healthpoints == 7) DrawTextureEx(health_7, {90.0f, 450.0f}, 0.0f, 0.15f, WHITE);
            if (player1Healthpoints == 6) DrawTextureEx(health_6, {90.0f, 450.0f}, 0.0f, 0.15f, WHITE);
            if (player1Healthpoints == 5) DrawTextureEx(health_5, {90.0f, 450.0f}, 0.0f, 0.18f, WHITE);
            if (player1Healthpoints == 4) DrawTextureEx(health_4, {90.0f, 450.0f}, 0.0f, 0.15f, WHITE);
            if (player1Healthpoints == 3) DrawTextureEx(health_3, {90.0f, 450.0f}, 0.0f, 0.15f, WHITE);
            if (player1Healthpoints == 2) DrawTextureEx(health_2, {90.0f, 450.0f}, 0.0f, 0.15f, WHITE);
            if (player1Healthpoints == 1) DrawTextureEx(health_1, {90.0f, 450.0f}, 0.0f, 0.15f, WHITE);

            // Draw Health for Player 2
            if (player2Healthpoints == 11) DrawTextureEx(health_11, {1580.0f, 450.0f}, 0.0f, 0.5f, WHITE);
            if (player2Healthpoints == 10) DrawTextureEx(health_10, {1580.0f, 450.0f}, 0.0f, 0.15f, WHITE);
            if (player2Healthpoints == 9) DrawTextureEx(health_9, {1580.0f, 450.0f}, 0.0f, 0.15f, WHITE);
            if (player2Healthpoints == 8) DrawTextureEx(health_8, {1580.0f, 450.0f}, 0.0f, 0.15f, WHITE);
            if (player2Healthpoints == 7) DrawTextureEx(health_7, {1580.0f, 450.0f}, 0.0f, 0.15f, WHITE);
            if (player2Healthpoints == 6) DrawTextureEx(health_6, {1580.0f, 450.0f}, 0.0f, 0.15f, WHITE);
            if (player2Healthpoints == 5) DrawTextureEx(health_5, {1580.0f, 450.0f}, 0.0f, 0.18f, WHITE);
            if (player2Healthpoints == 4) DrawTextureEx(health_4, {1580.0f, 450.0f}, 0.0f, 0.15f, WHITE);
            if (player2Healthpoints == 3) DrawTextureEx(health_3, {1580.0f, 450.0f}, 0.0f, 0.15f, WHITE);
            if (player2Healthpoints == 2) DrawTextureEx(health_2, {1580.0f, 450.0f}, 0.0f, 0.15f, WHITE);
            if (player2Healthpoints == 1) DrawTextureEx(health_1, {1580.0f, 450.0f}, 0.0f, 0.15f, WHITE);

            pauseBtn.DrawButton();
            break;
        case LEADERBOARDS:
            DrawTexture(leaderBoardBackground, 0, 0, WHITE);
            pauseBtn.DrawButton();

            // Draw the top 10 leaderboard entries
            for (size_t i = 0; i < leaderboard.size(); i++) {
                const LeaderboardEntry& entry = leaderboard[i];
                // Draw the player's rank and name separately
                std::string rankText = std::to_string(i + 1) + ". " + entry.playerName;
                DrawTextEx(arcadeFont, rankText.c_str(), (Vector2){550, yOffsets[i]}, 22.0f, 2.0f, BLACK);
                // Now, draw the player's score separately with a different offset or style
                std::string scoreText = std::to_string(entry.score);
                DrawTextEx(arcadeFont, scoreText.c_str(), (Vector2){1320, yOffsets[i]}, 22.0f, 2.0f, BLACK); // Adjust the X position (500 here)

                // Draw shadow effect by drawing the same text at various offsets for "PLAYER"
                for (int x = -2; x <= 2; x++) {
                    for (int y = -2; y <= 2; y++) {
                        if (x != 0 || y != 0) {
                            DrawTextEx(arcadeFont, "PLAYER", (Vector2){520 + (float)x, 400 + (float)y}, 45.0f, 2.0f, BLACK);
                        }
                    }
                }
                DrawTextEx(arcadeFont, "PLAYER", (Vector2){520, 400}, 45.0f, 2.0f, YELLOW);

                // Draw shadow effect by drawing the same text at various offsets for "SCORE"
                for (int x = -2; x <= 2; x++) {
                    for (int y = -2; y <= 2; y++) {
                        if (x != 0 || y != 0) {
                            DrawTextEx(arcadeFont, "SCORE", (Vector2){1220 + (float)x, 400 + (float)y}, 45.0f, 2.0f, BLACK);
                        }
                    }
                }
                DrawTextEx(arcadeFont, "SCORE", (Vector2){1220, 400}, 45.0f, 2.0f, YELLOW);
            }
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
        case SINGLEPLAYER_RULES:
            DrawTexture(rulesScreen, 0, 0, WHITE);
            DrawTextHorizontal(arcadeFont, "Press any button to start", 30, 1, WHITE, GetScreenHeight() - 200);
            break;
        case MULTIPLAYER_RULES:
            DrawTexture(rulesScreen1, 0, 0, WHITE);
            DrawTextHorizontal(arcadeFont, "Press any button to proceed", 30, 1, WHITE, GetScreenHeight() - 200);
            break;
        case MULTIPLAYER_CONTROLS:
            DrawTexture(controlScreen2, 0,0,WHITE);
             DrawTextHorizontal(arcadeFont, "Press any button to proceed", 30, 1, WHITE, GetScreenHeight() - 200);
            if (timer > inputCooldown) {
                if (IsAnyKeyPressed() || IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) currentScreen = PLAYERNAME;
            }
            break;
        case PLAYERNAME:
            DrawTexture(enterPlayerName, 0, 0, WHITE);
            playerNameBoxBtn.DrawButton();
            playerNameBox1Btn.DrawButton();

            //Player 1 name input
            DrawTextHighlight(arcadeFont, player1Name.c_str(), 840.0f, 455.0f, 30, 1.0f, ORANGE);

            if (enteringPlayer1Name) DrawTextHighlight(arcadeFont, "Typing...", 840.0f, 540.0f, 20.0f, 1.0f, ORANGE);

            //Player 2 name input        
            DrawTextHighlight(arcadeFont, player2Name.c_str(), 840.0f, 620.0f, 30.0f, 1.0f, PURPLE);

            if (enteringPlayer2Name) DrawTextHighlight(arcadeFont, "Typing... ", 840.0f, 705.0f, 20.0f, 1.0f, PURPLE);
            
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
        case SINGLEPLAYER_GAMEOVER:
            DrawTexture(gameoverBackground, 0, 0, WHITE);

            DrawTextHorizontal(arcadeFont, TextFormat("Score: %i", score), 50.0f, 1.0f, BLACK, 300.0f);
            DrawTextHorizontal(arcadeFont, TextFormat("High Score: %i", highscore), 50.0f, 1.0f, ORANGE, 400.0f);
          
            restartBtn.DrawButtonHorizontal();
            restartBtn.position.y = 500.0f;
            mainMenuBtn.DrawButtonHorizontal();
            mainMenuBtn.imgScale = 0.80f;
            mainMenuBtn.position.y = 600.0f;
            exitBtn.DrawButtonHorizontal();
            exitBtn.imgScale = 0.53f;
            exitBtn.position.y = 700.0f;

            break;
        case MULTIPLAYER_GAMEOVER:
            DrawTexture(gameoverBackground, 0, 0, WHITE);

            // Display the message after answers are evaluated
            if (!gameMessage.empty()) {
                // Draw the black highlight first (outline effect) using a small offset
                for (int x = -2; x <= 2; x++) {
                    for (int y = -2; y <= 2; y++) {
                        if (x != 0 || y != 0) { // Skip the center to avoid overlapping
                            DrawTextEx(arcadeFont, gameMessage.c_str(), 
                            {(float) (GetScreenWidth() - MeasureTextEx(arcadeFont, gameMessage.c_str(), 75.0f, 1.0f).x) / 2.0f + (float)x, 
                            (float) (GetScreenHeight() - 750) + (float)y}, 75.0f, 1.0f, BLACK);
                        }
                    }
                }

                // Then draw the actual message in yellow (on top of the green highlight)
                DrawTextEx(arcadeFont, gameMessage.c_str(), 
                {(float) (GetScreenWidth() - MeasureTextEx(arcadeFont, gameMessage.c_str(), 75.0f, 1.0f).x) / 2.0f, 
                (float) (GetScreenHeight() - 750)}, 75.0f, 1.0f, YELLOW);}
                
            if (!gameMessage1.empty()) {
                for (int x = -2; x <= 2; x++) {
                    for (int y = -2; y <= 2; y++) {
                        if (x != 0 || y != 0) { 
                            DrawTextEx(arcadeFont, gameMessage1.c_str(), 
                            {(float) (GetScreenWidth() - MeasureTextEx(arcadeFont, gameMessage1.c_str(), 30.0f, 1.0f).x) / 2.0f + (float)x,
                            (float) (GetScreenHeight() - 660) + (float)y}, 30.0f, 1.0f, BLACK);
                        }
                    }
                }

                DrawTextEx(arcadeFont, gameMessage1.c_str(), 
                {(float) (GetScreenWidth() - MeasureTextEx(arcadeFont, gameMessage1.c_str(), 30.0f, 1.0f).x) / 2.0f, 
                (float) (GetScreenHeight() - 660)}, 30.0f, 1.0f, YELLOW);}

            if (!gameMessage2.empty()) {
                for (int x = -2; x <= 2; x++) {
                    for (int y = -2; y <= 2; y++) {
                        if (x != 0 || y != 0) {
                            DrawTextEx(arcadeFont, gameMessage2.c_str(), 
                            {(float) (GetScreenWidth() - MeasureTextEx(arcadeFont, gameMessage2.c_str(), 30.0f, 1.0f).x) / 2.0f + (float)x, 
                            (float) (GetScreenHeight() - 610) + (float)y}, 30.0f, 1.0f, BLACK);
                        }
                    }
                }

                DrawTextEx(arcadeFont, gameMessage2.c_str(), 
                {(float) (GetScreenWidth() - MeasureTextEx(arcadeFont, gameMessage2.c_str(), 30.0f, 1.0f).x) / 2.0f, 
                (float) (GetScreenHeight() - 610)}, 30.0f, 1.0f, YELLOW);
            }
            
            leaderboardsBtn.DrawButtonHorizontal();
            leaderboardsBtn.position.y = 580.0f;
            restartBtn.DrawButtonHorizontal();
            restartBtn.imgScale = 0.75f;
            restartBtn.position.y = 670.0f;
            mainMenuBtn.DrawButtonHorizontal();
            mainMenuBtn.imgScale = 0.80f;
            mainMenuBtn.position.y = 760.0f;
            exitBtn.DrawButtonHorizontal();
            exitBtn.imgScale = 0.53f;
            exitBtn.position.y = 850.0f;
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