#pragma once
#include <raylib.h>

class Button {
    public:
        Button(const char* imagePath, Vector2 imagePosition, float scale);
        ~Button();
        void DrawButton();
        void DrawButtonHorizontal();
        bool isClicked(Vector2 mousePos, bool mousePressed);
        bool isDrawn;
        float imgScale, width, height;
        Vector2 position;
    private:
        Texture2D texture;
};