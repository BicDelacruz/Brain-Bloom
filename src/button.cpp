#include "button.hpp"

Button::Button(const char* imagePath, Vector2 imagePosition, float scale)
{
    Image img = LoadImage(imagePath);

    int origWidth = img.width;
    int origHeight = img.height;

    int newWidth = static_cast<int>(origWidth * scale);
    int newHeight = static_cast<int>(origHeight * scale);

    ImageResize(&img, newWidth, newHeight);

    texture = LoadTextureFromImage(img);
    imgScale = scale;
    position = imagePosition;
    
    width = texture.width;
    height = texture.height;

    UnloadImage(img);
}

Button::~Button() {
    UnloadTexture(texture);
}

void Button::DrawButton() {
    DrawTextureV(texture, position, WHITE);
}

void Button::DrawButtonHorizontal() {
    position.x = (float) (GetScreenWidth() - texture.width * imgScale) / 2;
    DrawTextureEx(texture, {position.x, position.y}, 0.0f, imgScale, WHITE);
}

bool Button::isClicked(Vector2 mousePos, bool mousePressed)
{
    Rectangle buttonRect = { 
        position.x, 
        position.y, 
        (float)texture.width * imgScale, 
        (float)texture.height * imgScale 
    };

    return mousePressed && CheckCollisionPointRec(mousePos, buttonRect);
}
