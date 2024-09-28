#pragma once
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////OPTIONAL VERSION FOR ATLAS UI 3.0 THIS WILL USE ONLY SDL2 NOT OPENGL//////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <vector>
#ifdef SDL_UI
namespace Atlas
{
    struct Widget; // Forward declaration
   
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
        SDL_Texture* texture = nullptr; // Use SDL_Texture instead of GLuint
        bool isResizing = false;
        bool resizingLeft = false, resizingRight = false, resizingTop = false, resizingBottom = false;
        std::vector<UIComponent*> components;
        DraggableComponent* draggableComponent = nullptr;
        TextComponent* textComponent = nullptr;
        int zOrder = 0;
        bool isActive = true;
        bool isCloseable = false;
        bool isVisable = true;

        SDL_Color color = { 255, 255, 255, 255 }; // Default color

        void setColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
            color = { r, g, b, a };
        }

        ~Widget() {
            if (texture) {
                SDL_DestroyTexture(texture); // Properly destroy the SDL texture
            }
        }
    };

    struct UIManager {
        std::vector<Widget*> widgets;
        Widget* currentWidget = nullptr; // Track the current widget context
        bool isCreatingWidget = false;
        SDL_Renderer* renderer = nullptr;
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



    // init SDL2
    void initSDL(const std::string& windowName, int windowWidth = 1800, int windowHeight = 900) {
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
            return;
        }
        if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
            std::cerr << "IMG_Init Error: " << IMG_GetError() << std::endl;
            SDL_Quit();
            return;
        }
        if (TTF_Init() != 0) {
            std::cerr << "TTF_Init Error: " << TTF_GetError() << std::endl;
            IMG_Quit();
            SDL_Quit();
            return;
        }

        SDL_Window* window = SDL_CreateWindow(windowName.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
        if (!window) {
            std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
            TTF_Quit();
            IMG_Quit();
            SDL_Quit();
            return;
        }

        SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (!renderer) {
            std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
            SDL_DestroyWindow(window);
            TTF_Quit();
            IMG_Quit();
            SDL_Quit();
            return;
        }

        uiManager.renderer = renderer; // Assign the renderer to the UIManager
        std::cout << "Renderer initialized successfully." << std::endl;

        // Set the draw color to white as default
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////UI COMPONENTS//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        //////////////////////////////////////////////////////////////////////////////
        ///////////////////TEXT COMPONENT/////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////
    struct TextComponent : public UIComponent {
        std::string text;
        TTF_Font* font = nullptr;
        SDL_Color color = { 255, 255, 255, 255 };
        SDL_Rect rect;
        SDL_Texture* texture = nullptr;
        SDL_Renderer* renderer; // Add renderer as a member variable

        TextComponent(std::string text, TTF_Font* font, SDL_Color color, int x, int y, SDL_Renderer* renderer)
            : text(text), font(font), color(color), renderer(renderer) { // Initialize renderer
            rect.x = x;
            rect.y = y;
            SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
            texture = SDL_CreateTextureFromSurface(renderer, surface);
            rect.w = surface->w;
            rect.h = surface->h;
            SDL_FreeSurface(surface);
        }

        ~TextComponent() {
            SDL_DestroyTexture(texture);
        }

        void Draw() override {
            SDL_RenderCopy(renderer, texture, NULL, &rect);
        }

        void handleEvents(SDL_Event* event) override {}
    };




//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////NO MORE COMPONENTS BEYOND THIS POINT!//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void drawWidget(Widget& widget, SDL_Renderer* renderer) {
        if (widget.isVisable) {
            std::cout << "Drawing widget at (" << widget.x << ", " << widget.y << ") with size (" << widget.width << ", " << widget.height << ")" << std::endl;
            SDL_Rect rect = { widget.x, widget.y, widget.width, widget.height };
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Set fill color to white
            SDL_RenderFillRect(renderer, &rect);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Set border color to black
            SDL_RenderDrawRect(renderer, &rect);
            for (auto component : widget.components) {
                component->Draw();
            }
        }
    }

        void handleWidgetEvents(Widget& widget, SDL_Event* event) {
            for (auto component : widget.components) {
                component->handleEvents(event);
            }
            if (widget.draggableComponent) {
                handleDrag(*widget.draggableComponent, event);
            }
    }

        void createWidget(int x, int y, int width, int height, int options) {
			Widget* widget = new Widget();
			widget->x = x;
			widget->y = y;
			widget->width = width;
			widget->height = height;
			if (hasFlag(options, WIDGET_DRAGGABLE)) {
				widget->draggableComponent = new DraggableComponent();
				widget->draggableComponent->parent = widget;
			}
			if (hasFlag(options, WIDGET_CLOSEABLE)) {
				widget->isCloseable = true;
			}
			if (hasFlag(options, WIDGET_SHOWN)) {
				widget->isVisable = true;
			}
			else if (hasFlag(options, WIDGET_HIDDEN)) {
				widget->isVisable = false;
			}
            uiManager.widgets.push_back(widget);
            uiManager.currentWidget = widget;
		}

        //function calls for components
        void addTextComponent(Widget& widget, std::string text, TTF_Font* font, SDL_Color color, int x, int y) {
            widget.textComponent = new TextComponent(text, font, color, x, y, uiManager.renderer);
            widget.textComponent->parent = &widget;
            widget.components.push_back(widget.textComponent);
        }







        //Close widget call
        void closewidget(int ID) {
            for (auto it = uiManager.widgets.begin(); it != uiManager.widgets.end(); /* no increment here */) {
                if ((*it)->ID == ID) {
                    auto widget = *it;
                    widget->isActive = false;
                    widget->isVisable = false;

                    // Check for errors
                    if (widget->texture) {
                        SDL_DestroyTexture(widget->texture); // Properly destroy the SDL texture
                    }

                    // Delete components
                    for (auto component : widget->components) {
                        delete component;
                    }

                    // Delete the widget
                    delete widget;

                    // Remove the widget from the list
                    it = uiManager.widgets.erase(it); // Correctly update the iterator

                    // Break after deleting the widget to avoid invalid iterator access
                    break;
                }
                else {
                    ++it; // Only increment if not erasing
                }
            }
        }


        void endWidget() {
            uiManager.currentWidget = nullptr;
            uiManager.isCreatingWidget = false;
        }

        void renderUI(SDL_Renderer* renderer) {
            std::cout << "Rendering UI..." << std::endl;
            for (auto& widget : uiManager.widgets) {
                drawWidget(*widget,renderer); // Draw the widget itself

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


}
#endif