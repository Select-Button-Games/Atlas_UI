#pragma once
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_ttf.h>
#include <vector>
#include <iostream>
#include <functional>
#include <string>
#include <algorithm>
#include <algorithm>
#include <functional>
#include <string>
#include "atlas_ui_utilities.h"
#include "atlas_text.h"
#include <filesystem>

namespace fs = std::filesystem;

namespace Atlas {


  
   

    void setProjectionMatrix(int screenWidth, int screenHeight) {
        projection = glm::ortho(0.0f, static_cast<float>(screenWidth), static_cast<float>(screenHeight), 0.0f);
        glUseProgram(shaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    }


    struct Widget; // Forward declaration
    struct TextRenderer; // Ensure this is forward-declared if its full definition comes later
    struct TextComponent;
    
    struct UIComponent {
        float x = 0.0f, y = 0.0f; // Initialized
        int width = 0, height = 0; // Initialized
        Widget* parent = nullptr;
        virtual void Draw() = 0;
        virtual void handleEvents(SDL_Event* event) = 0;
        virtual void updatePosition(float deltaX, float deltaY) {
            x += deltaX;
            y += deltaY;
        }
    };

    struct DraggableComponent {
        bool isDragging = false;
        int lastMouseX = 0, lastMouseY = 0;
        int offsetX = 0, offsetY = 0;
        Widget* parent = nullptr; // Correctly declared
    };

    struct Widget {
        int x = 0, y = 0, width = 0, height = 0, ID = 0;
        GLuint texture = 0;
        bool isResizing = false;
        bool resizingLeft = false, resizingRight = false, resizingTop = false, resizingBottom = false;
        std::vector<UIComponent*> components;
        DraggableComponent* draggableComponent = nullptr;
        TextComponent* textComponent = nullptr;
        int zOrder = 0;
        bool isActive = true;
        bool isCloseable = false;
        bool isVisable = true;

        glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); //default color
        
       

        void setColor(float r, float g, float b, float a)
        {
            color = glm::vec4(r, g, b, a);
        }
        ~Widget() {
            delete textComponent;
            // Remember to delete components in the vector to avoid memory leaks
            for (auto& component : components) {
                delete component;
            }

            
        }
    };

    struct UIManager {
        std::vector<Widget*> widgets;
        Widget* currentWidget = nullptr; // Track the current widget context
        bool isCreatingWidget = false;

        // Function to determine and set the active widget based on mouse position and z-order
        void setActiveWidget(int mouseX, int mouseY) {
            Widget* topWidget = nullptr;
            int highestZOrder = -1;
            for (auto& widget : widgets) {
                if (mouseX >= widget->x && mouseX <= widget->x + widget->width &&
                    mouseY >= widget->y && mouseY <= widget->y + widget->height) {
                    if (widget->zOrder > highestZOrder) {
                        topWidget = widget;
                        highestZOrder = widget->zOrder;
                    }
                }
            }
            currentWidget = topWidget;
        }
    };

    // Global UIManager instance
    UIManager uiManager;

    // Flags for widget options
    enum WidgetOptions {
        WIDGET_NONE = 0,
        WIDGET_DRAGGABLE = 1 << 0,
        WIDGET_RESIZABLE_LEFT = 1 << 1,
        WIDGET_RESIZABLE_RIGHT = 1 << 2,
        WIDGET_RESIZABLE_TOP = 1 << 3,
        WIDGET_RESIZABLE_BOTTOM = 1 << 4,
        WIDGET_PASSWORD = 1 << 5,
        WIDGET_CLOSEABLE = 1 << 6,
        WIDGET_SHOWN = 1 << 7,
        WIDGET_HIDDEN = 1 << 8
    };





    // Utility function to set flag
    bool hasFlag(int options, WidgetOptions flag) {
        return (options & flag) == flag;
    }

    // Functions for UIComponent
    void updatePosition(UIComponent& component, float deltaX, float deltaY) {
        component.x += deltaX;
        component.y += deltaY;
    }

    // Functions for DraggableComponent
    void handleDrag(DraggableComponent& draggable, SDL_Event* event) {
        Widget& parent = *draggable.parent;
        // Set the active widget based on the current mouse position
        uiManager.setActiveWidget(event->button.x, event->button.y);

        // Proceed only if the parent widget is the current active widget
        if (&parent == uiManager.currentWidget) {
            if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT) {
                int mouseX = event->button.x;
                int mouseY = event->button.y;
                // Check if the mouse is within the bounds of the widget
                if (mouseX > parent.x && mouseX < parent.x + parent.width &&
                    mouseY > parent.y && mouseY < parent.y + parent.height) {
                    draggable.isDragging = true;
                    draggable.offsetX = mouseX - parent.x; // Calculate offsets
                    draggable.offsetY = mouseY - parent.y;
                }
            }
            else if (event->type == SDL_MOUSEBUTTONUP && event->button.button == SDL_BUTTON_LEFT) {
                draggable.isDragging = false;
            }
            else if (event->type == SDL_MOUSEMOTION && draggable.isDragging) {
                int mouseX = event->motion.x;
                int mouseY = event->motion.y;
                // Calculate the delta movement
                int deltaX = mouseX - draggable.offsetX - parent.x;
                int deltaY = mouseY - draggable.offsetY - parent.y;
                // Update widget position
                parent.x = mouseX - draggable.offsetX;
                parent.y = mouseY - draggable.offsetY;
                // Update positions of all components relative to the new widget position
                for (auto component : parent.components) {
                    component->updatePosition(deltaX, deltaY);
                }
            }
        }
    }


    void checkShaderCompileErrors(GLuint shader, const std::string& type) {
        GLint success;
        GLchar infoLog[1024];
        if (type != "PROGRAM") {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
            else {
                std::cout << "Shader compiled successfully" << std::endl;
            }
        }
        else {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
            else {
                std::cout << "Shader program linked successfully" << std::endl;
            }
        }
    }

    // Function to compile a shader
    GLuint compileShader(const char* shaderSource, GLenum shaderType) {
        GLuint shader = glCreateShader(shaderType);
        glShaderSource(shader, 1, &shaderSource, NULL);
        glCompileShader(shader);
        checkShaderCompileErrors(shader, shaderType == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT");
        return shader;
    }

    // Function to create a shader program
    GLuint createShaderProgram(const char* vertexShaderSource, const char* fragmentShaderSource) {
        GLuint vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
        GLuint fragmentShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);

        GLuint shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
        checkShaderCompileErrors(shaderProgram, "PROGRAM");

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return shaderProgram;
    }

  


    const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 projection;
uniform float texOffset; // Assuming you have logic to calculate this based on the frame
uniform int numberOfFrames = 1; // Assuming you have a uniform to specify the number of frames

