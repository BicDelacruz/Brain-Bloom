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

    isDrawnHorizontal = false; // Set to false for the answerQ_Btn, answerW_Btn, ... buttons

    UnloadImage(img);
}

Button::~Button() {
    UnloadTexture(texture);
}

void Button::DrawButton(void) {
    DrawTextureV(texture, position, WHITE);
}

void Button::DrawButtonHorizontal(void) {
    isDrawnHorizontal = true;
    position.x = (float) ((GetScreenWidth() - width * imgScale) / 2);
    DrawTextureEx(texture, {position.x, position.y}, 0.0f, imgScale, WHITE);
}

bool Button::isClicked(Vector2 mousePos, bool mousePressed)
{
    Rectangle buttonRect = { 
        position.x, 
        position.y, 
        (isDrawnHorizontal) ? width * imgScale : width,     // isDrawnHorizontal will stay false until the Button object uses DrawButtonHorizontal() 
        (isDrawnHorizontal) ? height * imgScale : height    // only menu buttons use DrawButtonHorizontal(), answer buttons need the exact width and height for the buttonRect
    };

    return mousePressed && CheckCollisionPointRec(mousePos, buttonRect);
}
