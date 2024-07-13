#pragma once

//////////////////////////////////////////////////////////
////////////SV UI UTILITIES MUST BE INCLUDED IN SV_UI3.0//
//////////////////////////////////////////////////////////
#define APIENTRY GLAPIENTRY
#include <GL/glew.h>
#include <iostream>


namespace Atlas {

    ///////////////////////////////////////////////////////////////
    //////////////////SETUP SDL2 AND OPENGL CALLS FOR SV_UI////////
    ///////////////////OPTIONAL AS YOU CAN SETUP YOUR OWN CALLS/////
    ////////////////////////////////////////////////////////////////

#ifdef SETUP_SDL_OPENGL
    const int SCREEN_WIDTH = 1800;
    const int SCREEN_HEIGHT = 900;

    SDL_Window* g_window = nullptr;
    SDL_GLContext g_context = nullptr;

    bool initSDL(SDL_Window*& window, SDL_GLContext& context, const std::string& windowName, int windowWidth, int windowHeight) {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
            return false;
        }
        if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
            std::cerr << "Failed to initialize SDL_image: " << IMG_GetError() << std::endl;
            return false;
        }
        if (TTF_Init() == -1) {
            std::cerr << "TTF_Init: " << TTF_GetError() << std::endl;
            return false;
        }
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        window = SDL_CreateWindow(windowName.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
        if (!window) {
            std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
            return false;
        }

        context = SDL_GL_CreateContext(window);
        if (!context) {
            std::cerr << "Failed to create OpenGL context: " << SDL_GetError() << std::endl;
            return false;
        }

        if (SDL_GL_SetSwapInterval(1) < 0) {
            std::cerr << "Failed to set VSync: " << SDL_GetError() << std::endl;
            return false;
        }

        glewExperimental = GL_TRUE;
        GLenum err = glewInit();
        if (err != GLEW_OK) {
            std::cerr << "GLEW initialization error: " << glewGetErrorString(err) << std::endl;
            return false;
        }

        glViewport(0, 0, windowWidth, windowHeight);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        return true;
    }

    void closeSDL(SDL_Window* window, SDL_GLContext context) {
        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    void debugOpenGL() {
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            std::cerr << "OpenGL error: " << err << std::endl;
        }
    }

    // New Setup function
   // Modified Setup function to use global variables
    bool Setup(const std::string& windowName, int windowWidth = SCREEN_WIDTH, int windowHeight = SCREEN_HEIGHT) {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
            return false;
        }
        if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
            std::cerr << "Failed to initialize SDL_image: " << IMG_GetError() << std::endl;
            return false;
        }
        if (TTF_Init() == -1) {
            std::cerr << "TTF_Init: " << TTF_GetError() << std::endl;
            return false;
        }
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        g_window = SDL_CreateWindow(windowName.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
        if (!g_window) {
            std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
            return false;
        }

        g_context = SDL_GL_CreateContext(g_window);
        if (!g_context) {
            std::cerr << "Failed to create OpenGL context: " << SDL_GetError() << std::endl;
            return false;
        }

        if (SDL_GL_SetSwapInterval(1) < 0) {
            std::cerr << "Failed to set VSync: " << SDL_GetError() << std::endl;
            return false;
        }

        glewExperimental = GL_TRUE;
        GLenum err = glewInit();
        if (err != GLEW_OK) {
            std::cerr << "GLEW initialization error: " << glewGetErrorString(err) << std::endl;
            return false;
        }

        glViewport(0, 0, windowWidth, windowHeight);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        return true;
}


    void Shutdown() {
        if (g_context) {
            SDL_GL_DeleteContext(g_context);
            g_context = nullptr;
        }
        if (g_window) {
            SDL_DestroyWindow(g_window);
            g_window = nullptr;
        }
        IMG_Quit();
        TTF_Quit();
        SDL_Quit();
    }
#endif





    //////////////////////////////////////////////////
    ////////////OPENGL DEBUG STUFF////////////////////
    ///////////////////////////////////////////////////

    // Callback function for debugging
    void APIENTRY glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {

        // Ignore non-significant error/warning codes
        if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

        std::cerr << "---------------" << std::endl;
        std::cerr << "Debug message (" << id << "): " << message << std::endl;

        switch (source) {
        case GL_DEBUG_SOURCE_API:             std::cerr << "Source: API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cerr << "Source: Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cerr << "Source: Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cerr << "Source: Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION:     std::cerr << "Source: Application"; break;
        case GL_DEBUG_SOURCE_OTHER:           std::cerr << "Source: Other"; break;
        } std::cerr << std::endl;

        switch (type) {
        case GL_DEBUG_TYPE_ERROR:               std::cerr << "Type: Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cerr << "Type: Deprecated Behaviour"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cerr << "Type: Undefined Behaviour"; break;
        case GL_DEBUG_TYPE_PORTABILITY:         std::cerr << "Type: Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         std::cerr << "Type: Performance"; break;
        case GL_DEBUG_TYPE_MARKER:              std::cerr << "Type: Marker"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          std::cerr << "Type: Push Group"; break;
        case GL_DEBUG_TYPE_POP_GROUP:           std::cerr << "Type: Pop Group"; break;
        case GL_DEBUG_TYPE_OTHER:               std::cerr << "Type: Other"; break;
        } std::cerr << std::endl;

        switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:         std::cerr << "Severity: high"; break;
        case GL_DEBUG_SEVERITY_MEDIUM:       std::cerr << "Severity: medium"; break;
        case GL_DEBUG_SEVERITY_LOW:          std::cerr << "Severity: low"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: std::cerr << "Severity: notification"; break;
        } std::cerr << std::endl;
        std::cerr << std::endl;
    }

    // Function to initialize OpenGL debugging
    void initOpenGLDebug() {
        if (glewIsSupported("GL_KHR_debug")) {
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(static_cast<GLDEBUGPROC>(glDebugOutput), nullptr); // Cast to GLDEBUGPROC
            // Other setup remains the same...
        }
    }

    enum class Alignment {
        TopLeft,
        TopCenter,
        TopRight,
        CenterLeft,
        Center,
        CenterRight,
        BottomLeft,
        BottomCenter,
        BottomRight
    };

    void calculatePositionForAlignment(float& x, float& y, int compWidth, int compHeight, int parentWidth, int parentHeight, Alignment alignment) {
        switch (alignment) {
        case Alignment::TopLeft:
            // x and y remain unchanged
            break;
        case Alignment::TopCenter:
            x = (parentWidth - compWidth) / 2;
            break;
        case Alignment::TopRight:
            x = parentWidth - compWidth;
            break;
        case Alignment::CenterLeft:
            y = (parentHeight - compHeight) / 2;
            break;
        case Alignment::Center:
            x = (parentWidth - compWidth) / 2;
            y = (parentHeight - compHeight) / 2;
            break;
        case Alignment::CenterRight:
            x = parentWidth - compWidth;
            y = (parentHeight - compHeight) / 2;
            break;
        case Alignment::BottomLeft:
            y = parentHeight - compHeight;
            break;
        case Alignment::BottomCenter:
            x = (parentWidth - compWidth) / 2;
            y = parentHeight - compHeight;
            break;
        case Alignment::BottomRight:
            x = parentWidth - compWidth;
            y = parentHeight - compHeight;
            break;
        }
    }
}
