#pragma once

#include "Controls.h"

HWND ClientCreateWindow(HINSTANCE hInstance, WNDPROC WndProc, 
						LPCSTR lpClassName, LPCSTR lpWindowName,
						long WindowsWidth, long WindowsHeight)
{
	HWND hWnd = 0;
	WNDCLASS				wc;
	DWORD					dwExStyle;
	DWORD					dwStyle;
	RECT WindowRect;
	WindowRect.left			= (long)0;
	WindowRect.right		= (long)WindowsWidth;
	WindowRect.top			= (long)0;
	WindowRect.bottom		= (long)WindowsHeight;
	wc.style				= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc		= (WNDPROC)WndProc;
	wc.cbClsExtra			= 0;
	wc.cbWndExtra			= 0;
	wc.hInstance			= hInstance;
	wc.hIcon				= LoadIcon(NULL, IDI_WINLOGO);
	wc.hCursor				= LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground		= NULL;
	wc.lpszMenuName			= NULL;
	wc.lpszClassName		= (LPCSTR)lpClassName;
	if(!RegisterClass(&wc))
	{
		// Неудачная попытка зарегистрировать класс
		MessageBox( NULL, "Failed To Register The Window Class.", "ERROR", MB_OK | MB_ICONEXCLAMATION );
		return false;
	}
	dwExStyle						= WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	dwStyle							= WS_DLGFRAME;
	AdjustWindowRectEx( &WindowRect, dwStyle, false, dwExStyle );
	hWnd = CreateWindowEx(dwExStyle,								
		(LPCSTR)lpClassName,						
		lpWindowName,								
		WS_CLIPSIBLINGS |							
		WS_CLIPCHILDREN | 							
		dwStyle,									
		0, 0,										
		WindowRect.right - WindowRect.left,			
		WindowRect.bottom - WindowRect.top,			
		NULL,										
		NULL,										
		hInstance,									
		NULL);										
	return hWnd;
}
// Создание полей ввода
BOOL ClientCreateEdits(HINSTANCE hInstance, HWND hParent, 
					   HWND &hAdress, HWND &hUserName, HWND &hUserPassword)
{
	// Создаем поле ввода адреса
	hAdress = CreateWindow(WC_EDIT, 0, 
		WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_LEFT,
		20, 20, 155, 20, hParent, 0, hInstance, 0);
	// Создаем поле ввода логина
	hUserName = CreateWindow(WC_EDIT, 0, 
		WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_LEFT,
		20, 55, 155, 20, hParent, 0, hInstance, 0);
	// Создаем поле ввода пароля
	hUserPassword = CreateWindow(WC_EDIT, 0, 
		WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | 
		ES_LEFT | ES_PASSWORD,
		185, 55, 155, 20, hParent, 0, hInstance, 0);
	return hAdress && hUserName && hUserPassword;
}
// Создание кнопок
BOOL ClientCreateButtons(HINSTANCE hInstance, HWND hParent, 
						 HWND & hConnect, HWND & hDownload, HWND & hUpload, HWND & hSettings, HWND &hHelp)
{
	// Создаем кнопку соединения с сервером
	hConnect = CreateWindow(WC_BUTTON, 0, 
		WS_CHILD | WS_VISIBLE | WS_BORDER | BS_CENTER | WS_TABSTOP,
		185, 20, 155, 30, hParent, 0, hInstance, 0);
	// Кнопка загрузки
	hDownload = CreateWindow(WC_BUTTON, 0, 
		WS_CHILD | WS_VISIBLE | WS_BORDER | BS_CENTER | WS_TABSTOP,
		330 + 20, 55 + 150 + 15 + 80, 35, 30, hParent, 0, hInstance, 0);
	// Кнопка закачки
	hUpload = CreateWindow(WC_BUTTON, 0, 
		WS_CHILD | WS_VISIBLE | WS_BORDER | BS_CENTER | WS_TABSTOP,
		330 + 20, 55 + 150 + 15 - 70, 35, 30, hParent, 0, hInstance, 0);
	// Кнопка настроек
	hSettings = CreateWindow(WC_BUTTON, 0, 
		WS_CHILD | WS_VISIBLE | WS_BORDER | BS_CENTER | WS_TABSTOP,
		330 + 20 + 45, 20, 150, 30, hParent, 0, hInstance, 0);
	// Справка
	hHelp = CreateWindow(WC_BUTTON, 0, 
		WS_CHILD | WS_VISIBLE | WS_BORDER | BS_CENTER | WS_TABSTOP,
		330 + 20 + 45 + 150 + 20, 20, 150, 30, hParent, 0, hInstance, 0);
	// Отсылаем сообщения компонентам для обновления текста надписей кнопок
	SendMessage(hConnect, WM_SETTEXT, sizeof("Присоединиться"), LPARAM("Присоединиться"));
	SendMessage(hDownload, WM_SETTEXT, sizeof("->"), LPARAM("->"));
	SendMessage(hUpload, WM_SETTEXT, sizeof("<-"), LPARAM("<-"));
	SendMessage(hSettings, WM_SETTEXT, sizeof("Настройки"), LPARAM("Настройки"));
	SendMessage(hHelp, WM_SETTEXT, sizeof("Справка"), LPARAM("Справка"));
	// Делаем кнопки загрузки, закачки, удаления не активными
	EnableWindow(hDownload, FALSE);
	EnableWindow(hUpload, FALSE);
	EnableWindow(hSettings, TRUE);
	// Возвращаем результат построения
	return hConnect && hDownload && hUpload && hSettings;
}
// Создание списков
BOOL ClientCreateListViews(HINSTANCE hInstance, HWND hParent, HWND &hLocalList, HWND &hFtpList)
{
	// Список файлов FTP сервера
	hFtpList = CreateWindowEx(0, WC_LISTVIEW, 0,
		WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP | WS_VSCROLL |
		LVS_REPORT | LVS_SHOWSELALWAYS | LVS_ICON,
		20, 85, 300 + 20, 300, hParent, 0, hInstance, 0);
	// Добавляем столбцы
	LVCOLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.iSubItem = 0;
	lvc.pszText = "Имя";
	lvc.cx = 160;
	lvc.fmt = LVCFMT_LEFT;
	// Первый столбец
	ListView_InsertColumn(hFtpList, 0, &lvc);
	lvc.pszText = "Дата";
	lvc.cx = 70;
	// Второй столбец
	ListView_InsertColumn(hFtpList, 1, &lvc);
	lvc.pszText = "Размер";
	lvc.cx = 70;
	// Третий столбец
	ListView_InsertColumn(hFtpList, 2, &lvc);
	// Задаем элемент как недоступный
	EnableWindow(hFtpList, FALSE);
	ListView_SetExtendedListViewStyleEx(hFtpList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Добавляем список для отображения локальных папок и файлов
	hLocalList = CreateWindowEx(0, WC_LISTVIEW, 0,
		WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP | WS_VSCROLL |
		LVS_REPORT | LVS_SHOWSELALWAYS | LVS_ICON,
		75 + 300 + 20, 85, 300 + 20, 300, hParent, 0, hInstance, 0);
	// Обнуляем поля структуры
	memset(&lvc, 0, sizeof(LVCOLUMN));
	// Добавляем столбцы
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.iSubItem = 0;
	lvc.pszText = "Имя";
	lvc.cx = 160;
	lvc.fmt = LVCFMT_LEFT;
	// Первый столбец
	ListView_InsertColumn(hLocalList, 0, &lvc);
	lvc.pszText = "Дата";
	lvc.cx = 70;
	// Второй столбец
	ListView_InsertColumn(hLocalList, 1, &lvc);
	lvc.pszText = "Размер";
	lvc.cx = 70;
	// Третий столбец
	ListView_InsertColumn(hLocalList, 2, &lvc);
	ListView_SetExtendedListViewStyleEx(hLocalList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	// Результат построения
	return hFtpList && hLocalList;
}
// Статус бар
BOOL ClientCreateStatusBar(HINSTANCE hInstance, HWND hParent, int idStatus, int cParts, HWND &hStatus)
{
    RECT rcClient;
    HLOCAL hloc;
    PINT paParts;
    int i, nWidth;
    // Ensure that the common control DLL is loaded.
    InitCommonControls();
    // Create the status bar.
    hStatus = CreateWindowEx(
		0,													
        STATUSCLASSNAME,									
        (PCTSTR) NULL,										
         // includes a sizing grip
		WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP,		// creates a visible child window
		0, 0, 0, 0,
        hParent,											// handle to parent window
        (HMENU) idStatus,									// child window identifier
		hInstance,											// handle to application instance
        NULL);												// no window creation data

    // Get the coordinates of the parent window's client area.
    GetClientRect(hParent, &rcClient);

    // Allocate an array for holding the right edge coordinates.
    hloc = LocalAlloc(LHND, sizeof(int) * cParts);
    paParts = (PINT) LocalLock(hloc);

    // Calculate the right edge coordinate for each part, and
    // copy the coordinates to the array.
    nWidth = rcClient.right / cParts;
    int rightEdge = nWidth;
    for (i = 0; i < cParts; i++) { 
       paParts[i] = rightEdge;
       rightEdge += nWidth;
    }

    // Tell the status bar to create the window parts.
    SendMessage(hStatus, SB_SETPARTS, (WPARAM) cParts, (LPARAM)
               paParts);

    // Free the array, and return.
    LocalUnlock(hloc);
    LocalFree(hloc);

    return (BOOL)hStatus;
}  
// Комбо бокс
BOOL ClientCreateComboBox(HINSTANCE hInstance, HWND hParent, HWND &hComboBox)
{
	hComboBox = CreateWindow(WC_COMBOBOX, 0, 
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST,
		95 + 300, 55, 150, 100,
		hParent, 0, hInstance, 0);

	char LogicalDrives[MAX_PATH];
	GetLogicalDriveStringsA(sizeof(LogicalDrives), LogicalDrives);
	char *DRF [] = {"Unknown" , "Invalid path",	"Removable", "Fixed" , "Network drive","CD-ROM", "RAM disk"};
	for(char *s = LogicalDrives; *s; s += strlen(s) + 1)
	{
		if(GetDriveTypeA(s) == 2 || GetDriveTypeA(s) == 3 || GetDriveTypeA(s) == 4)
			SendMessage(hComboBox, CB_ADDSTRING,0,(LPARAM)s);
	}
	SendMessage(hComboBox, CB_SETCURSEL, 0, 0);
	SendMessage(hParent, WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hComboBox), CBN_SELENDOK), LPARAM(hComboBox));
	// Результат построения
	return (BOOL)hComboBox;
}