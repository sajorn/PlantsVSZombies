// PlantVSZombie.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "PlantVSZombie.h"



#pragma comment(lib, "msimg32.lib")
#pragma comment(lib, "winmm.lib")

using namespace std;

#define MAX_LOADSTRING 100
//位置
#define START_X_POS 61 //格子起始x坐标
#define START_Y_POS 90 //格子起始y坐标
#define LATTICE_WIDTH 80 //格子的宽
#define LATTICE_HEIGHT 110 //格子的高
#define ZOMBIE_WIDTH 55 //怪物宽
#define ZOMBIE_HEIGHT 106 //怪物高
#define ZOMBIE_SPEED 3 //移动速度
#define GUN_FIRE_TIME 500 //子弹发射时间间隔
#define BULLET_ATTACK_VALUE (rand() % 20 + 5) //子弹威力
#define GUN_PRICE 100 //枪的钱
#define FLOWER_PRICE 50 //花的钱
#define FLOWER_CREATE_MONEY 5000 //花多久创建一次金币
#define HITMAP_HEIGHT 10 //绘制僵尸的血条的高度
#define HITMAP_WIDTH 70 //绘制僵尸血条的宽度

// 全局变量: 
HINSTANCE hInst;								// 当前实例
TCHAR szTitle[MAX_LOADSTRING];					// 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING];			// 主窗口类名

HWND g_hWnd; //全局窗口句柄
HBITMAP g_hBitmapBack = NULL; //背景
HBITMAP g_hBitmapGun = NULL; //豌豆
HBITMAP g_hBitmapFlower = NULL; //花
HBITMAP g_hBitmapBullet = NULL; //子弹
HBITMAP g_hBitmapZombie = NULL; //僵尸

POINT g_posMouse; //记录鼠标位置
bool g_bMouseHaveItem = false; //判断鼠标上是否有物体
INT g_uMoney = 200; //金钱

//定时器
enum Timer
{
	TR_DRAW, //绘画
	TR_CREATEZOMBIE, //创建僵尸
	TR_CREATEBULLET, //创建子弹
	TR_CREATEGOLD //创建钱
};

//判断鼠标选中花还是豌豆
enum MouseType 
{
	MT_NONE, //什么都没选中
	MT_FLOWER, //选中了花
	MT_GUN //选中了豌豆
};
MouseType g_mType = MT_NONE;

struct Object
{
	POINT pObj; //坐标
	int uHP; //血量
	UINT uSpeed; //速度
	UINT uIndex; //僵尸的索引

	DWORD lastActionTime; //相对于上次发射的时间，以及花上次成熟的时间
};

list<Object> g_lBitmapGun; //豌豆
list<Object> g_lBitmapFlower; //花
list<Object> g_lBitmapZombie; //僵尸
list<Object> g_lBitmapBullet; //子弹

// 此代码模块中包含的函数的前向声明: 
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

void DrawAll(); //绘制所有资源
void PreLoadBitMap(); //预加载资源
void CreateZombie(); //创建僵尸
void CreateBullet(); //创建子弹
void CheckCollide(); //检测碰撞
void OnLeftDown(LPARAM lParam); //左键按下
void OnLeftUp(LPARAM lParam); //左键松开
bool IsInRect(RECT &r, POINT &p); //判断坐标是否在矩形区域中
void CreateMoney(); //创建金币
void PlayBackMusic(WCHAR* strPath); //播放背景音乐
void Run(); //运行播放音乐

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO:  在此放置代码。
	MSG msg = { 0 };
	HACCEL hAccelTable;

	// 初始化全局字符串
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_PLANTVSZOMBIE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 执行应用程序初始化: 
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PLANTVSZOMBIE));

	// 主消息循环: 
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
//  函数:  MyRegisterClass()
//
//  目的:  注册窗口类。
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
//   函数:  InitInstance(HINSTANCE, int)
//
//   目的:  保存实例句柄并创建主窗口
//
//   注释: 
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   srand(GetTickCount());

   hInst = hInstance; // 将实例句柄存储在全局变量中

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
	wsprintf(path,L"%s\\..\\Release\\music\\月亮代表我的心 - 齐秦.mp3", path);
	PlayMusic(path);*/


   Run();

   return TRUE;
}

//
//  函数:  WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:    处理主窗口的消息。
//
//  WM_COMMAND	- 处理应用程序菜单
//  WM_PAINT	- 绘制主窗口
//  WM_DESTROY	- 发送退出消息并返回
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
		// 分析菜单选择: 
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
		// TODO:  在此添加任意绘图代码...
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

