// PlantVSZombie.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "PlantVSZombie.h"



#pragma comment(lib, "msimg32.lib")
#pragma comment(lib, "winmm.lib")

using namespace std;

#define MAX_LOADSTRING 100
//λ��
#define START_X_POS 61 //������ʼx����
#define START_Y_POS 90 //������ʼy����
#define LATTICE_WIDTH 80 //���ӵĿ�
#define LATTICE_HEIGHT 110 //���ӵĸ�
#define ZOMBIE_WIDTH 55 //�����
#define ZOMBIE_HEIGHT 106 //�����
#define ZOMBIE_SPEED 3 //�ƶ��ٶ�
#define GUN_FIRE_TIME 500 //�ӵ�����ʱ����
#define BULLET_ATTACK_VALUE (rand() % 20 + 5) //�ӵ�����
#define GUN_PRICE 100 //ǹ��Ǯ
#define FLOWER_PRICE 50 //����Ǯ
#define FLOWER_CREATE_MONEY 5000 //����ô���һ�ν��
#define HITMAP_HEIGHT 10 //���ƽ�ʬ��Ѫ���ĸ߶�
#define HITMAP_WIDTH 70 //���ƽ�ʬѪ���Ŀ��

// ȫ�ֱ���: 
HINSTANCE hInst;								// ��ǰʵ��
TCHAR szTitle[MAX_LOADSTRING];					// �������ı�
TCHAR szWindowClass[MAX_LOADSTRING];			// ����������

HWND g_hWnd; //ȫ�ִ��ھ��
HBITMAP g_hBitmapBack = NULL; //����
HBITMAP g_hBitmapGun = NULL; //�㶹
HBITMAP g_hBitmapFlower = NULL; //��
HBITMAP g_hBitmapBullet = NULL; //�ӵ�
HBITMAP g_hBitmapZombie = NULL; //��ʬ

POINT g_posMouse; //��¼���λ��
bool g_bMouseHaveItem = false; //�ж�������Ƿ�������
INT g_uMoney = 200; //��Ǯ

//��ʱ��
enum Timer
{
	TR_DRAW, //�滭
	TR_CREATEZOMBIE, //������ʬ
	TR_CREATEBULLET, //�����ӵ�
	TR_CREATEGOLD //����Ǯ
};

//�ж����ѡ�л������㶹
enum MouseType 
{
	MT_NONE, //ʲô��ûѡ��
	MT_FLOWER, //ѡ���˻�
	MT_GUN //ѡ�����㶹
};
MouseType g_mType = MT_NONE;

struct Object
{
	POINT pObj; //����
	int uHP; //Ѫ��
	UINT uSpeed; //�ٶ�
	UINT uIndex; //��ʬ������

	DWORD lastActionTime; //������ϴη����ʱ�䣬�Լ����ϴγ����ʱ��
};

list<Object> g_lBitmapGun; //�㶹
list<Object> g_lBitmapFlower; //��
list<Object> g_lBitmapZombie; //��ʬ
list<Object> g_lBitmapBullet; //�ӵ�

// �˴���ģ���а����ĺ�����ǰ������: 
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

void DrawAll(); //����������Դ
void PreLoadBitMap(); //Ԥ������Դ
void CreateZombie(); //������ʬ
void CreateBullet(); //�����ӵ�
void CheckCollide(); //�����ײ
void OnLeftDown(LPARAM lParam); //�������
void OnLeftUp(LPARAM lParam); //����ɿ�
bool IsInRect(RECT &r, POINT &p); //�ж������Ƿ��ھ���������
void CreateMoney(); //�������
void PlayBackMusic(WCHAR* strPath); //���ű�������
void Run(); //���в�������

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO:  �ڴ˷��ô��롣
	MSG msg = { 0 };
	HACCEL hAccelTable;

	// ��ʼ��ȫ���ַ���
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_PLANTVSZOMBIE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// ִ��Ӧ�ó����ʼ��: 
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PLANTVSZOMBIE));

	// ����Ϣѭ��: 
	/*while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}*/
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{

		}
	}

	return (int) msg.wParam;
}



