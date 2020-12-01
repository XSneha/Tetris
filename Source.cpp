#include<windows.h>
#include<math.h>
#include<stdio.h>
#include<stdlib.h>
#include<GL/gl.h>
#include<GL/glu.h>
#include <time.h>
#include "MyIcon.h"

//#pragma once
#define MYICON 101

#pragma comment(lib,"OpenGl32.lib")
#pragma comment(lib,"GLU32.lib")

#define WIN_WIDTH 1300
#define WIN_HEIGHT 737

GLfloat win_width = WIN_WIDTH;
GLfloat win_height = WIN_HEIGHT;

//shapes
#define SQUARE 0
#define DOT 1
#define LINE 2
#define L 3
#define T 4
#define Z 5

//vertical and horizontal
//#define V 15
//#define H 10

GLfloat angle = 0.00f;
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
GLuint pink_texture;
GLuint red_texture;
GLuint yellow_texture;
GLuint orange_texture;
GLuint green_texture;
GLuint cyan_texture;
GLuint blue_texture;

HWND ghwnd;
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };
DWORD dwStyle;
FILE* gpFile = NULL;
bool gbActive = false;
bool gbFscreen = false;

HDC ghdc = NULL;
HGLRC ghrc = NULL;

//tatris variables

bool falling = false;

//game grid properties
GLfloat left = -5 * 0.05f;
GLfloat right = 5 * 0.05f;
GLfloat top = 7 * 0.05f;
GLfloat bottom = -8 * 0.05f;
GLfloat stackbottom = -7 * 0.05f;

//window bottom upto where the shape can translate on -y axis
GLfloat bot[10] = { -7 * 0.05f ,-7 * 0.05f ,-7 * 0.05f ,-7 * 0.05f ,-7 * 0.05f ,-7 * 0.05f,-7 * 0.05f, -7 * 0.05f ,-7 * 0.05f ,-7 * 0.05f };

//y transformation of current shape
GLfloat distance = 0.05f;

//nextShape
int nextShape;
//properties of current shape
int currentShape = SQUARE;
GLfloat shapePositionX = 0.0f;
GLfloat shapePositionY = 7 * 0.05f;
GLfloat shapeWidth = 0.5f;
GLfloat shapeHeight = 0.5f;
GLfloat shapeAngle = 0.0f;
int rotation = 0;

//array of shape[width,height]
GLfloat square[2] = { 0.5f , 0.5f };
GLfloat dot[2] = { 0.25f,0.25f };
GLfloat ln[2] = { 0.25f,0.75f };
GLfloat l[2] = { 0.5f , 0.5f };
GLfloat t[2] = { 0.5f , 0.5f };

struct node {
	int data[10];
	struct node* next;
};

struct node* list = NULL;

//to identify the column 
int x = 4;

//font
GLuint nFontList = 0;

//score
int level = 0, score = 0;

