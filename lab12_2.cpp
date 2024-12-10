#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath> 
#include "math.h"

// ID шейдерной программы
GLuint Program;
// ID атрибута
GLint Attrib_vertex;
// ID Vertex Buffer Object
GLuint VBO;
// ID атрибута цвета
GLint Attrib_color;
// ID VBO цвета
GLuint VBO_color;
GLuint VBO_texCoords;
GLint Attrib_texCoord;

GLint Uniform_mixFactor;

GLuint texture1;
float mixFactor = 0.5f;

struct Vertex {
    GLfloat x;
    GLfloat y;
    GLfloat z;
};

struct Color {
    GLfloat r;
    GLfloat g;
    GLfloat b;
};

// Исходный код вершинного шейдера
const char* VertexShaderSource = R"(
 #version 330 core

 in vec3 vertColor;
 in vec3 coord;
 in vec2 texCoord;     

 out vec3 fragColor;
 out vec2 fragTexCoord;

 float x_angle = radians(-45.0);
 float y_angle = radians(45.0);
 float z_angle = radians(45.0);

 mat3 mr1 = mat3(
    1, 0, 0,
    0, cos(x_angle), -sin(x_angle),
    0, sin(x_angle), cos(x_angle));

 mat3 mr2 = mat3(
    cos(y_angle), 0, sin(y_angle),
    0, 1, 0,
    -sin(y_angle), 0, cos(y_angle));
 
 mat3 mr3 = mat3(
    cos(z_angle), sin(z_angle), 0,
    -sin(z_angle),cos(z_angle), 0,
    0, 0, 1);

 void main() {
    vec3 position = coord * mr1 * mr2 * mr3;
    gl_Position = vec4(position, 1.0);
    fragColor = vertColor;
    fragTexCoord = texCoord;
 }
)";

// Исходный код фрагментного шейдера
const char* FragShaderSource = R"(
 #version 330 core
 
 in vec3 fragColor;    
 in vec2 fragTexCoord; 

 out vec4 color;  

 uniform sampler2D texture1; 
 uniform float mixFactor;   

 void main() {
    vec4 texColor = texture(texture1, fragTexCoord); 
    vec4 vertexColor = vec4(fragColor, 1.0);        
    color = mix(vertexColor, texColor, mixFactor); 
}
)";