//
//  ����:  MyRegisterClass()
//
//  Ŀ��:  ע�ᴰ���ࡣ
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PLANTVSZOMBIE));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_PLANTVSZOMBIE);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   ����:  InitInstance(HINSTANCE, int)
//
//   Ŀ��:  ����ʵ�����������������
//
//   ע��: 
//
//        �ڴ˺����У�������ȫ�ֱ����б���ʵ�������
//        ��������ʾ�����򴰿ڡ�
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   srand(GetTickCount());

   hInst = hInstance; // ��ʵ������洢��ȫ�ֱ�����

   hWnd = CreateWindow(szWindowClass, szTitle, 
	   WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME, 300, 100, 916, 559, 
	   NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   g_hWnd = hWnd;

   SetTimer(hWnd, TR_DRAW, 50, NULL);
   SetTimer(hWnd, TR_CREATEZOMBIE, 5000, NULL);
   SetTimer(hWnd, TR_CREATEBULLET, 1000, NULL);
   SetTimer(hWnd, TR_CREATEGOLD, 2000, NULL);
   PreLoadBitMap();

   Object obj;
   obj.pObj.x = 0; 
   obj.pObj.y = 0;
   obj.lastActionTime = GetTickCount();
   g_lBitmapFlower.push_back(obj);
   
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   /*wchar_t path[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, path);
	wsprintf(path,L"%s\\..\\Release\\music\\���������ҵ��� - ����.mp3", path);
	PlayMusic(path);*/


   Run();

   return TRUE;
}

//
//  ����:  WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  Ŀ��:    ���������ڵ���Ϣ��
//
//  WM_COMMAND	- ����Ӧ�ó���˵�
//  WM_PAINT	- ����������
//  WM_DESTROY	- �����˳���Ϣ������
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// �����˵�ѡ��: 
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO:  �ڴ���������ͼ����...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_LBUTTONDOWN:
		OnLeftDown(lParam); 
		break;
	case WM_LBUTTONUP:
		OnLeftUp(lParam);
		g_bMouseHaveItem = false;
		break;
	case WM_MOUSEMOVE:
		if (!g_bMouseHaveItem)
		{
			break;
		}
		g_posMouse.x = GET_X_LPARAM(lParam);
		g_posMouse.y = GET_Y_LPARAM(lParam);
		break;
	case WM_TIMER:
		switch (wParam)
		{
		case TR_DRAW:
			DrawAll();
			break;
		case TR_CREATEZOMBIE:
			CreateZombie();
			break;
		case TR_CREATEBULLET:
			CreateBullet();
			break;
		case TR_CREATEGOLD:
			CreateMoney();
			break;
		}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// �����ڡ������Ϣ�������
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


