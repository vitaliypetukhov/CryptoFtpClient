#pragma once
#define _CRT_SECURE_NO_DEPRECATE
#define _WTL_NO_CSTRING

#define ERROR_FTP_COMMAND_LIST WM_USER + 100
#define ERROR_FTP_STOR_FILE WM_USER + 101
#define ERROR_FTP_RETR_FILE WM_USER + 102
#define ERROR_LOCAL_CREATE_FILE WM_USER + 200
#define ERROR_LOCAL_CREATE_DIRECTORY WM_USER + 201
#define ERROR_GET_LOCAL_FILE_SIZE WM_USER + 202

#include "Client.h"

CRITICAL_SECTION locker;

HINTERNET hInternet = 0;
HINTERNET hIConnect = 0;

FtpClient::FtpClient()
{
	memset(szLocalPath, 0, sizeof(TCHAR) * MAX_PATH);
	memset(szFtpPath, 0, sizeof(TCHAR) * MAX_PATH);
	InitializeCriticalSection(&locker);
}
FtpClient::~FtpClient()
{
	CloseHandle(TransitEvent);
	DeleteCriticalSection(&locker);
}

// Установка хендлов элементов
void FtpClient::SetControls(HWND &_hEditAdress, HWND &_hEditUserName, HWND &_hEditUserPassword, 
		HWND &_hButtonConnect, HWND &_hButtonDownload, HWND &_hButtonUpload,
		HWND &_hLocalListView, HWND &_hFtpListView,
		HWND &_hComboBox, HWND &_hStatusBar)
{
	hEditAdress = _hEditAdress;
	hEditUserName = _hEditUserName;
	hEditUserPassword = _hEditUserPassword;
	hButtonConnect = _hButtonConnect;
	hButtonDownload = _hButtonDownload;
	hButtonUpload = _hButtonUpload;
	hLocalListView = _hLocalListView;
	hFtpListView = _hFtpListView;
	hComboBox = _hComboBox;
	hStatusBar = _hStatusBar;
}
// Сортировка списка файлов
void FtpClient::FilesSort(vector<WIN32_FIND_DATA> &vec)
{
	
	vector<WIN32_FIND_DATA> folders, files;
	
	WIN32_FIND_DATA temp;
	
	long len = vec.size();
	// Собираем векторы папок и файлов
	for(long i = 0; i < len; i++)
		if(vec.at(i).dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			folders.push_back(vec.at(i));
		else
			files.push_back(vec.at(i));
	
	int folderCount = folders.size();
	
	int filesCount = files.size();
	
	long dotsFound = -1;
	for(long i = 0; i < folderCount && dotsFound == -1; i++)
		if(strcmp(vec.at(i).cFileName, "..") == 0)
			dotsFound = i;
	// Если папка найдена
	if(dotsFound != -1)
	{
		
		memset(&temp, 0, sizeof(WIN32_FIND_DATA));
		temp = folders.at(0);
		folders.at(0) = folders.at(dotsFound);
		folders.at(dotsFound) = temp;
	}
	// Сортируем папки по алфавиту
	for(long i = (dotsFound == -1) ? 0 : 1; i < folderCount; i++)
		for(long j = i + 1; j < folderCount; j++)
			if(folders.at(i).cFileName > folders.at(j).cFileName)
			{
				memset(&temp, 0, sizeof(WIN32_FIND_DATA));
				temp = folders.at(i);
				folders.at(i) = folders.at(j);
				folders.at(j) = temp;
			}
	// Сортировка файлов
	for(long i = 0; i < filesCount ; i++)
		for(long j = i + 1; j < filesCount; j++)
			if(files.at(i).cFileName > files.at(j).cFileName)
			{
				memset(&temp, 0, sizeof(WIN32_FIND_DATA));
				temp = files.at(i);
				files.at(i) = files.at(j);
				files.at(j) = temp;
			}

	vec.clear();
	
	for(size_t i = 0; i < folders.size(); i++)
		vec.push_back(folders.at(i));
	for(size_t i = 0; i < files.size(); i++)
		vec.push_back(files.at(i));
}
// Формирование строки ошибки
void FtpClient::CheckLastError(DWORD code, char * szErrorBuff)
{
	switch(code)
	{
	case ERROR_INTERNET_LOGIN_FAILURE:
		sprintf(szErrorBuff, "Не удалось выполнить запрос на подключение и вход на FTP-сервер.");
		break;
	case ERROR_INTERNET_INCORRECT_PASSWORD:
		sprintf(szErrorBuff, "Не удалось выполнить запрос на подключение и вход на FTP-сервер, поскольку указан неверный пароль.");
		break;
	case ERROR_INTERNET_CANNOT_CONNECT:
		sprintf(szErrorBuff, "Не удалось подключиться к серверу.");
		break;
	case ERROR_INTERNET_CONNECTION_ABORTED:
		sprintf(szErrorBuff, "Соединение с сервером было разорвано.");
		break;
	case ERROR_FTP_DROPPED:
		sprintf(szErrorBuff, "FTP-операция не завершена из-за прекращения сеанса.");
		break;
	case ERROR_INTERNET_TIMEOUT:
		sprintf(szErrorBuff, "Запрос просрочен.");
		break;
	case ERROR_INTERNET_NAME_NOT_RESOLVED:
		sprintf(szErrorBuff, "Не удалось разрешить имя сервера.");
		break;
	case ERROR_FTP_RETR_FILE:
		sprintf(szErrorBuff, "Не удалось выполнить команду получаения серверного файла.");
		break;
	case ERROR_FTP_STOR_FILE:
		sprintf(szErrorBuff, "Не удалось выполнить команду размещения файла на сервере.");
		break;
	case ERROR_FTP_COMMAND_LIST:
		sprintf(szErrorBuff, "Ошибка получения списка файлов");
		break;
	case ERROR_LOCAL_CREATE_FILE:
		sprintf(szErrorBuff, "Ошибка создания локального файла");
		break;
	case ERROR_ALREADY_EXISTS:
		sprintf(szErrorBuff, "Ошибка каталог уже существует");
		break;
	case ERROR_LOCAL_CREATE_DIRECTORY:
		sprintf(szErrorBuff, "Ошибка создания локального каталога");
		break;
	case ERROR_GET_LOCAL_FILE_SIZE:
		sprintf(szErrorBuff, "Ошибка получения размера локального файла");
		break;
	case ERROR_INVALID_NAME:
		sprintf(szErrorBuff, "Синтаксическая ошибка в имени файла, имени папки или метке тома.");
		break;
	case ERROR_ACCESS_DENIED:
		sprintf(szErrorBuff, "Отказано в доступе.");
		break;
	case ERROR_INTERNET_EXTENDED_ERROR:
		{
			sprintf(szErrorBuff, "Сервер возвратил расширенную ошибку. Как правило, это строка или буфер, содержащие сообщение об ошибке. Вызовите функцию InternetGetLastResponseInfo для получения текста ошибки.");
			DWORD dwInetError;
			DWORD dwExtLength = 1000;
			TCHAR *szExtErrMsg = NULL;
			TCHAR errmsg[1000];
			szExtErrMsg = errmsg;
			if(InternetGetLastResponseInfo( &dwInetError, szExtErrMsg, &dwExtLength ))
			{
				sprintf(szErrorBuff, "%s", szExtErrMsg);
			}
		}
		break;
	case ERROR_FTP_TRANSFER_IN_PROGRESS:
		sprintf(szErrorBuff, "Запрошенную операцию невозможно выполнить для дескриптора FTP-сеанса, так как операция уже выполняется.");
		break;
	default:
		sprintf(szErrorBuff, "Неизвестная ошибка. Код ошибки: %d", code);
		break;
	}

	
}
// Функция коннекта
BOOL FtpClient::Connect(Settings &_settings)
{
	sprintf(szFtpPath, "/");
	// Выводим в статусбар текущий путь
	SendMessage((HWND) hStatusBar, (UINT) SB_SETTEXT, (WPARAM)(INT) 0 | 0, (LPARAM) szFtpPath);
	settings = _settings;
	
	hInternet = InternetOpen("Test FTP Client", INTERNET_OPEN_TYPE_PRECONFIG, 0, 0, 0);//INTERNET_FLAG_RELOAD | INTERNET_FLAG_RESYNCHRONIZE);
	if(!hInternet)
	{
		CheckLastError(GetLastError(), szErrorBuff);
		return FALSE;
	}
	if(strcmp(settings.Login.c_str(), "anonymous") == 0 && settings.Password.size() == 0)
		strcpy((char*)settings.Password.c_str(), "anonymous@mail.com");
	hIConnect = InternetConnect(hInternet, settings.Adress.c_str(), settings.connectionPort, 
		settings.Login.c_str(), settings.Password.c_str(), INTERNET_SERVICE_FTP, WININET_API_FLAG_SYNC, 0);//INTERNET_FLAG_RELOAD | INTERNET_FLAG_RESYNCHRONIZE, 0);
	if(!hIConnect)
	{
		CheckLastError(GetLastError(), szErrorBuff);
		InternetCloseHandle(hInternet);
		return FALSE;
	}
	return TRUE;
}
// Функция закрытия соединения
void FtpClient::Disconnect()
{
	sprintf(szFtpPath, "/");
	InternetCloseHandle(hIConnect);
	InternetCloseHandle(hInternet);
	hIConnect = 0;
	hInternet = 0;
	
	SendMessage(hEditAdress, EM_SETREADONLY, FALSE, 0);
	SendMessage(hEditUserName, EM_SETREADONLY, FALSE, 0);
	SendMessage(hEditUserPassword, EM_SETREADONLY, FALSE, 0);
	
	ListView_DeleteAllItems(hFtpListView);
	
	EnableWindow(hFtpListView, FALSE);
	EnableWindow(hButtonDownload, FALSE);
	EnableWindow(hButtonUpload, FALSE);
	
	SendMessage(hButtonConnect, WM_SETTEXT, sizeof("Присоединиться"), LPARAM("Присоединиться"));	
	SendMessage(hStatusBar, (UINT) SB_SETTEXT, (WPARAM)(INT) 0 | 0, (LPARAM) 0);
}
// Получить локальный список файлов 
DWORD FtpClient::LocalListDirectory(vector<WIN32_FIND_DATA> &localList, LPCSTR path)
{
	TCHAR BUFF[MAX_PATH] = {0};
	
	localList.clear();	
	
	HANDLE hFind = 0;
	
	WIN32_FIND_DATA w32FindData;
	
	memset(&w32FindData, 0, sizeof(WIN32_FIND_DATA));
	sprintf(BUFF, "%s\\*.*", path);
	
	hFind = FindFirstFile(BUFF, &w32FindData);
	if(!hFind)
		return GetLastError();
	do 
	{
		if(strcmp(w32FindData.cFileName, ".") != 0 && (w32FindData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) == 0)
			localList.push_back(w32FindData);
		// Переходим к следующему файлу
		memset(&w32FindData, 0, sizeof(WIN32_FIND_DATA));
	} while(FindNextFile(hFind, &w32FindData));
	FilesSort(localList);	
	FindClose(hFind);
	return 0;
}
// Показ содержимого локального каталога
void FtpClient::LocalShowDir(vector<WIN32_FIND_DATA> localList)
{
	// Список иконок
	HIMAGELIST imageList = 0;
	SHFILEINFO sfi;
	
	LVITEM lvItem;
	
	string str = "";
	
	ListView_DeleteAllItems(hLocalListView);
	ListView_SetExtendedListViewStyleEx(hLocalListView, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	
	imageList = ImageList_Create(
		GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_MASK | ILC_COLOR32, 1, 1);
	
	for(size_t i = 0; i < localList.size(); i++)
	{
		memset(&lvItem, 0, sizeof(LVITEM));
		
		lvItem.mask = LVIF_IMAGE|LVIF_TEXT;
		lvItem.state = 0;
		lvItem.stateMask = 0 ;
		lvItem.iItem = i;
		lvItem.iImage = i;
		lvItem.iSubItem = 0;
		lvItem.pszText = localList.at(i).cFileName;
		lvItem.cchTextMax = sizeof(localList.at(i).cFileName);
		// Добавляем в список
		ListView_InsertItem(hLocalListView, &lvItem);
		if(strcmp(localList.at(i).cFileName, "..") != 0)
		{
			lvItem.iSubItem = 1;
			SYSTEMTIME sysTime;
			FileTimeToSystemTime(&localList.at(i).ftCreationTime, &sysTime);
			stringstream strStream;
			strStream << sysTime.wDay << "." << sysTime.wMonth << "." << sysTime.wYear;
			string temp = strStream.str();
			lvItem.pszText = (LPSTR)temp.c_str();
			lvItem.cchTextMax = temp.size();
			ListView_SetItem(hLocalListView, &lvItem);

			strStream.str().resize(0);
			strStream.str().clear();
			strStream.str("");
				
			lvItem.iSubItem = 2;

			// Размер файла
			LONGLONG fileSize;
			LARGE_INTEGER sz;
			// Получаем размер файла
			sz.HighPart = localList.at(i).nFileSizeHigh;
			sz.LowPart = localList.at(i).nFileSizeLow;
			fileSize = sz.QuadPart;
			if (fileSize == 0)
				strStream << "";
			else if (fileSize < 1024)
				strStream << fileSize << " B";
			else if (fileSize < 1024 * 1024)
				strStream << 1.0 * fileSize / 1024 << " Kb";
			else
				strStream << 1.0 * fileSize / 1024 / 1024 << " Mb";
			temp = strStream.str();
			lvItem.pszText = (LPSTR)temp.c_str();
			lvItem.cchTextMax = temp.size();
			ListView_SetItem(hLocalListView, &lvItem);
		}			
		
		ZeroMemory(&sfi, sizeof(SHFILEINFO));
		
		SHGetFileInfo(localList.at(i).cFileName, localList.at(i).dwFileAttributes, &sfi, sizeof(SHFILEINFO), 
			SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES | SHGFI_EXETYPE);		
		
		ImageList_AddIcon(imageList, sfi.hIcon);
	}
	ListView_SetImageList(hLocalListView, imageList, LVSIL_SMALL);
}
// Смена локального каталога
DWORD FtpClient::LocalChangeDir(const char * szDirName)
{	
	// Новый путь перехода
	TCHAR szNewDirName[MAX_PATH] = {0};
	// Формируем новый путь добавляя указанную папку
	_tcscat(szNewDirName, szLocalPath);
	// Если название папки не ".." - добавим завершающий слеш
	if(strcmp(szNewDirName, "..") != 0)
		_tcscat(szNewDirName, "\\");
	// Формируем полную строку нового пути перехода
	_tcscat(szNewDirName, szDirName);

	// Меняем текущуее местоположение
	SetCurrentDirectory(szNewDirName);
	// Получаем путь
	if(!GetCurrentDirectory(MAX_PATH, szLocalPath))
		return GetLastError();
	// Если не добрались до корня диска 
	if(szLocalPath[strlen(szLocalPath) - 1] != '\\')
		// Добавляем к пути завершающий слеш
		_tcscat(szLocalPath, "\\");
	// Выводим в статусбар текущий путь
	SendMessage((HWND) hStatusBar, (UINT) SB_SETTEXT, (WPARAM)(INT) 0 | 1, (LPARAM) szLocalPath);
	// Возвращаем результат работы
	return GetLastError();
}
// Получить список файлов в заданной директории
DWORD FtpClient::FtpListDirectory(vector<WIN32_FIND_DATA> &ftpList, LPCSTR path)
{
	ftpList.clear();
	// Создаем структуру поиска
	HINTERNET hInternetFind = 0;
	// Структура для поиска файлов
	WIN32_FIND_DATA w32FindData;
	// Обнуляем поля структуры
	memset(&w32FindData, 0, sizeof(WIN32_FIND_DATA));	


//	InternetGetConnectedState(

	// Ищем первый файл в директории
	hInternetFind = FtpFindFirstFile(hIConnect, path,
		&w32FindData, INTERNET_FLAG_RELOAD, 0);
	if(!hInternetFind)
	{
		
		CheckLastError(GetLastError(), szErrorBuff);
		DWORD dwInetError;
		DWORD dwExtLength = 1000;
		TCHAR szExtErrMsg[1000] = {0};
		TCHAR szErr[1200] = {0};
		// Получаем код ошибки
		if(InternetGetLastResponseInfo( &dwInetError, szExtErrMsg, &dwExtLength ))
		{
			if(strstr(szExtErrMsg, "150") == 0 && strstr(szExtErrMsg, "226") == 0)
			{
				MessageBox(0, szExtErrMsg, "Ошибка получения списка каталогов.", MB_OK | MB_ICONEXCLAMATION);
				return ERROR_FTP_COMMAND_LIST;
			}
			return 0;
		} 	
	}
	do 
	{
		// Получаем имя файла
		if(strcmp(w32FindData.cFileName, ".") != 0 /*&& strcmp(w32FindData.cFileName, "..") != 0*/ && strlen(w32FindData.cFileName) > 0)
			ftpList.push_back(w32FindData);
		memset(&w32FindData, 0, sizeof(WIN32_FIND_DATA));		
	} while(InternetFindNextFile(hInternetFind, &w32FindData));
	
	FilesSort(ftpList);
	
	InternetCloseHandle(hInternetFind);
	
	return 0;
}
// Сбор и вывод информации о содержимом текущего каталога
void FtpClient::FtpShowDir(vector<WIN32_FIND_DATA> ftpList)
{
	// Список иконок
	HIMAGELIST imageList = 0;
	SHFILEINFO sfi;
	LVITEM lvItem;
	
	string str = "";
	// Очищаем сисок файлов
	ListView_DeleteAllItems(hFtpListView);
	ListView_SetExtendedListViewStyleEx(hFtpListView, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	imageList = ImageList_Create(
		GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_MASK | ILC_COLOR32, 1, 1);
	
	for(size_t i = 0; i < ftpList.size(); i++)
	{
		str =  (ftpList.at(i).cFileName);
		memset(&lvItem, 0, sizeof(LVITEM));
	
		lvItem.mask = LVIF_IMAGE|LVIF_TEXT;
		lvItem.state = 0;
		lvItem.stateMask = 0 ;
		lvItem.iItem = i;
		lvItem.iImage = i;
		lvItem.iSubItem = 0;
		lvItem.pszText = (LPSTR)str.c_str();
		lvItem.cchTextMax = str.size();
		// Добавляем в список
		ListView_InsertItem(hFtpListView, &lvItem);
		lvItem.iSubItem = 1;
		SYSTEMTIME sysTime;
		memset(&sysTime, 0, sizeof(SYSTEMTIME));
		FileTimeToSystemTime(&ftpList.at(i).ftCreationTime, &sysTime);
		stringstream strStream;
		strStream << sysTime.wDay << "." << sysTime.wMonth << "." << sysTime.wYear;
		string temp = strStream.str();
		lvItem.pszText = (LPSTR)temp.c_str();
		lvItem.cchTextMax = temp.size();
		ListView_SetItem(hFtpListView, &lvItem);

		strStream.str().resize(0);
		strStream.str().clear();
		strStream.str("");
				
		lvItem.iSubItem = 2;
		// Размер файла
		LONGLONG fileSize;
		LARGE_INTEGER sz;
		// Получаем размер файла
		sz.HighPart = ftpList.at(i).nFileSizeHigh;
		sz.LowPart = ftpList.at(i).nFileSizeLow;
		fileSize = sz.QuadPart;
		if (fileSize == 0)
			strStream << "";
		else if (fileSize < 1024)
			strStream << fileSize << " B";
		else if (fileSize < 1024 * 1024)
			strStream << 1.0 * fileSize / 1024 << " Kb";
		else
			strStream << 1.0 * fileSize / 1024 / 1024 << " Mb";
		temp = strStream.str();
		lvItem.pszText = (LPSTR)temp.c_str();
		lvItem.cchTextMax = temp.size();
		ListView_SetItem(hFtpListView, &lvItem);	
		// Обнуляем поля
		ZeroMemory(&sfi, sizeof(SHFILEINFO));
		// Получаем информацию о переданном элементе
		SHGetFileInfo(ftpList.at(i).cFileName, ftpList.at(i).dwFileAttributes, &sfi, sizeof(SHFILEINFO), 
			SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES | SHGFI_EXETYPE);		
		ImageList_AddIcon(imageList, sfi.hIcon);
	}
	// Добавляем иконки к списку элементов
	ListView_SetImageList(hFtpListView, imageList, LVSIL_SMALL);
}
// Перейти в новый каталог
DWORD FtpClient::FtpChangeDir(const char * szDirName)
{
	// Проверка открытия соединения
	if(!hIConnect)
		return GetLastError();
	TCHAR szNewDirName[MAX_PATH] = {0};
	_tcscat(szNewDirName, szFtpPath);
	if(strcmp(szFtpPath, "/") != 0)
		_tcscat(szNewDirName, "/");
	_tcscat(szNewDirName, szDirName);
	FtpSetCurrentDirectory(hIConnect, szNewDirName);
	DWORD dwSize = MAX_PATH;
	if(!FtpGetCurrentDirectory(hIConnect, szFtpPath, &dwSize))
		return GetLastError();
	SendMessage((HWND) hStatusBar, (UINT) SB_SETTEXT, (WPARAM)(INT) 0 | 0, (LPARAM) szFtpPath);
	return GetLastError();
}
// Получить файл
DWORD FtpClient::GetFile(LPCSTR ftpPath, LPCSTR localPath, HWND hProgBar, HWND hStatic)
{
	EnterCriticalSection(&locker);
	DWORD dwInetError;
	DWORD dwExtLength = 1000;
	TCHAR *szExtErrMsg = NULL;
	TCHAR errmsg[1000];
	TCHAR szErr[1000] = {0};
	szExtErrMsg = errmsg;
	
	DWORD dwRead = -1;
	LONGLONG fileSize = 0;
	TCHAR readBuffer[1024] = {0};
	TCHAR szMsgBuff[1000];
	HINTERNET hFtpFile = FtpOpenFile(hIConnect, ftpPath, GENERIC_READ, FTP_TRANSFER_TYPE_BINARY | INTERNET_FLAG_RELOAD, 0);
	if(hFtpFile)
	{
		fileSize = FtpGetFileSize(hFtpFile, 0);
		sprintf(szMsgBuff, "Копирование\r\n из \"%s\"\r\n в \"%s\"", ftpPath, localPath);
		SetWindowText(hStatic, szMsgBuff);
		LONGLONG onePercent = fileSize / 100;
		SendMessage(hProgBar, PBM_SETPOS, (WPARAM)0, 0);
		SendMessage(hProgBar, PBM_SETRANGE, 0, (LPARAM)MAKELONG(0, 100));
		SendMessage(hProgBar, PBM_SETSTEP, (WPARAM)1, 0);
		HANDLE localfile = CreateFile(localPath, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		if(localfile == INVALID_HANDLE_VALUE)
		{
			InternetCloseHandle(hFtpFile);
			LeaveCriticalSection(&locker);
			return ERROR_LOCAL_CREATE_FILE;
		}
		DWORD dwBytesWritten = -1;
		LONGLONG dwReaded = -1;
		while (WaitForSingleObject(TransitEvent, 0) != WAIT_OBJECT_0 && dwRead && dwBytesWritten)
		{
			memset(readBuffer, 0, sizeof(TCHAR) * 1024);
			InternetReadFile(hFtpFile, readBuffer, 1024, &dwRead);
			WriteFile(localfile, readBuffer, dwRead, &dwBytesWritten, 0);
			dwReaded += dwRead;
			if(dwReaded >= onePercent)
			{
				dwReaded = 0;
				SendMessage(hProgBar, PBM_STEPIT, 0, 0);
			}
		}
		SendMessage(hProgBar, PBM_SETPOS, 100, 0);
		UpdateWindow(hProgBar);
		CloseHandle(localfile);
	}
	else
	{
		InternetGetLastResponseInfo( &dwInetError, szExtErrMsg, &dwExtLength );
		sprintf(szErr, "%s", szExtErrMsg);
		MessageBox(0, szErr, "Расширенная ошибка.", MB_OK | MB_ICONEXCLAMATION);
		LeaveCriticalSection(&locker);
		return ERROR_FTP_RETR_FILE;
	}
	InternetCloseHandle(hFtpFile);
	LeaveCriticalSection(&locker);
	return 0;
}
// Отправить
DWORD FtpClient::PutFile(LPCSTR ftpPath, LPCSTR localPath, HWND hProgBar, HWND hStatic)
{
	EnterCriticalSection(&locker);
	DWORD dwInetError;
	DWORD dwExtLength = 1000;
	TCHAR *szExtErrMsg = NULL;
	TCHAR errmsg[1000];
	TCHAR szErr[1000] = {0};
	szExtErrMsg = errmsg;

	DWORD dwRead = 0;
	LONGLONG fileSize = 0;
	TCHAR readBuffer[1024] = {0};
	TCHAR szMsgBuff[1000] = {0};
	HINTERNET hFtpFile = 0;
	// Получаем ключ
	string curKey;
	curKey.assign(settings.Key);
	transform(curKey.begin(), curKey.end(), curKey.begin(), tolower);
	// Буфер для команды
	TCHAR command[1024] = {0};
	sprintf(command, "%s\t%s", ftpPath, curKey.c_str());

	//MessageBox(0, curKey.c_str(), 0, 0);

	hFtpFile = FtpOpenFile(hIConnect, command, GENERIC_WRITE, FTP_TRANSFER_TYPE_BINARY | INTERNET_FLAG_RELOAD | INTERNET_FLAG_RESYNCHRONIZE, 0);
	if(!hFtpFile)
	{
		InternetGetLastResponseInfo( &dwInetError, szExtErrMsg, &dwExtLength );
		// Копируем в буфер
		sprintf(szErr, "%s", szExtErrMsg);
		MessageBox(0, szErr, "Ошибка загрузки на сервер.", MB_OK | MB_ICONEXCLAMATION);
		InternetCloseHandle(hFtpFile);
		LeaveCriticalSection(&locker);
		return ERROR_FTP_STOR_FILE;
	}
	sprintf(szMsgBuff, "Копирование\r\n из \"%s\"\r\n в \"%s\"", localPath, ftpPath);
	
	SetWindowText(hStatic, szMsgBuff);
	
	HANDLE localfile = CreateFile(localPath, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	
	if(localfile == INVALID_HANDLE_VALUE)
	{
		MessageBox(0, "Ошибка открытия локального файла.", "Ошибка.", MB_OK | MB_ICONEXCLAMATION);
		
		InternetCloseHandle(hFtpFile);
		LeaveCriticalSection(&locker);
		
		return ERROR_INVALID_HANDLE;
	}
	LARGE_INTEGER sz = {0};
	// Получаем размер файла
	if(!GetFileSizeEx(localfile, &sz))
	{
		InternetCloseHandle(hFtpFile);
		LeaveCriticalSection(&locker);
		return ERROR_GET_LOCAL_FILE_SIZE;
	}
	LONGLONG onePercent = sz.QuadPart / 100;
	
	SendMessage(hProgBar, PBM_SETPOS, (WPARAM)0, 0);
	SendMessage(hProgBar, PBM_SETRANGE, 0, (LPARAM)MAKELONG(0, 100));
	SendMessage(hProgBar, PBM_SETSTEP, (WPARAM)1, 0);

	DWORD dwBytesWritten = -1;
	DWORD dwReaded = -1;
	dwRead = -1;
	int pos = 0;
	// Читаем файл блоками по 1024 байт
	while (WaitForSingleObject(TransitEvent, 0) != WAIT_OBJECT_0 && dwBytesWritten && dwRead)
	{
		memset(readBuffer, 0, sizeof(TCHAR) * 1024);
		// Читаем блок
		ReadFile(localfile, readBuffer, sizeof(TCHAR) * 1024, &dwRead, 0);
		// Если задан ключ
		if(curKey != "")
		{
			////// encrypt section start //////						
			for(size_t i = 0; i < dwRead; i++, pos++)
			{
				if(pos == curKey.size())
					pos = 0;
				readBuffer[i] ^= curKey.at(pos);
			}
			////// encrypt section end //////
		}
		
		// Записываем считанный блок
		InternetWriteFile(hFtpFile, readBuffer, sizeof(TCHAR) * dwRead, &dwBytesWritten);
		// Суммируем количество считанных байтов
		dwReaded += dwRead;
		if(dwReaded >= onePercent)
		{
			dwReaded = 0;
			SendMessage(hProgBar, PBM_STEPIT, 0, 0);
		}
	}
	SendMessage(hProgBar, PBM_SETPOS, 100, 0);
	UpdateWindow(hProgBar);
	CloseHandle(localfile);
	InternetCloseHandle(hFtpFile);
	LeaveCriticalSection(&locker);
	return 0;
}