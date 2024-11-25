#pragma once
#include <raylib.h>

class TextButton {
    public:
        TextButton(const char* buttonText, Vector2 buttonPosition, float fontSize, float fontSpacing);
        ~TextButton();
        void DrawTextButton();
        void DrawTextButtonCenter(int screenWidth, int screenHeight);
        void DrawTextButtonVertical(int screenHeight, float posYOffset);
        void DrawTextButtonHorizontal(int screenWidth, float posXOffset);
        bool isClicked(Vector2 mousePos, bool mousePressed);
        bool isDrawn;

    private:
        Vector2 position;
        Vector2 textSize;
        float size;
        float spacing;
        Font font;
        const char* text;
};