void DrawAll()
{
	//���Ʊ���
	HDC hDc = GetDC(g_hWnd);

	HDC hDcMem = CreateCompatibleDC(hDc);
	HBITMAP hBitmapBack = CreateCompatibleBitmap(hDc, 900, 506);
	SelectObject(hDcMem, hBitmapBack);

	HDC hDcTmp = CreateCompatibleDC(hDc); //���ݿͻ���DC
	SelectObject(hDcTmp, g_hBitmapBack);
	BITMAP bitInfo;
	GetObject(g_hBitmapBack, sizeof(BITMAP), &bitInfo);
	BitBlt(hDcMem, 0, 0, bitInfo.bmWidth, bitInfo.bmHeight, hDcTmp, 0, 0, SRCCOPY);



	//���ƻ����ӵ�����ʬ���㶹
	for (Object obj : g_lBitmapFlower)
	{
		SelectObject(hDcTmp, g_hBitmapFlower);
		GetObject(g_hBitmapFlower, sizeof(BITMAP), &bitInfo);
		POINT pt = obj.pObj;
		pt.x = pt.x * LATTICE_WIDTH + START_X_POS;
		pt.y = pt.y * LATTICE_HEIGHT + START_Y_POS;
		pt.x = pt.x + (LATTICE_WIDTH - bitInfo.bmWidth) / 2;
		pt.y = pt.y + (LATTICE_HEIGHT - bitInfo.bmHeight) / 2;
		TransparentBlt(hDcMem, pt.x, pt.y, bitInfo.bmWidth, bitInfo.bmHeight,
			hDcTmp, 0, 0, bitInfo.bmWidth, bitInfo.bmHeight, RGB(255, 0, 255));
	}

	for (Object obj : g_lBitmapGun)
	{
		SelectObject(hDcTmp, g_hBitmapGun);
		GetObject(g_hBitmapGun, sizeof(BITMAP), &bitInfo);
		POINT pt = obj.pObj;
		pt.x = pt.x * LATTICE_WIDTH + START_X_POS;
		pt.y = pt.y * LATTICE_HEIGHT + START_Y_POS;
		pt.x = pt.x + (LATTICE_WIDTH - bitInfo.bmWidth) / 2;
		pt.y = pt.y + (LATTICE_HEIGHT - bitInfo.bmHeight) / 2;
		TransparentBlt(hDcMem, pt.x, pt.y, bitInfo.bmWidth, bitInfo.bmHeight,
			hDcTmp, 0, 0, bitInfo.bmWidth, bitInfo.bmHeight, RGB(255, 0, 255));
	}

	for (Object& obj : g_lBitmapBullet)
	{
		SelectObject(hDcTmp, g_hBitmapBullet);
		GetObject(g_hBitmapBullet, sizeof(BITMAP), &bitInfo);
		TransparentBlt(hDcMem, obj.pObj.x, obj.pObj.y, bitInfo.bmWidth, bitInfo.bmHeight,
			hDcTmp, 0, 0, bitInfo.bmWidth, bitInfo.bmHeight, RGB(255, 0, 255));
		obj.pObj.x += obj.uSpeed;
	}

	//for (Object& obj : g_lBitmapZombie)
	for (list<Object>::iterator it = g_lBitmapZombie.begin(); it != g_lBitmapZombie.end(); ++it)
	{
		SelectObject(hDcTmp, g_hBitmapZombie);
		GetObject(g_hBitmapZombie, sizeof(BITMAP), &bitInfo);
		int iIndex = it->uIndex; //��ʬͼƬ������ֵ
		int lineIndex = iIndex / 4; //������
		int rankIndex = iIndex % 4; //������
		POINT pt;
		pt.x = rankIndex * ZOMBIE_WIDTH; //x����
		pt.y = lineIndex * ZOMBIE_HEIGHT; //y����
		it->uSpeed = ZOMBIE_SPEED;

		TransparentBlt(hDcMem, it->pObj.x, it->pObj.y, ZOMBIE_WIDTH, ZOMBIE_HEIGHT,
			hDcTmp, pt.x, pt.y, ZOMBIE_WIDTH, ZOMBIE_HEIGHT, RGB(255, 0, 255));
		it->uIndex = (it->uIndex + 1) % 14;
		it->pObj.x -= it->uSpeed;
		if (it->pObj.x < 0)
		{			
			g_lBitmapZombie.erase(it);
		}
	}

	//���ƻ����㶹
	SelectObject(hDcTmp, g_hBitmapFlower);
	GetObject(g_hBitmapFlower, sizeof(BITMAP), &bitInfo);
	TransparentBlt(hDcMem, 150, 10, bitInfo.bmWidth, bitInfo.bmHeight, hDcTmp, 0, 0,
		bitInfo.bmWidth, bitInfo.bmHeight, RGB(255, 0, 255));

	SelectObject(hDcTmp, g_hBitmapGun);
	GetObject(g_hBitmapGun, sizeof(BITMAP), &bitInfo);
	TransparentBlt(hDcMem, 210, 10, bitInfo.bmWidth, bitInfo.bmHeight, hDcTmp, 0, 0,
		bitInfo.bmWidth, bitInfo.bmHeight, RGB(255, 0, 255));

	//����Ǯ
	SetBkMode(hDcMem, 0);
	SetTextColor(hDcMem, RGB(235, 235, 31));
	WCHAR wMoneyText[20];
	wsprintf(wMoneyText, _T("$%d"), g_uMoney);
	TextOut(hDcMem, 70, 10, wMoneyText, lstrlen(wMoneyText));
	memset(wMoneyText, 0, sizeof(wMoneyText));
	wsprintf(wMoneyText, _T("$%d"), FLOWER_PRICE);
	TextOut(hDcMem, 165, 60, wMoneyText, lstrlen(wMoneyText));
	memset(wMoneyText, 0, sizeof(wMoneyText));
	wsprintf(wMoneyText, _T("$%d"), GUN_PRICE);
	TextOut(hDcMem, 220, 60, wMoneyText, lstrlen(wMoneyText));
	//UI��

	//���ƽ�ʬ��Ѫ��
	HBRUSH hBrush = CreateSolidBrush(RGB(255, 0, 0));
	for (Object& obj : g_lBitmapZombie)
	{
		int hpValue = obj.uHP;
		int maxHP = 100;
		float fHP = hpValue / (float)maxHP;
		//����Ѫ���ĵײ�
		HBRUSH hBrushBottom = (HBRUSH)GetStockObject(NULL_BRUSH);
		SelectObject(hDcMem, hBrushBottom);
		Rectangle(hDcMem, obj.pObj.x, obj.pObj.y - 10, obj.pObj.x + HITMAP_WIDTH,
			obj.pObj.y - 10 + HITMAP_HEIGHT);
		//����Ѫ��
		RECT rectHP;
		rectHP.left = obj.pObj.x + 1;
		rectHP.top = obj.pObj.y - 10 + 1;
		rectHP.right = rectHP.left + fHP * (HITMAP_WIDTH - 2);
		rectHP.bottom = rectHP.top + HITMAP_HEIGHT - 2;
		FillRect(hDcMem, &rectHP, hBrush);
	}
	if (g_bMouseHaveItem) //���������Ʒ
	{
		if (g_mType == MT_FLOWER)
		{
			SelectObject(hDcTmp, g_hBitmapFlower);
			GetObject(g_hBitmapFlower, sizeof(BITMAP), &bitInfo);
		}
		if (g_mType == MT_GUN)
		{
			SelectObject(hDcTmp, g_hBitmapGun);
			GetObject(g_hBitmapGun, sizeof(BITMAP), &bitInfo);
		}
		//����λ��
		POINT ptMouse;
		ptMouse.x = g_posMouse.x - bitInfo.bmWidth / 2;
		ptMouse.y = g_posMouse.y - bitInfo.bmHeight / 2;
		TransparentBlt(hDcMem, ptMouse.x, ptMouse.y, bitInfo.bmWidth, bitInfo.bmHeight,
			hDcTmp, 0, 0, bitInfo.bmWidth, bitInfo.bmHeight, RGB(255, 0, 255));
	}

	CheckCollide();

	BitBlt(hDc, 0, 0, 900, 506, hDcMem, 0, 0, SRCCOPY);

	DeleteObject(hDcTmp);
	ReleaseDC(g_hWnd, hDc);
}