void InitVBO() {
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &VBO_color);
    glGenBuffers(1, &VBO_texCoords);

    Vertex cube[] = {
        // Вершины кубика
        { -0.5, -0.5, +0.5 }, { -0.5, +0.5, +0.5 }, { +0.5, +0.5, +0.5 },
        { +0.5, +0.5, +0.5 }, { +0.5, -0.5, +0.5 }, { -0.5, -0.5, +0.5 },

        { -0.5, -0.5, -0.5 }, { +0.5, +0.5, -0.5 }, { -0.5, +0.5, -0.5 },
        { +0.5, +0.5, -0.5 }, { -0.5, -0.5, -0.5 }, { +0.5, -0.5, -0.5 },

        { -0.5, +0.5, -0.5 }, { -0.5, +0.5, +0.5 }, { +0.5, +0.5, +0.5 },
        { +0.5, +0.5, +0.5 }, { +0.5, +0.5, -0.5 }, { -0.5, +0.5, -0.5 },

        { -0.5, -0.5, -0.5 }, { +0.5, -0.5, +0.5 }, { -0.5, -0.5, +0.5 },
        { +0.5, -0.5, +0.5 }, { -0.5, -0.5, -0.5 }, { +0.5, -0.5, -0.5 },

        { +0.5, -0.5, -0.5 }, { +0.5, -0.5, +0.5 }, { +0.5, +0.5, +0.5 },
        { +0.5, +0.5, +0.5 }, { +0.5, +0.5, -0.5 }, { +0.5, -0.5, -0.5 },

        { -0.5, -0.5, -0.5 }, { -0.5, +0.5, +0.5 }, { -0.5, -0.5, +0.5 },
        { -0.5, +0.5, +0.5 }, { -0.5, -0.5, -0.5 }, { -0.5, +0.5, -0.5 }
    };


    Color colors[36];
    for (int i = 0; i < 36; i++) {
        float r = (cube[i].x + 0.5f);
        float g = (cube[i].y + 0.5f);
        float b = (cube[i].z + 0.5f);
        colors[i] = { r, g, b };
    }

    float texCoords[] = {
        // Передняя грань
        0.0f, 0.0f, // Нижний левый
        0.0f, 1.0f, // Верхний левый
        1.0f, 1.0f, // Верхний правый
        1.0f, 1.0f, // Верхний правый
        1.0f, 0.0f, // Нижний правый
        0.0f, 0.0f, // Нижний левый

        // Задняя грань
        0.0f, 0.0f, // Нижний левый
        1.0f, 1.0f, // Верхний правый
        0.0f, 1.0f, // Верхний левый
        1.0f, 1.0f, // Верхний правый
        0.0f, 0.0f, // Нижний левый
        1.0f, 0.0f, // Нижний правый

        // Верхняя грань
        0.0f, 1.0f, // Верхний левый
        0.0f, 0.0f, // Нижний левый
        1.0f, 0.0f, // Нижний правый
        1.0f, 0.0f, // Нижний правый
        1.0f, 1.0f, // Верхний правый
        0.0f, 1.0f, // Верхний левый

        // Нижняя грань
        0.0f, 1.0f, // Верхний левый
        1.0f, 0.0f, // Нижний правый
        0.0f, 0.0f, // Нижний левый
        1.0f, 0.0f, // Нижний правый
        0.0f, 1.0f, // Верхний левый
        1.0f, 1.0f, // Верхний правый

        // Правая грань
        1.0f, 0.0f, // Нижний правый
        1.0f, 1.0f, // Верхний правый
        0.0f, 1.0f, // Верхний левый
        0.0f, 1.0f, // Верхний левый
        0.0f, 0.0f, // Нижний левый
        1.0f, 0.0f, // Нижний правый

        // Левая грань
        1.0f, 0.0f, // Нижний правый
        0.0f, 1.0f, // Верхний левый
        0.0f, 0.0f, // Нижний левый
        0.0f, 1.0f, // Верхний левый
        1.0f, 0.0f, // Нижний правый
        1.0f, 1.0f, // Верхний правый
    };

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_color);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_texCoords);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);
}


void ShaderLog(unsigned int shader)
{
    int infologLen = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologLen);
    if (infologLen > 1)
    {
        int charsWritten = 0;
        std::vector<char> infoLog(infologLen);
        glGetShaderInfoLog(shader, infologLen, &charsWritten, infoLog.data());
        std::cout << "InfoLog: " << infoLog.data() << std::endl;
    }

}


void InitShader() {
    // Создаем вершинный шейдер
    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    // Передаем исходный код
    glShaderSource(vShader, 1, &VertexShaderSource, NULL);
    // Компилируем шейдер
    glCompileShader(vShader);
    std::cout << "vertex shader \n";
    // Функция печати лога шейдера
    ShaderLog(vShader); //Пример функции есть в лабораторной
    // Создаем фрагментный шейдер
    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    // Передаем исходный код
    glShaderSource(fShader, 1, &FragShaderSource, NULL);
    // Компилируем шейдер
    glCompileShader(fShader);
    std::cout << "fragment shader \n";
    // Функция печати лога шейдера
    ShaderLog(fShader);
    // Создаем программу и прикрепляем шейдеры к ней
    Program = glCreateProgram();
    glAttachShader(Program, vShader);
    glAttachShader(Program, fShader);
    // Линкуем шейдерную программу
    glLinkProgram(Program);
    // Проверяем статус сборки
    int link_ok;
    glGetProgramiv(Program, GL_LINK_STATUS, &link_ok);
    if (!link_ok) {
        std::cout << "error attach shaders \n";
        return;
    }

    // Вытягиваем ID атрибута из собранной программы
    const char* attr_name = "coord"; //имя в шейдере
    Attrib_vertex = glGetAttribLocation(Program, attr_name);
    if (Attrib_vertex == -1) {
        std::cout << "could not bind attrib " << attr_name << std::endl;
        return;
    }

    const char* attr_name_color = "vertColor";
    Attrib_color = glGetAttribLocation(Program, attr_name_color);
    if (Attrib_color == -1) {
        std::cout << "could not bind attrib " << attr_name_color << std::endl;
        return;
    }

    const char* attr_name_texCoord = "texCoord";
    Attrib_texCoord = glGetAttribLocation(Program, attr_name_texCoord);
    if (Attrib_texCoord == -1) {
        std::cout << "could not bind attrib " << attr_name_texCoord << std::endl;
        return;
    }

    const char* uniform_name_mixFactor = "mixFactor";
    Uniform_mixFactor = glGetUniformLocation(Program, uniform_name_mixFactor);
    if (Uniform_mixFactor == -1) {
        std::cout << "could not bind uniform " << uniform_name_mixFactor << std::endl;
        return;
    }
}