bool over = false;

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpStrCmd, int iCmdShow) {
	//function declerations
	void Initialize(void);
	void Display(void);
	void update(void);

	//variables
	WNDCLASSEX wndclass;
	bool bDone = false;
	MSG msg;
	HWND hwnd;
	RECT rct;
	TCHAR appName[] = TEXT("SPNING TRIANGLE");

	if (fopen_s(&gpFile, "MyLog.txt", "w") != 0) {
		MessageBox(NULL, TEXT("Failed to Open file Mylog.txt"), TEXT("ERROR"), MB_OK);
		return (0);
	}

	//initialise wnd class
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.hInstance = hInstance;
	wndclass.lpszClassName = appName;
	wndclass.lpfnWndProc = WndProc;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));
	wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));
	wndclass.lpszMenuName = NULL;
	wndclass.cbWndExtra = 0;
	wndclass.cbClsExtra = 0;

	//register wndclass
	RegisterClassEx(&wndclass);

	SystemParametersInfo(SPI_GETWORKAREA, 0, &rct, 0);
	int sheight = rct.bottom - rct.top;
	int swidth = rct.right - rct.left;

	//create windows
	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
		appName,
		TEXT("Tetris"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_VISIBLE,
		(swidth - WIN_WIDTH) / 2,
		(sheight - WIN_HEIGHT) / 2,
		WIN_WIDTH,
		WIN_HEIGHT,
		0,
		0,
		hInstance,
		NULL);

	ghwnd = hwnd;
	//show window
	ShowWindow(hwnd, iCmdShow);
	Initialize();
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);
	//game loop
	while (bDone == false) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				bDone = true;
			}
			else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else {
			if (gbActive == true) {
				Display();
				//update();
			}
		}
	}
	return (msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
	void ToggleFullScreen(void);
	void Uninitialize(void);
	void Resize(int width, int height);
	switch (iMsg) {
	case WM_SETFOCUS: gbActive = true;
		break;
	case WM_KILLFOCUS:gbActive = false;
		break;
	case WM_ERASEBKGND: return 0;
	case WM_SIZE:
		Resize(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_KEYDOWN:
		switch (wParam) {
		case 'f':
		case'F': ToggleFullScreen();
			break;
		case 'L':
		case 'l' :
			rotation--;
			shapeAngle -= 90.0f;
			if (rotation == -4) {
				rotation = 0;
				shapeAngle = 0.0f;
			}
			break;
		case 'R' :
		case 'r':
			rotation++;
			shapeAngle += 90.0f;
			if (rotation == 4) {
				shapeAngle = 0.0f;
			}
			break;
		case VK_LEFT:
			if (shapePositionX > (left + (1.0f * 0.05f))) {
				shapePositionX = shapePositionX - 0.05f;
				x--;
			}
			break;
		case VK_RIGHT:
			if (shapePositionX < (right - (1.0f * 0.05f))) {
				shapePositionX = shapePositionX + 0.05f;
				x++;
			}
			break;
		case VK_ESCAPE: DestroyWindow(hwnd);
			break;
		}
		break;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		Uninitialize();
		PostQuitMessage(0);
		break;
	default:break;
	}
	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}

void ToggleFullScreen(void) {
	MONITORINFO mi = { sizeof(MONITORINFO) };
	if (gbFscreen == false) {
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
		if (dwStyle & WS_OVERLAPPEDWINDOW) {
			if (GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi)) {
				SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
				SetWindowPos(ghwnd,
					HWND_TOP,
					mi.rcMonitor.left,
					mi.rcMonitor.top,
					mi.rcMonitor.right - mi.rcMonitor.left,
					mi.rcMonitor.bottom - mi.rcMonitor.top,
					SWP_FRAMECHANGED | SWP_NOZORDER
				);
			}
			win_width =(GLfloat) mi.rcMonitor.right - mi.rcMonitor.left;
			win_height = (GLfloat)mi.rcMonitor.bottom - mi.rcMonitor.top;
		}
		gbFscreen = true;
		ShowCursor(false);
	}
	else {
		win_width = WIN_WIDTH;
		win_height = WIN_HEIGHT;
		ShowCursor(true);
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle);
		SetWindowPlacement(ghwnd, &wpPrev);
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);
		gbFscreen = false;
	}
}

void Resize(int width, int height) {
	if (height == 0) {
		height = 1;
	}
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
}

