#include <SFML/Window.hpp>
//#include <SFML/OpenGL.hpp> 
#include <SFML/Graphics.hpp>
#include <GL/glew.h>      
#include <GL/gl.h>       
#include <iostream>
#include <vector>
#include <cmath>
#include <array>
#include <initializer_list>

using namespace std;

// ID шейдерной программы
GLuint Program;
// ID атрибутов
GLint Attrib_vertex;
GLint Attrib_color;
// ID буферов
GLuint VBO_Vertex, VBO_Color;

GLint X_scale, Y_scale;

// Структура для координат вершины
struct Vertex {
    GLfloat x, y; // Координаты вершины
};

// Структура для цвета вершины
struct Color {
    GLfloat r, g, b; // Цвет вершины
    Color() {

    }
    Color(GLfloat newr, GLfloat newg, GLfloat newb) {
        r = newr;
        g = newg;
        b = newb;
    }
};

// Исходный код вершинного шейдера
const char* VertexShaderSource = R"(
#version 330 core
in vec2 coord;        //  координаты вершины
in vec3 vertColor;    // цвет вершины

uniform float x_scale;
uniform float y_scale;

out vec3 fragColor;   // во фрагментный шейдер

void main() {
    vec3 pos=vec3(coord, 1.0) * mat3(x_scale, 0, 0,
                                     0, y_scale, 0,
                                     0, 0, 1);
    gl_Position = vec4(pos[0], pos[1], 0.0, 1.0); // положение вершины на экране
    fragColor = vertColor;              // цвет дальше
}
)";

// Исходный код фрагментного шейдера
const char* FragShaderSource = R"(
#version 330 core
in vec3 fragColor;  // цвет из вершинного шейдера
out vec4 color;

void main() {
    color = vec4(fragColor, 1.0); // используем интерполированный цвет
}
)";

void ShaderLog(unsigned int shader) {
    int infologLen = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologLen);
    if (infologLen > 1) {
        int charsWritten = 0;
        std::vector<char> infoLog(infologLen);
        glGetShaderInfoLog(shader, infologLen, &charsWritten, infoLog.data());
        std::cout << "InfoLog: " << infoLog.data() << std::endl;
    }
}

float func(float color)
{
    return (1 / 100.0) * color;
}
//тон насыщенность яркость
std::array<float, 3> HSVtoRGB(float hue, float saturation = 100.0, float value = 100.0)
{
    int sw = (int)floor(hue / 60) % 6; //сектор круга
    float vmin = ((100.0f - saturation) * value) / 100.0; //минимальное значение цвета зависит от насыщенности 
    float a = (value - vmin) * (((int)hue % 60) / 60.0); 
    float vinc = vmin + a;
    float vdec = value - a;
    switch (sw)
    {
    case 0: return { func(value), func(vinc), func(vmin) };
    case 1: return { func(vdec), func(value), func(vmin) };
    case 2: return { func(vmin), func(value), func(vinc) };
    case 3: return { func(vmin), func(vdec), func(value) };
    case 4: return { func(vinc), func(vmin), func(value) };
    case 5: return { func(value), func(vmin), func(vdec) };
    }
    return { 0, 0, 0 };
}

int const cnt = 360;
//Vertex fig;
void InitVBO() {
    const float pi = 3.14159265358979323846f;
    const float r = 0.8f;

    
    Vertex fig[3*cnt];
    Color colors[3 * cnt];
    float sz = 0.5f;
    for (int i = 0; i < cnt; i++) {
        double angle1 = 2 * pi * i / cnt;
        double angle2 = 2 * pi * (i+1) / cnt;
        fig[i * 3] = { sz * (float)cos(angle1), sz * (float)sin(angle1) };
        fig[i * 3 + 1] = { sz * (float)cos(angle2), sz * (float)sin(angle2) };
        fig[i * 3 + 2] = { 0.0f,0.0f };
        colors[i * 3] = { HSVtoRGB(i % 360)[0], HSVtoRGB(i % 360)[1],HSVtoRGB(i % 360)[2] };
        colors[i * 3 + 1] = { HSVtoRGB((i + 1) % 360)[0], HSVtoRGB((i + 1) % 360)[1],HSVtoRGB((i + 1) % 360)[2] };
        colors[i * 3 + 2] = { 1.0,1.0,1.0 };
    }

    


    // Создаем буфер для координат вершин
    glGenBuffers(1, &VBO_Vertex);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_Vertex);
    glBufferData(GL_ARRAY_BUFFER, sizeof(fig), fig, GL_STATIC_DRAW);

    // Создаем буфер для цветов
    glGenBuffers(1, &VBO_Color);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_Color);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
}

