#pragma once
#include <ft2build.h>
#include FT_FREETYPE_H
#include <map>
#include <iostream>
#include <stdexcept>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "atlas_ui3.0.h" 
#include "atlas_ui_utilities.h"


///////////////////////////////////////////////////////////////////////////////////////////
///////////////ATLAS UI TEXT RENDERING LIBRARY////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
namespace Atlas {


    /////////////////////////////////////////////////////////////////
    ////////////SHADERS FOR ATLAS TEXT//////////////////////////////

    GLuint compileTextShader(const char* shaderSource, GLenum shaderType) {
        GLuint shader = glCreateShader(shaderType);
        glShaderSource(shader, 1, &shaderSource, NULL);
        glCompileShader(shader);

        return shader;
    }

    GLuint createTextShaderProgram(const char* vertexShaderSource, const char* fragmentShaderSource) {
        GLuint vertexShader = compileTextShader(vertexShaderSource, GL_VERTEX_SHADER);
        GLuint fragmentShader = compileTextShader(fragmentShaderSource, GL_FRAGMENT_SHADER);

        GLuint shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);


        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return shaderProgram;
    }




    const char* textVertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}
)";

    const char* textFragmentShaderSource = R"(
#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;

void main()
{
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
    color = vec4(textColor, 1.0) * sampled;
}
)";

    //Class for text rendering
    class TextRenderer {
    public:
        TextRenderer(float fontSize);
        ~TextRenderer();
        void RenderText(const std::string& text, float x, float y, float scale, glm::vec3 color = glm::vec3(0.0f, 0.0f, 0.0f));

        struct Character {
            GLuint TextureID;  // ID handle of the glyph texture
            glm::ivec2 Size;   // Size of glyph
            glm::ivec2 Bearing; // Offset from baseline to left/top of glyph
            GLuint Advance;    // Offset to advance to next glyph
        };
        float GetCharacterWidth(char c) {
            // Get the character width
            return Characters[c].Advance >> 6;
        }

        float GetTextWidth(const std::string& text) {
            float width = 0.0f;
            for (char c : text) {
                width += GetCharacterWidth(c); // Call the method to get the width of each character
            }
            return width;
        }
        static void SetGlobalFont(const std::string& fontPath);

        float GetTextHeight(const std::string& text)
        {
            float height = 0.0f;
            for (char c : text) {

                if (Characters[c].Size.y > height) {
                    height = Characters[c].Size.y;
                }
            }
            return height;
        }
    private:
        FT_Library ft;
        FT_Face face;
        std::map<char, Character> Characters;
        GLuint VAO, VBO;
        GLuint textShaderProgram; // New shader program for text rendering

        void LoadCharacters();
        void SetupRenderData();

        static std::string globalFontPath; // Static member to store the global font path

    };

    std::string TextRenderer::globalFontPath = "UI/svf.ttf";


    TextRenderer::TextRenderer(float fontSize) {
        if (FT_Init_FreeType(&ft)) {
            throw std::runtime_error("ERROR::FREETYPE: Could not init FreeType Library");
        }

        if (globalFontPath.empty()) {
            throw std::runtime_error("ERROR::FREETYPE: Global font path is not set");
        }

        if (FT_New_Face(ft, globalFontPath.c_str(), 0, &face)) {
            throw std::runtime_error("ERROR::FREETYPE: Failed to load font");
        }

        FT_Set_Pixel_Sizes(face, 0, static_cast<FT_UInt>(fontSize));
        LoadCharacters();
        SetupRenderData();

        // Create the text shader program
        textShaderProgram = createTextShaderProgram(textVertexShaderSource, textFragmentShaderSource);
    }

    void TextRenderer::SetGlobalFont(const std::string& fontPath) {
        globalFontPath = fontPath;
    }


    TextRenderer::~TextRenderer() {
        FT_Done_Face(face);
        FT_Done_FreeType(ft);
        glDeleteProgram(textShaderProgram); // Clean up the shader program
    }

    void TextRenderer::LoadCharacters() {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction

        for (unsigned char c = 0; c < 128; c++) {
            // Load character glyph
            if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
                std::cerr << "ERROR::FREETYPE: Failed to load Glyph" << std::endl;
                continue;
            }
            // Generate texture
            GLuint texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
            );
            // Set texture options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // Now store character for later use
            Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<unsigned int>(face->glyph->advance.x)
            };
            Characters.insert(std::pair<char, Character>(c, character));
        }
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture
        FT_Done_Face(face);
        FT_Done_FreeType(ft);
    }

    void TextRenderer::SetupRenderData() {
        // Configure VAO/VBO for texture quads
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void TextRenderer::RenderText(const std::string& text, float x, float y, float scale, glm::vec3 color) {
        glUseProgram(textShaderProgram);
        GLint textColorLocation = glGetUniformLocation(textShaderProgram, "textColor");
        glUniform3f(textColorLocation, color.x, color.y, color.z);

        GLint projectionLocation = glGetUniformLocation(textShaderProgram, "projection");
        glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));

        glUniform1i(glGetUniformLocation(textShaderProgram, "text"), 0); // Use text texture
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(VAO);

        GLenum error = glGetError();


        std::string::const_iterator c;
        for (c = text.begin(); c != text.end(); c++) {
            Character ch = Characters[*c];

            float xpos = x + ch.Bearing.x * scale;
            float ypos = y - (ch.Bearing.y * scale); // Adjusted for baseline alignment

            float w = ch.Size.x * scale;
            float h = ch.Size.y * scale;

            float vertices[6][4] = {
                { xpos,     ypos + h,   0.0f, 1.0f },
                { xpos,     ypos,       0.0f, 0.0f },
                { xpos + w, ypos,       1.0f, 0.0f },

                { xpos,     ypos + h,   0.0f, 1.0f },
                { xpos + w, ypos,       1.0f, 0.0f },
                { xpos + w, ypos + h,   1.0f, 1.0f }
            };
            glBindTexture(GL_TEXTURE_2D, ch.TextureID);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            error = glGetError();


            x += (ch.Advance >> 6) * scale;
        }
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);

        error = glGetError();

    }







}
