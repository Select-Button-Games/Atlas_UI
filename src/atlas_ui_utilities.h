#pragma once
//////////////////////////////////////////////////////////
////////////SV UI UTILITIES MUST BE INCLUDED IN SV_UI3.0//
//////////////////////////////////////////////////////////
#define APIENTRY GLAPIENTRY
#include <GL/glew.h>
#include <GL/glu.h> // Include GLU header
#include <iostream>

namespace Atlas {
    GLuint shaderProgram; // shader program declaration
   
    GLuint VAO, VBO, EBO; // VAO, VBO, EBO declaration 
    glm::mat4 projection; // Declaration without initialization
    
    
    
    ///////////////////////////////////////////////////////////////
    //////////////////SETUP SDL2 AND OPENGL CALLS FOR SV_UI////////
    ///////////////////OPTIONAL AS YOU CAN SETUP YOUR OWN CALLS/////
    ////////////////////////////////////////////////////////////////

#ifdef SETUP_SDL_OPENGL
    int SCREEN_WIDTH = 1800;
    int SCREEN_HEIGHT = 900;

    SDL_Window* g_window = nullptr;
    SDL_GLContext g_context = nullptr;

    // Add SDL_Renderer declaration
    SDL_Renderer* g_renderer = nullptr;

    // Streamlined Setup function
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

        g_window = SDL_CreateWindow(windowName.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
        if (!g_window) {
            std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
            return false;
        }
        //SDL_SetWindowFullscreen(g_window, SDL_WINDOW_FULLSCREEN);
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

        int actualWidth, actualHeight;
        SDL_GetWindowSize(g_window, &actualWidth, &actualHeight);

        glViewport(0, 0, actualWidth, actualHeight);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        return true;
    }

    void Shutdown() {
        if (g_renderer) {
            SDL_DestroyRenderer(g_renderer);
            g_renderer = nullptr;
        }
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

    //getter function to get the SDL_Renderer
    SDL_Renderer* GetRenderer() {
        return g_renderer;
    }
#endif








    //////////////////////////////////////////////////
    ////////////OPENGL DEBUG STUFF////////////////////
    ///////////////////////////////////////////////////
    void checkOpenGLError(const char* stmt, const char* fname, int line) {
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            const char* error;
            switch (err) {
            case GL_INVALID_OPERATION:      error = "INVALID_OPERATION";      break;
            case GL_INVALID_ENUM:           error = "INVALID_ENUM";           break;
            case GL_INVALID_VALUE:          error = "INVALID_VALUE";          break;
            case GL_OUT_OF_MEMORY:          error = "OUT_OF_MEMORY";          break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:  error = "INVALID_FRAMEBUFFER_OPERATION";  break;
            default:                        error = "UNKNOWN_ERROR";          break;
            }
            std::cerr << "OpenGL error (" << error << ") " << " at " << fname << ":" << line << " - for " << stmt << std::endl;
        }
    }

#define GL_CHECK(stmt) do { \
    stmt; \
    checkGLError(#stmt, __FILE__, __LINE__); \
} while (0)
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
            
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

            std::cout << "OpenGL debug output initialized" << std::endl;


        }
    }
    void checkGLError(const char* stmt, const char* fname, int line) {
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            std::cerr << "OpenGL error " << err << ", at " << fname << ":" << line << " - for " << stmt << std::endl;
            exit(1);
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


    ///////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////LOGGING MECHANISM/////////////////////////////////////////////////
#ifdef ATLAS_LOGGING 
    enum class LogLevel {
        Info,
        Warning,
        Error
    };

    class Logger {
    public:
        static void Log(const std::string& message, LogLevel level = LogLevel::Info) {
            switch (level) {
            case LogLevel::Info:
                std::cout << "[INFO] " << message << std::endl;
                break;
            case LogLevel::Warning:
                std::cout << "[WARNING] " << message << std::endl;
                break;
            case LogLevel::Error:
                std::cerr << "[ERROR] " << message << std::endl;
                break;
            }
        }

        // Optional: Function to log SDL errors with a custom message
        static void LogSDLError(const std::string& message) {
            Log(message + " SDL Error: " + SDL_GetError(), LogLevel::Error);
        }

        // Optional: Function to log OpenGL debug messages
        static void LogOpenGLDebug(const std::string& message, GLenum source, GLenum type, GLuint id, GLenum severity) {
            std::string debugMessage = "OpenGL Debug message (" + std::to_string(id) + "): " + message;
            LogLevel level = LogLevel::Info; // Default to info

            if (severity == GL_DEBUG_SEVERITY_HIGH) {
                level = LogLevel::Error;
            }
            else if (severity == GL_DEBUG_SEVERITY_MEDIUM) {
                level = LogLevel::Warning;
            }

            Log(debugMessage, level);
        }

        
       
        
    
    };

#endif

    ////////////////////////////////////////////////////////////////
    /////////////////TEXTURE LOADING///////////////////////////////
    ///////////////////////////////////////////////////////////////
// Define a struct to hold texture information
    struct TextureInfo {
        GLuint id;
        int width;
        int height;
    };

    // Update the loadTexture function to return TextureInfo
    TextureInfo loadTexture(const char* path) {
        SDL_Surface* surface = IMG_Load(path);
        if (!surface) {
            std::cerr << "Failed to load texture: " << IMG_GetError() << std::endl;
            return { 0, 0, 0 }; // Return an empty texture info on failure
        }

        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        GLenum format;
        if (surface->format->BytesPerPixel == 4) {
            format = surface->format->Rmask == 0x000000ff ? GL_RGBA : GL_BGRA;
        }
        else {
            format = surface->format->Rmask == 0x000000ff ? GL_RGB : GL_BGR;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, format == GL_RGBA ? GL_RGBA : GL_RGB, surface->w, surface->h, 0, format, GL_UNSIGNED_BYTE, surface->pixels);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        TextureInfo textureInfo = { texture, surface->w, surface->h };
        SDL_FreeSurface(surface);
        std::cout << "Texture loaded successfully from " << path << std::endl;
        return textureInfo;
    }

   

   
	


};