void InitTexture() {
    sf::Image image;
    if (!image.loadFromFile("texture1.png")) { 
        std::cerr << "Failed to load texture" << std::endl;
        return;
    }

    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.getSize().x, image.getSize().y, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, image.getPixelsPtr());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}


void Init() {
    // Шейдеры
    InitShader();
    // Вершинный буфер
    InitVBO();
    // Загружаем текстуру
    InitTexture();
    // Включаем проверку глубины
    glEnable(GL_DEPTH_TEST);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
}


void Draw() {
    glUseProgram(Program); // Устанавливаем шейдерную программу текущей
    glEnableVertexAttribArray(Attrib_vertex); // Включаем массив атрибутов
    glBindBuffer(GL_ARRAY_BUFFER, VBO); // Подключаем VBO
    glVertexAttribPointer(Attrib_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
    // Подключаем VBO_color
    glEnableVertexAttribArray(Attrib_color);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_color);
    glVertexAttribPointer(Attrib_color, 3, GL_FLOAT, GL_FALSE, 0, 0);
    // Подключаем VBO_texCoord
    glEnableVertexAttribArray(Attrib_texCoord);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_texCoords);
    glVertexAttribPointer(Attrib_texCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glUniform1f(Uniform_mixFactor, mixFactor);

    glBindBuffer(GL_ARRAY_BUFFER, 0); // Отключаем VBO
    // Выполняет рендеринг массива вершин
    glDrawArrays(GL_TRIANGLES, 0, 36); // Рисуем

    glDisableVertexAttribArray(Attrib_vertex); // Отключаем массив атрибутов
    glDisableVertexAttribArray(Attrib_color);
    glDisableVertexAttribArray(Attrib_texCoord);

    glUseProgram(0); // Отключаем шейдерную программу

    //checkOpenGLerror();
}


// Освобождение буфера
void ReleaseVBO() {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &VBO);
}


// Освобождение шейдеров
void ReleaseShader() {
    // Передавая ноль, мы отключаем шейдерную программу
    glUseProgram(0);
    // Удаляем шейдерную программу
    glDeleteProgram(Program);
}


void Release() {
    // Шейдеры
    ReleaseShader();
    // Вершинный буфер
    ReleaseVBO();
}



int main() {
    sf::Window window(sf::VideoMode(600, 600), "My OpenGL window", sf::Style::Default, sf::ContextSettings(24));
    window.setVerticalSyncEnabled(true);
    window.setActive(true);
    glewInit();
    Init();
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) { window.close(); }
            else if (event.type == sf::Event::Resized) { glViewport(0, 0, event.size.width, event.size.height); }
            else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Up) {
                    mixFactor += 0.05f;
                    if (mixFactor > 1.0f) mixFactor = 1.0f; 
                }
                else if (event.key.code == sf::Keyboard::Down) {
                    mixFactor -= 0.05f;
                    if (mixFactor < 0.0f) mixFactor = 0.0f; 
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