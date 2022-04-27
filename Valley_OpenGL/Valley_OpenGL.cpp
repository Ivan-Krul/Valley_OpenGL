#include <iostream>
#include <Windows.h>
#include <gl/GL.h>
#include <string>
#include <math.h>
#include <vector>

#pragma comment(lib, "opengl32.lib")

#define WINDOW_X 512
#define WINDOW_Y 512
#define FREQUENCY 30.0f //frames per second (FPS)

std::vector<POINTFLOAT> mas;
float scaleY = 1;
float scaleX = 10;

double Func(double x) {
	return (rand()%100-50)/100.0;
}

void Init(float start, float finish, int count) {
	mas.resize(count);
	float dx = (finish - start) / (count - 1);

	for (int i = 0;i < count;i++) {
		mas[i].x = start;
		mas[i].y = Func(start);
		start += dx;
	}
}

void AddGraph(float x,float y) {
	for (int i = 1;i < mas.size();i++) mas[i - 1] = mas[i];
	mas[mas.size() - 1].x = x;
	mas[mas.size() - 1].y = y;
}

void DrawOs(float alpha) {
	static float d = 0.05;
	glPushMatrix();
	glRotated(alpha, 0, 0, 1);
	glBegin(GL_LINES);
	glVertex2d(-1, 0);
	glVertex2d(1, 0);
	glVertex2d(1, 0);
	glVertex2d(1-d, 0+d);
	glVertex2d(1, 0);
	glVertex2d(1-d, 0-d);
	glEnd();
	glPopMatrix();
}

void DrawGraph() {
	float sx = 2.0 / (mas[mas.size() - 1].x - mas[0].x);
	float dx = (mas[mas.size() - 1].x + mas[0].x) * 0.5;

	glPushMatrix();
	glScaled(sx, scaleY, 1);
	glTranslated(-dx, 0, 0);

	glBegin(GL_LINE_STRIP);
	glColor3d(0, 0, 1);
	for (int i = 0;i < mas.size();i++) glVertex2d(mas[i].x,mas[i].y);
	glEnd();

	glPopMatrix();
}

void Paint() {
	glLoadIdentity();
	glLineWidth(3);
	glColor3d(1, 0, 0);
	DrawOs(0);
	glColor3d(0, 1, 0);
	DrawOs(90);

	scaleX += 0.5;
	AddGraph(scaleX, Func(scaleX));

	DrawGraph();
}

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);

int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	WNDCLASSEX wcex;
	HWND hwnd;
	HDC hDC;
	HGLRC hRC;
	MSG msg;
	BOOL bQuit = FALSE;
	float theta = 0.0f;

	bool is_pause = false;

	/* register window class */
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_OWNDC;
	wcex.lpfnWndProc = WindowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"GLSample";
	wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);;


	if (!RegisterClassEx(&wcex))
		return 0;

	/* create main window */
	hwnd = CreateWindowEx(0,
		L"GLSample",
		L"ValleyGL",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		512,
		512,
		NULL,
		NULL,
		hInstance,
		NULL);

	ShowWindow(hwnd, nCmdShow);

	/* enable OpenGL for the window */
	EnableOpenGL(hwnd, &hDC, &hRC);

	Init(6, scaleX,100);

	/* program main loop */
	while (!bQuit)
	{

		/* check for messages */
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			/* handle or dispatch messages */
			if (msg.message == WM_QUIT)
			{
				bQuit = TRUE;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if (GetAsyncKeyState(' ')) { is_pause = !is_pause; Sleep(1000.0 / FREQUENCY); }

			/* OpenGL animation code goes here */
			if (!is_pause) {

				glClearColor(0, 0, 0, 0.0f);
				glClear(GL_COLOR_BUFFER_BIT);

				Paint();

				SwapBuffers(hDC);

				theta += 1000.0 / FREQUENCY;
			}

			std::wstring WindowText = L"ValleyGL";
			WindowText += is_pause ? L" paused" : L"";
			SetWindowText(hwnd, WindowText.c_str());

			Sleep(1000.0 / FREQUENCY);
		}
	}

	/* shutdown OpenGL */
	DisableOpenGL(hwnd, hDC, hRC);

	/* destroy the window explicitly */
	DestroyWindow(hwnd);

	return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		break;

	case WM_MOUSEWHEEL:
		if (int(wParam) > 0) scaleY *= 1.5;
		else scaleY *= 0.7;
		if (scaleY < 0.02)scaleY = 0.02;
		break;

	case WM_DESTROY:
		return 0;

	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		}
	}
	break;

	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	return 0;
}

void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
	PIXELFORMATDESCRIPTOR pfd;

	int iFormat;

	/* get the device context (DC) */
	*hDC = GetDC(hwnd);

	/* set the pixel format for the DC */
	ZeroMemory(&pfd, sizeof(pfd));

	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;

	iFormat = ChoosePixelFormat(*hDC, &pfd);

	SetPixelFormat(*hDC, iFormat, &pfd);

	/* create and enable the render context (RC) */
	*hRC = wglCreateContext(*hDC);

	wglMakeCurrent(*hDC, *hRC);
}

void DisableOpenGL(HWND hwnd, HDC hDC, HGLRC hRC)
{
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hRC);
	ReleaseDC(hwnd, hDC);
}