void Initialize(void) {
	//function decleration 
	void Resize(int, int);
	bool LoadGLTexture(GLuint*, TCHAR[]);

	//variable declertion
	INT iPixelFormatIndex;
	PIXELFORMATDESCRIPTOR pfd;

	//code
	ghdc = GetDC(ghwnd);
	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 32;
	pfd.cRedBits = 8;
	pfd.cGreenBits = 8;
	pfd.cBlueBits = 8;
	pfd.cAlphaBits = 8;

	iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
	if (iPixelFormatIndex == 0) {
		fprintf(gpFile, "ChoosePixelFormat() Failed");
		DestroyWindow(ghwnd);
	}
	if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE) {
		fprintf(gpFile, "SetPixelFormat() Failed");
		DestroyWindow(ghwnd);
	}
	ghrc = wglCreateContext(ghdc);
	if (ghrc == NULL) {
		fprintf(gpFile, "wglCreateContext() Failed");
		DestroyWindow(ghwnd);
	}
	if (wglMakeCurrent(ghdc, ghrc) == FALSE) {
		fprintf(gpFile, "wglMakeCurrent() Failed");
		DestroyWindow(ghwnd);
	}
	glShadeModel(GL_SMOOTH);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	//font
	//Font Initalize
	HFONT hFont;
	LOGFONT logfont;
	logfont.lfHeight = -20;
	logfont.lfWidth = 0;
	logfont.lfEscapement = 0;
	logfont.lfOrientation = 0;
	logfont.lfWeight = FW_BOLD;
	logfont.lfItalic = FALSE;
	logfont.lfUnderline = FALSE;
	logfont.lfStrikeOut = FALSE;
	logfont.lfCharSet = ANSI_CHARSET;
	logfont.lfOutPrecision = OUT_DEFAULT_PRECIS;
	logfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	logfont.lfQuality = DEFAULT_QUALITY;
	logfont.lfPitchAndFamily = DEFAULT_PITCH;
	// Create the font and display list
	hFont = CreateFontIndirect(&logfont);
	SelectObject(ghdc, hFont);
	nFontList = glGenLists(128);
	wglUseFontBitmaps(ghdc, 0, 128, nFontList);
	DeleteObject(hFont);

	//texture
	glEnable(GL_TEXTURE_2D);
	LoadGLTexture(&pink_texture, MAKEINTRESOURCE(PINK_TILE));
	LoadGLTexture(&red_texture, MAKEINTRESOURCE(RED_TILE));
	LoadGLTexture(&yellow_texture, MAKEINTRESOURCE(YELLOW_TILE));
	LoadGLTexture(&orange_texture, MAKEINTRESOURCE(ORANGE_TILE));
	LoadGLTexture(&green_texture, MAKEINTRESOURCE(GREEN_TILE));
	LoadGLTexture(&cyan_texture, MAKEINTRESOURCE(CYAN_TILE));
	LoadGLTexture(&blue_texture, MAKEINTRESOURCE(BLUE_TILE));
	glBindTexture(GL_TEXTURE_2D, 0);

	Resize(WIN_WIDTH, WIN_HEIGHT);
}

bool LoadGLTexture(GLuint* texture, TCHAR resourceId[]) {
	bool bResult = false;
	HBITMAP hBitmap = NULL;
	BITMAP bmp;

	hBitmap = (HBITMAP)LoadImage(GetModuleHandle(NULL), resourceId, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);

	if (hBitmap) {
		bResult = true;
		GetObject(hBitmap, sizeof(BITMAP), &bmp);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		glGenTextures(1, texture);
		glBindTexture(GL_TEXTURE_2D, *texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		gluBuild2DMipmaps(GL_TEXTURE_2D, 3, bmp.bmWidth, bmp.bmHeight, GL_BGR_EXT, GL_UNSIGNED_BYTE, bmp.bmBits);
		DeleteObject(hBitmap);
	}
	return bResult;
}

void Display(void) {
	void update();
	void RandomFallingShape();
	void DrawSettledPixels();
	void DrawNextShape();
	void DrawInstructionBoard();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);

	glViewport((GLint)(win_width /15.0f ), (GLint)0, (GLsizei)(win_width / 3.5f), (GLsizei)win_height);
	DrawNextShape();
	glBindTexture(GL_TEXTURE_2D, 0);
	glColor3f(1.0f,1.0f,1.0f);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -1.0f);


	glViewport((GLint)0, (GLint)0, (GLsizei)win_width, (GLsizei)win_height);

	//grid graph 
	// vertical lines
	
	for (int i = -5; i < 6; i++) {
		glBegin(GL_LINES);
		if(i==-5 || i==5)
		glColor3f(1.0f, 1.0f, 1.0f);
		else
		glColor3f(0.0f, 0.0f, 0.0f);
		glVertex3f(i * 0.05f, bottom, 0.0f);
		glVertex3f(i * 0.05f, top, 0.0f);
		glEnd();
	}
	//horizontal lines
	for (int j = -8; j < 8; j++) {
		glBegin(GL_LINES);
		if (j == -8 || j == 7)
			glColor3f(1.0f, 1.0f, 1.0f);
		else
			glColor3f(0.0f, 0.0f, 0.0f);
		glVertex3f(left, j * 0.05f, 0.0f);
		glVertex3f(right, j * 0.05f, 0.0f);
		glEnd();
	}

	RandomFallingShape();
	glBindTexture(GL_TEXTURE_2D, 0);
	glColor3f(1.0f, 1.0f, 1.0f);

	DrawSettledPixels();
	glBindTexture(GL_TEXTURE_2D, 0);
	glColor3f(1.0f, 1.0f, 1.0f);

	glViewport((GLint)(win_width /1.54) , (GLint)0, (GLsizei)(win_width / 3.5), (GLsizei)win_height);
	DrawInstructionBoard();
	glBindTexture(GL_TEXTURE_2D, 0);
	glColor3f(1.0f, 1.0f, 1.0f);

	update();
	SwapBuffers(ghdc);
}

