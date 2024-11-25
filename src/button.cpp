#include "button.hpp"

Button::Button(const char* imagePath, Vector2 imagePosition, float scale)
{
    isDrawn = false;
    Image img = LoadImage(imagePath);

    int origWidth = img.width;
    int origHeight = img.height;

    int newWidth = static_cast<int>(origWidth * scale);
    int newHeight = static_cast<int>(origHeight * scale);

    ImageResize(&img, newWidth, newHeight);

    texture = LoadTextureFromImage(img);
    imgScale = scale;
    position = imagePosition;

    UnloadImage(img);
}

Button::~Button() {
    UnloadTexture(texture);
    isDrawn = false;
}

void Button::DrawButton() {
    DrawTextureV(texture, position, WHITE);
    isDrawn = true;
}

void Button::DrawButtonHorizontal() {
    float centeredX = (GetScreenWidth() - texture.width * imgScale) / 2;

    position.x = centeredX;

    DrawTextureEx(texture, {position.x, position.y}, 0.0f, imgScale, WHITE);

    isDrawn = true;
}

bool Button::isClicked(Vector2 mousePos, bool mousePressed)
{
    Rectangle buttonRect = { 
        position.x, 
        position.y, 
        texture.width * imgScale, 
        texture.height * imgScale 
    };

    return mousePressed && CheckCollisionPointRec(mousePos, buttonRect);
}
