#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <iterator> 
#include <fstream>
#include <sstream>
#include "TextBlitting/Textblitter.h"
#include "stb_image.h"

int g_viewportWidth = 876;
int g_viewportHeight = 765;
GLFWwindow* g_window; 
GLint g_shader;
GLint g_fontTexture;
GLint g_backgroundTexture;
GLint g_backgroundQuadVAO;

void Init();
void Update();
void Render();
GLint CreateQuadVAO();
GLint LoadTexture(const std::string filepath); 
GLint LoadShader(const std::string vertPath, const std::string fragPath);

int main() {
    Init();
    while (!glfwWindowShouldClose(g_window)) {
        Update();
        Render();
    }
    glfwTerminate();
    return 0;
}

void Init() {
    // Init GL
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    g_window = glfwCreateWindow(g_viewportWidth, g_viewportHeight, "Text Blitter", NULL, NULL);
    glfwMakeContextCurrent(g_window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // Init text blitter (Exports packed sprite sheet and imports it)
    TextBlitter::Init();

    // Init resources
    g_shader = LoadShader("res/shaders/gl_text_blitter.vert", "res/shaders/gl_text_blitter.frag");
    g_fontTexture = LoadTexture("res/fonts/StandardFont.png");
    g_backgroundTexture = LoadTexture("res/background/Background.png");
    g_backgroundQuadVAO = CreateQuadVAO();
}

void Update() {
    int originX = 42;
    int originY = 42;
    float scale = 3.5f;
    std::string text = "Sirens blaring at us,\nbut he only sped up,\nmight leave in a [COL=0.9,0.1,0.1]bodybag[COL=1,1,1], \nnever in [COL=0,0.9,0]cuffs[COL=1,1,1].";
    TextBlitter::BlitText(text, "StandardFont", originX, originY, g_viewportWidth, g_viewportHeight, scale);
    TextBlitter::Update();
}

void Render() {
    glfwGetWindowSize(g_window, &g_viewportWidth, &g_viewportHeight);
    glViewport(0, 0, g_viewportWidth, g_viewportHeight);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUseProgram(g_shader);

    // Render background
    glBindVertexArray(g_backgroundQuadVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_backgroundTexture);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Render text
    OpenGLFontMesh* fontMesh = TextBlitter::GetGLFontMesh("StandardFont");
    if (fontMesh) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, g_fontTexture);
        glBindVertexArray(fontMesh->GetVAO());
        glDrawElements(GL_TRIANGLES, fontMesh->GetIndexCount(), GL_UNSIGNED_INT, 0);
    }
    glfwSwapBuffers(g_window);
    glfwPollEvents();
}

GLint LoadTexture(const std::string filepath) {
    GLuint texture;
    int w, h, c;
    unsigned char* data = stbi_load(filepath.c_str(), &w, &h, &c, 4);
    if (!data) return 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);
    return texture;
}

GLint LoadShader(const std::string vertPath, const std::string fragPath) {
    auto load = [](const std::string& path) {
        std::ifstream file(path);
        return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    };
    GLuint vert = glCreateShader(GL_VERTEX_SHADER);
    std::string vertSrc = load(vertPath);
    const char* vSrc = vertSrc.c_str();
    glShaderSource(vert, 1, &vSrc, nullptr);
    glCompileShader(vert);
    GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
    std::string fragSrc = load(fragPath);
    const char* fSrc = fragSrc.c_str();
    glShaderSource(frag, 1, &fSrc, nullptr);
    glCompileShader(frag);
    GLuint program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);
    glDeleteShader(vert);
    glDeleteShader(frag);
    return program;
}

GLint CreateQuadVAO() {
    GLuint vao = 0;
    GLuint vbo = 0;
    float vertices[] = {
        -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
         1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
         1.0f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f
    };
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(4 * sizeof(float)));
    glBindVertexArray(0);
    return vao;
}