void main()
{
    vec4 pos = vec4(aPos, 0.0, 1.0); // Extend aPos to a 4-component vector
    gl_Position = projection * model * pos;
    TexCoord.x = (aTexCoord.x + texOffset) / float(numberOfFrames); // Adjusted for clarity
    TexCoord.y = aTexCoord.y;
}

)";

    const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    in vec2 TexCoord;

    uniform sampler2D texture1;
    uniform int useTexture; // Uniform flag to indicate whether to use the texture
    uniform vec4 fallbackColor;

    void main() {
        if (useTexture != 0) {
            FragColor = texture(texture1, TexCoord);
        } else {
            FragColor = fallbackColor;
        }
    }
)";





    // Initialize shader program and VAO, VBO


    void initOpenGL() {
        glewInit();

        shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);
     
        
        float vertices[] = {
            // positions    // texture coords
            0.0f,  1.0f,    0.0f, 1.0f,
            1.0f,  1.0f,    1.0f, 1.0f,
            1.0f,  0.0f,    1.0f, 0.0f,
            0.0f,  0.0f,    0.0f, 0.0f
        };
        unsigned int indices[] = {
            0, 1, 2,
            2, 3, 0
        };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);


        glDisable(GL_DEPTH_TEST);

    }


    ///////////////////////////////////////////////////////////////////////////////
 /////////////// TEXT RENDERING STRUCTS////////////////////////////////////////
 //////////////////////////////////////////////////////////////////////////////
    struct TextComponent : public UIComponent {
        std::string text;
        float fontSize;
        TextRenderer* textRenderer;
        glm::vec3 color;

        TextComponent(const std::string& text, float fontSize, float x = 0.0f, float y = 0.0f, glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f))
            : UIComponent(), text(text), fontSize(fontSize), color(color) {
            this->x = x;
            this->y = y;
            textRenderer = new TextRenderer(fontSize);
        }

        virtual void Draw() override {
            if (textRenderer) {
                float globalX = x;
                float globalY = y;
                if (parent) {
                    globalX += parent->x;
                    globalY += parent->y;
                }
                textRenderer->RenderText(text, globalX, globalY, 1.0f, color);
            }
        }

        virtual void handleEvents(SDL_Event* event) override {
            // Handle events for text component if needed
        }

        virtual void updatePosition(float deltaX, float deltaY) override {
            x += deltaX;
            y += deltaY;
        }

        void setText(const std::string& newText) {
            text = newText;
        }

        ~TextComponent() {
            delete textRenderer;
        }
    };



    //////////////////////////////////////////////////////////////////////////////////////////
    // /////////////////////////////BUTTON COMPONENT OF WIDGETS///////////////////////////////
    // //////////////////////////////////////////////////////////////////////////////////////
    struct ButtonComponent : public UIComponent {
        GLuint texture = 0;
        std::function<void()> onClick;
        bool hasTexture = false; // New flag to indicate if the button has a texture
        int width;
        int height;
        int x;
        int y;
        std::string text;
        float fontSize;
        glm::vec3 textColor;
        glm::vec4 buttonColor = glm::vec4(0.5f, 0.5f, 0.5f,0.5f); // Default button color
        TextRenderer* textRenderer;
        bool isHovered;

        ButtonComponent(const std::string& text, float fontSize, const std::string& texturePath = "", std::function<void()> onClick = nullptr, int width = 100, int height = 50, int x = 0, int y = 0, glm::vec3 textColor = glm::vec3(1.0f, 1.0f, 1.0f))
            : onClick(onClick), text(text), fontSize(fontSize), textColor(textColor), isHovered(false) {

            this->width = width; // Set button width
            this->height = height; // Set button height
            this->x = x;
            this->y = y;

            textRenderer = new TextRenderer(fontSize);

            // Load the texture if a path is provided and it's not empty
            if (!texturePath.empty()) {
                SDL_Surface* surface = IMG_Load(texturePath.c_str());
                if (surface) {
                    glGenTextures(1, &texture);
                    glBindTexture(GL_TEXTURE_2D, texture);

                    GLenum format = surface->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB;
                    glTexImage2D(GL_TEXTURE_2D, 0, format, surface->w, surface->h, 0, format, GL_UNSIGNED_BYTE, surface->pixels);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                    SDL_FreeSurface(surface);
                    hasTexture = true; // Texture successfully loaded
                }
                else {
                    std::cerr << "Failed to load texture " << IMG_GetError() << std::endl;
                }
            }
        }

        virtual void Draw() override {
            // Draw the button texture if it exists
            if (hasTexture) {
                glUseProgram(shaderProgram);
                glBindVertexArray(VAO);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, texture);
                glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
                glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 1);

                float globalX = parent->x + x;
                float globalY = parent->y + y;

                glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(globalX, globalY, 0.0f));
                model = glm::scale(model, glm::vec3(width, height, 1.0f)); // Use button's width and height
                GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
            else {
                glUseProgram(shaderProgram);
                glBindVertexArray(VAO);

                // Draw the border
                glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 0);
                glUniform4f(glGetUniformLocation(shaderProgram, "fallbackColor"), 0.0f, 0.0f, 0.0f, 1.0f); // Border color

                glm::mat4 borderModel = glm::translate(glm::mat4(1.0f), glm::vec3(parent->x + x, parent->y + y, 0.0f));
                borderModel = glm::scale(borderModel, glm::vec3(width + 4.0f, height + 4.0f, 1.0f));
                glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(borderModel));
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

                // Draw the button itself
                glm::vec4 buttonColor = isHovered ? glm::vec4(0.7f, 0.7f, 0.7f, 1.0f) : glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
                glUniform4f(glGetUniformLocation(shaderProgram, "fallbackColor"), buttonColor.r, buttonColor.g, buttonColor.b, buttonColor.a);

                glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(parent->x + x, parent->y + y, 0.0f));
                model = glm::scale(model, glm::vec3(width, height, 1.0f));
                glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }


            // Draw the text directly using TextRenderer
            if (textRenderer) {
                float textWidth = textRenderer->GetTextWidth(text);
                float textHeight = textRenderer->GetTextHeight(text);

                // Compute text positions
                float textX = parent->x + x + (width - textWidth) / 2.0f;  // Center horizontally
                float textY = parent->y + y + (height + textHeight) / 2.0f;  // Center vertically, adjust for baseline

                textRenderer->RenderText(text, textX, textY, 1.0f, textColor);
            }
        }

        virtual void handleEvents(SDL_Event* event) override {
            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);

            bool wasHovered = isHovered;
            isHovered = (mouseX > parent->x + x && mouseX < parent->x + x + width &&
                mouseY > parent->y + y && mouseY < parent->y + y + height);

            // Check for button click
            if (wasHovered && event->type == SDL_MOUSEBUTTONUP && event->button.button == SDL_BUTTON_LEFT) {
                if (onClick) {
                    onClick(); // Call the callback function
                }
            }
        }
        

        virtual void updatePosition(float deltaX, float deltaY) override {
            parent->x += deltaX;
            parent->y += deltaY;
        }

        ~ButtonComponent() {
            delete textRenderer;
        }
    };




    ///////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////LIST BOX COMPONENT//////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////

    struct ListBoxComponent : public UIComponent {
        std::vector<std::string> items; // List of items to display
        std::function<void(const std::string&)> onItemSelected; // Callback function for item selection
        int selectedItemIndex = -1; // Index of the currently selected item, -1 if none
        int hoveredItemIndex = -1; // Index of the item under the mouse cursor
        int x, y;
        int width, height;
        int scrollPosition = 0; // Tracks the current scroll position
        int totalContentHeight = 0; // Total height of the content
        float fontSize;
        TextRenderer* textRenderer; // Use TextRenderer directly

        ListBoxComponent(const std::vector<std::string>& items, std::function<void(const std::string&)> onItemSelected = nullptr, int width = 100, int height = 150, int x = 0, int y = 0, float fontSize = 16.0f)
            : items(items), onItemSelected(onItemSelected), width(width), height(height), x(x), y(y), fontSize(fontSize) {
            textRenderer = new TextRenderer(fontSize); // Initialize TextRenderer
        }

        virtual void Draw() override {
            if (!shaderProgram || !VAO) {
                std::cerr << "Shader program or VAO not initialized." << std::endl;
                return;
            }

            if (!textRenderer) {
                std::cerr << "Text renderer not initialized." << std::endl;
                return;
            }

            glUseProgram(shaderProgram);
            glBindVertexArray(VAO);

            GLint useTextureLoc = glGetUniformLocation(shaderProgram, "useTexture");
            GLint fallbackColorLoc = glGetUniformLocation(shaderProgram, "fallbackColor");
            GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
            GLint projLoc = glGetUniformLocation(shaderProgram, "projection");

            if (useTextureLoc == -1 || fallbackColorLoc == -1 || modelLoc == -1 || projLoc == -1) {
                std::cerr << "Failed to get uniform locations." << std::endl;
                glBindVertexArray(0);
                glUseProgram(0);
                return;
            }

            // Draw the border (black)
            glUniform1i(useTextureLoc, 0); // Not using texture
            glUniform4f(fallbackColorLoc, 0.0f, 0.0f, 0.0f, 1.0f); // Black border color

            // Calculate border dimensions
            float borderWidth = width + 4.0f; // Add 4 for 2 pixels border on each side
            float borderHeight = height + 4.0f;
            glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x - 2.0f, y - 2.0f, 0.0f));
            model = glm::scale(model, glm::vec3(borderWidth, borderHeight, 1.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            // Draw the background (grey)
            glUniform4f(fallbackColorLoc, 0.7f, 0.7f, 0.7f, 1.0f); // Grey background color
            model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));
            model = glm::scale(model, glm::vec3(width, height, 1.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            // Draw each item in the list
            float itemY = y - scrollPosition;
            for (size_t i = 0; i < items.size(); ++i) {
                // Skip items that are not within the visible area
                if (itemY + fontSize < y || itemY > y + height) {
                    itemY += fontSize + 5.0f;
                    continue;
                }

                // Calculate the width and height of the text
                float textWidth = textRenderer->GetTextWidth(items[i]);
                float textHeight = textRenderer->GetTextHeight(items[i]);

                // Center the text horizontally and vertically within the list box item
                float textX = x + (width - textWidth) / 2.0f;
                float textY = itemY + (fontSize - textHeight) / 2.0f;

                glm::vec3 textColor = (i == selectedItemIndex) ? glm::vec3(1.0f, 0.0f, 0.0f) : glm::vec3(1.0f, 1.0f, 1.0f); // Red for selected, white otherwise
                textRenderer->RenderText(items[i], textX, textY, 1.0f, textColor);
                itemY += fontSize + 5.0f; // Adjust spacing between items
            }

            glBindVertexArray(0);
            glUseProgram(0);
        }



        virtual void handleEvents(SDL_Event* event) override {
            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);

            hoveredItemIndex = -1;
            float itemY = y - scrollPosition;
            for (size_t i = 0; i < items.size(); ++i) {
                if (mouseX > x && mouseX < x + width && mouseY > itemY && mouseY < itemY + fontSize) {
                    hoveredItemIndex = i;
                    break;
                }
                itemY += fontSize + 5.0f;
            }

            if (event->type == SDL_MOUSEBUTTONUP && event->button.button == SDL_BUTTON_LEFT) {
                if (hoveredItemIndex != -1) {
                    selectedItemIndex = hoveredItemIndex;
                    if (onItemSelected) {
                        onItemSelected(items[selectedItemIndex]);
                    }
                }
            }

            // Handle mouse wheel scrolling
            if (event->type == SDL_MOUSEWHEEL) {
                int scrollAmount = event->wheel.y * (fontSize + 5); // Adjust this value as needed
                scrollPosition = std::max(0, scrollPosition - scrollAmount);
                totalContentHeight = static_cast<int>(items.size() * (fontSize + 5));
                scrollPosition = std::min(scrollPosition, totalContentHeight - height);
            }
        }

        ~ListBoxComponent() {
            delete textRenderer;
        }
    };



    ////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////CHECK BOX COMPONENT////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////
    struct CheckBoxComponent : public UIComponent {
        int x, y, width = 20, height = 20; // Default size for the checkbox
        bool isChecked;
        std::function<void(bool)> onCheckedChanged;
        std::string labelText;
        TextComponent* textComponent = nullptr;
        int scrollPosistion = 0;
        int totalContentHeight = 0;

        CheckBoxComponent(int x, int y, bool isChecked, std::function<void(bool)> onCheckedChanged, const std::string& labelText)
            : x(x), y(y), isChecked(isChecked), onCheckedChanged(onCheckedChanged), labelText(labelText) {};

        virtual void Draw() override {

            //draw the checkbox box
            glUseProgram(shaderProgram);
            glBindVertexArray(VAO);
            glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 0); // Indicate not using texture
            glUniform4f(glGetUniformLocation(shaderProgram, "fallbackColor"), 0.5f, 0.5f, 0.5f, 1.0f); // Example: Gray color

            glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));
            model = glm::scale(model, glm::vec3(width, height, 1.0f));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
            glUseProgram(0);

            //draw the checkbox checkmark if the checkbox is checked

            if (isChecked) {
                glUseProgram(shaderProgram);
                glBindVertexArray(VAO);
                glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 0); // Indicate not using texture
                glUniform4f(glGetUniformLocation(shaderProgram, "fallbackColor"), 1.0f, 1.0f, 1.0f, 1.0f); // Example: White color

                glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x + 5, y + 5, 0.0f));
                model = glm::scale(model, glm::vec3(width - 10, height - 10, 1.0f));
                glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
                glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);
                glUseProgram(0);
            }



        }

        virtual void handleEvents(SDL_Event* event) override {
            if (event->type == SDL_MOUSEBUTTONDOWN) {
                int mouseX = event->button.x;
                int mouseY = event->button.y;

                // Check if the click is within the checkbox bounds
                if (mouseX >= x && mouseX <= x + width && mouseY >= y && mouseY <= y + height) {
                    // Toggle the checkbox state
                    isChecked = !isChecked;

                    // Call the onCheckedChanged callback if provided
                    if (onCheckedChanged) {
                        onCheckedChanged(isChecked);
                    }
                }
            }
        }
        virtual void updatePosition(float deltaX, float deltaY) override {
            // First, call the base class method to update the button's position
            UIComponent::updatePosition(deltaX, deltaY);

            //Update the checkbox x and y based on the widgets movement
            x += deltaX;
            y += deltaY;
        }
    };

    ///////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////PROGRESS BAR///////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////
    struct ProgressBarComponent : public UIComponent {
        float progress = 0.0f; // Progress value between 0.0 and 1.0
        int x, y;
        int width, height;
        std::function<void(bool)> isComplete;

        ProgressBarComponent(int x, int y, int width, int height, std::function<void(bool)> isComplete, float initProgress = 0.0f)
            : x(x), y(y), width(width), height(height), isComplete(isComplete), progress(initProgress) {}

        virtual void Draw() override {
            glUseProgram(shaderProgram);
            glBindVertexArray(VAO);

            glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 0); // Not using texture
            glUniform4f(glGetUniformLocation(shaderProgram, "fallbackColor"), 0.5f, 0.5f, 0.5f, 1.0f); // Background color

            // Draw background
            glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));
            model = glm::scale(model, glm::vec3(width, height, 1.0f));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            // Draw filled portion
            glUniform4f(glGetUniformLocation(shaderProgram, "fallbackColor"), 0.0f, 0.7f, 0.0f, 1.0f); // Filled color
            model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));
            model = glm::scale(model, glm::vec3(width * progress, height, 1.0f)); // Scale based on progress
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            glBindVertexArray(0);
            glUseProgram(0);
        }

        virtual void handleEvents(SDL_Event* event) override {
            // Example: Update progress on mouse click within the progress bar bounds
            if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT) {
                int mouseX = event->button.x;
                int mouseY = event->button.y;
                // Check if the click is within the progress bar's bounds
                if (mouseX > x && mouseX < x + width && mouseY > y && mouseY < y + height) {
                    // Calculate new progress based on the click position
                    float newProgress = static_cast<float>(mouseX - x) / static_cast<float>(width);
                    setProgress(newProgress);
                }
            }
        }

        virtual void updatePosition(float deltaX, float deltaY) override {
            // First, call the base class method to update the button's position
            UIComponent::updatePosition(deltaX, deltaY);

            // Update the progress bar's position
            x += deltaX;
            y += deltaY;
        }

        void setProgress(float newProgress) {
            progress = std::clamp(newProgress, 0.0f, 1.0f); // Ensure progress is between 0.0 and 1.0
            if (progress >= 1.0f && isComplete) {
                isComplete(true); // Call the completion callback if progress is complete
            }
        }

        void incrementProgress(float delta) {
            setProgress(progress + delta); // Increment progress by delta
        }
    };


    //////////////////////////////////////////////////////////////////////////////////////////////////////
   //////////////////////////////TEXT BOX COMPONENT/////////////////////////////////////////////////////
   //////////////////////////////////////////////////////////////////////////////////////////////////////
    struct TextBoxComponent : public UIComponent {
        int x, y;
        int width, height;
        std::string text;
        std::function<void(const std::string&)> onTextChanged;
        TextComponent* textComponent = nullptr;
        bool isFocused = false;
        float transparency;
        int scrollPosition = 0;
        int lineHeight = 20;
        int maxCharsPerLine = 0;
        int scrollSpeed = 5;
        int totalTextHeight = 0;

        int selectionStart = -1, selectionEnd = -1;
        bool isSelecting = false;
        float fontSize;
        TextBoxComponent(int x, int y, int width, int height, std::function<void(const std::string&)> onTextChanged, const std::string& text = "", float transparency = 1.0f)
            : x(x), y(y), width(width), height(height), onTextChanged(onTextChanged), text(text), transparency(transparency) {
            textComponent = new TextComponent(text, fontSize,x, y);
            maxCharsPerLine = width / 28;
        }

        std::string wrapText(const std::string& text, int maxCharsPerLine) {
            std::string wrappedText;
            int lineLength = 0;
            for (char c : text) {
                if (lineLength >= maxCharsPerLine) {
                    wrappedText += '\n';
                    lineLength = 0;
                }
                wrappedText += c;
                lineLength++;
            }
            return wrappedText;
        }

        virtual void Draw() override {
            // Draw the text box background
            glUseProgram(shaderProgram);
            glBindVertexArray(VAO);
            glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 0); // Indicate not using texture
            // Use transparency when setting the background color
            glUniform4f(glGetUniformLocation(shaderProgram, "fallbackColor"), 0.5f, 0.5f, 0.5f, transparency); // Use transparency here

            glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));
            model = glm::scale(model, glm::vec3(width, height, 1.0f));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
            glUseProgram(0);

            // Update the text component with the wrapped text
            textComponent->text = wrapText(text, maxCharsPerLine);
            // Adjust the text component's position
            textComponent->x = x + 5;
            textComponent->y = y + 5 - scrollPosition;
            // Enable scissor test for clipping
            glEnable(GL_SCISSOR_TEST);
            int scissorY = SCREEN_HEIGHT - (y + height);
            glScissor(x, scissorY, width, height);
            textComponent->Draw();
            glDisable(GL_SCISSOR_TEST);

            // Calculate if scrolling is needed
            totalTextHeight = static_cast<int>((textComponent->text.length() / maxCharsPerLine + 1) * lineHeight);

            if (totalTextHeight > height) {
                // Calculate the scroll bar height based on the content height
                float scrollBarHeight = (height / static_cast<float>(totalTextHeight)) * height;
                // Calculate the scroll bar position based on the scroll position
                float scrollBarY = y + (scrollPosition / static_cast<float>(totalTextHeight)) * height;
                // Draw the scroll bar
                glUseProgram(shaderProgram);
                glBindVertexArray(VAO);
                glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 0); // Indicate not using texture
                glUniform4f(glGetUniformLocation(shaderProgram, "fallbackColor"), 1.5f, 0.5f, 0.5f, 1.0f); // Gray color
                glm::mat4 scrollBarModel = glm::translate(glm::mat4(1.0f), glm::vec3(x + width - 10.0f, scrollBarY, 0.0f));
                scrollBarModel = glm::scale(scrollBarModel, glm::vec3(10.0f, scrollBarHeight, 1.0f));
                glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(scrollBarModel));
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);
                glUseProgram(0);
            }

            // Draw the cursor when the box is in focus
            if (isFocused) {
                glUseProgram(shaderProgram);
                glBindVertexArray(VAO);
                glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 0); // Indicate not using texture
                glUniform4f(glGetUniformLocation(shaderProgram, "fallbackColor"), 1.0f, 1.0f, 1.0f, 1.0f); // Example: White color

                // Calculate cursor position based on text length
                int cursorPos = (text.length() % maxCharsPerLine) * 10 + 10; // Assuming each character is approx 10 pixels wide
                int cursorYOffset = (text.length() / maxCharsPerLine) * lineHeight;

                glm::mat4 cursorModel = glm::translate(glm::mat4(1.0f), glm::vec3(x + cursorPos, y + 5 + cursorYOffset - scrollPosition, 0.0f));
                cursorModel = glm::scale(cursorModel, glm::vec3(2, lineHeight - 10, 1.0f));
                glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(cursorModel));
                glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);
                glUseProgram(0);

                if (isSelecting || (selectionStart != -1 && selectionEnd != -1)) {
                    // Draw highlight background for selected text
                    int startLine = selectionStart / maxCharsPerLine;
                    int endLine = selectionEnd / maxCharsPerLine;
                    int startOffset = selectionStart % maxCharsPerLine;
                    int endOffset = selectionEnd % maxCharsPerLine;

                    for (int line = startLine; line <= endLine; ++line) {
                        int highlightX = x + ((line == startLine) ? startOffset : 0) * 10 + 5;
                        int highlightEndX = x + ((line == endLine) ? endOffset : maxCharsPerLine) * 10 + 5;
                        int highlightWidth = highlightEndX - highlightX;
                        int highlightY = y + line * lineHeight + 5 - scrollPosition;
                        int highlightHeight = lineHeight;

                        glUseProgram(shaderProgram);
                        glBindVertexArray(VAO);
                        glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 0); // Indicate not using texture
                        glUniform4f(glGetUniformLocation(shaderProgram, "fallbackColor"), 0.0f, 0.0f, 1.0f, 0.5f); // Blue color with 50% transparency
                        glm::mat4 highlightModel = glm::translate(glm::mat4(1.0f), glm::vec3(highlightX, highlightY, 0.0f));
                        highlightModel = glm::scale(highlightModel, glm::vec3(highlightWidth, highlightHeight, 1.0f));
                        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(highlightModel));
                        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
                        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                        glBindVertexArray(0);
                        glUseProgram(0);
                    }
                }
            }
        }

        virtual void handleEvents(SDL_Event* event) override {
            if (event->type == SDL_TEXTINPUT && isFocused) {
                if (isSelecting && selectionStart != selectionEnd) {
                    deleteSelectedText();
                }
                text.insert(selectionStart, event->text.text);
                selectionStart += strlen(event->text.text);
                selectionEnd = selectionStart;
                textComponent->text = text;
                if (onTextChanged) {
                    onTextChanged(text);
                }
            }
            else if (event->type == SDL_KEYDOWN && isFocused) {
                if (event->key.keysym.sym == SDLK_BACKSPACE && !text.empty()) {
                    if (isSelecting && selectionStart != selectionEnd) {
                        deleteSelectedText();
                    }
                    else if (selectionStart > 0) {
                        text.erase(selectionStart - 1, 1);
                        selectionStart--;
                        selectionEnd = selectionStart;
                    }
                    textComponent->text = text;
                    if (onTextChanged) {
                        onTextChanged(text);
                    }
                }
                else if (event->key.keysym.sym == SDLK_c && SDL_GetModState() & KMOD_CTRL) {
                    std::string selectedText = getSelectedText();
                    SDL_SetClipboardText(selectedText.c_str());
                }
                else if (event->key.keysym.sym == SDLK_v && SDL_GetModState() & KMOD_CTRL) {
                    if (SDL_HasClipboardText()) {
                        char* clipboardText = SDL_GetClipboardText();
                        if (isSelecting && selectionStart != selectionEnd) {
                            deleteSelectedText();
                        }
                        text.insert(selectionStart, clipboardText);
                        selectionStart += strlen(clipboardText);
                        selectionEnd = selectionStart;
                        SDL_free(clipboardText);
                        textComponent->text = text;
                        if (onTextChanged) {
                            onTextChanged(text);
                        }
                    }
                }
                else if (event->key.keysym.sym == SDLK_RETURN || event->key.keysym.sym == SDLK_KP_ENTER) {
                    if (isSelecting && selectionStart != selectionEnd) {
                        deleteSelectedText();
                    }
                    text.insert(selectionStart, "\n");
                    selectionStart++;
                    selectionEnd = selectionStart;
                    textComponent->text = text;
                    if (onTextChanged) {
                        onTextChanged(text);
                    }
                }
            }
            else if (event->type == SDL_MOUSEBUTTONDOWN) {
                int mouseX = event->button.x;
                int mouseY = event->button.y;
                if (mouseX > x && mouseX < x + width && mouseY > y && mouseY < y + height) {
                    isFocused = true;
                    isSelecting = true;
                    selectionStart = calculateTextIndexAtPosition(event->button.x, event->button.y);
                    selectionEnd = selectionStart;
                }
                else {
                    isFocused = false;
                    isSelecting = false;
                    selectionStart = selectionEnd = -1;
                }
            }
            else if (event->type == SDL_MOUSEBUTTONUP) {
                isSelecting = false;
            }
            else if (event->type == SDL_MOUSEMOTION && isSelecting) {
                selectionEnd = calculateTextIndexAtPosition(event->motion.x, event->motion.y);
            }
            else if (event->type == SDL_MOUSEWHEEL && isFocused) {
                scrollPosition += event->wheel.y * scrollSpeed;
                scrollPosition = std::max(0, std::min(scrollPosition, totalTextHeight - height));
            }
        }

        int calculateTextIndexAtPosition(int mouseX, int mouseY) {
            int line = (mouseY - y + scrollPosition) / lineHeight;
            int charIndex = (mouseX - x) / 10; // Assuming each character is approx 10 pixels wide
            int totalIndex = line * maxCharsPerLine + charIndex;
            return std::min(totalIndex, static_cast<int>(text.length()));
        }

        std::string getSelectedText() {
            if (selectionStart != -1 && selectionEnd != -1 && selectionStart != selectionEnd) {
                int start = std::min(selectionStart, selectionEnd);
                int end = std::max(selectionStart, selectionEnd);
                return text.substr(start, end - start);
            }
            return "";
        }

        void deleteSelectedText() {
            if (selectionStart != -1 && selectionEnd != -1 && selectionStart != selectionEnd) {
                int start = std::min(selectionStart, selectionEnd);
                int end = std::max(selectionStart, selectionEnd);
                text.erase(start, end - start);
                selectionStart = start;
                selectionEnd = selectionStart;
            }
        }

        virtual void updatePosition(float deltaX, float deltaY) override {
            UIComponent::updatePosition(deltaX, deltaY);
            x += deltaX;
            y += deltaY;
        }
    };


    //////////////////////////////////////////////////////////////////////////////////
   /////////////////////////TEXT INPUT BOX//////////////////////////////////////////
   //////////////////////////////////////////////////////////////////////////////////
    struct TextInputBoxComponent : public UIComponent {
        int x, y;
        int width, height;
        std::string text;
        std::function<void(const std::string&)> onTextChanged;
        TextRenderer* textRenderer = nullptr; // Use TextRenderer from atlas_text.h
        bool isFocused = false;
        int options;
        float fontSize;

        // Declare missing variables
        bool isSelecting = false;
        int selectionStart = -1;
        int selectionEnd = -1;
        int scrollPosition = 0;
        int scrollSpeed = 10; // Example value, adjust as needed
        int totalTextHeight = 1000; // Example value, adjust as needed
        int lineHeight = 20; // Example value, adjust as needed
        int maxCharsPerLine = 50; // Example value, adjust as needed

        TextInputBoxComponent(int x, int y, int width, int height, std::function<void(const std::string&)> onTextChanged, const std::string& text = "", int options = 0)
            : x(x), y(y), width(width), height(height), onTextChanged(onTextChanged), text(text), options(options), fontSize(24.0f) { // Initialize fontSize
            textRenderer = new TextRenderer(fontSize); // Initialize TextRenderer
        }

        virtual void Draw() override {
            // Draw the box for the input box
            glUseProgram(shaderProgram);
            glBindVertexArray(VAO);
            glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 0); // Indicate not using texture

            glUniform4f(glGetUniformLocation(shaderProgram, "fallbackColor"), 0.5f, 0.5f, 0.5f, 1.0f); // Example: Gray color

            glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));
            model = glm::scale(model, glm::vec3(width, height, 1.0f));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
            glUseProgram(0);

            // Render the text using TextRenderer
            textRenderer->RenderText(text, x + 5, y + height / 2, 1.0f, glm::vec3(0.0f, 0.0f, 0.0f)); // Adjust position and scale as needed
        }

        virtual void handleEvents(SDL_Event* event) override {
            if (event->type == SDL_MOUSEBUTTONDOWN) {
                if (event->button.x >= x && event->button.x <= x + width &&
                    event->button.y >= y && event->button.y <= y + height) {
                    isFocused = true;
                    isSelecting = true;
                    selectionStart = calculateTextIndexAtPosition(event->button.x, event->button.y);
                    selectionEnd = selectionStart;
                }
                else {
                    isFocused = false;
                    isSelecting = false;
                    selectionStart = selectionEnd = -1;
                }
            }
            else if (event->type == SDL_MOUSEBUTTONUP) {
                isSelecting = false;
            }
            else if (event->type == SDL_MOUSEMOTION && isSelecting) {
                selectionEnd = calculateTextIndexAtPosition(event->motion.x, event->motion.y);
            }
            else if (event->type == SDL_MOUSEWHEEL && isFocused) {
                scrollPosition += event->wheel.y * scrollSpeed;
                scrollPosition = std::max(0, std::min(scrollPosition, totalTextHeight - height));
            }
            else if (event->type == SDL_TEXTINPUT && isFocused) {
                text.insert(selectionEnd, event->text.text);
                selectionEnd += strlen(event->text.text);
                onTextChanged(text);
            }
            else if (event->type == SDL_KEYDOWN && isFocused) {
                if (event->key.keysym.sym == SDLK_BACKSPACE && selectionEnd > 0) {
                    text.erase(selectionEnd - 1, 1);
                    selectionEnd--;
                    onTextChanged(text);
                }
            }
        }

        int calculateTextIndexAtPosition(int mouseX, int mouseY) {
            int line = (mouseY - y + scrollPosition) / lineHeight;
            int charIndex = (mouseX - x) / 10; // Assuming each character is approx 10 pixels wide
            int totalIndex = line * maxCharsPerLine + charIndex;
            return std::min(totalIndex, static_cast<int>(text.length()));
        }

        std::string getSelectedText() {
            if (selectionStart != -1 && selectionEnd != -1 && selectionStart != selectionEnd) {
                int start = std::min(selectionStart, selectionEnd);
                int end = std::max(selectionStart, selectionEnd);
                return text.substr(start, end - start);
            }
            return "";
        }

        void deleteSelectedText() {
            if (selectionStart != -1 && selectionEnd != -1 && selectionStart != selectionEnd) {
                int start = std::min(selectionStart, selectionEnd);
                int end = std::max(selectionStart, selectionEnd);
                text.erase(start, end - start);
                selectionStart = start;
                selectionEnd = selectionStart;
            }
        }

        virtual void updatePosition(float deltaX, float deltaY) override {
            UIComponent::updatePosition(deltaX, deltaY);
            x += deltaX;
            y += deltaY;
        }
    };



    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////IMAGE COMPONENT////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    struct ImageComponent : public UIComponent {
        int width, height;
        int x, y;
        GLuint texture;

        ImageComponent(int x, int y, int width, int height, const std::string& imagePath)
            : x(x), y(y), width(width), height(height) {
            SDL_Surface* surface = IMG_Load(imagePath.c_str());
            if (surface) {
                glGenTextures(1, &texture);
                glBindTexture(GL_TEXTURE_2D, texture);

                GLenum format;
                if (surface->format->BytesPerPixel == 4) {
                    format = GL_RGBA;
                }
                else {
                    format = GL_RGB;
                }

                glTexImage2D(GL_TEXTURE_2D, 0, format, surface->w, surface->h, 0, format, GL_UNSIGNED_BYTE, surface->pixels);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                SDL_FreeSurface(surface);
            }
            else {
                std::cerr << "Failed to load texture: " << IMG_GetError() << std::endl;
            }
        }

        virtual void Draw() override {
            glUseProgram(shaderProgram);
            glBindVertexArray(VAO);

            glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 1); // Indicate using texture
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture);
            glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);

            glUniform4f(glGetUniformLocation(shaderProgram, "fallbackColor"), 1.0f, 1.0f, 1.0f, 1.0f); // White color

            glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));
            model = glm::scale(model, glm::vec3(width, height, 1.0f));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            glBindVertexArray(0);
            glUseProgram(0);
        }

        virtual void handleEvents(SDL_Event* event) override {
            // No event handling for the image component
        }

        virtual void updatePosition(float deltaX, float deltaY) override {
            // First, call the base class method to update the button's position
            x += deltaX;
            y += deltaY;
        }
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////ANIMATED IMAGE COMPONENT DO NO USE DOES NOT WORK CORRECTLY//////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    struct animatedImage : public UIComponent {
        int x, y;
        int width, height;
        int frameWidth, frameHeight; // Size of each frame
        int frames; // Total number of frames in the sprite sheet
        int keyFrame = 0;
        int currentFrame = 0; // Current frame to display
        GLuint texture;
        float frameDuration; // Duration of each frame in seconds
        float elapsedTime = 0.0f; // Time elapsed since the last frame change
        float deltaTime = 0.0f;

        animatedImage(int x, int y, int width, int height, const std::string& imagePath, int frames, float frameDuration)
            : x(x), y(y), width(width), height(height), frames(frames), frameDuration(frameDuration) {
            SDL_Surface* surface = IMG_Load(imagePath.c_str());
            if (surface) {
                glGenTextures(1, &texture);
                glBindTexture(GL_TEXTURE_2D, texture);

                GLenum format;
                if (surface->format->BytesPerPixel == 4) {
                    format = GL_RGBA;
                }
                else {
                    format = GL_RGB;
                }

                glTexImage2D(GL_TEXTURE_2D, 0, format, surface->w, surface->h, 0, format, GL_UNSIGNED_BYTE, surface->pixels);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                frameWidth = surface->w / frames; // Assuming all frames are evenly spaced in the sprite sheet
                frameHeight = surface->h;

                SDL_FreeSurface(surface);
            }
            else {
                std::cerr << "Failed to load texture: " << IMG_GetError() << std::endl;
            }
        }

        virtual void Draw() override {
            elapsedTime += deltaTime;
            if (elapsedTime >= frameDuration) {
                currentFrame = (currentFrame + 1) % frames;
                elapsedTime = 0.0f;
            }

            // Calculate the texture offset for the current frame
            float texOffset = (float)currentFrame / (float)frames;

            // Bind the texture
            glUseProgram(shaderProgram);
            glBindVertexArray(VAO);
            glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 1); // Indicate using texture
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture);
            glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);

            // Pass the texOffset to the shader
            glUniform1f(glGetUniformLocation(shaderProgram, "texOffset"), texOffset);

            glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));
            model = glm::scale(model, glm::vec3(frameWidth, frameHeight, 1.0f));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            glBindVertexArray(0);
            glUseProgram(0);
        }

        virtual void handleEvents(SDL_Event* event) override {
            //Iterate over each frame to cause image to be animated
            if (event->type == SDL_MOUSEBUTTONDOWN) {
                keyFrame++;
                if (keyFrame >= frames) {
                    keyFrame = 0;
                }
            }
        }

        virtual void updatePosition(float deltaX, float deltaY) override {
            x += deltaX;
            y += deltaY;
        }
    };





    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////TAB COMPONENT//////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    struct Tab : public UIComponent {
        int x, y;
        int width, height;

        std::string text;
        std::function<void()> onClick;
        TextComponent* textComponent = nullptr;
        float fontSize;
        Tab(int x, int y, int width, int height, const std::string& text, std::function<void()> onClick)
            : x(x), y(y), width(width), height(height), text(text), onClick(onClick) {
            textComponent = new TextComponent(text, fontSize,x, y);
        }

        virtual void Draw() override {
            // Draw the tab background
            glUseProgram(shaderProgram);
            glBindVertexArray(VAO);
            glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 0); // Indicate not using texture
            glUniform4f(glGetUniformLocation(shaderProgram, "fallbackColor"), 0.5f, 0.5f, 0.5f, 1.0f); // Example: Gray color

            // Render a border around the tab
            glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x - 2, y - 2, 0.0f));
            model = glm::scale(model, glm::vec3(width + 4, height + 4, 1.0f));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            // Draw the tab itself
            model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f)); // Modify the existing 'model' variable
            model = glm::scale(model, glm::vec3(width, height, 1.0f));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            //draw the text on the tab

            if (textComponent) {
                textComponent->x = this->x;
                textComponent->y = this->y;
                textComponent->Draw();
            }

            glBindVertexArray(0);
            glUseProgram(0);
        }

        virtual void handleEvents(SDL_Event* event) override {
            if (event->type == SDL_MOUSEBUTTONDOWN) {
                int mouseX = event->button.x;
                int mouseY = event->button.y;
                if (mouseX > x && mouseX < x + width && mouseY > y && mouseY < y + height) {
                    if (onClick) {
                        onClick();
                    }
                }
            }
        }

        virtual void updatePosition(float deltaX, float deltaY) override {
            // First, call the base class method to update the button's position
            x += deltaX;
            y += deltaY;
        }
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////FILE BROWSER COMPONENT//////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    struct FileBrowser : public UIComponent {
        int x, y;
        int width, height;
        std::string currentPath; // Base path for the sidebar directory list
        std::vector<std::string> directories; // Static list of directories
        std::vector<std::string> files; // Dynamic file list
        std::function<void(const std::string&)> onFileSelected;
        TextComponent* textComponent = nullptr;
        float fontSize;
        TextRenderer* textRenderer;
        int sidebarScrollOffset = 0; // Separate scroll offset for the sidebar
        int fileListScrollOffset = 0; // Separate scroll offset for the main file list area

        std::string selectedDirectory; // Track the currently selected directory

        bool isMouseOverSidebar;
        bool isMouseOverFileList;

        ImageComponent* directoryIcon = nullptr; // Add an ImageComponent for the directory icon

        FileBrowser(int x, int y, int width, int height, const std::string& currentPath, float fontSize, std::function<void(const std::string&)> onFileSelected)
            : x(x), y(y), width(width), height(height), currentPath(currentPath), fontSize(fontSize), onFileSelected(onFileSelected) {
            textComponent = new TextComponent(currentPath, fontSize, x, y);
            textRenderer = new TextRenderer(fontSize);
            directoryIcon = new ImageComponent(0, 0, 16, 16, "UI/directory.png"); // Load the directory icon
            updateDirectoryList();
            updateFileList(); // Initialize file list (empty at start)
        }

        ~FileBrowser() {
            delete textRenderer;
            delete textComponent;
            delete directoryIcon; // Clean up the directory icon
        }

        void updateDirectoryList() {
            directories.clear();
            if (!currentPath.empty()) {
                try {
                    for (const auto& entry : fs::directory_iterator(currentPath)) {
                        if (fs::is_directory(entry.path())) {
                            directories.push_back(entry.path().filename().string());
                        }
                    }
                }
                catch (const fs::filesystem_error& e) {
                    std::cerr << "Filesystem error: " << e.what() << std::endl;
                }
            }
        }

        void updateFileList() {
            files.clear();
            if (!selectedDirectory.empty()) {
                try {
                    for (const auto& entry : fs::directory_iterator(selectedDirectory)) {
                        files.push_back(entry.path().filename().string());
                    }
                }
                catch (const fs::filesystem_error& e) {
                    std::cerr << "Filesystem error: " << e.what() << std::endl;
                }
            }
        }

        virtual void Draw() override {
            // Use the shader program
            glUseProgram(shaderProgram);
            glBindVertexArray(VAO);
            glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 0); // Not using texture

            // Render the border (e.g., light gray)
            glUniform4f(glGetUniformLocation(shaderProgram, "fallbackColor"), 0.9f, 0.9f, 0.9f, 1.0f);
            glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x - 1, y - 1, 0.0f));
            model = glm::scale(model, glm::vec3(width + 2, height + 2, 1.0f));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            // Render the main background (e.g., white or very light gray)
            glUniform4f(glGetUniformLocation(shaderProgram, "fallbackColor"), 0.97f, 0.97f, 0.97f, 1.0f);
            model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));
            model = glm::scale(model, glm::vec3(width, height, 1.0f));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            // Render the sidebar (e.g., light blue or gray) - Static directory list
            glUniform4f(glGetUniformLocation(shaderProgram, "fallbackColor"), 0.85f, 0.85f, 0.90f, 1.0f);
            model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));
            model = glm::scale(model, glm::vec3(width / 4, height, 1.0f));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            // Render the main file list area (e.g., slightly darker than background)
            glUniform4f(glGetUniformLocation(shaderProgram, "fallbackColor"), 0.94f, 0.94f, 0.94f, 1.0f);
            model = glm::translate(glm::mat4(1.0f), glm::vec3(x + width / 4, y, 0.0f));
            model = glm::scale(model, glm::vec3(width * 3 / 4, height, 1.0f));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            // Render the header/toolbar (e.g., dark gray)
            glUniform4f(glGetUniformLocation(shaderProgram, "fallbackColor"), 0.6f, 0.6f, 0.6f, 1.0f);
            model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y + height - 20, 0.0f)); // Assuming header height of 20
            model = glm::scale(model, glm::vec3(width, 20, 1.0f));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            // Render the text in the sidebar (directory list)
            float textX = x + 10; // Adjust as needed
            float textY = y + 30 - sidebarScrollOffset; // Adjust initial position with scrollOffset
            int textHeight = textRenderer->GetTextHeight("A") + 5; // Calculate text height including spacing
            int iconSize = 16; // Assuming icon size is 16x16
            for (const auto& directory : directories) {
                
                // Skip rendering text above the viewable area
                if (textY + textHeight < y) {
                    textY += textHeight;
                    continue;
                }
                // Stop rendering if below the viewable area
                if (textY > y + height) break;

                directoryIcon->x = x + 5;
                directoryIcon->y = textY;
                directoryIcon->Draw(); // Draw the directory icon

                textRenderer->RenderText(directory, textX, textY, 1.0f, glm::vec3(0.0f, 0.0f, 0.0f));
                textY += textHeight; // Adjust spacing as needed
            }

            // Render the text in the main file list area (only if a directory is selected)
            if (!selectedDirectory.empty()) {
                textX = x + width / 4 + 10; // Adjust position for the main file list box
                textY = y + 30 - fileListScrollOffset; // Adjust initial position with scrollOffset

                for (const auto& file : files) {
                    // Skip rendering text above the viewable area
                    if (textY + textHeight < y) {
                        textY += textHeight;
                        continue;
                    }
                    // Stop rendering if below the viewable area
                    if (textY > y + height) break;

                    textRenderer->RenderText(file, textX, textY, 1.0f, glm::vec3(0.0f, 0.0f, 0.0f));
                    textY += textHeight; // Adjust spacing as needed
                }
            }

            glBindVertexArray(0);
            glUseProgram(0);
        }
        virtual void handleEvents(SDL_Event* event) override {
            if (event->type == SDL_MOUSEBUTTONDOWN) {
                int mouseX = event->button.x;
                int mouseY = event->button.y;

               

                // Check if clicked in the sidebar
                if (mouseX > x && mouseX < x + width / 4 && mouseY > y && mouseY < y + height) {
                    int directoryIndex = (mouseY - y + sidebarScrollOffset) / (textRenderer->GetTextHeight("A") + 5);
                 
                    if (directoryIndex >= 0 && directoryIndex < directories.size()) {
                        std::string selectedDir = directories[directoryIndex];
                        std::string fullPath = currentPath + "/" + selectedDir;

                        try {
                            if (std::filesystem::exists(fullPath) && std::filesystem::is_directory(fullPath)) {
                                selectedDirectory = fullPath;
                                textComponent->text = selectedDirectory;
                                if (onFileSelected) {
                                    onFileSelected(selectedDirectory);
                                }
                                updateFileList();
                            }
                            else {
                                std::cerr << "Selected path is not a directory or does not exist: " << fullPath << std::endl;
                            }
                        }
                        catch (const std::filesystem::filesystem_error& e) {
                            std::cerr << "Filesystem error: " << e.what() << std::endl;
                        }
                    }
                }
            }

            if (event->type == SDL_MOUSEMOTION) {
                int mouseX = event->motion.x;
                int mouseY = event->motion.y;

                // Check if the mouse is over the sidebar
                if (mouseX > x && mouseX < x + width / 4 && mouseY > y && mouseY < y + height) {
                    isMouseOverSidebar = true;
                    isMouseOverFileList = false;
                }
                // Check if the mouse is over the main file list area
                else if (mouseX > x + width / 4 && mouseX < x + width && mouseY > y && mouseY < y + height) {
                    isMouseOverSidebar = false;
                    isMouseOverFileList = true;
                }
                else {
                    isMouseOverSidebar = false;
                    isMouseOverFileList = false;
                }
            }

            if (event->type == SDL_MOUSEWHEEL) {
                if (isMouseOverSidebar) {
                    sidebarScrollOffset -= event->wheel.y * 20;
                    if (sidebarScrollOffset < 0) sidebarScrollOffset = 0;
                    int maxScroll = std::max(0, static_cast<int>(directories.size() * (textRenderer->GetTextHeight("A") + 5) - height));
                    if (sidebarScrollOffset > maxScroll) sidebarScrollOffset = maxScroll;
                   
                }

                if (isMouseOverFileList) {
                    fileListScrollOffset -= event->wheel.y * 20;
                    if (fileListScrollOffset < 0) fileListScrollOffset = 0;
                    int maxScroll = std::max(0, static_cast<int>(files.size() * (textRenderer->GetTextHeight("A") + 5) - height));
                    if (fileListScrollOffset > maxScroll) fileListScrollOffset = maxScroll;
                 
                }
            }
        }





        virtual void updatePosition(float deltaX, float deltaY) override {
            x += deltaX;
            y += deltaY;
        }
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////LABEL COMPONENT//////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    struct LabelComponent : public UIComponent {
        std::string text;
        TextRenderer* textRenderer = nullptr;
        float fontSize;
        glm::vec3 textColor;

        LabelComponent(int x, int y, const std::string& text, float fontSize, int width, int height, glm::vec3 textColor = glm::vec3(1.0f, 0.0f, 1.0f))
            : text(text), fontSize(fontSize), textColor(textColor) {
            this->x = x;
            this->y = y;
            this->width = width;
            this->height = height;
            textRenderer = new TextRenderer(fontSize);
        }

        virtual void Draw() override {
            if (!textRenderer) {
                std::cerr << "Error: textRenderer is not initialized." << std::endl;
                return;
            }

            glUseProgram(shaderProgram);
            glBindVertexArray(VAO);
            glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 0);
            glUniform4f(glGetUniformLocation(shaderProgram, "fallbackColor"), 0.9f, 0.9f, 0.9f, 1.0f);

            glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));
            model = glm::scale(model, glm::vec3(width, height, 1.0f));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            if (textRenderer) {
                float textWidth = textRenderer->GetTextWidth(text);
                float textHeight = textRenderer->GetTextHeight(text);

                float textX = x + (width - textWidth) / 2.0f;
                float textY = y + (height + textHeight) / 2.0f;

                if (parent) {
                    textX += parent->x;
                    textY += parent->y;
                }

                textRenderer->RenderText(text, textX, textY, 1.0f, textColor);
            }
        }

        virtual void handleEvents(SDL_Event* event) override {
            // No event handling for the label component
        }

        virtual void updatePosition(float deltaX, float deltaY) override {
            x += deltaX;
            y += deltaY;
        }
        ~LabelComponent() {
            delete textRenderer;
        }
    };



    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////NO COMPONENTS BEYOND THIS POINT!///////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Functions for Widget
    void drawWidget(const Widget& widget) {
        glUseProgram(shaderProgram);

        // Set up transformation and projection matrices
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(widget.x, widget.y, 0.0f));
        model = glm::scale(model, glm::vec3(widget.width, widget.height, 1.0f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        // Bind texture if available
        int useTextureUniform = glGetUniformLocation(shaderProgram, "useTexture");
        if (widget.texture) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, widget.texture);
            glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
            glUniform1i(useTextureUniform, 1);
        }
        else {
            glUniform1i(useTextureUniform, 0);
            glUniform4f(glGetUniformLocation(shaderProgram, "fallbackColor"), widget.color.r, widget.color.g, widget.color.b, widget.color.a);
        }

        // Draw the widget's base rectangle
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        //draw each component after the widget
        for (auto component : widget.components) {
            component->Draw();
        }

        //if it has text make sure we draw that as well
        if (widget.textComponent) {
            widget.textComponent->Draw();
        }

       
        glBindVertexArray(0);
        glUseProgram(0);
    }





    void handleWidgetEvents(Widget& widget, SDL_Event* event) {
        for (auto component : widget.components) {
            component->handleEvents(event);
        }
        if (widget.draggableComponent) {
            handleDrag(*widget.draggableComponent, event);
        }
    }

    // Public API functions
    void createWidget(int id, int x, int y, int width, int height, int options, const std::string& texturePath) {
        if (uiManager.isCreatingWidget) {
            throw std::runtime_error("EndWidget must be called before calling a new widget");
            return;
        }

        uiManager.isCreatingWidget = true;
        auto widget = new Widget();

        if (hasFlag(options, WIDGET_HIDDEN))
        {
            widget->isVisable = false;
        }
        else if (hasFlag(options, WIDGET_SHOWN))
        {
            widget->isVisable = true;
        }
        else {
            widget->isVisable = true;
        }

        if (widget->isVisable)
        {

            widget->ID = id;
            widget->x = x;
            widget->y = y;
            widget->width = width;
            widget->height = height;
            widget->texture = 0;

            SDL_Surface* surface = IMG_Load(texturePath.c_str());
            if (surface) {
                glGenTextures(1, &widget->texture);
                glBindTexture(GL_TEXTURE_2D, widget->texture);

                GLenum format;
                if (surface->format->BytesPerPixel == 4) {
                    format = GL_RGBA;
                }
                else {
                    format = GL_RGB;
                }

                glTexImage2D(GL_TEXTURE_2D, 0, format, surface->w, surface->h, 0, format, GL_UNSIGNED_BYTE, surface->pixels);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                SDL_FreeSurface(surface);

            }
            else {
                std::cerr << "Failed to load texture: " << IMG_GetError() << std::endl;
            }

            if (hasFlag(options, WIDGET_DRAGGABLE)) {
                widget->draggableComponent = new DraggableComponent();
                widget->draggableComponent->parent = widget;
            }

            uiManager.widgets.push_back(widget);
            uiManager.currentWidget = widget;
        }


    }





    void Text(const std::string& text, float fontSize, float x = 0.0f, float y = 0.0f, std::string fontPath = "arial.ttf") {
        if (!uiManager.currentWidget) {
            std::cerr << "No widget selected" << std::endl;
            return;
        }
        // Create a new TextComponent with the provided x and y as offsets from the widget's position
        auto textComponent = new TextComponent(text, fontSize,x, y);
        textComponent->parent = uiManager.currentWidget;
        uiManager.currentWidget->components.push_back(textComponent);
    }
    void Button(const std::string& text, float fontSize, const std::string& texturePath, std::function<void()> onClick, int buttonWidth = 100, int buttonHeight = 50, int x = 0, int y = 0) {
        if (!uiManager.currentWidget) {
            std::cerr << "No widget selected" << std::endl;
            return;
        }
        auto buttonComponent = new ButtonComponent(text, fontSize, texturePath, onClick, buttonWidth, buttonHeight, x, y);
        // buttonComponent->y = static_cast<float>(uiManager.currentWidget->y);
        // buttonComponent->x = static_cast<float>(uiManager.currentWidget->x);
         // Set buttonComponent's width and height to the specified values or defaults
        buttonComponent->width = buttonWidth;
        buttonComponent->height = buttonHeight;
        buttonComponent->parent = uiManager.currentWidget;
        uiManager.currentWidget->components.push_back(buttonComponent);
    }

    void ListBox(const std::vector<std::string>& items, std::function<void(const std::string&)> onItemSelected, int ListBoxwidth = 100, int ListBoxheight = 100, int x = 0, int y = 0) {
        if (!uiManager.currentWidget) {
            std::cerr << "No widget selected" << std::endl;
            return;
        }
        auto listBoxComponent = new ListBoxComponent(items, onItemSelected, ListBoxwidth, ListBoxheight, x, y);
        listBoxComponent->y += static_cast<float>(uiManager.currentWidget->y);
        listBoxComponent->x = static_cast<float>(uiManager.currentWidget->x);
        listBoxComponent->width = ListBoxwidth;
        listBoxComponent->height = ListBoxheight;
        listBoxComponent->parent = uiManager.currentWidget;
        uiManager.currentWidget->components.push_back(listBoxComponent);
    }

    void CheckBox(int x, int y, bool isChecked, std::function<void(bool)> onCheckedChanged, const std::string& labelText) {
        if (!uiManager.currentWidget) {
            std::cerr << "No widget selected" << std::endl;
            return;
        }
        // Create a new CheckBoxComponent
        auto checkBoxComponent = new CheckBoxComponent(x, y, isChecked, onCheckedChanged, labelText);
        // Adjust position relative to the current widget
        checkBoxComponent->x += static_cast<float>(uiManager.currentWidget->x);
        checkBoxComponent->y += static_cast<float>(uiManager.currentWidget->y);
        checkBoxComponent->width = 20;
        checkBoxComponent->height = 20;

        // Add the CheckBoxComponent to the current widget's components for it to be drawn and interacted with
        uiManager.currentWidget->components.push_back(checkBoxComponent);
    }

    void ProgressBar(int x, int y, bool isFilled, std::function<void(bool)> isComplete, int width = 200, int height = 20) {
        if (!uiManager.currentWidget) {
            std::cerr << "No widget selected" << std::endl;
            return;
        }
        // Create a new ProgressBarComponent
        auto progressBarComponent = new ProgressBarComponent(x, y, width, height, isComplete);
        // Adjust position relative to the current widget
        progressBarComponent->x += static_cast<float>(uiManager.currentWidget->x);
        progressBarComponent->y += static_cast<float>(uiManager.currentWidget->y);
        progressBarComponent->width = width;
        progressBarComponent->height = height;

        // Add the ProgressBarComponent to the current widget's components for it to be drawn and interacted with
        uiManager.currentWidget->components.push_back(progressBarComponent);
    }

    void TextBox(int x, int y, int width, int height, std::function<void(const std::string&)> onTextChanged, const std::string& text = "", float transparency = 1.0f) {
        if (!uiManager.currentWidget) {
            std::cerr << "No widget selected" << std::endl;
            return;
        }
        // Create a new TextBoxComponent
        auto textBoxComponent = new TextBoxComponent(x, y, width, height, onTextChanged, text, transparency);

        // Adjust position relative to the current widget
        textBoxComponent->x += static_cast<float>(uiManager.currentWidget->x);
        textBoxComponent->y += static_cast<float>(uiManager.currentWidget->y);
        textBoxComponent->width = width;
        textBoxComponent->height = height;

        // Add the TextBoxComponent to the current widget's components for it to be drawn and interacted with
        uiManager.currentWidget->components.push_back(textBoxComponent);
    }

    void TextInput(int x, int y, int width, int height, std::function<void(const std::string&)> onTextChanged, const std::string& text = "", int options = 0) {
        if (!uiManager.currentWidget) {
            std::cerr << "No widget selected" << std::endl;
            return;
        }
        // Create a new TextInputBox
        auto textInputBoxComponent = new TextInputBoxComponent(x, y, width, height, onTextChanged, text, options);
        // Adjust position relative to the current widget
        textInputBoxComponent->x += static_cast<float>(uiManager.currentWidget->x);
        textInputBoxComponent->y += static_cast<float>(uiManager.currentWidget->y);
        textInputBoxComponent->width = width;
        textInputBoxComponent->height = height;

        // Add the TextInputBox to the current widget's components for it to be drawn and interacted with
        uiManager.currentWidget->components.push_back(textInputBoxComponent);
    }


    void Image(int x, int y, int width, int height, const std::string& imagePath) {
        if (!uiManager.currentWidget) {
            std::cerr << "No widget selected" << std::endl;
            return;
        }
        // Create a new ImageComponent
        auto imageComponent = new ImageComponent(x, y, width, height, imagePath);
        // Adjust position relative to the current widget
        imageComponent->x += static_cast<float>(uiManager.currentWidget->x);
        imageComponent->y += static_cast<float>(uiManager.currentWidget->y);
        imageComponent->width = width;
        imageComponent->height = height;

        // Add the ImageComponent to the current widget's components for it to be drawn
        uiManager.currentWidget->components.push_back(imageComponent);
    }

    void AnimatedImage(int x, int y, int width, int height, const std::string& imagePath, int frames, float frameDuration) {
        if (!uiManager.currentWidget) {
            std::cerr << "No widget selected" << std::endl;
            return;
        }
        // Create a new AnimatedImageComponent
        auto animatedImageComponent = new animatedImage(x, y, width, height, imagePath, frames, frameDuration);
        // Adjust position relative to the current widget
        animatedImageComponent->x += static_cast<float>(uiManager.currentWidget->x);
        animatedImageComponent->y += static_cast<float>(uiManager.currentWidget->y);
        animatedImageComponent->width = width;
        animatedImageComponent->height = height;

        // Add the AnimatedImageComponent to the current widget's components for it to be drawn
        uiManager.currentWidget->components.push_back(animatedImageComponent);
    }
    void Tabs(int x, int y, int width, int height, const std::string& text, std::function<void()> onClick) {
        if (!uiManager.currentWidget) {
            std::cerr << "No widget selected" << std::endl;
            return;
        }
        // Create a new Tab
        auto tab = new Tab(x, y, width, height, text, onClick);
        // Adjust position relative to the current widget
        tab->x += static_cast<float>(uiManager.currentWidget->x);
        tab->y += static_cast<float>(uiManager.currentWidget->y);
        tab->width = width;
        tab->height = height;

        // Add the Tab to the current widget's components for it to be drawn and interacted with
        uiManager.currentWidget->components.push_back(tab);
    }

    //File browser component
    void File(int x, int y, int width, int height, const std::string& currentPath, std::function<void(const std::string&)> onFileSelected, float fontSize) {
        if (!uiManager.currentWidget) {
            std::cerr << "No widget selected" << std::endl;
            return;
        }
        // Create a new FileBrowser
        auto fileBrowser = new FileBrowser(x, y, width, height, currentPath, fontSize, onFileSelected);
        // Adjust position relative to the current widget
        fileBrowser->x += static_cast<float>(uiManager.currentWidget->x);
        fileBrowser->y += static_cast<float>(uiManager.currentWidget->y);
        fileBrowser->width = width;
        fileBrowser->height = height;

        // Add the FileBrowser to the current widget's components for it to be drawn and interacted with
        uiManager.currentWidget->components.push_back(fileBrowser);
    }

    //label function
    void Label(int x, int y, const std::string& text, float fontSize, int width, int height) {
		if (!uiManager.currentWidget) {
			std::cerr << "No widget selected" << std::endl;
			return;
		}
		// Create a new LabelComponent
		auto labelComponent = new LabelComponent(x, y, text, fontSize, width, height);
		// Adjust position relative to the current widget
		labelComponent->x += static_cast<float>(uiManager.currentWidget->x);
		labelComponent->y += static_cast<float>(uiManager.currentWidget->y);
		labelComponent->width = width;
		labelComponent->height = height;

		// Add the LabelComponent to the current widget's components for it to be drawn
		uiManager.currentWidget->components.push_back(labelComponent);
	}


    //Close widget call
    void closewidget(int ID) {
        for (auto widget : uiManager.widgets) {
            if (widget->ID == ID) {
                widget->isActive = false;
                // Optionally, break here if IDs are unique


            }
        }
    }
    void endWidget() {
        uiManager.currentWidget = nullptr;
        uiManager.isCreatingWidget = false;
    }

    void renderUI() {
        for (auto& widget : uiManager.widgets) {
            drawWidget(*widget); // Draw the widget itself

            // Now draw the components of the widget
            for (auto& component : widget->components) {
                component->Draw(); // This includes TextComponent, ButtonComponent, and dropDownComponent


            }
        }
    }

    void handleEvents(SDL_Event* event) {
        for (auto widget : uiManager.widgets) {
            handleWidgetEvents(*widget, event);
        }
    }

    // Additional utility functions and widget operations can be defined here...

} // namespace SV_UI
