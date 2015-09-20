#define GLM_FORCE_RADIANS

#include "ft_text_renderer.h"
#include "gl_util.h"
#include <stdexcept>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace procdraw {

    FtTextRenderer::FtTextRenderer()
    {
        try {
            if (FT_Init_FreeType(&ft_)) {
                throw std::runtime_error("Error FT_Init_FreeType");
            }

            // TODO: Need another solution, rather than hardcoding the font path
            if (FT_New_Face(ft_, "/usr/share/fonts/truetype/DejaVuSansMono.ttf", 0, &face_)) {
                throw std::runtime_error("Error FT_New_Face");
            }

            FT_Set_Pixel_Sizes(face_, 0, 240);

            CompileShaders();
            MakeTextRectangleVao();
            MakeTextTexture();
        }
        catch (...) {
            Cleanup();
            throw;
        }
    }

    FtTextRenderer::~FtTextRenderer()
    {
        Cleanup();
    }

    void FtTextRenderer::Cleanup()
    {
        glDeleteVertexArrays(1, &textRectangleVao_);
        glDeleteBuffers(1, &textRectangleVertexBuffer_);
        glDeleteTextures(1, &textTexture_);
        glDeleteProgram(program_);
    }

    void FtTextRenderer::BeginText(int width, int height)
    {
        glUseProgram(program_);
        // TODO: Cache the 2d projection matrix -- no need to calculate
        // each time, only when the renderer size changes
        auto projection = glm::ortho(0.0f, static_cast<float>(width),
                                     static_cast<float>(height), 0.0f);
        glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(projection));
        glUniform1i(2, 0);
        glDisable(GL_DEPTH_TEST);
    }

    void FtTextRenderer::Text(int x, int y)
    {
        FT_GlyphSlot g = LoadChar();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textTexture_);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            g->bitmap.width,
            g->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            g->bitmap.buffer
        );

        // Top left
        textRectangleVertices_[0] = x;
        textRectangleVertices_[1] = y;
        textRectangleVertices_[2] = 0.0f;
        textRectangleVertices_[3] = 0.0f;
        // Bottom left
        textRectangleVertices_[4] = x;
        textRectangleVertices_[5] = y + g->bitmap.rows;
        textRectangleVertices_[6] = 0.0f;
        textRectangleVertices_[7] = 1.0f;
        // Top right
        textRectangleVertices_[8] = x + g->bitmap.width;
        textRectangleVertices_[9] = y;
        textRectangleVertices_[10] = 1.0f;
        textRectangleVertices_[11] = 0.0f;
        // Bottom right
        textRectangleVertices_[12] = x + g->bitmap.width;
        textRectangleVertices_[13] = y + g->bitmap.rows;
        textRectangleVertices_[14] = 1.0f;
        textRectangleVertices_[15] = 1.0f;

        glBindVertexArray(textRectangleVao_);
        glBindBuffer(GL_ARRAY_BUFFER, textRectangleVertexBuffer_);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(textRectangleVertices_), textRectangleVertices_);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    void FtTextRenderer::CompileShaders()
    {
        static const GLchar *vertexShaderSource[] = {
            "#version 430 core                                                          \n"
            "layout (location = 0) in vec4 position;                                    \n"
            "layout (location = 1) uniform mat4 projection;                             \n"
            "out vec2 tc;                                                               \n"
            "void main(void)                                                            \n"
            "{                                                                          \n"
            "    gl_Position = projection * vec4(position.xy, 0, 1);                    \n"
            "    tc = position.zw;                                                      \n"
            "}                                                                          \n"
        };

        static const GLchar *fragmentShaderSource[] = {
            "#version 430 core                                  \n"
            "layout (location = 2) uniform sampler2D tex;       \n"
            "in vec2 tc;                                        \n"
            "out vec4 color;                                    \n"
            "void main(void)                                    \n"
            "{                                                  \n"
            "    color = vec4(0, 0, 0, texture(tex, tc).r);     \n"
            "}                                                  \n"
        };

        program_ = CompileProgram(vertexShaderSource, fragmentShaderSource);
    }

    void FtTextRenderer::MakeTextRectangleVao()
    {
        glGenVertexArrays(1, &textRectangleVao_);
        glBindVertexArray(textRectangleVao_);

        glGenBuffers(1, &textRectangleVertexBuffer_);
        glBindBuffer(GL_ARRAY_BUFFER, textRectangleVertexBuffer_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(textRectangleVertices_), textRectangleVertices_, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);
    }

    void FtTextRenderer::MakeTextTexture()
    {
        glActiveTexture(GL_TEXTURE0);
        glGenTextures(1, &textTexture_);
        glBindTexture(GL_TEXTURE_2D, textTexture_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    FT_GlyphSlot FtTextRenderer::LoadChar()
    {
        if (FT_Load_Char(face_, 'Q', FT_LOAD_RENDER)) {
            throw std::runtime_error("Error FT_Load_Char");
        }
        return face_->glyph;
    }

}