void InitShader() {
    // Создаем вершинный шейдер
    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vShader, 1, &VertexShaderSource, NULL);
    glCompileShader(vShader);
    ShaderLog(vShader);

    // Создаем фрагментный шейдер
    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &FragShaderSource, NULL);
    glCompileShader(fShader);
    ShaderLog(fShader);

    // Создаем шейдерную программу и прикрепляем шейдеры
    Program = glCreateProgram();
    glAttachShader(Program, vShader);
    glAttachShader(Program, fShader);
    glLinkProgram(Program);

    // Проверяем статус сборки
    int link_ok;
    glGetProgramiv(Program, GL_LINK_STATUS, &link_ok);
    if (!link_ok) {
        std::cerr << "Error: Could not link shader program." << std::endl;
        return;
    }

    // Получаем атрибуты
    Attrib_vertex = glGetAttribLocation(Program, "coord");
    if (Attrib_vertex == -1) {
        std::cerr << "Error: Could not bind attribute 'coord'." << std::endl;
        return;
    }

    Attrib_color = glGetAttribLocation(Program, "vertColor");
    if (Attrib_color == -1) {
        std::cerr << "Error: Could not bind attribute 'vertColor'." << std::endl;
        return;
    }

    // Вытягиваем ID юниформ
    const char* unif_name = "x_scale";
    X_scale = glGetUniformLocation(Program, unif_name);
    if (X_scale == -1)
    {
        std::cout << "could not bind uniform " << unif_name << std::endl;
        return;
    }

    unif_name = "y_scale";
    Y_scale = glGetUniformLocation(Program, unif_name);
    if (Y_scale == -1)
    {
        std::cout << "could not bind uniform " << unif_name << std::endl;
        return;
    }
}

void Init() {
    InitShader();
    InitVBO();
}


float scaleX = 1.0;
float scaleY = 1.0;

void changeScale(float scaleXinc, float scaleYinc) {
    scaleX += scaleXinc;
    scaleY += scaleYinc;
}
void Draw() {
    glUseProgram(Program);

    glUniform1f(X_scale, scaleX);
    glUniform1f(Y_scale, scaleY);

    // Подключаем буфер с координатами
    glBindBuffer(GL_ARRAY_BUFFER, VBO_Vertex);
    glEnableVertexAttribArray(Attrib_vertex);
    glVertexAttribPointer(Attrib_vertex, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    // Подключаем буфер с цветами
    glBindBuffer(GL_ARRAY_BUFFER, VBO_Color);
    glEnableVertexAttribArray(Attrib_color);
    glVertexAttribPointer(Attrib_color, 3, GL_FLOAT, GL_FALSE, sizeof(Color), (void*)0);

    // Рисуем пятиугольник
    glDrawArrays(GL_TRIANGLES, 0, cnt*3);

    // Отключаем атрибуты
    glDisableVertexAttribArray(Attrib_vertex);
    glDisableVertexAttribArray(Attrib_color);

    glUseProgram(0);
}

void ReleaseShader() {
    glUseProgram(0);
    glDeleteProgram(Program);
}

void ReleaseVBO() {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &VBO_Vertex);
    glDeleteBuffers(1, &VBO_Color);
}

void Release() {
    ReleaseShader();
    ReleaseVBO();
}

int main() {
    sf::Window window(sf::VideoMode(600, 600), "Gradient Pentagon", sf::Style::Default, sf::ContextSettings(24));
    window.setVerticalSyncEnabled(true);
    window.setActive(true);
    glewInit();
    Init();

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            else if (event.type == sf::Event::Resized) glViewport(0, 0, event.size.width, event.size.height);
            else if (event.type == sf::Event::KeyPressed) {
                switch (event.key.code) {
                case (sf::Keyboard::W): changeScale(0, 0.1); break;
                case (sf::Keyboard::S): changeScale(0, -0.1); break;
                case (sf::Keyboard::A): changeScale(-0.1, 0); break;
                case (sf::Keyboard::D): changeScale(0.1, 0); break;
                default: break;
                }
            }
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        Draw();
        window.display();
    }

    Release();
    return 0;
}
