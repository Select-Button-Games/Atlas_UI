#include <iostream>
#define SETUP_SDL_OPENGL
#include "atlas_ui3.0.h"
#include "atlas_ui_utilities.h"

GLuint loadTexture(const char* path) {
    SDL_Surface* surface = IMG_Load(path); // Use IMG_Load instead of SDL_LoadBMP
    if (!surface) {
        std::cerr << "Failed to load texture: " << IMG_GetError() << std::endl; // Use IMG_GetError for error reporting
        return 0;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Determine the format of the image
    GLenum format;
    if (surface->format->BytesPerPixel == 4) {
        format = surface->format->Rmask == 0x000000ff ? GL_RGBA : GL_BGRA;
    }
    else {
        format = surface->format->Rmask == 0x000000ff ? GL_RGB : GL_BGR;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, format == GL_RGBA ? GL_RGBA : GL_RGB, surface->w, surface->h, 0, format, GL_UNSIGNED_BYTE, surface->pixels);

    // Generate mipmaps
    glGenerateMipmap(GL_TEXTURE_2D);

    // Set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // Use mipmaps for minification
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Set texture wrapping parameters (optional, but good to check)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // Changed to GL_CLAMP_TO_EDGE for better edge handling
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);

    SDL_FreeSurface(surface);
    std::cout << "Texture loaded successfully from " << path << std::endl;
    return texture;
}

int main(int argc, char* argv[]) {
    Atlas::Setup("Atlas UI Example", Atlas::SCREEN_WIDTH, Atlas::SCREEN_HEIGHT);
    // Initialize the UI library
    Atlas::initOpenGL();
    Atlas::setProjectionMatrix(Atlas::SCREEN_WIDTH, Atlas::SCREEN_HEIGHT);
    Atlas::initOpenGLDebug();
    // Load texture
    GLuint texture = loadTexture("metalPanel_green.png");
    if (!texture) {
        std::cerr << "Failed to load texture" << std::endl;
        return -1;
    }

    // Parent widget to hold checkboxes and labels
    Atlas::createWidget(1, 0, 0, 400, 300, Atlas::WidgetOptions::WIDGET_DRAGGABLE, "metalPanel_green.png");

    // First checkbox and label
    Atlas::Text("Enable Feature A", 1.0f, 60, 20);
    auto featureACallback = [](bool isChecked) {
        std::cout << "Feature A is " << (isChecked ? "enabled" : "disabled") << "." << std::endl;
        };
    Atlas::CheckBox(60, 40, false, featureACallback, "Feature A");

    // Second checkbox and label
    Atlas::Text("Enable Feature B", 1.0f, 60, 70);
    auto featureBCallback = [](bool isChecked) {
        std::cout << "Feature B is " << (isChecked ? "enabled" : "disabled") << "." << std::endl;
        };
    Atlas::CheckBox(60, 90, false, featureBCallback, "Feature B");

    // Third checkbox and label
    Atlas::Text("Enable Feature C", 1.0f, 60, 120);
    auto featureCCallback = [](bool isChecked) {
        std::cout << "Feature C is " << (isChecked ? "enabled" : "disabled") << "." << std::endl;
        };
    Atlas::CheckBox(60, 140, false, featureCCallback, "Feature C");

    std::string detectedUrl = ""; // Global or member variable to store the detected URL

    auto textBox = [](const std::string& text) {
        std::cout << "Text entered: " << text << std::endl;

    
        };
    //UI BUTTON
    Atlas::TextInput(60, 200, 200, 30, textBox, "Test", Atlas::WidgetOptions::WIDGET_PASSWORD);


    // Close the parent widget
    Atlas::endWidget();

    bool quit = false;
    SDL_Event event;

    while (!quit) {
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
            Atlas::handleEvents(&event);
        }

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        Atlas::renderUI();

        SDL_GL_SwapWindow(Atlas::g_window);
    }

    Atlas::Shutdown();
    return 0;
}