void DrawInstructionBoard() {
	void drawShapeWithHeightAndWidth(GLfloat shapeWidth, GLfloat shapeHeight, GLfloat textureWidth, GLfloat textureHeight);
	void drawLShapeHeightAndWidth(GLfloat shapeWidth, GLfloat shapeHeight, GLfloat textureWidth, GLfloat textureHeight);

	GLfloat shapeWidth, shapeHeight , instructionY=0.0f ;
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -1.0f);
	glBegin(GL_QUADS);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(-0.6f, top, 0.0f);
	glVertex3f(-0.6f, bottom, 0.0f);
	glVertex3f(0.6f, bottom, 0.0f);
	glVertex3f(0.6f, top, 0.0f);
	glEnd();

	glLoadIdentity();
	glTranslatef(-0.55f, top - 0.1, -1.0f);

	glColor3f(0.0f,0.0f,0.1f);
	glRasterPos3f(0.3f, 0.0f, 0.0f);
	glListBase(nFontList);
	glCallLists(12, GL_UNSIGNED_BYTE, "INSTRUCTIONS");
	instructionY -= 0.05f;
	instructionY -= 0.05f;
	glRasterPos3f(0.0f, instructionY, 0.0f);
	glCallLists(25, GL_UNSIGNED_BYTE, "Press 'F' For Full Screen");
	instructionY -= 0.05f;
	glRasterPos3f(0.0f, instructionY, 0.0f);
	glCallLists(36, GL_UNSIGNED_BYTE, "Right arrow key to move right");
	instructionY -= 0.05f;
	glRasterPos3f(0.0f, instructionY, 0.0f);
	glCallLists(36, GL_UNSIGNED_BYTE, "Left arrow key to move left");
	instructionY -= 0.05f;
	glRasterPos3f(0.0f, instructionY, 0.0f);
	glCallLists(31, GL_UNSIGNED_BYTE, "'L' to rotate in left direction");
	instructionY -= 0.05f;
	glRasterPos3f(0.0f, instructionY, 0.0f);
	glCallLists(36, GL_UNSIGNED_BYTE, "'R' to rotate in right direction");
}

