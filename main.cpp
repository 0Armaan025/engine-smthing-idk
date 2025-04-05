#include <glad/glad.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <iostream>

using namespace std;

#undef main



// Vertex and Fragment shader sources
const char *vertexShaderSource = R"(#version 330 core
layout (location = 0) in vec2 aPos;
uniform mat4 uProjection;
void main() {
    gl_Position = uProjection * vec4(aPos, 0.0, 1.0);
}
)";

const char *fragmentShaderSource = R"(#version 330 core
out vec4 FragColor;
uniform vec3 uColor;
void main() {
    FragColor = vec4(uColor, 1.0);
}
)";

// Generates rectangle vertices from center (cx, cy)
std::vector<float> generateRectangleVertices(float cx, float cy, float width, float height)
{
   float halfW = width / 2.0f;
   float halfH = height / 2.0f;
   float x1 = cx - halfW;
   float x2 = cx + halfW;
   float y1 = cy - halfH;
   float y2 = cy + halfH;

   return {
       x1, y1, x2, y1, x1, y2,
       x2, y1, x1, y2, x2, y2};
}

// Compiles a shader and returns its ID
unsigned int compileShader(unsigned int type, const char *source)
{
   unsigned int shader = glCreateShader(type);
   glShaderSource(shader, 1, &source, nullptr);
   glCompileShader(shader);

   int success;
   char infoLog[512];
   glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
   if (!success)
   {
      glGetShaderInfoLog(shader, 512, nullptr, infoLog);
      cerr << "Shader compilation error:\n"
           << infoLog << endl;
   }
   return shader;
}

// Creates a shader program using vertex and fragment shaders
unsigned int createShaderProgram()
{
   unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
   unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

   unsigned int program = glCreateProgram();
   glAttachShader(program, vs);
   glAttachShader(program, fs);
   glLinkProgram(program);

   glDeleteShader(vs);
   glDeleteShader(fs);
   return program;
}

// Draws a colored rectangle at given position/size
void drawRectangle(GLuint shaderProgram, float x, float y, float width, float height, float r, float g, float b)
{
   std::vector<float> vertices = generateRectangleVertices(x, y, width, height);

   GLuint VAO, VBO;
   glGenVertexArrays(1, &VAO);
   glGenBuffers(1, &VBO);

   glBindVertexArray(VAO);
   glBindBuffer(GL_ARRAY_BUFFER, VBO);
   glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

   glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
   glEnableVertexAttribArray(0);

   glUseProgram(shaderProgram);
   glUniform3f(glGetUniformLocation(shaderProgram, "uColor"), r, g, b);

   glDrawArrays(GL_TRIANGLES, 0, 6);

   glDeleteBuffers(1, &VBO);
   glDeleteVertexArrays(1, &VAO);
}

// -------- Main --------
int main(int argc, char *argv[])
{
   if (SDL_Init(SDL_INIT_VIDEO) < 0)
   {
      cerr << "SDL init failed: " << SDL_GetError() << endl;
      return -1;
   }

   SDL_Window *window = SDL_CreateWindow("SDL2 + OpenGL",
                                         SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                         800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

   if (!window)
   {
      cerr << "Failed to create window: " << SDL_GetError() << endl;
      SDL_Quit();
      return -1;
   }

   SDL_GLContext context = SDL_GL_CreateContext(window);
   if (!context)
   {
      cerr << "Failed to create GL context: " << SDL_GetError() << endl;
      SDL_DestroyWindow(window);
      SDL_Quit();
      return -1;
   }

   if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
   {
      cerr << "Failed to initialize GLAD." << endl;
      SDL_GL_DeleteContext(context);
      SDL_DestroyWindow(window);
      SDL_Quit();
      return -1;
   }

   if(TTF_Init() == -1) {
      cout <<"TTF Init error: "<< TTF_GetError() << endl;
      return -1;
   }

   SDL_GL_SetSwapInterval(1); // vsync

   unsigned int shaderProgram = createShaderProgram();

   TTF_Font* font = TTF_OpenFont("OpenSans.ttf", 24);

   if(!font) {
      cout<<"Failed to load font" << TTF_GetError()<<endl;
      return -1;
   }

   bool running = true;
   SDL_Event event;

   while (running)
   {
      while (SDL_PollEvent(&event))
      {
         if (event.type == SDL_QUIT)
            running = false;
      }

      int w, h;
      int barWidth = w;
      float barHeight = 10.0f;

      float x = w / 2.0f;
      float y = barHeight / 2.0f;
      SDL_GetWindowSize(window, &w, &h);

      float ortho[16] = {
          2.0f / w, 0, 0,
          0, -2.0f / h, 0, 0,
          0, 0, -1, 0,
          -1, 1, 0, 1};

      glViewport(0, 0, w, h);
      glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);

      glUseProgram(shaderProgram);
      glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjection"), 1, GL_FALSE, ortho);

      drawRectangle(shaderProgram, w / 2.0f, h / 2.0f, 200.0f, 100.0f, 0.2f, 0.7f, 0.9f);
      drawRectangle(shaderProgram, x, y, barWidth, barHeight, 0.2f, 0.7f, 0.9f);

      SDL_GL_SwapWindow(window);
   }

   glDeleteProgram(shaderProgram);
   SDL_GL_DeleteContext(context);
   SDL_DestroyWindow(window);
   SDL_Quit();
   return 0;
}
