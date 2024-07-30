# Atlas_UI
Atlas Game Engine UI Library(public release)

# UPDATE TO v3.1

1- Atlas_Text - modular text rendering using FreeType and OpenGL to allow for your own fonts 
useage 
```cpp
#include "atlas_text.h" //make sure to include as this is needed to render text in atlas_ui3.0.h

int main...
//start up like normal
 Atlas::Setup("Atlas UI Example", Atlas::SCREEN_WIDTH, Atlas::SCREEN_HEIGHT);
 Atlas::initOpenGL();
 Atlas::setProjectionMatrix(Atlas::SCREEN_WIDTH, Atlas::SCREEN_HEIGHT);
 Atlas::TextRenderer::SetGlobalFont("C:/Windows/Fonts/arial.ttf"); //add this to declare what font you are wanting to use
```

2- Labels added labels for better use of text instead of using Atlas::Text use Atlas::Label
```cpp
Atlas::Label(x,y,"string",fontsize,height, width); 
```

3- Updated most of the UI to use atlas_text.h directly instead of using outdated code from glText 

4 - Added a file browser for the use of file loading etc. 
```cpp
Atlas::File(50, 100, 800, 600, "C:/Windows", nullptr, 16.0f);
```
This will gather directories from C/Windows and allow you to see what is inside these directories. 
# Still a work in progress!! 

# Documentation
https://select-button-games.github.io/Atlas_UI/

# WARNING THIS LIBRARY IS STILL UNDER DEVELOPMENT THERE COULD BE BUGS OR ERRORS THAT PREVENT USE. 

# Example Use
```cpp

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <iostream>
#define SETUP_SDL_OPENGL
#include "atlas_ui3.0.h"
#include "atlas_ui_utilities.h"
#pragma comment(linker,"/ENTRY:mainCRTStartup")

void setupDemoUI() {
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
    
    //DEMO OF ATLAS UI
    setupDemoUI();

 
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



        Atlas::renderUI();
        
        SDL_GL_SwapWindow(Atlas::g_window);
    }

    Atlas::Shutdown();
    return 0;
}



```

![image](https://github.com/user-attachments/assets/a0d48382-b62a-4ca6-b810-5ca24c6d7c71)
![image](https://github.com/user-attachments/assets/d8a8e22b-f1d3-41a2-82c7-a26e9737c6c2)


# Using in your project

Currrently Atlas UI can be easily added to your project it only has a few dependencies: SDL2 and OpenGL3. All you need to do to use it in your project is clone the repo of this github

```
git clone https://github.com/Select-Button-Games/Atlas_UI
```

Once you have cloned it simply add the two header files atlas_ui3.0.h and atlas_ui_uitilites.h to your project, make sure you are linking SDL2 and OpenGL3 to ensure it will function properly. 

Atlas UI can handle the creation of your window for you by simply using 

```cpp
#define SETUP_SDL_OPENGL
#include "atlas_ui3.0.h"
#include "atlas_ui_utilities.h"

int main //psudo code
{
   Atlas::Setup("Atlas UI Example", Atlas::SCREEN_WIDTH, Atlas::SCREEN_HEIGHT);
 // Initialize the UI library
 Atlas::initOpenGL();
 Atlas::setProjectionMatrix(Atlas::SCREEN_WIDTH, Atlas::SCREEN_HEIGHT);


}
```
This will create the SDL window for you, and create the OpenGL context for you as well. 

# License 
Atlas UI is currently being released with no license and no gurantee. I would appreciate credits, but it is not nessicary. 

# BUGS 
Please inform me of any bugs you find and possible changes if you know of a fix you can email me at ajax@selectbuttongames.com 

# Community 
You can join our community where I am developing a 2D Isometric MMORPG called Shine Ville <p>
https://discord.gg/GcFdMrxmZy
