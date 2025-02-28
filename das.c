#pragma warning(disable:4996)
#pragma warning(disable:4703)
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include<GLFW/stb_easy_font.h>
#include<GLFW/glfw3.h>
#define GL_SILENCE_DEPRECATION
#define INCLUDE_STB_EASY_FONT_H
typedef enum symbol {
	pusto = '_',
	nolik = 'O',
	krestik = 'X',
} symbol;
typedef struct {
	symbol sign;
	int priority;
	int line_priority_X;
	int line_priority_O;
	int block;
} square;
typedef struct {
	int coordx;
	int coordy;
	int priority;
	int flag;
} squarePriority;
//------------------------------------------- OPENGL -----------------------------------------
void putKrestik(int x, int y, int kolvoKletok, int windowSize) {
	int med = kolvoKletok / 2;
	GLfloat coordx, coordy, shag, delta;
	shag = 2 / kolvoKletok;
	coordx = 0 - (med - x) * shag;
	coordy = (med - y) * shag;
	delta = shag * 0.7 * 0.5;
	glColor3f(0.953f, 0.539f, 0.184f);
	glLineWidth(10);
	glBegin(GL_LINES);
	glVertex2f(coordx - delta, coordy - delta);
	glVertex2f(coordx + delta, coordy + delta);
	glVertex2f(coordx - delta, coordy + delta);
	glVertex2f(coordx + delta, coordy - delta);
	glEnd();
}
void putNolik(int x, int y, int kolvoKletok, int windowSize) {
	int med = kolvoKletok / 2;
	GLfloat coordx, coordy, shag, rad1, rad2;
	shag = 2 / kolvoKletok;
	coordx = 0 - (med - x) * shag;
	coordy = (med - y) * shag;
	rad1 = shag * 0.8 * 0.5;
	rad2 = rad1 * 0.6;
	glColor3f(1.0f, 0.349f, 0.561f);
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(coordx, coordy);
	for (int i = 0; i <= 50; i++) {
		float a = (float)i / 50 * 3.1415 * 2.0f;
		glVertex2f(coordx + cos(a) * rad1, coordy + sin(a) * rad1);
	}
	glEnd();
	glColor3f(0.62f, 0.906f, 0.961f);
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(coordx, coordy);
	for (int i = 0; i <= 50; i++) {
		float a = (float)i / 50 * 3.1415 * 2.0f;
		glVertex2f(coordx + cos(a) * rad2, coordy + sin(a) * rad2);
	}
	glEnd();
}
void print_string(float x, float y, char* text, float r, float g, float b)
{
	static char buffer[99999]; // ~500 chars
	int num_quads;
	num_quads = stb_easy_font_print(x, y, text, NULL, buffer, sizeof(buffer));
	glColor3f(r, g, b);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 16, buffer);
	glDrawArrays(GL_QUADS, 0, num_quads * 4);
	glDisableClientState(GL_VERTEX_ARRAY);
}
void mainScreen(int windowSize) {
	glEnable(GL_SCISSOR_TEST);
	glClearColor(0.62f, 0.906f, 0.961f, 1.0f);
	glScissor(0, 0, windowSize, windowSize);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_SCISSOR_TEST);
	print_string(-57, -45, "NOUGHTS AND CROSSES", 0.137f, 0.431f, 0.588f);
	print_string(-9.5, -22, "Play", 0.055f, 0.663f, 0.306f);
	glColor3f(0.055f, 0.663f, 0.306f);
	glBegin(GL_TRIANGLES);
	glVertex2f(-13.0, 0.0);
	glVertex2f(-13.0, 30.0);
	glVertex2f(17.0, 15.0);
	glEnd();
}
void chooseDesk(int windowSize) {
	glEnable(GL_SCISSOR_TEST);
	glClearColor(0.62f, 0.906f, 0.961f, 1.0f);
	glScissor(0, 0, windowSize, windowSize);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_SCISSOR_TEST);
	print_string(-55, -50, "Select the field size", 0.137f, 0.431f, 0.588f);
	glColor3f(0.137f, 0.431f, 0.588f);
	glBegin(GL_QUADS);
	glVertex2f(-45.0, -35.0);
	glVertex2f(-45.0, -15.0);
	glVertex2f(45.0, -15.0);
	glVertex2f(45.0, -35.0);
	glEnd();
	print_string(-20, -28, "15 x 15", 1.0f, 0.349f, 0.561f);
	glColor3f(0.137f, 0.431f, 0.588f);
	glBegin(GL_QUADS);
	glVertex2f(-45.0, -10.0);
	glVertex2f(-45.0, 10.0);
	glVertex2f(45.0, 10.0);
	glVertex2f(45.0, -10.0);
	glEnd();
	print_string(-20, -3, "25 x 25", 0.953f, 0.539f, 0.184f);
	glColor3f(0.137f, 0.431f, 0.588f);
	glBegin(GL_QUADS);
	glVertex2f(-45.0, 15.0);
	glVertex2f(-45.0, 35.0);
	glVertex2f(45.0, 35.0);
	glVertex2f(45.0, 15.0);
	glEnd();
	print_string(-20, 22, "35 x 35", 1.0f, 0.349f, 0.561f);
}
void chooseSymbol(int windowSize) {
	glEnable(GL_SCISSOR_TEST);
	glClearColor(0.62f, 0.906f, 0.961f, 1.0f);
	glScissor(0, 0, windowSize, windowSize);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_SCISSOR_TEST);
	print_string(-53, -50, "Choose what do you \n want to play as", 0.137f, 0.431f, 0.588f);
	glColor3f(0.137f, 0.431f, 0.588f);
	glBegin(GL_QUADS);
	glVertex2f(-50.0, -10.0);
	glVertex2f(-50.0, 30.0);
	glVertex2f(-10.0, 30.0);
	glVertex2f(-10.0, -10.0);
	glEnd();
	glColor3f(0.137f, 0.431f, 0.588f);
	glBegin(GL_QUADS);
	glVertex2f(10.0, -10.0);
	glVertex2f(10.0, 30.0);
	glVertex2f(50.0, 30.0);
	glVertex2f(50.0, -10.0);
	glEnd();
	glColor3f(0.953f, 0.539f, 0.184f);
	glLineWidth(40);
	glBegin(GL_LINES);
	glVertex2f(-50 + 5, -10 + 5);
	glVertex2f(-10 - 5, 30 - 5);
	glVertex2f(-10 - 5, -10 + 5);
	glVertex2f(-50 + 5, 30 - 5);
	glEnd();
	glColor3f(1.0f, 0.349f, 0.561f);
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(30, 10);
	for (int i = 0; i <= 50; i++) {
		float a = (float)i / 50 * 3.1415 * 2.0f;
		glVertex2f(30 + cos(a) * 18, 10 + sin(a) * 18);
	}
	glEnd();
	glColor3f(0.137f, 0.431f, 0.588f);
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(30, 10);
	for (int i = 0; i <= 50; i++) {
		float a = (float)i / 50 * 3.1415 * 2.0f;
		glVertex2f(30 + cos(a) * 12, 10 + sin(a) * 12);
	}
	glEnd();
}
void victory(int windowSize) {
	glEnable(GL_SCISSOR_TEST);
	glClearColor(0.62f, 0.906f, 0.961f, 1.0f);
	glScissor(0, 0, windowSize, windowSize);
	glClear(GL_COLOR_BUFFER_BIT);
	glScalef(0.016, -0.016, 1);
	glDisable(GL_SCISSOR_TEST);
	print_string(-41, -38, " You won!\nCongratulations!", 0.137f, 0.431f, 0.588f);
	glColor3f(1.0f, 0.843f, 0.0f);
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(15, 6);
	for (int i = 0; i <= 50; i++) {
		float a = (float)i / 50 * 3.1415 * 2.0f;
		glVertex2f(15 + cos(a) * 9, 6 + sin(a) * 9);
	}
	glEnd();
	glColor3f(0.62f, 0.906f, 0.961f);
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(15, 6);
	for (int i = 0; i <= 50; i++) {
		float a = (float)i / 50 * 3.1415 * 2.0f;
		glVertex2f(15 + cos(a) * 5, 6 + sin(a) * 5);
	}
	glEnd();
	glColor3f(1.0f, 0.843f, 0.0f);
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(-15, 6);
	for (int i = 0; i <= 50; i++) {
		float a = (float)i / 50 * 3.1415 * 2.0f;
		glVertex2f(-15 + cos(a) * 9, 6 + sin(a) * 9);
	}
	glEnd();
	glColor3f(0.62f, 0.906f, 0.961f);
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(-15, 6);
	for (int i = 0; i <= 50; i++) {
		float a = (float)i / 50 * 3.1415 * 2.0f;
		glVertex2f(-15 + cos(a) * 5, 6 + sin(a) * 5);
	}
	glEnd();
	glColor3f(1.0f, 0.843f, 0.0f);
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(0, 10);
	for (int i = 0; i <= 50; i++) {
		float a = (float)i / 50 * 3.1415 * 2.0f;
		glVertex2f(0 + cos(a) * 15, 10 + sin(a) * 15);
	}
	glEnd();
	glColor3f(1.0f, 0.843f, 0.0f);
	glBegin(GL_QUADS);
	glVertex2f(-15.0, -7.0);
	glVertex2f(-15.0, 10.0);
	glVertex2f(15.0, 10.0);
	glVertex2f(15.0, -7.0);
	glEnd();
	glColor3f(1.0f, 0.843f, 0.0f);
	glBegin(GL_QUADS);
	glVertex2f(-3.0, 10.0);
	glVertex2f(-3.0, 40.0);
	glVertex2f(3.0, 40.0);
	glVertex2f(3.0, 10.0);
	glEnd();
	glColor3f(1.0f, 0.843f, 0.0f);
	glBegin(GL_QUADS);
	glVertex2f(-10.0, 40.0);
	glVertex2f(-10.0, 47.0);
	glVertex2f(10.0, 47.0);
	glVertex2f(10.0, 40.0);
	glEnd();
}
void fail(int windowSize) {
	glEnable(GL_SCISSOR_TEST);
	glClearColor(0.62f, 0.906f, 0.961f, 1.0f);
	glScissor(0, 0, windowSize, windowSize);
	glClear(GL_COLOR_BUFFER_BIT);
	glScalef(0.016, -0.016, 1);
	glDisable(GL_SCISSOR_TEST);
	print_string(-41, -30, "Unfortunately,\n you lost", 0.137f, 0.431f, 0.588f);
}
void renderDesk(int kolvoKletok, int windowSize) {
	glEnable(GL_SCISSOR_TEST);
	glClearColor(0.62f, 0.906f, 0.961f, 1.0f);
	glScissor(0, 0, windowSize, windowSize);
	glClear(GL_COLOR_BUFFER_BIT);
	for (int i = 1; i < kolvoKletok; i++) {
		glEnable(GL_SCISSOR_TEST);
		glClearColor(0.137f, 0.431f, 0.588f, 1.0f);
		float coordx = (float)i * (windowSize / kolvoKletok);
		glScissor(coordx - 2.5, 0, 5, windowSize);
		glClear(GL_COLOR_BUFFER_BIT);
	}
	for (int i = 1; i < kolvoKletok; i++) {
		glEnable(GL_SCISSOR_TEST);
		glClearColor(0.137f, 0.431f, 0.588f, 1.0f);
		float coordy = (float)i * (windowSize / kolvoKletok);
		glScissor(0, coordy - 2.5, windowSize, 5);
		glClear(GL_COLOR_BUFFER_BIT);
	}
	glDisable(GL_SCISSOR_TEST);
}
//-------------------------------------
//печатает все фигуры на поле
void printDesk(square** desk, int kolvoKletok, GLFWwindow* window, int windowSize) {
	printf(" ");
	for (int k = 1; k < kolvoKletok - 1; k++) {
		printf("%3d", k);
	}
	printf("\n");
	renderDesk(kolvoKletok - 2, windowSize);
	for (int i = 1; i < kolvoKletok - 1; i++) {
		printf("%3d ", i);
		for (int j = 1; j < kolvoKletok - 1; j++) {
			printf("%c ", desk[i][j].sign);
			if (desk[i][j].sign == 'X') putKrestik(j - 1, i - 1, kolvoKletok - 2, windowSize);
			if (desk[i][j].sign == 'O') putNolik(j - 1, i - 1, kolvoKletok - 2, windowSize);
		}
		printf("\n");
	}
	glfwSwapBuffers(window);
	glClear(GL_COLOR_BUFFER_BIT);
}
//функции, высчитывающие колиество поставленных вряд фигур
int checkDiagonal1(square** desk, int kolvoKletok, int coordx, int coordy, int flag) {
	symbol symb = desk[coordx][coordy].sign;
	int count = 0;
	int x = coordx;
	int y = coordy;
	while (x < kolvoKletok && y < kolvoKletok) {
		if (symb == desk[x][y].sign) count++;
		else {
			if (desk[x][y].sign != pusto) flag = 1;
			break;
		}
		x++;
		y++;
	}
	x = coordx - 1;
	y = coordy - 1;
	while (x >= 0 && y >= 0) {
		if (symb == desk[x][y].sign) count++;
		else {
			if (desk[x][y].sign != pusto) flag = 1;
			break;
		}
		x--;
		y--;
	}
	return count;
}
int checkDiagonal2(square** desk, int kolvoKletok, int coordx, int coordy, int flag) {
	symbol symb = desk[coordx][coordy].sign;
	int count = 0;
	int x = coordx;
	int y = coordy;
	while (x < kolvoKletok && y >= 0) {
		if (symb == desk[x][y].sign) count++;
		else {
			if (desk[x][y].sign != pusto) flag = 1;
			break;
		}
		x++;
		y--;
	}
	x = coordx - 1;
	y = coordy + 1;
	while (x >= 0 && y < kolvoKletok) {
		if (symb == desk[x][y].sign) count++;
		else {
			if (desk[x][y].sign != pusto) flag = 1;
			break;
		}
		x--;
		y++;
	}
	return count;
}
int checkGorizont(square** desk, int kolvoKletok, int coordx, int coordy, int flag) {
	symbol symb = desk[coordx][coordy].sign;
	int count = 0;
	int x = coordx;
	int y = coordy;
	while (x < kolvoKletok) {
		if (symb == desk[x][y].sign) count++;
		else {
			if (desk[x][y].sign != pusto) flag = 1;
			break;
		}
		x++;
	}
	x = coordx - 1;
	while (x >= 0) {
		if (symb == desk[x][y].sign) count++;
		else {
			if (desk[x][y].sign != pusto) flag = 1;
			break;
		}
		x--;
	}
	return count;
}
int checkVertikal(square** desk, int kolvoKletok, int coordx, int coordy, int flag) {
	symbol symb = desk[coordx][coordy].sign;
	int count = 0;
	int x = coordx;
	int y = coordy;
	while (y < kolvoKletok) {
		if (symb == desk[x][y].sign) count++;
		else {
			if (desk[x][y].sign != pusto) flag = 1;
			break;
		}
		y++;
	}
	y = coordy - 1;
	while (y >= 0) {
		if (symb == desk[x][y].sign) count++;
		else {
			if (desk[x][y].sign != pusto) flag = 1;
			break;
		}
		y--;
	}
	return count;
}
//ищет самое приоритетное направление и соответствующую ему клетку поля
void checkLines(square** desk, int kolvoKletok, int coordx, int coordy) {
	squarePriority count;
	count.priority = 0;
	count.flag = 0;
	int flag = 0;
	int countDiag1 = checkDiagonal1(desk, kolvoKletok, coordx, coordy, flag);
	if (countDiag1 >= count.priority) {
		if (desk[coordx + 1][coordy + 1].sign == pusto && coordx + 1 < kolvoKletok && coordy + 1 <
			kolvoKletok) {
			count.priority = countDiag1;
			count.coordx = coordx + 1;
			count.coordy = coordy + 1;
			count.flag = flag;
			desk[count.coordx][count.coordy].priority = count.priority;
			desk[count.coordx][count.coordy].block = count.flag;
			if (desk[coordx][coordy].sign == krestik && desk[count.coordx][count.coordy].line_priority_X <
				count.priority)
				desk[count.coordx][count.coordy].line_priority_X = count.priority;
			else if (desk[coordx][coordy].sign == nolik && desk[count.coordx][count.coordy].line_priority_O
				< count.priority)
				desk[count.coordx][count.coordy].line_priority_O = count.priority;
		}
		if (desk[coordx - 1][coordy - 1].sign == pusto && coordx - 1 >= 0 && coordy - 1 >= 0) {
			count.priority = countDiag1;
			count.coordx = coordx - 1;
			count.coordy = coordy - 1;
			count.flag = flag;
			desk[count.coordx][count.coordy].priority = count.priority;
			desk[count.coordx][count.coordy].block = count.flag;
			if (desk[coordx][coordy].sign == krestik && desk[count.coordx][count.coordy].line_priority_X <
				count.priority)
				desk[count.coordx][count.coordy].line_priority_X = count.priority;
			else if (desk[coordx][coordy].sign == nolik && desk[count.coordx][count.coordy].line_priority_O
				< count.priority)
				desk[count.coordx][count.coordy].line_priority_O = count.priority;
		}
	}
	flag = 0;
	int countDiag2 = checkDiagonal2(desk, kolvoKletok, coordx, coordy, flag);
	if (countDiag2 >= count.priority) {
		if (desk[coordx + 1][coordy - 1].sign == pusto && coordx + 1 < kolvoKletok && coordy - 1 >= 0)
		{
			count.priority = countDiag2;
			count.coordx = coordx + 1;
			count.coordy = coordy - 1;
			count.flag = flag;
			desk[count.coordx][count.coordy].priority = count.priority;
			desk[count.coordx][count.coordy].block = count.flag;
			if (desk[coordx][coordy].sign == krestik && desk[count.coordx][count.coordy].line_priority_X <
				count.priority)
				desk[count.coordx][count.coordy].line_priority_X = count.priority;
			else if (desk[coordx][coordy].sign == nolik && desk[count.coordx][count.coordy].line_priority_O
				< count.priority)
				desk[count.coordx][count.coordy].line_priority_O = count.priority;
		}
		if (desk[coordx - 1][coordy + 1].sign == pusto && coordx - 1 >= 0 && coordy + 1 < kolvoKletok)
		{
			count.priority = countDiag2;
			count.coordx = coordx - 1;
			count.coordy = coordy + 1;
			count.flag = flag;
			desk[count.coordx][count.coordy].priority = count.priority;
			desk[count.coordx][count.coordy].block = count.flag;
			if (desk[coordx][coordy].sign == krestik && desk[count.coordx][count.coordy].line_priority_X <
				count.priority)
				desk[count.coordx][count.coordy].line_priority_X = count.priority;
			else if (desk[coordx][coordy].sign == nolik && desk[count.coordx][count.coordy].line_priority_O
				< count.priority)
				desk[count.coordx][count.coordy].line_priority_O = count.priority;
		}
	}
	flag = 0;
	int countGor = checkGorizont(desk, kolvoKletok, coordx, coordy, flag);
	if (countGor >= count.priority) {
		if (desk[coordx + 1][coordy].sign == pusto && coordx + 1 < kolvoKletok) {
			count.priority = countGor;
			count.coordx = coordx + 1;
			count.coordy = coordy;
			count.flag = flag;
			desk[count.coordx][count.coordy].priority = count.priority;
			desk[count.coordx][count.coordy].block = count.flag;
			if (desk[coordx][coordy].sign == krestik && desk[count.coordx][count.coordy].line_priority_X <
				count.priority)
				desk[count.coordx][count.coordy].line_priority_X = count.priority;
			else if (desk[coordx][coordy].sign == nolik && desk[count.coordx][count.coordy].line_priority_O
				< count.priority)
				desk[count.coordx][count.coordy].line_priority_O = count.priority;
		}
		if (desk[coordx - 1][coordy].sign == pusto && coordx - 1 >= 0) {
			count.priority = countGor;
			count.coordx = coordx - 1;
			count.coordy = coordy;
			count.flag = flag;
			desk[count.coordx][count.coordy].priority = count.priority;
			desk[count.coordx][count.coordy].block = count.flag;
			if (desk[coordx][coordy].sign == krestik && desk[count.coordx][count.coordy].line_priority_X <
				count.priority)
				desk[count.coordx][count.coordy].line_priority_X = count.priority;
			else if (desk[coordx][coordy].sign == nolik && desk[count.coordx][count.coordy].line_priority_O
				< count.priority)
				desk[count.coordx][count.coordy].line_priority_O = count.priority;
		}
	}
	flag = 0;
	int countVertik = checkVertikal(desk, kolvoKletok, coordx, coordy, flag);
	if (countVertik >= count.priority) {
		if (desk[coordx][coordy + 1].sign == pusto && coordy + 1 < kolvoKletok) {
			count.priority = countVertik;
			count.coordx = coordx;
			count.coordy = coordy + 1;
			count.flag = flag;
			desk[count.coordx][count.coordy].priority = count.priority;
			desk[count.coordx][count.coordy].block = count.flag;
			if (desk[coordx][coordy].sign == krestik && desk[count.coordx][count.coordy].line_priority_X <
				count.priority)
				desk[count.coordx][count.coordy].line_priority_X = count.priority;
			else if (desk[coordx][coordy].sign == nolik && desk[count.coordx][count.coordy].line_priority_O
				< count.priority)
				desk[count.coordx][count.coordy].line_priority_O = count.priority;
		}
		if (desk[coordx][coordy - 1].sign == pusto && coordy - 1 >= 0) {
			count.priority = countVertik;
			count.coordx = coordx;
			count.coordy = coordy - 1;
			count.flag = flag;
			desk[count.coordx][count.coordy].priority = count.priority;
			desk[count.coordx][count.coordy].block = count.flag;
			if (desk[coordx][coordy].sign == krestik && desk[count.coordx][count.coordy].line_priority_X <
				count.priority)
				desk[count.coordx][count.coordy].line_priority_X = count.priority;
			else if (desk[coordx][coordy].sign == nolik && desk[count.coordx][count.coordy].line_priority_O
				< count.priority)
				desk[count.coordx][count.coordy].line_priority_O = count.priority;
		}
	}
}
int razrivFlyagi(square** desk, int kolvoKletok, int coordx, int coordy, symbol sign) {
	int priority = 0;
	//обе диагонали
	if (desk[coordx - 1][coordy - 1].sign == desk[coordx + 1][coordy + 1].sign && desk[coordx - 1]
		[coordy - 1].sign == sign) {
		int priority1 = checkDiagonal1(desk, kolvoKletok, coordx - 1, coordy - 1, 0);
		int priority2 = checkDiagonal1(desk, kolvoKletok, coordx + 1, coordy + 1, 0);
		if (priority1 + priority2 >= priority) priority = priority1 + priority2;
	}
	if (desk[coordx + 1][coordy - 1].sign == desk[coordx - 1][coordy + 1].sign && desk[coordx + 1]
		[coordy - 1].sign == sign) {
		int priority1 = checkDiagonal2(desk, kolvoKletok, coordx + 1, coordy - 1, 0);
		int priority2 = checkDiagonal2(desk, kolvoKletok, coordx - 1, coordy + 1, 0);
		if (priority1 + priority2 >= priority) priority = priority1 + priority2;
	}
	//горизонталь -- атаки сверху и снизу
	if (desk[coordx - 1][coordy].sign == desk[coordx + 1][coordy].sign && desk[coordx - 1][coordy].sign
		== sign) {
		int priority1 = checkGorizont(desk, kolvoKletok, coordx - 1, coordy, 0);
		int priority2 = checkGorizont(desk, kolvoKletok, coordx + 1, coordy, 0);
		if (priority1 + priority2 >= priority) priority = priority1 + priority2;
	}
	//вертикаль -- атаки снизу и сверху
	if (desk[coordx][coordy - 1].sign == desk[coordx][coordy + 1].sign && desk[coordx][coordy - 1].sign
		== sign) {
		int priority1 = checkVertikal(desk, kolvoKletok, coordx, coordy - 1, 0);
		int priority2 = checkVertikal(desk, kolvoKletok, coordx, coordy + 1, 0);
		if (priority1 + priority2 >= priority) priority = priority1 + priority2;
	}
	return priority;
}
//
void setPriority(square** desk, int kolvoKletok, int coordx, int coordy, int turn) {
	desk[coordx][coordy].priority = 0;
	desk[coordx][coordy].line_priority_X = 0;
	desk[coordx][coordy].line_priority_O = 0;
	desk[coordx][coordy].block = 0;
	for (int i = 1; i < kolvoKletok - 1; i++) {
		for (int j = 1; j < kolvoKletok - 1; j++) {
			if (desk[i][j].sign != pusto) checkLines(desk, kolvoKletok, i, j);
			else {
				int flyaga_X = razrivFlyagi(desk, kolvoKletok, i, j, krestik);
				int flyaga_O = razrivFlyagi(desk, kolvoKletok, i, j, nolik);
				if (desk[i][j].line_priority_O < flyaga_O) desk[i][j].line_priority_O = flyaga_O;
				if (desk[i][j].line_priority_X < flyaga_X) desk[i][j].line_priority_X = flyaga_X;
				if (desk[i][j].priority < flyaga_O) desk[i][j].priority = flyaga_O;
				if (desk[i][j].priority < flyaga_X) desk[i][j].priority = flyaga_X;
			}
		}
	}
}
//Ищет самую выгодную клетку
squarePriority checkPositions(square** desk, int kolvoKletok, int turn) {
	squarePriority maxPrioritet;
	maxPrioritet.priority = 0;
	maxPrioritet.coordx = 0;
	maxPrioritet.coordy = 0;
	maxPrioritet.flag = 0;
	for (int i = 1; i < kolvoKletok - 1; i++) {
		for (int j = 1; j < kolvoKletok - 1; j++) {
			if (turn % 2 == 0) { //сценарии для крестика
				if (desk[i][j].line_priority_X == 4 && maxPrioritet.priority < desk[i][j].line_priority_X) { //если находит 4 крестика, ставит пятый -- выигрышная позиция
						maxPrioritet.priority = desk[i][j].line_priority_X;
					maxPrioritet.coordx = i;
					maxPrioritet.coordy = j;
				}
				else if (desk[i][j].line_priority_O == 4 && maxPrioritet.priority < desk[i][j].line_priority_O) {
					//если находит 4 нолика, перекрывает атаку
					maxPrioritet.priority = desk[i][j].line_priority_O;
					maxPrioritet.coordx = i;
					maxPrioritet.coordy = j;
				}
				else if (desk[i][j].line_priority_O == 3 && desk[i][j].block == 0 && maxPrioritet.priority <
					desk[i][j].line_priority_O) { //если находит 3 нолика (атака не заблокирована ни с одной из сторон), перекрывает с одной стороны
					maxPrioritet.priority = desk[i][j].line_priority_O;
					maxPrioritet.coordx = i;
					maxPrioritet.coordy = j;
				}
				else if (desk[i][j].line_priority_X == 3 && maxPrioritet.priority < desk[i][j].line_priority_X) {
					//находит 3 вряд крестика (база)
					maxPrioritet.priority = desk[i][j].line_priority_X;
					maxPrioritet.coordx = i;
					maxPrioritet.coordy = j;
				}
				else if (desk[i][j].line_priority_O == 3 && desk[i][j].block == 1 && maxPrioritet.priority <
					desk[i][j].line_priority_O) { //если находит 3 нолика (заблокировано с одной стороны)
					maxPrioritet.priority = desk[i][j].line_priority_O;
					maxPrioritet.coordx = i;
					maxPrioritet.coordy = j;
				}
				else if (desk[i][j].line_priority_X >= maxPrioritet.priority && maxPrioritet.priority < desk[i]
					[j].line_priority_X) { //остальные случаи (базированная база) -- поиск крестиков
					maxPrioritet.priority = desk[i][j].line_priority_X;
					maxPrioritet.coordx = i;
					maxPrioritet.coordy = j;
				}
				else if (desk[i][j].line_priority_O >= maxPrioritet.priority && maxPrioritet.priority < desk[i]
					[j].line_priority_O) { //остальные случаи (базированная база) -- поиск ноликов
					maxPrioritet.priority = desk[i][j].line_priority_O;
					maxPrioritet.coordx = i;
					maxPrioritet.coordy = j;
				}
			}
			if (turn % 2 == 1) { //сценарии для нолика
				if (desk[i][j].line_priority_O == 4 && maxPrioritet.priority < desk[i][j].line_priority_O) { //если находит 4 нолика, ставит пятый -- выигрышная позиция
						maxPrioritet.priority = desk[i][j].line_priority_O;
					maxPrioritet.coordx = i;
					maxPrioritet.coordy = j;
				}
				else if (desk[i][j].line_priority_X == 4 && maxPrioritet.priority < desk[i][j].line_priority_X) {
					//если находит 4 крестика, перекрывает атаку
					maxPrioritet.priority = desk[i][j].line_priority_X;
					maxPrioritet.coordx = i;
					maxPrioritet.coordy = j;
				}
				else if (desk[i][j].line_priority_X == 3 && desk[i][j].block == 0 && maxPrioritet.priority <
					desk[i][j].line_priority_X) { //если находит 3 крестика (атака не заблокирована ни с одной из сторон), перекрывает с одной стороны
					maxPrioritet.priority = desk[i][j].line_priority_X;
					maxPrioritet.coordx = i;
					maxPrioritet.coordy = j;
				}
				else if (desk[i][j].line_priority_O == 3 && maxPrioritet.priority < desk[i][j].line_priority_O) {
					//находит 3 вряд нолика (база)
					maxPrioritet.priority = desk[i][j].line_priority_O;
					maxPrioritet.coordx = i;
					maxPrioritet.coordy = j;
				}
				else if (desk[i][j].line_priority_X == 3 && desk[i][j].block == 1 && maxPrioritet.priority <
					desk[i][j].line_priority_X) { //если находит 3 крестика (заблокировано с одной стороны)
					maxPrioritet.priority = desk[i][j].line_priority_X;
					maxPrioritet.coordx = i;
					maxPrioritet.coordy = j;
				}
				else if (desk[i][j].line_priority_O >= maxPrioritet.priority && turn != 1 &&
					maxPrioritet.priority < desk[i][j].line_priority_O) { //остальные случаи (базированная база) -- поиск ноликов
						maxPrioritet.priority = desk[i][j].line_priority_O;
					maxPrioritet.coordx = i;
					maxPrioritet.coordy = j;
				}
				else if (desk[i][j].line_priority_X >= maxPrioritet.priority && maxPrioritet.priority < desk[i]
					[j].line_priority_X) { //остальные случаи (базированная база) -- поиск крестиков
					maxPrioritet.priority = desk[i][j].line_priority_X;
					maxPrioritet.coordx = i;
					maxPrioritet.coordy = j;
				}
			}
		}
	}
	return maxPrioritet;
}
//Функция, проверяющая наличие победы
int checkWin(square** desk, int kolvoKletok, int windowSize, GLFWwindow* window, int option) {
	for (int i = 1; i < kolvoKletok - 1; i++) {
		for (int j = 1; j < kolvoKletok - 1; j++) {
			int pos_X = 0;
			int pos_O = 0;
			if (desk[i][j].sign == krestik) {
				if (pos_X < checkDiagonal1(desk, kolvoKletok, i, j, 0)) pos_X = checkDiagonal1(desk,
					kolvoKletok, i, j, 0);
				if (pos_X < checkDiagonal2(desk, kolvoKletok, i, j, 0)) pos_X = checkDiagonal2(desk,
					kolvoKletok, i, j, 0);
				if (pos_X < checkGorizont(desk, kolvoKletok, i, j, 0)) pos_X = checkGorizont(desk,
					kolvoKletok, i, j, 0);
				if (pos_X < checkVertikal(desk, kolvoKletok, i, j, 0)) pos_X = checkVertikal(desk, kolvoKletok,
					i, j, 0);
			}
			if (desk[i][j].sign == nolik) {
				if (pos_O < checkDiagonal1(desk, kolvoKletok, i, j, 0)) pos_O = checkDiagonal1(desk,
					kolvoKletok, i, j, 0);
				if (pos_O < checkDiagonal2(desk, kolvoKletok, i, j, 0)) pos_O = checkDiagonal2(desk,
					kolvoKletok, i, j, 0);
				if (pos_O < checkGorizont(desk, kolvoKletok, i, j, 0)) pos_O = checkGorizont(desk,
					kolvoKletok, i, j, 0);
				if (pos_O < checkVertikal(desk, kolvoKletok, i, j, 0)) pos_O = checkVertikal(desk, kolvoKletok,
					i, j, 0);
			}
			if (pos_O >= 5) {
				printf("Nolik won!!! Congrat!!!");
				if (option == 1) return 6;
				else return 5;
			}
			else if (pos_X >= 5) {
				printf("Krestik won!!! Congrat!!!");
				if (option == 1) return 5;
				else return 6;
			}
		}
	}
	return 4;
}
//---------------------------------------------------------------------------------------------------
int main(void) {
	int popa = 0;
	int kolvoKletok = 0;
	square** desk;
	int option = 0;
	GLFWwindow* window;
	int windowSize = 1050;
	if (!glfwInit())return -1;
	window = glfwCreateWindow(windowSize, windowSize, "Krestik-nolic2024", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}
	glClear(GL_COLOR_BUFFER_BIT);
	glfwMakeContextCurrent(window);
	double x, y;
	int flag = 0;
	glClear(GL_COLOR_BUFFER_BIT);
	int step = 0;
	int coordx = 0, coordy = 0;
	int turn;
	glScalef(0.016, -0.016, 1);
	mainScreen(windowSize);
	glfwSwapBuffers(window);
	while (!glfwWindowShouldClose(window)) {
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
			glfwGetCursorPos(window, &x, &y);
			flag = 1;
		}
		else if (flag == 1) {
			flag = 0;
			if (step == 0) {
				glClear(GL_COLOR_BUFFER_BIT);
				if (x > (windowSize / 2) * 0.8 && x < (windowSize / 2) * 1.3 && y >(double)windowSize / 2 &&
					y < (windowSize / 2) * 1.45) {
					step++;
					chooseDesk(windowSize);
					glfwSwapBuffers(window);
				}
			}
			else if (step == 1) {
				glClear(GL_COLOR_BUFFER_BIT);
				if (x > (windowSize / 2) * 0.28 && x < (windowSize / 2) * 1.72 && y >(windowSize / 2) * 0.45
					&& y < (windowSize / 2) * 0.76) {
					popa = 15;
					step++;
				}
				else if (x > (windowSize / 2) * 0.28 && x < (windowSize / 2) * 1.72 && y >(windowSize / 2) *
					0.83 && y < (windowSize / 2) * 1.16) {
					popa = 25;
					step++;
				}
				else if (x > (windowSize / 2) * 0.28 && x < (windowSize / 2) * 1.72 && y >(windowSize / 2) *
					1.24 && y < (windowSize / 2) * 1.56) {
					popa = 35;
					step++;
				}
				else continue;
				kolvoKletok = popa + 2;
				desk = (square**)malloc(sizeof(square*) * kolvoKletok);
				for (int i = 0; i < kolvoKletok; i++) {
					desk[i] = (square*)malloc(sizeof(square) * kolvoKletok);
				}
				for (int i = 0; i < kolvoKletok; i++) {
					for (int j = 0; j < kolvoKletok; j++) {
						desk[i][j].priority = 0;
						desk[i][j].line_priority_X = 0;
						desk[i][j].line_priority_O = 0;
						desk[i][j].sign = pusto;
						desk[i][j].block = 0;
					}
				}
				chooseSymbol(windowSize);
				glfwSwapBuffers(window);
			}
			else if (step == 2) {
				glClear(GL_COLOR_BUFFER_BIT);
				chooseSymbol(windowSize);
				glfwSwapBuffers(window);
				if (x > (windowSize / 2) * 0.3 && x < (windowSize / 2) * 0.86 && y >(windowSize / 2) * 0.86
					&& y < (windowSize / 2) * 1.42) {
					option = 1;
					step++;
					printf("Krestik");
				}
				else if (x > (windowSize / 2) * 1.14 && x < (windowSize / 2) * 1.7 && y >(windowSize / 2) *
					0.86 && y < (windowSize / 2) * 1.42) {
					option = 2;
					step++;
					printf("Nolik");
				}
				glScalef(62.5, -62.5, 1);
				if (option == 1) {
					renderDesk(kolvoKletok - 2, windowSize);
					glfwSwapBuffers(window);
					turn = 0;
					step++;
				}
				else if (option == 2) {
					turn = 0;
					coordx = (kolvoKletok - 2) / 2 + 1;
					coordy = (kolvoKletok - 2) / 2 + 1;
					if (desk == NULL) return 1;
					desk[coordx][coordy].sign = krestik;
					setPriority(desk, kolvoKletok, coordx, coordy, turn);
					turn++;
					printDesk(desk, kolvoKletok, window, windowSize);
					step++;
				}
				else {
					printf("Ups! Incorrect option!");
					continue;
				}
			}
			else if (step == 4) {
				glClear(GL_COLOR_BUFFER_BIT);
				coordy = ((int)x / (windowSize / (kolvoKletok - 2))) + 1;
				coordx = ((int)y / (windowSize / (kolvoKletok - 2))) + 1;
				printf("(%d %d)\n", coordx, coordy);
				if (desk[coordx][coordy].sign != pusto) {
					printf("Oh no! There is already a sign here!\n");
					continue;
				}
				if (turn % 2 == 0) desk[coordx][coordy].sign = krestik;
				else desk[coordx][coordy].sign = nolik;
				setPriority(desk, kolvoKletok, coordx, coordy, turn);
				turn++;
				printDesk(desk, kolvoKletok, window, windowSize);
				if (turn > 6) step = checkWin(desk, kolvoKletok, windowSize, window, option);
				if (step == 5 || step == 6) continue;
				coordx = checkPositions(desk, kolvoKletok, turn).coordx;
				coordy = checkPositions(desk, kolvoKletok, turn).coordy;
				if (turn % 2 == 0) desk[coordx][coordy].sign = krestik;
				else desk[coordx][coordy].sign = nolik;
				setPriority(desk, kolvoKletok, coordx, coordy, turn);
				turn++;
				printDesk(desk, kolvoKletok, window, windowSize);
				if (turn > 6) step = checkWin(desk, kolvoKletok, windowSize, window, option);
				if (step == 5 || step == 6) continue;
			}
			else if (step == 5) {
				step += 2;
				victory(windowSize);
				glfwSwapBuffers(window);
			}
			else if (step == 6) {
				step++;
				fail(windowSize);
				glfwSwapBuffers(window);
			}
			else if (step == 7) {
				break;
			}
		}
		glfwPollEvents();
	}
	glfwTerminate();
	for (int i = 0; i < kolvoKletok; i++) {
		free(desk[i]);
	}
	free(desk);
	return 0;
}