void DrawNextShape() {
	void drawShapeWithHeightAndWidth(GLfloat shapeWidth, GLfloat shapeHeight, GLfloat textureWidth, GLfloat textureHeight);
	void drawLShapeHeightAndWidth(GLfloat shapeWidth, GLfloat shapeHeight, GLfloat textureWidth, GLfloat textureHeight);

	GLfloat shapeWidth, shapeHeight, instructionY = 0.0f;
	char *sscore;
	char *slevel;

	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -1.0f);
	glBegin(GL_QUADS);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(-0.6f, top, 0.0f);
	glVertex3f(-0.6f, bottom, 0.0f);
	glVertex3f(0.6f, bottom, 0.0f);
	glVertex3f(0.6f, top, 0.0f);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(0.0f, 0.0f, 0.0f);
	glVertex3f(-0.45f, top - 0.02f, 0.0f);
	glVertex3f(-0.45f, top - 0.20f, 0.0f);
	glVertex3f(0.45f, top - 0.20f, 0.0f);
	glVertex3f(0.45f, top - 0.02f, 0.0f);
	glEnd();

	if (over == false) {
		glLoadIdentity();
		//glViewport(0, 0, win_width /4, win_height);
		glTranslatef(0, top - 0.11f, -1.0f);
		glScalef(0.355f, 0.1f, 0.1f);
		if (SQUARE == nextShape) {
			glColor3f(1.0f, 1.0f, 1.0f);
			shapeWidth = square[0];
			shapeHeight = square[1];
			glBindTexture(GL_TEXTURE_2D, yellow_texture);
			drawShapeWithHeightAndWidth(shapeWidth, shapeHeight, 2.0f, 2.0f);
		}
		else if (DOT == nextShape) {
			glColor3f(1.0f, 1.0f, 1.0f);
			shapeWidth = dot[0];
			shapeHeight = dot[1];
			glBindTexture(GL_TEXTURE_2D, red_texture);
			drawShapeWithHeightAndWidth(shapeWidth, shapeHeight, 1.0f, 1.0f);
		}
		else if (LINE == nextShape) {
			glColor3f(1.0f, 1.0f, 1.0f);
			shapeWidth = ln[0];
			shapeHeight = ln[1];
			glBindTexture(GL_TEXTURE_2D, cyan_texture);
			drawShapeWithHeightAndWidth(shapeWidth, shapeHeight, 1.0f, 3.0f);
		}
		else if (L == nextShape) {
			glColor3f(0.0f, 1.0f, 1.0f);
			shapeWidth = shapeWidth = square[0];
			shapeHeight = ln[1];
			glBindTexture(GL_TEXTURE_2D, orange_texture);
			drawLShapeHeightAndWidth(shapeWidth, shapeHeight, 2.0f, 3.0f);
		}
	}
	glLoadIdentity();
	glTranslatef(-0.60f, top - 0.3, -1.0f);
	glColor3f(0.0f, 0.0f, 0.1f);
	glListBase(nFontList);
	if (over == false) {
		glRasterPos3f(0.3f, 0.0f, 0.0f);
		glCallLists(12, GL_UNSIGNED_BYTE, "SCORE BOARD");
	}
	else{
		glRasterPos3f(0.35f, 0.0f, 0.0f);
		glCallLists(12, GL_UNSIGNED_BYTE, "GAME OVER");
	}
	instructionY -= 0.07f;
	glRasterPos3f(0.45f, instructionY, 0.0f);
	glCallLists(5, GL_UNSIGNED_BYTE, "LEVEL");

	char buffer[128];
	int ret = snprintf(buffer, sizeof(buffer), "%ld", level);
	slevel = buffer; //String terminator is added by snprintf

	instructionY -= 0.05f;
	glRasterPos3f(0.55f, instructionY, 0.0f);
	glCallLists(2, GL_UNSIGNED_BYTE, slevel);

	instructionY -= 0.05f;
	glRasterPos3f(0.45f, instructionY, 0.0f);
	glCallLists(5, GL_UNSIGNED_BYTE, "SCORE");


	ret = snprintf(buffer, sizeof(buffer), "%ld", score);
	sscore = buffer; //String terminator is added by snprintf

	instructionY -= 0.05f;
	glRasterPos3f(0.55f, instructionY, 0.0f);
	glCallLists(4, GL_UNSIGNED_BYTE, sscore);

}

void RandomFallingShape() {
	void drawShapeWithHeightAndWidth(GLfloat shapeWidth, GLfloat shapeHeight, GLfloat textureWidth, GLfloat textureHeight);
	void drawLShapeHeightAndWidth(GLfloat shapeWidth, GLfloat shapeHeight, GLfloat textureWidth, GLfloat textureHeight);
	glLoadIdentity();
	glTranslatef(shapePositionX, shapePositionY, -1.0f);
	glRotatef(shapeAngle,0.0f,0.0f,1.0f);
	glScalef(0.1f, 0.1f, 0.1f);
	if (SQUARE == currentShape) {
		glBindTexture(GL_TEXTURE_2D, yellow_texture);
		drawShapeWithHeightAndWidth(shapeWidth, shapeHeight, 2.0f, 2.0f);
	}
	if (DOT == currentShape) {
		glBindTexture(GL_TEXTURE_2D, red_texture);
		drawShapeWithHeightAndWidth(shapeWidth, shapeHeight, 1.0f, 1.0f);
	}
	//	glColor3f(1.0f, 0.0f, 0.0f);
	if (LINE == currentShape) {
		glBindTexture(GL_TEXTURE_2D, cyan_texture);
		drawShapeWithHeightAndWidth(shapeWidth, shapeHeight, 1.0f, 3.0f);
	}
	else if (L == nextShape) {
		glColor3f(0.0f, 1.0f, 1.0f);
		shapeWidth = shapeWidth = square[0];
		shapeHeight = ln[1];
		glBindTexture(GL_TEXTURE_2D, orange_texture);
		drawLShapeHeightAndWidth(shapeWidth, shapeHeight, 2.0f, 3.0f);
	}
}

