
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <iostream>
#define SETUP_SDL_OPENGL
#include "atlas_ui3.0.h"
#include "atlas_ui_utilities.h"
#pragma comment(linker,"/ENTRY:mainCRTStartup")

#include "atlas_ui_shapes.h"
#include <unordered_map>
bool isshown;

std::unordered_map<int, int> widgetOptions;

void setWidgetOptions(int widgetID, Atlas::WidgetOptions options)
{
    widgetOptions[widgetID] = options;
}

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

int main(int argc, char* argv[])
{
    Atlas::Setup("Atlas UI Example", Atlas::SCREEN_WIDTH, Atlas::SCREEN_HEIGHT);
    // Initialize the UI library
    Atlas::initOpenGL();
    Atlas::setProjectionMatrix(Atlas::SCREEN_WIDTH, Atlas::SCREEN_HEIGHT);
    Atlas::initOpenGLDebug();
   

    // Parent widget to hold new UI elements
    //Atlas::createWidget(1, 0, 0, Atlas::SCREEN_WIDTH, Atlas::SCREEN_HEIGHT, Atlas::WidgetOptions::WIDGET_DRAGGABLE," ");
    /**
    // Assuming the widget's dimensions are 600x300
    int widgetWidth = 600;
    int widgetHeight = 300;

    // Central alignment and spacing
    int centerX = widgetWidth / 2;
    int startY = 100; // Starting Y position for the first element
    int verticalSpacing = 70; // Space between elements

    // Username label and text input
    Atlas::Text("Username", 1.0f, centerX - 100, startY); // Adjust X position as needed for label
    auto usernameCallback = [](const std::string& text) {
        std::cout << "Username entered: " << text << std::endl;
        };
    Atlas::TextInput(centerX - 100, startY + 20, 200, 30, usernameCallback, "", Atlas::WidgetOptions::WIDGET_NONE);

    // Password label and text input
    Atlas::Text("Password", 1.0f, centerX - 100, startY + verticalSpacing); // Adjust X position as needed for label
    auto passwordCallback = [](const std::string& text) {
        std::cout << "Password entered: " << text << std::endl;
        };
    Atlas::TextInput(centerX - 100, startY + verticalSpacing + 20, 200, 30, passwordCallback, "", Atlas::WidgetOptions::WIDGET_PASSWORD);

    // Login button
    auto loginCallback = []() {
        std::cout << "Login button clicked." << std::endl;
        };
    Atlas::Button("        Login", 1.0f, "UI/Btn_v02.png", loginCallback, 150, 29, centerX - 75, startY + 2 * verticalSpacing);
    **/
   
    Atlas::createWidget(1, 0, 0, 400, 400, Atlas::WidgetOptions::WIDGET_SHOWN, "");
    Atlas::Text("Hello World", 1.0f, 100, 100);
    Atlas::endWidget();
 
    bool quit = false;
    SDL_Event event;
    
    while (!quit) {
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }

            Atlas::handleEvents(&event);
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
                int newWidth = event.window.data1;
                int newHeight = event.window.data2;

                // Assuming you have access to an instance of the class or namespace where setProjectionMatrix is defined
                Atlas::setProjectionMatrix(newWidth, newHeight);

                glViewport(0, 0, newWidth, newHeight);
            }
            
        }
    
     
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (isshown)
        {
            Atlas::renderUI();
            }
    


        Atlas::renderUI();
        
        SDL_GL_SwapWindow(Atlas::g_window);
    }

    Atlas::Shutdown();
    return 0;
}

