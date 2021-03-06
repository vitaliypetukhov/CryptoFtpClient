
#include "Controls.h"
#include "Client.h"

#include "Headers.h"

#define WM_CLIENT_INIT WM_USER + 1 

extern HINTERNET hInternet, hIConnect;
FtpClient client;
Settings settings;

const int WindowWidth = 745;
const int WindowHeight = 455;

char szWindowName[MAX_PATH] = "FTP Client";

char szClassName[MAX_PATH] = "FTP Client Test App";

char szErrorBuff[MAX_PATH] = {0};




HANDLE thID = 0;
HANDLE InitEvent = 0;

bool Connected = false;

HWND hWndMain = 0, hEditAdress = 0, hEditUserName = 0, hEditUserPassword = 0, 
	hButtonConnect = 0, hButtonDownload = 0, hButtonUpload = 0, hButtonSettings = 0, hButtonHelp = 0,
	hLocalListView = 0, hFtpListView = 0, hComboBox = 0, hStatusBar = 0, 
	hProgBar = 0, hStatic = 0, hDlg = 0;

vector<WIN32_FIND_DATA> FtpFileList;

vector<WIN32_FIND_DATA> LocalFileList;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

BOOL CALLBACK ProgressDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK SettingsDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

DWORD WINAPI Download(LPVOID lpParam);

DWORD WINAPI Upload(LPVOID lpParam);