void drawLShapeHeightAndWidth(GLfloat shapeWidth, GLfloat shapeHeight, GLfloat textureWidth, GLfloat textureHeight) {
	glBegin(GL_QUADS);
	glTexCoord2d(0.0f, textureHeight);
	glVertex3f(-shapeWidth, shapeHeight, 0.0f);
	glTexCoord2d(0.0f, 0.0f);
	glVertex3f(-shapeWidth, -shapeHeight, 0.0f);
	glTexCoord2d(textureWidth, 0.0f);
	glVertex3f(shapeWidth, -shapeHeight, 0.0f);
	glTexCoord2d(textureWidth, textureHeight);
	glVertex3f(shapeWidth, -(2 * shapeHeight) / 3, 0.0f);
	glTexCoord2d(textureWidth, textureHeight);
	glVertex3f(0.0f, -(2*shapeHeight)/ 3, 0.0f);
	glTexCoord2d(textureWidth, textureHeight);
	glVertex3f(0.0f,  shapeHeight, 0.0f);
	glEnd();
}

void drawShapeWithHeightAndWidth(GLfloat shapeWidth, GLfloat shapeHeight ,GLfloat textureWidth, GLfloat textureHeight) {
	glBegin(GL_QUADS);
	glTexCoord2d(0.0f, textureHeight);
	glVertex3f(-shapeWidth, shapeHeight, 0.0f);
	glTexCoord2d(0.0f, 0.0f);
	glVertex3f(-shapeWidth, -shapeHeight, 0.0f);
	glTexCoord2d(textureWidth, 0.0f);
	glVertex3f(shapeWidth, -shapeHeight, 0.0f);
	glTexCoord2d(textureWidth, textureHeight);
	glVertex3f(shapeWidth, shapeHeight, 0.0f);
	glEnd();
}

void DrawSettledPixels() {
	void drawSettledPixel();
	struct node* temp = list;
	int row = 0;
	while (temp != NULL) {
		for (int col = 0; col < 10; col++) {
			if (temp->data[col] == SQUARE) {
				glLoadIdentity();
				glTranslatef((0.05f * (col - 4)) - 0.025f, stackbottom + (0.05f * row) - 0.025f, -1.0f);
				glScalef(0.1f, 0.1f, 0.1f);
				glBindTexture(GL_TEXTURE_2D, yellow_texture);
				drawSettledPixel();
			}
			if (temp->data[col] == DOT) {
				glLoadIdentity();
				glTranslatef((0.05f * (col - 4)) - 0.025f, stackbottom + (0.05f * row) - 0.025f, -1.0f);
				glScalef(0.1f, 0.1f, 0.1f);
				glBindTexture(GL_TEXTURE_2D, red_texture);
				drawSettledPixel();
			}
			if (temp->data[col] == LINE) {
				glLoadIdentity();
				glTranslatef((0.05f * (col - 4)) - 0.025f, stackbottom + (0.05f * row) - 0.025f, -1.0f);
				glScalef(0.1f, 0.1f, 0.1f);
				glBindTexture(GL_TEXTURE_2D, cyan_texture);
				drawSettledPixel();
			}
		}
		row++;
		temp = temp->next;
	}
}
void drawSettledPixel() {
	glBegin(GL_QUADS);
	glTexCoord2d(0.0f, 1.0f);
	glVertex3f(-dot[0], dot[0], 0.0);
	glTexCoord2d(0.0f, 0.0f);
	glVertex3f(-dot[0], -dot[0], 0.0f);
	glTexCoord2d(1.0f, 0.0f);
	glVertex3f(dot[0], -dot[0], 0.0f);
	glTexCoord2d(1.0f, 1.0f);
	glVertex3f(dot[0], dot[0], 0.0f);
	glEnd();
}

