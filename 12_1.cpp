//#include <some_library_header.h>

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>

using namespace std;
// ID шейдерной программы
GLuint Program;
// ID атрибута
GLint Attrib_vertex;
// ID Vertex Buffer Object
GLuint VBO;


GLint Uniform_angleX;
GLint Uniform_angleY;
GLint Uniform_angleZ;

GLint Attrib_color;

// ID VBO цвета
GLuint VBO_color;

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
 in vec3 coord;
in vec3 vertColor;    
out vec3 fragColor; 
uniform float angleX, angleY, angleZ;

        mat4 rotateX(float angle) {
            float rad = radians(angle);
            return mat4(
                1.0, 0.0, 0.0, 0.0,
                0.0, cos(rad), -sin(rad), 0.0,
                0.0, sin(rad), cos(rad), 0.0,
                0.0, 0.0, 0.0, 1.0
            );
        }

        mat4 rotateY(float angle) {
            float rad = radians(angle);
            return mat4(
                cos(rad), 0.0, sin(rad), 0.0,
                0.0, 1.0, 0.0, 0.0,
                -sin(rad), 0.0, cos(rad), 0.0,
                0.0, 0.0, 0.0, 1.0
            );
        }

        mat4 rotateZ(float angle) {
            float rad = radians(angle);
            return mat4(
                cos(rad), -sin(rad), 0.0, 0.0,
                sin(rad), cos(rad), 0.0, 0.0,
                0.0, 0.0, 1.0, 0.0,
                0.0, 0.0, 0.0, 1.0
            );
        }
 void main() {
 mat4 rotation = rotateX(angleX) * rotateY(angleY) * rotateZ(angleZ);
 gl_Position = rotation * vec4(coord, 1.0);
fragColor = vertColor; 
 }
)";

// Исходный код фрагментного шейдера
//const char* FragShaderSource = R"(
// #version 330 core
// out vec4 color;
// void main() {
// color = vec4(0, 1, 0, 1);
// }
//)";

const char* FragShaderSource = R"(
#version 330 core
in vec3 fragColor;  // цвет из вершинного шейдера
out vec4 color; 
void main() {
     color = vec4(fragColor, 1.0); 
}
)";

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
	cout << "vertex shader \n";
	// Функция печати лога шейдера
	ShaderLog(vShader);
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

	// Вытягиваем ID uniform-переменной
	//const char* uniform_name = "color"; // имя в шейдере
	//Uniform_color = glGetUniformLocation(Program, uniform_name);
	//if (Uniform_color == -1) {
	//	std::cout << "could not bind uniform " << uniform_name << std::endl;
	//	return;
	//}


	Uniform_angleX = glGetUniformLocation(Program, "angleX");
	if (Uniform_angleX == -1) {
		std::cout << "could not bind angleX " << std::endl;
		return;
	}
	Uniform_angleY = glGetUniformLocation(Program, "angleY");
	if (Uniform_angleY == -1) {
		std::cout << "could not bind angleY " << std::endl;
		return;
	}
	Uniform_angleZ = glGetUniformLocation(Program, "angleZ");
	if (Uniform_angleZ == -1) {
		std::cout << "could not bind angleZ " << std::endl;
		return;
	}
	Attrib_color = glGetAttribLocation(Program, "vertColor");
	if (Attrib_color == -1) {
		std::cerr << "Error: Could not bind attribute 'vertColor'." << std::endl;
		return;
	}
	//checkOpenGLerror();
}

void Draw(float angleX, float angleY, float angleZ) { //куб
	glUseProgram(Program); // Устанавливаем шейдерную программу текущей

	glUniform1f(Uniform_angleX, angleX);
	glUniform1f(Uniform_angleY, angleY);
	glUniform1f(Uniform_angleZ, angleZ);

	//glUniform4f(Uniform_color, 0.0f, 0.5f, 1.0f, 1.0f);
	glEnableVertexAttribArray(Attrib_vertex); // Включаем массив атрибутов
	glBindBuffer(GL_ARRAY_BUFFER, VBO); // Подключаем VBO
	// Указываем, как и куда читаем данные из VBO
	glVertexAttribPointer(Attrib_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(Attrib_color);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_color);
	glVertexAttribPointer(Attrib_color, 3, GL_FLOAT, GL_FALSE, sizeof(Color), (void*)0);


	glBindBuffer(GL_ARRAY_BUFFER, 0); // Отключаем VBO


	glDrawArrays(GL_TRIANGLES, 0, 12); // Рисуем


	glDisableVertexAttribArray(Attrib_vertex); // Отключаем массив атрибутов
	glUseProgram(0); // Отключаем шейдерную программу

}




void InitVBO() {


	glGenBuffers(1, &VBO);
	glGenBuffers(1, &VBO_color);
	Vertex triangle[] = {
		
		{ 0.0f, 0.5f, 0.0f }, { -0.5f, -0.5f, 0.5f }, { 0.5f, -0.5f, 0.5f },
		
		{ 0.0f, 0.5f, 0.0f }, { 0.5f, -0.5f, 0.5f }, { 0.0f, -0.5f, -0.5f },
	
		{ 0.0f, 0.5f, 0.0f }, { 0.0f, -0.5f, -0.5f }, { -0.5f, -0.5f, 0.5f },
		
		{ -0.5f, -0.5f, 0.5f }, { 0.5f, -0.5f, 0.5f }, { 0.0f, -0.5f, -0.5f }
	};
	Color colors[] = {
		 { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f },
	
		 { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 0.0f },
		
		 { 1.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f, 1.0f },
		 
		 { 1.0f, 0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.5f, 0.5f, 0.5f }
	};




	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// Передаем вершины в буфер
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_color);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

}

void Init() {
	// Шейдеры
	InitShader();
	// Вершинный буфер
	InitVBO();

	glEnable(GL_DEPTH_TEST);
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
	float x = 0.0f, y = 0.0f, z = 0.0f;
	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) { window.close(); }
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) y += 5.0f;
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) y -= 5.0f;
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) x -= 5.0f;
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) x += 5.0f;
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) z += 5.0f;
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::E)) z -= 5.0f;
			else if (event.type == sf::Event::Resized) { glViewport(0, 0, event.size.width, event.size.height); }
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		Draw(x, y, z);
		window.display();
	}
	Release();
	return 0;
}

