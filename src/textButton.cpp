#include "textButton.hpp"

TextButton::TextButton(const char* buttonText, Vector2 buttonPosition, float fontSize, float fontSpacing)
{
    isDrawn = false;

    font = LoadFont("fonts/arcade.ttf");
    position = buttonPosition;
    text = buttonText;
    size = fontSize;
    spacing = fontSpacing;

    textSize = MeasureTextEx(font, text, fontSize, spacing);
}

TextButton::~TextButton()
{
    UnloadFont(font);
    isDrawn = false;
}

void TextButton::DrawTextButton()
{
    isDrawn = true;
    DrawTextEx(font, text, position, size, spacing, WHITE);
}

void TextButton::DrawTextButtonCenter(int screenWidth, int screenHeight)
{
    isDrawn = true;
    Vector2 textSize = MeasureTextEx(font, text, size, spacing);
    Vector2 centerPosition = {(screenWidth - textSize.x) / 2, (screenHeight - textSize.y) / 2};

    DrawTextEx(font, text, centerPosition, size, spacing, WHITE);
}

void TextButton::DrawTextButtonVertical(int screenHeight, float posYOffset)
{
    isDrawn = true;
    Vector2 textSize = MeasureTextEx(font, text, size, spacing);
    Vector2 verticalPosition = {position.x, ((screenHeight - textSize.y) / 2) + posYOffset};

    DrawTextEx(font, text, verticalPosition, size, spacing, WHITE);
}

void TextButton::DrawTextButtonHorizontal(int screenWidth, float posXOffset)
{
    isDrawn = true;
    Vector2 textSize = MeasureTextEx(font, text, size, spacing);
    Vector2 horizontalPosition = {((screenWidth - textSize.x) / 2) + posXOffset, position.y};

    DrawTextEx(font, text, horizontalPosition, size, spacing, WHITE);
}

bool TextButton::isClicked(Vector2 mousePos, bool mousePressed)
{
    Rectangle rect = {position.x, position.y, textSize.x, textSize.y};

    if (CheckCollisionPointRec(mousePos, rect) && mousePressed) {
        return true;
    } 
    return false;
}