bool checkFit(int arr1[], int arr2[]) {
	for (int i = 0; i < 10; i++) {
		if (arr1[i] != -1 && arr2[i] != -1) {
			return false;
		}
	}
	return true;
}

	 
struct node* addShapeToList(int shape[]){
	bool checkFit(int arr1[],int arr2[]);
	if (list == NULL) {
		fprintf(gpFile, "list is empty \n");
		//if list is empty
		struct node* newNode = (struct node*)malloc(sizeof(struct node));
		newNode->next = NULL;
		for (int i = 0; i < 10; i++)
			newNode->data[i] = shape[i];
		list = newNode;
		fprintf(gpFile, "entered 1st element in list \n");
	}
	else {
		fprintf(gpFile, "list is not empty \n");
		struct node* temp = list;
		//search 1st node where xth column is empty
		//if found make data[x] = Shape
		while (temp != NULL) {
			if(checkFit(temp->data, shape)) {
				for (int i = 0; i < 10; i++) {
					if(shape[i]!=-1)
						temp->data[i] = shape[i];
				}
				return list;
			}
			temp = temp->next;
		}
		//if not found add new node with data[x]=shape
		temp = list;
		struct node* newNode = (struct node*)malloc(sizeof(struct node));
		newNode->next = NULL;
		for (int i = 0; i < 10; i++)
			newNode->data[i] = shape[i];
		while (temp->next != NULL) {
			temp = temp->next;
		}
		temp->next = newNode;
	}
	fprintf(gpFile, "list is empty \n");
	return list;
}

struct node* checkForCompletedrowAndDelete() {
	bool allFilled(int[]);
	struct node* temp = list;
	while (temp != NULL) {
		if (allFilled(temp->data)) {
			score += 10;
			if (score % 50 == 0) {
				level += 1;
			}
			list = temp->next;
			free(temp);
			temp = list;
			for (int i = 0; i < 10; i++) {
				bot[i] = bot[i] - (0.025f);
			}
		}
		else if (temp->next != NULL && allFilled(temp->next->data)) {
			score += 10;
			if (score % 50 == 0) {
				level += 1;
			}
			struct node* nodeToDelete = temp->next;
			temp->next = nodeToDelete->next;
			free(nodeToDelete);
			for (int i = 0; i < 10; i++) {
				bot[i] = bot[i] - (0.025f);
			}
		}
		temp = temp->next;
	}
	return list;
}

bool allFilled(int data[]) {
	for (int i = 0; i < 10; i++) {
		if (data[i] == -1) {
			return false;
		}
	}
	return true;
}