// “关于”框的消息处理程序。
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
	//绘制背景
	HDC hDc = GetDC(g_hWnd);

	HDC hDcMem = CreateCompatibleDC(hDc);
	HBITMAP hBitmapBack = CreateCompatibleBitmap(hDc, 900, 506);
	SelectObject(hDcMem, hBitmapBack);

	HDC hDcTmp = CreateCompatibleDC(hDc); //兼容客户区DC
	SelectObject(hDcTmp, g_hBitmapBack);
	BITMAP bitInfo;
	GetObject(g_hBitmapBack, sizeof(BITMAP), &bitInfo);
	BitBlt(hDcMem, 0, 0, bitInfo.bmWidth, bitInfo.bmHeight, hDcTmp, 0, 0, SRCCOPY);



	//绘制花，子弹，僵尸，豌豆
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
		int iIndex = it->uIndex; //僵尸图片的索引值
		int lineIndex = iIndex / 4; //行索引
		int rankIndex = iIndex % 4; //列索引
		POINT pt;
		pt.x = rankIndex * ZOMBIE_WIDTH; //x坐标
		pt.y = lineIndex * ZOMBIE_HEIGHT; //y坐标
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

	//绘制花和豌豆
	SelectObject(hDcTmp, g_hBitmapFlower);
	GetObject(g_hBitmapFlower, sizeof(BITMAP), &bitInfo);
	TransparentBlt(hDcMem, 150, 10, bitInfo.bmWidth, bitInfo.bmHeight, hDcTmp, 0, 0,
		bitInfo.bmWidth, bitInfo.bmHeight, RGB(255, 0, 255));

	SelectObject(hDcTmp, g_hBitmapGun);
	GetObject(g_hBitmapGun, sizeof(BITMAP), &bitInfo);
	TransparentBlt(hDcMem, 210, 10, bitInfo.bmWidth, bitInfo.bmHeight, hDcTmp, 0, 0,
		bitInfo.bmWidth, bitInfo.bmHeight, RGB(255, 0, 255));

	//绘制钱
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
	//UI层

	//绘制僵尸的血条
	HBRUSH hBrush = CreateSolidBrush(RGB(255, 0, 0));
	for (Object& obj : g_lBitmapZombie)
	{
		int hpValue = obj.uHP;
		int maxHP = 100;
		float fHP = hpValue / (float)maxHP;
		//绘制血条的底部
		HBRUSH hBrushBottom = (HBRUSH)GetStockObject(NULL_BRUSH);
		SelectObject(hDcMem, hBrushBottom);
		Rectangle(hDcMem, obj.pObj.x, obj.pObj.y - 10, obj.pObj.x + HITMAP_WIDTH,
			obj.pObj.y - 10 + HITMAP_HEIGHT);
		//绘制血条
		RECT rectHP;
		rectHP.left = obj.pObj.x + 1;
		rectHP.top = obj.pObj.y - 10 + 1;
		rectHP.right = rectHP.left + fHP * (HITMAP_WIDTH - 2);
		rectHP.bottom = rectHP.top + HITMAP_HEIGHT - 2;
		FillRect(hDcMem, &rectHP, hBrush);
	}
	if (g_bMouseHaveItem) //鼠标上有物品
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
		//鼠标的位置
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

void CreateBullet() //创建子弹
{
	DWORD timeNow = GetTickCount();
	for (Object& obj : g_lBitmapGun)
	{
		if (timeNow >= obj.lastActionTime + GUN_FIRE_TIME) //检测发射时间间隔
		{
			//发射子弹
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
	obj.pObj.y = (rand() % 3) * LATTICE_HEIGHT + START_Y_POS + 10; //y坐标
	obj.uHP = 100;
	obj.uSpeed = rand() % 5 + 1;
	obj.uIndex = 0;
	g_lBitmapZombie.push_back(obj);
}

void CheckCollide()
{
	//子弹和怪的碰撞
	for (list<Object>::iterator itBullet = g_lBitmapBullet.begin();
		itBullet != g_lBitmapBullet.end(); )
	{
		bool bCollide = false; //判断是否碰撞
		POINT posBullet = itBullet->pObj; //获取子弹的坐标
		for (list<Object>::iterator itZombie = g_lBitmapZombie.begin();
			itZombie != g_lBitmapZombie.end(); ++itZombie)
		{
			POINT posZombie = itZombie->pObj; //获取僵尸的坐标
			if (posZombie.x <= posBullet.x + 15 && posZombie.x > posBullet.x - 220 / 4
				&& posBullet.y - posZombie.y > 0 && posBullet.y - posZombie.y < 50)
			{
				itZombie->uHP -= BULLET_ATTACK_VALUE; //扣血
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

	//怪物和花的碰撞
	for (list<Object>::iterator itFlower = g_lBitmapFlower.begin();
		itFlower != g_lBitmapFlower.end();)
	{
		//计算花的坐标
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
				&& ptFlower.y > ptZombie.y && ptFlower.y - ptZombie.y < 50) //满足碰撞条件
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

	//怪物和豌豆碰撞
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
	//获取鼠标的位置
	g_posMouse.x = GET_X_LPARAM(lParam);
	g_posMouse.y = GET_Y_LPARAM(lParam);
	//点击的是花还是豌豆
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
	//有没钱买花买豌豆
	if (g_mType == MT_FLOWER && g_uMoney < FLOWER_PRICE)
	{
		return;
	}
	if (g_mType == MT_GUN && g_uMoney < GUN_PRICE)
	{
		return;
	}

	//确定种植的位置
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

	//判断该格子是否已种植
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

	//// 打开设备
	//mciOpenParms.lpstrDeviceType = _T("sequencer");
	//mciSendCommand(NULL,
	//	MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_TYPE_ID | MCI_WAIT,
	//	(DWORD)(LPVOID)&mciOpenParms);
	//m_nDeviceID = mciOpenParms.wDeviceID;

	//// 打开音乐资源
	//MCI_OPEN_PARMS mciOpen;
	//memset(&mciOpen, 0, sizeof(MCI_OPEN_PARMS));
	//mciOpen.lpstrElementName = strPath;    // 播放制定文件
	//mciSendCommand(m_nDeviceID, MCI_OPEN, MCI_OPEN_ELEMENT, (DWORD)(LPVOID)&mciOpen);
	//m_nElementID = mciOpen.wDeviceID;

	//// 发送播放命令
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