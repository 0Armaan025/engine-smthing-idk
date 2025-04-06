#include <windows.h>
#include <glad/glad.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL.h>
#include <vector>
#include <iostream>

#undef main

const char *textVertexShaderSource = R"(#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 uProjection;

void main() {
    gl_Position = uProjection * vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}

)";

const char *textFragmentShaderSource = R"(#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D uTexture;

void main() {
    FragColor = texture(uTexture, TexCoord);
}

)";

// Vertex Shader
const char *vertexShaderSource = R"(#version 330 core
layout (location = 0) in vec2 aPos;
uniform mat4 uProjection;
void main() {
    gl_Position = uProjection * vec4(aPos, 0.0, 1.0);
}
)";

// Fragment Shader
const char *fragmentShaderSource = R"(#version 330 core
out vec4 FragColor;
uniform vec3 uColor;
void main() {
    FragColor = vec4(uColor, 1.0);
}
)";

// Generate rectangle vertex positions from center x, y
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

// Compile shader
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
      std::cerr << "Shader compilation error:\n"
                << infoLog << std::endl;
   }
   return shader;
}

// Create shader program
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

unsigned int createTextShaderProgram()
{
   unsigned int vs = compileShader(GL_VERTEX_SHADER, textVertexShaderSource);
   unsigned int fs = compileShader(GL_FRAGMENT_SHADER, textFragmentShaderSource);
   unsigned int program = glCreateProgram();
   glAttachShader(program, vs);
   glAttachShader(program, fs);
   glLinkProgram(program);
   glDeleteShader(vs);
   glDeleteShader(fs);
   return program;
}

// Draw a rectangle at position with color
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

// now we are going to render the text

// -------- Main Loop --------

int renderText(SDL_Window *window, SDL_GLContext *context, float w, float h, float x, float y, float r, float g, float b, std::string text)
{
   GLuint textShaderProgram = createTextShaderProgram();

   if (TTF_Init() == -1)
   {
      std::cerr << "TTF_Init failed: " << TTF_GetError() << std::endl;
      SDL_GL_DeleteContext(context);
      SDL_DestroyWindow(window);
      SDL_Quit();
      return -1;
   }

   TTF_Font *font = TTF_OpenFont("OpenSans.ttf", 24);
   if (!font)
   {
      std::cout << "error loading font" << TTF_GetError() << std::endl;
   }

   SDL_Color color = {0, 0, 0};

   SDL_Surface *textSurface = TTF_RenderText_Blended(font, "trying test", color);
   if (!textSurface)
   {
      std::cout << "error loading text surface" << TTF_GetError() << std::endl;
   }

   GLuint fontTexture;
   glGenTextures(1, &fontTexture);
   glBindTexture(GL_TEXTURE_2D, fontTexture);

   // set texture params

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

   // ortho

   float ortho[16] = {
       2.0f / w, 0, 0, 0,
       0, -2.0f / h, 0, 0,
       0, 0, -1, 0,
       -1, 1, 0, 1};

   // upload the texture data

   SDL_Surface *formattedSurface = SDL_ConvertSurfaceFormat(textSurface, SDL_PIXELFORMAT_ABGR8888, 0);
   if (!formattedSurface)
   {
      std::cerr << "Surface format conversion failed: " << SDL_GetError() << std::endl;
   }

   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                formattedSurface->w, formattedSurface->h, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, formattedSurface->pixels);

   float textX = 50.0f, textY = 10.0f;
   float textW = (float)textSurface->w;
   float textH = (float)textSurface->h;

   float quadVertices[] = {
       // pos      // tex
       textX, textY + textH, 0.0f, 1.0f,
       textX, textY, 0.0f, 0.0f,
       textX + textW, textY, 1.0f, 0.0f,

       textX, textY + textH, 0.0f, 1.0f,
       textX + textW, textY, 1.0f, 0.0f,
       textX + textW, textY + textH, 1.0f, 1.0f};

   GLuint textVAO, textVBO;
   glGenVertexArrays(1, &textVAO);
   glGenBuffers(1, &textVBO);
   glBindVertexArray(textVAO);

   glBindBuffer(GL_ARRAY_BUFFER, textVBO);
   glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

   // position
   glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
   glEnableVertexAttribArray(0);
   // tex coords
   glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
   glEnableVertexAttribArray(1);

   glUniform1i(glGetUniformLocation(textShaderProgram, "uTexture"), 0);
   glUseProgram(textShaderProgram);
   glUniformMatrix4fv(glGetUniformLocation(textShaderProgram, "uProjection"), 1, GL_FALSE, ortho);
   glActiveTexture(GL_TEXTURE0);
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   // glBindTexture(GL_TEXTURE_2D, fontTexture);
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

   glBindTexture(GL_TEXTURE_2D, fontTexture);
   glBindVertexArray(textVAO);
   glDrawArrays(GL_TRIANGLES, 0, 6);

   SDL_FreeSurface(textSurface);
   SDL_FreeSurface(formattedSurface);
   glDeleteTextures(1, &fontTexture);
   TTF_CloseFont(font);

   glDeleteVertexArrays(1, &textVAO);
   glDeleteBuffers(1, &textVBO);

   return 0;
}

int main()
{
   if (SDL_Init(SDL_INIT_VIDEO) < 0)
   {
      std::cerr << "SDL init failed: " << SDL_GetError() << std::endl;
      return -1;
   }

   SDL_Window *window = SDL_CreateWindow("Top File Bar",
                                         SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                         800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

   if (!window)
   {
      std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
      SDL_Quit();
      return -1;
   }

   SDL_GLContext context = SDL_GL_CreateContext(window);
   if (!context)
   {
      std::cerr << "GL context failed: " << SDL_GetError() << std::endl;
      SDL_DestroyWindow(window);
      SDL_Quit();
      return -1;
   }

   if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
   {
      std::cerr << "GLAD init failed." << std::endl;
      SDL_GL_DeleteContext(context);
      SDL_DestroyWindow(window);
      SDL_Quit();
      return -1;
   }

   SDL_GL_SetSwapInterval(1); // Enable vsync

   GLuint shaderProgram = createShaderProgram();

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
      SDL_GetWindowSize(window, &w, &h);

      // Projection matrix (Orthographic)
      float ortho[16] = {
          2.0f / w, 0, 0, 0,
          0, -2.0f / h, 0, 0,
          0, 0, -1, 0,
          -1, 1, 0, 1};

      glViewport(0, 0, w, h);
      glClearColor(0.12f, 0.12f, 0.12f, 1.0f); // dark bg
      glClear(GL_COLOR_BUFFER_BIT);

      glUseProgram(shaderProgram);
      glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProjection"), 1, GL_FALSE, ortho);

      // Render top bar (stretching full width of screen)
      float barHeight = 30.0f; // Height of top bar
      drawRectangle(shaderProgram, w / 2.0f, barHeight / 2.0f, (float)w, barHeight, 0.3f, 0.3f, 0.35f);

      glDrawArrays(GL_TRIANGLES, 0, 6);
      renderText(window, &context, (float)w, (float)h);
      SDL_GL_SwapWindow(window);
   }

   TTF_Quit();
   glDeleteProgram(shaderProgram);

   SDL_GL_DeleteContext(context);
   SDL_DestroyWindow(window);
   SDL_Quit();
   return 0;
}