void update(void) {
	void delay(int milliseconds);
	bool CheckIfOver();
	struct node* checkForCompletedrowAndDelete();
	struct node* addShapeToList(int shape[]);
	int shapeArray[10] = { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1 };
	//delay to get grid to grid transition effect
	delay(400);
	if (CheckIfOver()) {
		over = true;
	}
	else {
		switch (currentShape) {
		case SQUARE:
			if (shapePositionY > bot[x] && falling == false) {
				falling = true;
				//nextShape = 1;
				nextShape = rand() % 3;
			}
			else if (shapePositionY > bot[x] && shapePositionY > bot[x + 1]) {
				shapePositionY = shapePositionY - distance;
				fprintf(gpFile, "falling\n");
			}
			else if ((shapePositionY <= bot[x] || shapePositionY <= bot[x + 1]) && falling == true) {
				//modify bottom of every column upto which the shape should fall
				fprintf(gpFile, "adding shape to list\n");
				shapeArray[x] = currentShape;
				shapeArray[x + 1] = currentShape;
				list = addShapeToList(shapeArray);
				list = addShapeToList(shapeArray);
				bot[x] = bot[x] + (2 * 0.05f);
				bot[x + 1] = bot[x + 1] + (2 * 0.05f);
				falling = false;
			}
			break;
		case DOT:
			if (shapePositionY > bot[x] && falling == false) {
				falling = true;
				nextShape = rand() % 3;
			}
			else if (shapePositionY > bot[x]) {
				shapePositionY = shapePositionY - distance;
			}
			else if (shapePositionY <= bot[x] && falling == true) {
				shapeArray[x] = currentShape;
				list = addShapeToList(shapeArray);
				bot[x] = bot[x] + (2 * 0.025f);
				falling = false;
			}
			break;
		case LINE:
			if (shapePositionY > (bot[x] + 0.05) && falling == false) {
				falling = true;
				nextShape = rand() % 3;
			}
			else if (shapePositionY > (bot[x] + 0.05)) {
				shapePositionY = shapePositionY - distance;
			}
			else if (shapePositionY <= (bot[x] + 0.05) && falling == true) {
				//modify bottom of every column upto which the shape should fall
				falling = false;
				if (rotation == 0 || rotation == 2 || rotation == -2) {
					shapeArray[x] = currentShape;
					list = addShapeToList(shapeArray);
					list = addShapeToList(shapeArray);
					list = addShapeToList(shapeArray);
					bot[x] = bot[x] + (2 * 0.075f);
				}
				else {
					shapeArray[x] = currentShape;
					shapeArray[x + 1] = currentShape;
					shapeArray[x - 1] = currentShape;
					list = addShapeToList(shapeArray);
					bot[x] = bot[x] + (2 * 0.025f);
				}

			}
			break;
		}
		//select new Shape
		if (falling == false) {
			currentShape = nextShape;
			rotation = 0;
			shapeAngle = 0.0f;
			list = checkForCompletedrowAndDelete();
			x = 4;
			//setting properties of next selected shape
			if (currentShape == SQUARE) {
				shapePositionY = top;
				shapePositionX = 0.0f;
				shapeWidth = square[0];
				shapeHeight = square[1];
			}
			if (currentShape == DOT) {
				shapePositionY = top + 0.025f;
				shapePositionX = 0.0f - 0.025f;
				shapeWidth = dot[0];
				shapeHeight = dot[1];
			}
			if (currentShape == LINE) {
				shapePositionY = top + 0.075f;
				shapePositionX = 0.0f - 0.025f;
				shapeWidth = ln[0];
				shapeHeight = ln[1];
			}
		}
	}
}

bool CheckIfOver() {
	struct node* temp = list;
	int count = 0;
	bool reachedtop = false;
	for (int i = 0; i < 10; i++) {
		if (bot[i] >= top) {
			reachedtop = true;
			break;
		}
	}
	while (temp != NULL) {
		temp = temp->next;
		count ++;
	}
	if (count >= 15 || reachedtop == true)
		return true;
	else
		return false;
}

void delay(int milliseconds)
{
	long pause;
	clock_t now, then;

	pause = milliseconds * (CLOCKS_PER_SEC / 1000);
	now = then = clock();
	while ((now - then) < pause)
		now = clock();
}


void Uninitialize(void) {
	if (gbFscreen == true) {
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
		ShowCursor(TRUE);
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle);
		SetWindowPlacement(ghwnd, &wpPrev);
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	}
	if (wglGetCurrentContext() == ghrc) {
		wglMakeCurrent(NULL, NULL);
	}
	if (ghrc) {
		wglDeleteContext(ghrc);
		ghrc = NULL;
	}
	if (ghdc) {
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}
	glDeleteTextures(1, &red_texture);
	glDeleteTextures(1, &pink_texture);
	glDeleteTextures(1, &yellow_texture);
	glDeleteTextures(1, &blue_texture);
	glDeleteTextures(1, &cyan_texture);
	glDeleteTextures(1, &orange_texture);
	glDeleteTextures(1, &green_texture);

	if (gpFile)
	{
		fprintf(gpFile, "Log fileClosed successfully \n");
		fclose(gpFile);
		gpFile = NULL;
	}
}
