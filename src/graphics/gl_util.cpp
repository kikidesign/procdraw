#include "gl_util.h"
#include <stdexcept>

namespace procdraw {

    void ThrowOnGlewError(GLenum err)
    {
        if (err != GLEW_OK) {
            throw std::runtime_error(reinterpret_cast<const char*>(glewGetErrorString(err)));
        }
    }

    GLuint CompileProgram(const GLchar **vertexShaderSource, const GLchar **fragmentShaderSource,
                          std::map<GLuint, const GLchar*> attribLocations)
    {
        GLuint vertexShader = CompileShader(vertexShaderSource, GL_VERTEX_SHADER);
        GLuint fragmentShader = CompileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
        GLuint program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);

        for (const auto& attrib : attribLocations) {
            glBindAttribLocation(program, attrib.first, attrib.second);
        }

        glLinkProgram(program);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return program;
    }

    GLuint CompileShader(const GLchar **shaderSource, GLenum shaderType)
    {
        GLuint shader = glCreateShader(shaderType);
        glShaderSource(shader, 1, shaderSource, NULL);
        glCompileShader(shader);
        return shader;
    }

}