int WINAPI WinMain(HINSTANCE hInstance, 
				   HINSTANCE hPrevInstance,
				   LPSTR lpCmdLine,
				   int nCmdShow)
{
	
	hInstance				= GetModuleHandle(NULL);
		
	if((hWndMain = ClientCreateWindow(hInstance, WndProc, 
								szClassName, szWindowName,
								WindowWidth, WindowHeight)) == 0)
	{
		MessageBox( NULL, "Window Creation Error.", "ERROR", MB_OK | MB_ICONEXCLAMATION );
		// ������ �������� ����
		return false;
	}
	// �������� ����
	ShowWindow(hWndMain, SW_SHOW);
	// ������ ������� ���������
	SetForegroundWindow(hWndMain);
	
	SetFocus(hWndMain);
	///////////////////////////////// �������� ����������� ///////////////////////////////////
	if(!ClientCreateEdits(hInstance, hWndMain, hEditAdress, hEditUserName, hEditUserPassword) ||
		!ClientCreateButtons(hInstance, hWndMain, hButtonConnect, hButtonDownload, hButtonUpload, hButtonSettings, hButtonHelp) ||
		!ClientCreateListViews(hInstance, hWndMain, hLocalListView, hFtpListView) ||
		!ClientCreateStatusBar(hInstance, hWndMain, 0, 2, hStatusBar) ||
		!ClientCreateComboBox(hInstance, hWndMain, hComboBox))
	{
		MessageBox(0, "Error Create Client Components", "Error initialize components", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}
	
	client.SetControls(hEditAdress, hEditUserName, hEditUserPassword, 
		hButtonConnect, hButtonDownload, hButtonUpload,
		hLocalListView, hFtpListView, hComboBox, hStatusBar);
	if(!settings.Load())
	{
		settings.Default();
		settings.Save();
	}
	
	SendMessage(hEditAdress, WM_SETTEXT, settings.Adress.size(), LPARAM(settings.Adress.c_str()));
	SendMessage(hEditUserName, WM_SETTEXT, settings.Login.size(), LPARAM(settings.Login.c_str()));
	SendMessage(hEditUserPassword, WM_SETTEXT, settings.Password.size(), LPARAM(settings.Password.c_str()));
	SendMessage(hComboBox, SB_GETTEXT, 0, (LPARAM) client.szLocalPath);
	SendMessage(hStatusBar, SB_SETTEXT, (WPARAM)(INT) 1 | 0, (LPARAM) client.szLocalPath);
	
	// ���������
	MSG Msg;
	// ���� ������� ���������
	while(GetMessage(&Msg, 0, 0, 0))
	{
		
		switch (Msg.message)
        {
            case WM_KEYDOWN:
				if(LOWORD(Msg.wParam) == VK_ESCAPE)
				{
					client.Disconnect();
					PostQuitMessage(0);
				}     
            break;	
		case WM_SYSCHAR:
		case WM_SYSKEYUP:
		case WM_SYSKEYDOWN:
			if(LOWORD(Msg.wParam) == VK_ESCAPE)
			{
				client.Disconnect();
				PostQuitMessage(0);
				return 0;	
			}
        }
		
		if(!IsDialogMessage(hWndMain, &Msg))
        {
            TranslateMessage( &Msg );   
            DispatchMessage( &Msg );
		}
	}
	return Msg.wParam;
}
// ������� ���������
LRESULT CALLBACK WndProc(HWND hWnd, 
						 UINT uMsg, 
						 WPARAM wParam, 
						 LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_SYSCHAR:
	case WM_SYSKEYUP:
	case WM_SYSKEYDOWN:
		if(LOWORD(wParam) == VK_ESCAPE)
		{
			client.Disconnect();
			PostQuitMessage(0);
		}
		return 0;
	case WM_INITDIALOG:
		return 0;
	case WM_ACTIVATE:
		return 0;
	case WM_COMMAND:
		{
			// ������������ ��������� ���������� ��� ��������� ������ ����������� ����
			if((HWND)lParam == hComboBox)
			{
				if(HIWORD(wParam) == CBN_SELENDOK)
				{
					int idx_row;
					idx_row = SendMessage(hComboBox, CB_GETCURSEL, 0, 0);
					SendMessage(hComboBox, CB_GETLBTEXT, idx_row, (LPARAM)client.szLocalPath);
					// �������� ��������� ����� ������������
					SendMessage(hStatusBar, SB_SETTEXT, (WPARAM)(INT) 1 | 0, (LPARAM) client.szLocalPath);
					// ��������� ������ �������� �������� � ������
					SetCurrentDirectory(client.szLocalPath);
					client.SetControls(hEditAdress, hEditUserName, hEditUserPassword, 
						hButtonConnect, hButtonDownload, hButtonUpload,
						hLocalListView, hFtpListView, hComboBox, hStatusBar);
					client.LocalListDirectory(LocalFileList, client.szLocalPath);
					// ���������� ������ ������
					client.LocalShowDir(LocalFileList);
				}
			}     
			// ������������ ������� ������ ����������
			if((HWND)lParam == hButtonConnect)
			{
				// ��������� ������
				EnableWindow(hButtonConnect, FALSE);
				// ����������� ��� ������������� � ����������� �� ��������� ������� ����������
				if(Connected == false)
				{
					// �������� ������ ��� �������
					SendMessage(hEditAdress, WM_GETTEXT, MAX_PATH, LPARAM(szErrorBuff));
					settings.Adress = szErrorBuff;
					// �����
					SendMessage(hEditUserName, WM_GETTEXT, MAX_PATH, LPARAM(szErrorBuff));
					settings.Login = szErrorBuff;
					// ������
					SendMessage(hEditUserPassword, WM_GETTEXT, MAX_PATH, LPARAM(szErrorBuff));
					settings.Password = szErrorBuff;
					// ������� ����������
					if(!client.Connect(settings))
						// ���������� �� �������
					{
						
						MessageBox(0, client.szErrorBuff, "������ ����������", MB_OK | MB_ICONEXCLAMATION);
						
						client.Disconnect();
						
						Connected = false;
						// �������� ������
						EnableWindow(hButtonConnect, TRUE);
						return 0;
					}
					// ������ ��������� ������� ����������
					Connected = true;

					if(ListView_GetNextItem(hLocalListView, -1, LVNI_SELECTED) != -1)
						EnableWindow(hButtonUpload, TRUE);
					
					SendMessage(hEditAdress, EM_SETREADONLY, TRUE, 0);
					
					SendMessage(hEditUserName, EM_SETREADONLY, TRUE, 0);
	
					SendMessage(hEditUserPassword, EM_SETREADONLY, TRUE, 0);
					
					EnableWindow(hFtpListView, TRUE);
					
					SendMessage(hButtonConnect, WM_SETTEXT, sizeof("�������������"), LPARAM("�������������"));
					FtpFileList.clear();
					DWORD code = client.FtpListDirectory(FtpFileList, client.szFtpPath);
					if(code != 0)
					{
						client.CheckLastError(code, szErrorBuff);
						// ������� ������
						MessageBox(0, szErrorBuff, "������ ��������� ������ ����������", MB_OK | MB_ICONEXCLAMATION);
						// �����������
						client.Disconnect();
						Connected = false;
					}
					client.FtpShowDir(FtpFileList);									
				}
				else
				{
					
					client.Disconnect();
				
					Connected = false;
				}
				EnableWindow(hButtonConnect, TRUE);
			}
			if((HWND)lParam == hButtonDownload)
			{
				// ��������� ������ ��������
				EnableWindow(hButtonDownload, FALSE);
				// ������� ������� �������������
				InitEvent = CreateEvent(0, TRUE, FALSE, 0);
				// ������� �����
				thID = CreateThread(0, 0, Download, 0, 0, 0);
				
				DialogBox(0, MAKEINTRESOURCE(IDD_DIALOGBAR), hWndMain, ProgressDlgProc);
				
				WaitForSingleObject(thID, INFINITE);
				
				DWORD code = client.LocalListDirectory(LocalFileList, client.szLocalPath);
				if(code != 0)
				{
					
					client.CheckLastError(code, szErrorBuff);
					MessageBox(hWndMain, szErrorBuff, "������", MB_OK | MB_ICONEXCLAMATION);
				}
				client.LocalShowDir(LocalFileList);
				EnableWindow(hButtonDownload, TRUE);
			}
			if((HWND)lParam == hButtonUpload)
			{
				
				EnableWindow(hButtonDownload, FALSE);
				
				InitEvent = CreateEvent(0, TRUE, FALSE, 0);
				
				thID = CreateThread(0, 0, Upload, 0, 0, 0);
				
				DialogBox(0, MAKEINTRESOURCE(IDD_DIALOGBAR), hWndMain, ProgressDlgProc);
				
				WaitForSingleObject(thID, INFINITE);
				DWORD code;
				
				FtpFileList.clear();
				code = client.FtpListDirectory(FtpFileList, client.szFtpPath);
				if(code != 0)
				{
					
					client.CheckLastError(code, szErrorBuff);
					
					MessageBox(0, szErrorBuff, "������ ��������� ������ ����������", MB_OK | MB_ICONEXCLAMATION);
					
					client.Disconnect();
					
					Connected = false;
					return 0;
				}
				
				client.FtpShowDir(FtpFileList);
				
				EnableWindow(hButtonUpload, TRUE);
			}
			if((HWND)lParam == hButtonSettings)
			{
				DialogBox(0, MAKEINTRESOURCE(IDD_DIALOGBAR_SETTINGS), hWndMain, SettingsDlgProc);
			}
			if((HWND)lParam == hButtonHelp)
			{
				MessageBox(
					hWnd, 
					"��������� ���������� �� ������ � �����������:\n\n��� ������ ������ ������������� ������ �� ������� ������� ������� ������ ������������.\n����� ���������� ������ �� ������ �������, �����,������ � ������ ������ '�������������'.\n����� �������� ���� �� ������ ���������� ������� ����������� � ������� �� ������ �������� ���� �� ������.\n����� ���� ��� ������� ������ ����, �������� ������ '<-' � � ���������� ���� ����� ���������� �� ������.\n��� ���� ����� ������� � ������� ���������� ������� ����  � ������ ������ '->'.\n�� ������� '���������' ����� ������ ���� � ������� �������� ����� ���������� ����. ", 
					"�������", 
					MB_OK | MB_ICONASTERISK
					);
				//system("start C:\\manual.html");
			}
		}
		return 0;
		case WM_NOTIFY:
            {
				// ��������� �� ������ ��������� ������
				if((((LPNMHDR)lParam)->hwndFrom) == hLocalListView)
				{
					switch (((LPNMHDR)lParam)->code)
                    {
						case LVN_ITEMCHANGED:
						{
							if(SendMessage(hLocalListView, LVM_GETNEXTITEM, -1, LVNI_SELECTED) != -1 && Connected)
								EnableWindow(hButtonUpload, TRUE);
							else
								EnableWindow(hButtonUpload, FALSE);
						}
						break;
                        case NM_DBLCLK:
                        {
							
							int ItemIndex = SendMessage(hLocalListView, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
							
							if(ItemIndex == -1)
								return 0;
							
							if(!(LocalFileList.at(ItemIndex).dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
								return 0;
							
							DWORD code = client.LocalChangeDir(LocalFileList.at(ItemIndex).cFileName);
							
							if(code != 0)
							{
								
								client.CheckLastError(code, szErrorBuff);
								
								MessageBox(hWndMain, szErrorBuff, "������", MB_OK | MB_ICONEXCLAMATION);
							}
							code = client.LocalListDirectory(LocalFileList, client.szLocalPath);
							if(code != 0)
							{
								
								client.CheckLastError(code, szErrorBuff);
								
								MessageBox(hWndMain, szErrorBuff, "������", MB_OK | MB_ICONEXCLAMATION);
							}
							
							client.LocalShowDir(LocalFileList);
							
							ListView_SetItemState(hLocalListView, 0, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);
							
							ListView_SetItemState(hLocalListView, 0, LVIS_FOCUSED & LVIS_SELECTED, 0x000F);							
                        }
						break;
					default:
						break;
                    }
				}
				// ��������� �� ������ ��� ������
				if((((LPNMHDR)lParam)->hwndFrom) == hFtpListView)
                {
                    switch (((LPNMHDR)lParam)->code)
                    {
					case LVN_ITEMCHANGED:
						{
							if(SendMessage(hFtpListView, LVM_GETNEXTITEM, -1, LVNI_SELECTED) != -1)
								EnableWindow(hButtonDownload, TRUE);
							else
								EnableWindow(hButtonDownload, FALSE);
						}
						break;		
                        case NM_DBLCLK:
                        {
							
							int ItemIndex = SendMessage(hFtpListView, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
							
							if(ItemIndex == -1)
								return 0;
							
							if(!(FtpFileList.at(ItemIndex).dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
								return 0;			
							
							DWORD code = client.FtpChangeDir(FtpFileList.at(ItemIndex).cFileName);
							
							if(code != 0)
							{
								
								client.CheckLastError(code, szErrorBuff);
								
								MessageBox(hWndMain, szErrorBuff, "������", MB_OK | MB_ICONEXCLAMATION);
								
								if(code == ERROR_INTERNET_CONNECTION_ABORTED)
								
								client.Disconnect();
							}
							code = client.FtpListDirectory(FtpFileList, client.szFtpPath);
							if(code != 0)
							{
								
								client.CheckLastError(code, szErrorBuff);
								
								MessageBox(0, szErrorBuff, "������ ��������� ������ ����������", MB_OK | MB_ICONEXCLAMATION);
								
								client.Disconnect();
								
								Connected = false;
							}
							
							client.FtpShowDir(FtpFileList);
							ListView_SetItemState(hFtpListView, 0, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);
							
							ListView_SetItemState(hFtpListView, 0, LVIS_FOCUSED & LVIS_SELECTED, 0x000F);					
                        }
						break;
					default:
						break;
                    }
                }
            }
			return 0;
	case WM_DESTROY:
		client.Disconnect();
		InternetCloseHandle(hIConnect);
		InternetCloseHandle(hInternet);		
		PostQuitMessage(0);
		return  0;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
// ������ ��������� ����������� ���� ��������
BOOL CALLBACK ProgressDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg) 
	{
		case WM_COMMAND:
		{
			if((HWND)lParam == GetDlgItem(hWnd, IDC_BUTTON1))
			{
				
				SetEvent(client.TransitEvent);
				
				EndDialog(hWnd, 0);
			}
		}
		case WM_INITDIALOG:
		{
			INITCOMMONCONTROLSEX InitCtrlEx;
			InitCtrlEx.dwSize = sizeof(INITCOMMONCONTROLSEX);
			InitCtrlEx.dwICC  = ICC_PROGRESS_CLASS;
			InitCommonControlsEx(&InitCtrlEx);	
			hProgBar = GetDlgItem(hWnd, IDC_PROGRESS1);
			hStatic = GetDlgItem(hWnd, IDC_STATIC_133);
			hDlg = hWnd;
			// ������ ������� ������������� ��������
			SetEvent(InitEvent);
			return TRUE;
		}
		case WM_CLOSE:
			
			SetEvent(client.TransitEvent);
			// ���������� ������
			EndDialog(hWnd, 0);
			break;
		default:
			return FALSE;
	}	
	return FALSE;
}
// ������ ��������� ����������� ���� ��������
BOOL CALLBACK SettingsDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	char buff[MAX_PATH] = {0};
	// ������������ ���������
	switch(uMsg) 
	{
		case WM_COMMAND:
		{
			// ���������� ��������
			if((HWND)lParam == GetDlgItem(hWnd, IDC_BTN_SAVE))
			{
				// ����
				SendMessage(GetDlgItem(hWnd, IDC_EDIT_DEFAULT_PORT), WM_GETTEXT, MAX_PATH, LPARAM(buff));
				settings.connectionPort = atoi(buff);
				memset(buff, 0, sizeof(char) * MAX_PATH);
				// �����
				SendMessage(GetDlgItem(hWnd, IDC_EDIT_DEFAULT_URL), WM_GETTEXT, MAX_PATH, LPARAM(settings.Adress.c_str()));
				// �����
				SendMessage(GetDlgItem(hWnd, IDC_EDIT_DEFAULT_LOGIN), WM_GETTEXT, MAX_PATH, LPARAM(settings.Login.c_str()));
				// ������
				SendMessage(GetDlgItem(hWnd, IDC_EDIT_DEFAULT_PASS), WM_GETTEXT, MAX_PATH, LPARAM(settings.Password.c_str()));
				// ����
				SendMessage(GetDlgItem(hWnd, IDC_EDIT_USER_KEY), WM_GETTEXT, MAX_PATH, LPARAM(settings.Key.c_str()));
				// ��������� ���������
				if(!settings.Save())
				{
					settings.Default();
					settings.Save();
				}
				// ���������� ������
				SendMessage(hWnd, WM_CLOSE, 0, 0);
			}
			// ������
			if((HWND)lParam == GetDlgItem(hWnd, IDC_BTN_CANCEL))
			{
				// ���������� ������
				SendMessage(hWnd, WM_CLOSE, 0, 0);
			}
		}
		break;
		case WM_INITDIALOG:
			{
				if(!settings.Load())
				{
					MessageBox(hWnd, "���� �������� �� ������!\r\n������� ���� �������� �� ���������...", "������ ������ ��������", MB_OK | MB_ICONEXCLAMATION);
					settings.Default();
					settings.Save();
				}		
				// ���������� ��������� � ����
				sprintf(buff, "%d", settings.connectionPort);
				SendMessage(GetDlgItem(hWnd, IDC_EDIT_DEFAULT_PORT), WM_SETTEXT, MAX_PATH, LPARAM(buff));
				// �����
				SendMessage(GetDlgItem(hWnd, IDC_EDIT_DEFAULT_URL), WM_SETTEXT, MAX_PATH, LPARAM(settings.Adress.c_str()));
				// �����
				SendMessage(GetDlgItem(hWnd, IDC_EDIT_DEFAULT_LOGIN), WM_SETTEXT, MAX_PATH, LPARAM(settings.Login.c_str()));
				// ������
				SendMessage(GetDlgItem(hWnd, IDC_EDIT_DEFAULT_PASS), WM_SETTEXT, MAX_PATH, LPARAM(settings.Password.c_str()));
				// ����
				SendMessage(GetDlgItem(hWnd, IDC_EDIT_USER_KEY), WM_SETTEXT, MAX_PATH, LPARAM(settings.Key.c_str()));
			}
			return TRUE;
		case WM_CLOSE:
			{
				
				SendMessage(hEditAdress, WM_SETTEXT, MAX_PATH, LPARAM(settings.Adress.c_str()));
				
				SendMessage(hEditUserName, WM_SETTEXT, MAX_PATH, LPARAM(settings.Login.c_str()));
				
				SendMessage(hEditUserPassword, WM_SETTEXT, MAX_PATH, LPARAM(settings.Password.c_str()));
				if(settings.Save())
					client.settings.Load();
				
				EndDialog(hWnd, 0);
			}
			break;
		default:
			return FALSE;
	}	
	return FALSE;
}
// ������� �������
struct Task
{
	TCHAR from[MAX_PATH];
	TCHAR to[MAX_PATH];
	WIN32_FIND_DATA file;
};
// ������� ��������� ������ � �������
DWORD WINAPI Download(LPVOID lpParam)
{
	TCHAR szMsgBuff[MAX_PATH] = {0};
	TCHAR from[MAX_PATH] = {0};
	TCHAR to[MAX_PATH] = {0};
	// ����: ���������/���������
	TCHAR szFrom[MAX_PATH] = {0};
	TCHAR szTo[MAX_PATH] = {0};
	DWORD code = 0;
	// �������
	queue<Task> todo;
	vector<WIN32_FIND_DATA> list;
	// ������� ������� "���������"
	client.TransitEvent = CreateEvent(0, TRUE, FALSE, 0);
	// ������� ������� �������������
	while(WaitForSingleObject(InitEvent, 100) != WAIT_OBJECT_0);
	
	CloseHandle(InitEvent);	
	
	int ItemIndex = SendMessage(hFtpListView, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
	
	SendMessage(hStatusBar, SB_GETTEXT, (WPARAM)(INT) 0 | 0, (LPARAM)client.szFtpPath);
	
	SendMessage(hStatusBar, SB_GETTEXT, (WPARAM)(INT) 1 | 0, (LPARAM)client.szLocalPath);
	// ���� ���� ���������� ��������
	while(ItemIndex != -1 && code == 0)
	{
		Task task;
		
		sprintf(task.from, "%s", client.szFtpPath);
		sprintf(task.to, "%s", client.szLocalPath);
		// �������� �������� ���������
		memcpy(&task.file, &FtpFileList.at(ItemIndex), sizeof(FtpFileList.at(ItemIndex)));
		if(strcmp(task.file.cFileName, "..") != 0)
			
			todo.push(task);
		
		ItemIndex = ListView_GetNextItem(hFtpListView, ItemIndex, LVNI_SELECTED); 
	}					
	while(todo.size() && WaitForSingleObject(client.TransitEvent, 0) != WAIT_OBJECT_0)
	{
		
		Task curTask = todo.front();
		
		todo.pop();
		
		if(curTask.file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			sprintf(szFrom, "%s/%s/", curTask.from, curTask.file.cFileName);
			sprintf(szTo, "%s\\%s\\", curTask.to, curTask.file.cFileName);
			////// ������������ ����� //////
			int len = strlen(szFrom);
			string temp = "";
			for(int j = 0; j < len; j++)
			{
				if(temp.size() != 0)
				{
					if(szFrom[j] == '/' && temp.at(temp.size() - 1) == '/')
						continue;
					else
						temp += szFrom[j];
				}
				else
				{
					temp += szFrom[j];
				}
			}
			sprintf(szFrom, "%s", temp.c_str());
			len = strlen(szTo);
			temp.clear();
			for(int j = 0; j < len; j++)
			{
				if(temp.size() != 0)
				{
					if(szTo[j] == '\\' && temp.at(temp.size() - 1) == '\\')
						continue;
					else
						temp += szTo[j];
				}
				else
				{
					temp += szTo[j];
				}
			}
			sprintf(szTo, "%s", temp.c_str());
			////// ������������ ����� //////
			
			if(!CreateDirectory(szTo, 0) && (code = GetLastError()) != ERROR_ALREADY_EXISTS)
			{
				
				client.CheckLastError(code, szMsgBuff);
				
				MessageBox(0, szMsgBuff, "������ �������� ���������� ��������.", MB_OK | MB_ICONSTOP);
				break;
			}

			
			code = client.FtpListDirectory(list, szFrom);
			if(code != 0)
			{
				
				client.CheckLastError(code, szMsgBuff);
				
				MessageBox(0, szMsgBuff, "������.", MB_OK | MB_ICONSTOP);
				break;
			}
			
			for(size_t i = 0; i < list.size(); i++)
			{
				Task cur;
				sprintf(cur.from, "%s", szFrom);
				sprintf(cur.to, "%s", szTo);
				memcpy(&cur.file, &list.at(i), sizeof(list.at(i)) );
				if(strcmp(cur.file.cFileName, "..") != 0)
					todo.push(cur);
			}
		}
		// ���� ����
		else
		{
			sprintf(szFrom, "%s/%s", curTask.from, curTask.file.cFileName);
			sprintf(szTo, "%s\\%s", curTask.to, curTask.file.cFileName);
			////// ������������ ����� //////
			int len = strlen(szFrom);
			string temp = "";
			for(int j = 0; j < len; j++)
			{
				if(temp.size() != 0)
				{
					if(szFrom[j] == '/' && temp.at(temp.size() - 1) == '/')
						continue;
					else
						temp += szFrom[j];
				}
				else
				{
					temp += szFrom[j];
				}
			}
			sprintf(szFrom, "%s", temp.c_str());
			len = strlen(szTo);
			temp.clear();
			for(int j = 0; j < len; j++)
			{
				if(temp.size() != 0)
				{
					if(szTo[j] == '\\' && temp.at(temp.size() - 1) == '\\')
						continue;
					else
						temp += szTo[j];
				}
				else
				{
					temp += szTo[j];
				}
			}
			sprintf(szTo, "%s", temp.c_str());
			// ��������� ����
			code = client.GetFile(szFrom, szTo, hProgBar, hStatic);
			if(code != 0)
			{
				client.CheckLastError(code, szErrorBuff);
				MessageBox(0, szErrorBuff, "Error", MB_OK | MB_ICONSTOP);
				break;
			}
		}
	}
	// ������� �� ������ ���������
	SetEvent(client.TransitEvent);
	CloseHandle(client.TransitEvent);
	// ��������� ������
	EndDialog(hDlg, 0);
	return 0;
}
// ������� �������� ������ �� ������
DWORD WINAPI Upload(LPVOID lpParam)
{
	TCHAR szMsgBuff[MAX_PATH] = {0};
	TCHAR from[MAX_PATH] = {0};
	TCHAR to[MAX_PATH] = {0};
	
	TCHAR szFrom[MAX_PATH] = {0};
	TCHAR szTo[MAX_PATH] = {0};
	DWORD code = 0;
	// �������
	queue<Task> todo;
	vector<WIN32_FIND_DATA> list;
	
	client.TransitEvent = CreateEvent(0, TRUE, FALSE, 0);
	
	while(WaitForSingleObject(InitEvent, 100) != WAIT_OBJECT_0);
	// ��������� �������
	CloseHandle(InitEvent);
	// �������� ������ �����
	int ItemIndex = SendMessage(hLocalListView, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
	
	SendMessage(hStatusBar, SB_GETTEXT, (WPARAM)(INT) 0 | 0, (LPARAM)client.szFtpPath);
	
	SendMessage(hStatusBar, SB_GETTEXT, (WPARAM)(INT) 1 | 0, (LPARAM)client.szLocalPath);
	
	while(ItemIndex != -1 && code == 0 && WaitForSingleObject(client.TransitEvent, 0) != WAIT_OBJECT_0)
	{
		Task task;
		
		sprintf(task.from, "%s", client.szLocalPath);
		sprintf(task.to, "%s", client.szFtpPath);
		
		memcpy(&task.file, &LocalFileList.at(ItemIndex), sizeof(LocalFileList.at(ItemIndex)));
		if(strcmp(task.file.cFileName, "..") != 0)
			
			todo.push(task);
		
		ItemIndex = ListView_GetNextItem(hLocalListView, ItemIndex, LVNI_SELECTED); 
	}
	while(todo.size() && WaitForSingleObject(client.TransitEvent, 0) != WAIT_OBJECT_0)
	{
		
		Task curTask = todo.front();
		
		todo.pop();
		
		if(curTask.file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			sprintf(szFrom, "%s\\%s\\", curTask.from, curTask.file.cFileName);
			sprintf(szTo, "%s/%s/", curTask.to, curTask.file.cFileName);
			////// ������������ ����� //////
			int len = strlen(szFrom);
			string temp = "";
			for(int j = 0; j < len; j++)
			{
				if(temp.size() != 0)
				{
					if(szFrom[j] == '\\' && temp.at(temp.size() - 1) == '\\')
						continue;
					else
						temp += szFrom[j];
				}
				else
				{
					temp += szFrom[j];
				}
			}
			sprintf(szFrom, "%s", temp.c_str());
			len = strlen(szTo);
			temp.clear();
			for(int j = 0; j < len; j++)
			{
				if(temp.size() != 0)
				{
					if(szTo[j] == '/' && temp.at(temp.size() - 1) == '/')
						continue;
					else
						temp += szTo[j];
				}
				else
				{
					temp += szTo[j];
				}
			}
			sprintf(szTo, "%s", temp.c_str());
			////// ������������ ����� //////
			
			if(!FtpCreateDirectory(hIConnect, szTo))
			{
				
				client.CheckLastError(GetLastError(), szMsgBuff);
				
				MessageBox(0, szMsgBuff, "������ �������� ���������� ��������.", MB_OK | MB_ICONSTOP);
				break;
			}
			
			code = client.LocalListDirectory(list, szFrom);
			if(code != 0)
			{
				
				client.CheckLastError(code, szMsgBuff);
				
				MessageBox(0, szMsgBuff, "������.", MB_OK | MB_ICONSTOP);
				break;
			}
			// ��������� ������ � �������
			for(size_t i = 0; i < list.size(); i++)
			{
				Task cur;
				sprintf(cur.from, "%s", szFrom);
				sprintf(cur.to, "%s", szTo);
				memcpy(&cur.file, &list.at(i), sizeof(list.at(i)) );
				if(strcmp(cur.file.cFileName, "..") != 0)
					todo.push(cur);
			}
		}
		// ���� ����
		else
		{
			sprintf(szFrom, "%s\\%s", curTask.from, curTask.file.cFileName);
			sprintf(szTo, "%s/%s", curTask.to, curTask.file.cFileName);
			////// ������������ ����� //////
			int len = strlen(szFrom);
			string temp = "";
			for(int j = 0; j < len; j++)
			{
				if(temp.size() != 0)
				{
					if(szFrom[j] == '\\' && temp.at(temp.size() - 1) == '\\')
						continue;
					else
						temp += szFrom[j];
				}
				else
				{
					temp += szFrom[j];
				}
			}
			sprintf(szFrom, "%s", temp.c_str());
			len = strlen(szTo);
			temp.clear();
			for(int j = 0; j < len; j++)
			{
				if(temp.size() != 0)
				{
					if(szTo[j] == '/' && temp.at(temp.size() - 1) == '/')
						continue;
					else
						temp += szTo[j];
				}
				else
				{
					temp += szTo[j];
				}
			}
			sprintf(szTo, "%s", temp.c_str());
			// ��������� ����
			code = client.PutFile(szTo, szFrom, hProgBar, hStatic);
			if(code != 0)
			{
				client.CheckLastError(code, szErrorBuff);
				MessageBox(0, szErrorBuff, "Error", MB_OK | MB_ICONSTOP);
				break;
			}
		}
	}
	// ������� �� ������ ���������
	SetEvent(client.TransitEvent);
	CloseHandle(client.TransitEvent);
	// ��������� ������
	EndDialog(hDlg, 0);
	return 0;
}