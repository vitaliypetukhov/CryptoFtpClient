#pragma once
#include "Headers.h"
#include "Settings.h"

class FtpClient
{
public:
	FtpClient();
	~FtpClient();
	
	void SetControls(HWND &hEditAdress, HWND &hEditUserName, HWND &hEditUserPassword, 
		HWND &hButtonConnect, HWND &hButtonDownload, HWND &hButtonUpload,
		HWND &hLocalListView, HWND &hFtpListView,
		HWND &hComboBox, HWND &hStatusBar);
	
	void FilesSort(vector<WIN32_FIND_DATA> &);
	void CheckLastError(DWORD code, char *szErrBuff);
	
	BOOL Connect(Settings &settings);
	void Disconnect();
	
	DWORD LocalListDirectory(vector<WIN32_FIND_DATA> &ftpList, LPCSTR path);
	void LocalShowDir(vector<WIN32_FIND_DATA> localList);
	
	DWORD LocalChangeDir(const char * szDirName);	
	DWORD FtpListDirectory(vector<WIN32_FIND_DATA> &ftpList, LPCSTR path);

	void FtpShowDir(vector<WIN32_FIND_DATA> ftpList);
	
	DWORD FtpChangeDir(const char * szDirName);	
	// Принять
	DWORD GetFile(LPCSTR ftpPath, LPCSTR localPath, HWND hProgBar, HWND hStatic);
	// Отправить
	DWORD PutFile(LPCSTR ftpPath, LPCSTR localPath, HWND hProgBar, HWND hStatic);
	
	HWND hEditAdress, hEditUserName, hEditUserPassword, 
		hButtonConnect, hButtonDownload, hButtonUpload,
		hLocalListView, hFtpListView,
		hComboBox, hStatusBar;
	// Настройки
	Settings settings;
	// Событие отправки/получения файла
	HANDLE TransitEvent;
	
	TCHAR szLocalPath[MAX_PATH];
	TCHAR szFtpPath[MAX_PATH];
	TCHAR szErrorBuff[1024];
};