void PreLoadBitMap()
{
	g_hBitmapBack = 
		(HBITMAP)LoadImage(NULL, _T("images/map.BMP"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	g_hBitmapBullet =
		(HBITMAP)LoadImage(NULL, _T("images/bullet.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	g_hBitmapFlower =
		(HBITMAP)LoadImage(NULL, _T("images/flower.BMP"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	g_hBitmapZombie =
		(HBITMAP)LoadImage(NULL, _T("images/monster.BMP"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	g_hBitmapGun =
		(HBITMAP)LoadImage(NULL, _T("images/gun.BMP"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
}

void CreateBullet() //�����ӵ�
{
	DWORD timeNow = GetTickCount();
	for (Object& obj : g_lBitmapGun)
	{
		if (timeNow >= obj.lastActionTime + GUN_FIRE_TIME) //��ⷢ��ʱ����
		{
			//�����ӵ�
			Object objBullet;
			objBullet.uSpeed = 5;
			POINT pt = obj.pObj;
			pt.x = pt.x * LATTICE_WIDTH + START_X_POS;
			pt.y = pt.y * LATTICE_HEIGHT + START_Y_POS;
			objBullet.pObj.x = pt.x + 60;
			objBullet.pObj.y = pt.y + 30;
			g_lBitmapBullet.push_back(objBullet);
		}
	}
}

void CreateZombie()
{
	Object obj;
	obj.pObj.x = 900;
	obj.pObj.y = (rand() % 3) * LATTICE_HEIGHT + START_Y_POS + 10; //y����
	obj.uHP = 100;
	obj.uSpeed = rand() % 5 + 1;
	obj.uIndex = 0;
	g_lBitmapZombie.push_back(obj);
}

void CheckCollide()
{
	//�ӵ��͹ֵ���ײ
	for (list<Object>::iterator itBullet = g_lBitmapBullet.begin();
		itBullet != g_lBitmapBullet.end(); )
	{
		bool bCollide = false; //�ж��Ƿ���ײ
		POINT posBullet = itBullet->pObj; //��ȡ�ӵ�������
		for (list<Object>::iterator itZombie = g_lBitmapZombie.begin();
			itZombie != g_lBitmapZombie.end(); ++itZombie)
		{
			POINT posZombie = itZombie->pObj; //��ȡ��ʬ������
			if (posZombie.x <= posBullet.x + 15 && posZombie.x > posBullet.x - 220 / 4
				&& posBullet.y - posZombie.y > 0 && posBullet.y - posZombie.y < 50)
			{
				itZombie->uHP -= BULLET_ATTACK_VALUE; //��Ѫ
				if (itZombie->uHP <= 0)
				{
					g_lBitmapZombie.erase(itZombie);
				}
				itBullet = g_lBitmapBullet.erase(itBullet);
				bCollide = true;
				break;
			}
		}
		if (!bCollide)
		{
			itBullet++;
		}
	}

	//����ͻ�����ײ
	for (list<Object>::iterator itFlower = g_lBitmapFlower.begin();
		itFlower != g_lBitmapFlower.end();)
	{
		//���㻨������
		POINT ptFlower = itFlower->pObj;
		POINT pt = ptFlower;
		pt.x = pt.x * LATTICE_WIDTH + START_X_POS;
		pt.y = pt.y * LATTICE_HEIGHT + START_Y_POS;
		pt.x = pt.x + (LATTICE_WIDTH - 50) / 2;
		pt.y = pt.y + (LATTICE_HEIGHT - 62) / 2;
		ptFlower = pt;

		bool bCollide = false;
		for (list<Object>::iterator itZombie = g_lBitmapZombie.begin();
			itZombie != g_lBitmapZombie.end(); ++itZombie)
		{
			POINT ptZombie = itZombie->pObj;
			if (ptZombie.x <= ptFlower.x + 35 && ptZombie.x > ptFlower.x - 55
				&& ptFlower.y > ptZombie.y && ptFlower.y - ptZombie.y < 50) //������ײ����
			{
				bCollide = true;
				itFlower = g_lBitmapFlower.erase(itFlower);
				break;
			}
		}
		if (!bCollide)
		{
			++itFlower;
		}
	}

	//������㶹��ײ
	for (list<Object>::iterator itGun = g_lBitmapGun.begin(); itGun != g_lBitmapGun.end();)
	{
		bool bCollide = false;
		POINT ptGun = itGun->pObj;
		POINT pt = ptGun;
		pt.x = pt.x * LATTICE_WIDTH + START_X_POS;
		pt.y = pt.y * LATTICE_HEIGHT + START_Y_POS;
		pt.x = pt.x + (LATTICE_WIDTH - 50) / 2;
		pt.y = pt.y + (LATTICE_HEIGHT - 62) / 2;
		ptGun = pt;

		for (list<Object>::iterator itZombie = g_lBitmapZombie.begin();
			itZombie != g_lBitmapZombie.end(); ++itZombie)
		{
			POINT ptZombie = itZombie->pObj;
			if (ptZombie.x <= ptGun.x + 35 && ptZombie.x > ptGun.x - 55
				&& ptGun.y > ptZombie.y && ptGun.y - ptZombie.y < 50)
			{
				bCollide = true;
				itGun = g_lBitmapGun.erase(itGun);
				break;
			}
		}
		if (!bCollide)
		{
			++itGun;
		}
	}
}

void OnLeftDown(LPARAM lParam)
{
	//��ȡ����λ��
	g_posMouse.x = GET_X_LPARAM(lParam);
	g_posMouse.y = GET_Y_LPARAM(lParam);
	//������ǻ������㶹
	RECT rect = { 0 };
	rect.left = 150;
	rect.top = 10;
	rect.right = 150 + 50;
	rect.bottom = 10 + 62;
	if (IsInRect(rect, g_posMouse))
	{
		g_mType = MT_FLOWER;
		g_bMouseHaveItem = true;
		return;
	}
	rect.left = 210;
	rect.top = 10;
	rect.right = 210 + 50;
	rect.bottom = 10 + 62;
	if (IsInRect(rect, g_posMouse))
	{
		g_mType = MT_GUN;
		g_bMouseHaveItem = true;
		return;
	}
}

bool IsInRect(RECT &r, POINT &p)
{
	if (p.x >= r.left && p.x <= r.right && p.y >= r.top && p.y <= r.bottom)
	{
		return true;
	}
	return false;
}

void OnLeftUp(LPARAM lParam)
{
	if (!g_bMouseHaveItem)
	{
		return;
	}
	//��ûǮ�����㶹
	if (g_mType == MT_FLOWER && g_uMoney < FLOWER_PRICE)
	{
		return;
	}
	if (g_mType == MT_GUN && g_uMoney < GUN_PRICE)
	{
		return;
	}

	//ȷ����ֲ��λ��
	POINT ptPaint;
	ptPaint.x = GET_X_LPARAM(lParam);
	ptPaint.y = GET_Y_LPARAM(lParam);

	RECT rArea;
	rArea.left = START_X_POS;
	rArea.right = rArea.left + 10 * LATTICE_WIDTH;
	rArea.top = START_Y_POS;
	rArea.bottom = rArea.top + 3 * LATTICE_HEIGHT;
	if (!IsInRect(rArea, ptPaint))
	{
		return;
	}

	//�жϸø����Ƿ�����ֲ
	ptPaint.x = (ptPaint.x - START_X_POS) / LATTICE_WIDTH;
	ptPaint.y = (ptPaint.y - START_Y_POS) / LATTICE_HEIGHT;

	for (Object& obj : g_lBitmapFlower)
	{
		if (ptPaint.x == obj.pObj.x && ptPaint.y == obj.pObj.y)
		{
			return;
		}
	}
	for (Object& obj : g_lBitmapGun)
	{
		if (ptPaint.x == obj.pObj.x && ptPaint.y == obj.pObj.y)
		{
			return;
		}
	}
	Object obj;
	obj.pObj = ptPaint;
	obj.lastActionTime = GetTickCount();

	if (g_mType == MT_FLOWER)
	{
		g_uMoney -= FLOWER_PRICE;
		g_lBitmapFlower.push_back(obj);
	}
	else if (g_mType == MT_GUN)
	{
		g_uMoney -= GUN_PRICE;
		g_lBitmapGun.push_back(obj);
	}
}

void CreateMoney()
{
	DWORD timeNow = GetTickCount();
	for (Object& obj : g_lBitmapFlower)
	{
		if (timeNow >= obj.lastActionTime + FLOWER_CREATE_MONEY)
		{
			g_uMoney += 30;
			obj.lastActionTime = timeNow;
		}
	}
}

void PlayBackMusic(WCHAR* strPath)
{
	MCI_OPEN_PARMS mciOpen;
	mciOpen.lpstrDeviceType = _T("sequencer");
	mciOpen.lpstrElementName = strPath;
	
	mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT,
		(DWORD)&mciOpen);
	DWORD deviceID = mciOpen.wDeviceID;
	
	MCI_PLAY_PARMS mciPlay;
	mciSendCommand(deviceID, MCI_PLAY, MCI_NOTIFY,
		(DWORD)&mciPlay);

	//MCIDEVICEID m_nDeviceID;
	//MCIDEVICEID m_nElementID;
	//MCI_OPEN_PARMS mciOpenParms;

	//// ���豸
	//mciOpenParms.lpstrDeviceType = _T("sequencer");
	//mciSendCommand(NULL,
	//	MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_TYPE_ID | MCI_WAIT,
	//	(DWORD)(LPVOID)&mciOpenParms);
	//m_nDeviceID = mciOpenParms.wDeviceID;

	//// ��������Դ
	//MCI_OPEN_PARMS mciOpen;
	//memset(&mciOpen, 0, sizeof(MCI_OPEN_PARMS));
	//mciOpen.lpstrElementName = strPath;    // �����ƶ��ļ�
	//mciSendCommand(m_nDeviceID, MCI_OPEN, MCI_OPEN_ELEMENT, (DWORD)(LPVOID)&mciOpen);
	//m_nElementID = mciOpen.wDeviceID;

	//// ���Ͳ�������
	//MCI_PLAY_PARMS mciPlay;
	//mciPlay.dwCallback = NULL;
	//mciSendCommand(m_nElementID, MCI_PLAY, MCI_DGV_PLAY_REPEAT, (DWORD)(LPVOID)&mciPlay);
}

void Run()
{
	WCHAR strPath[1000];
	GetCurrentDirectory(1000, strPath);
	wsprintf(strPath, L"%s\\..\\Release\\music\\Laura Shigihara - Grasswalk.mp3", strPath);
	PlayBackMusic(strPath);
}