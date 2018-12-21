#pragma once

#include "Settings.h"

// �����������
Settings::Settings()
{
	// ������������ ����
	connectionPort = 21;
	Adress.reserve(MAX_PATH);
	Login.reserve(MAX_PATH);
	Password.reserve(MAX_PATH);
	Key.reserve(MAX_PATH);
	// ����� �������
	Adress = "";
	// �����
	Login = "";
	// ������
	Password = "";
	// ����
	Key = "";
	// ���� � ����� ��������
	cmdLine = new char[MAX_PATH];
	// �������� ����
	GetModuleFileName(GetModuleHandle(NULL), cmdLine, MAX_PATH);
	// �������� ������ �������
    *strrchr(cmdLine, '\\') = NULL;
	// ��������� ������ ���� � ����� ��������
	strcat(cmdLine, "\\client.ini");
}
// ����������� �����
Settings::Settings(const Settings &href)
{
	// ������������ ����
	connectionPort = href.connectionPort;
	// ����� �������
	Adress.resize(href.Adress.size());
	Adress.assign(href.Adress);
	// �����
	Login.resize(href.Login.size());
	Login.assign(href.Login);
	// ������
	Password.resize(href.Password.size());
	Password.assign(href.Password);
	// ����
	Key.resize(href.Key.size());
	Key.assign(href.Key);
	// ���� � ����� ��������
	cmdLine = new char[MAX_PATH];
	// �������� ����
	GetModuleFileName(GetModuleHandle(NULL), cmdLine, MAX_PATH);
	// �������� ������ �������
    *strrchr(cmdLine, '\\') = NULL;
	// ��������� ������ ���� � ����� ��������
	strcat(cmdLine, "\\client.ini");
}

Settings::~Settings()
{
	delete []cmdLine;
}

Settings &Settings::operator=(const Settings &href)
{
	if(this == &href)
		return *this;
	
	// ������������ ����
	connectionPort = href.connectionPort;
	// ����� �������
	Adress.resize(href.Adress.size());
	Adress.assign(href.Adress);
	// �����
	Login.resize(href.Login.size());
	Login.assign(href.Login);
	// ������
	Password.resize(href.Password.size());
	Password.assign(href.Password);
	// ����
	Key.resize(href.Key.size());
	Key.assign(href.Key);

	delete cmdLine;
	cmdLine = new char[strlen(href.cmdLine) + 1];
	strcpy(cmdLine, href.cmdLine);
	return *this;
}
// �������� �������� �� �����
BOOL Settings::Load()
{
	char buff[MAX_PATH] = {0};
	memset(buff, 0, sizeof(char) * MAX_PATH);
	// ������ ������������ ����
	GetPrivateProfileString("client settings", "connection_port", "fail", buff, MAX_PATH, cmdLine);
	if(strcmp(buff, "fail") == 0)
		return FALSE;
	// �������� ���� ��� ���������� � ��������
	connectionPort = atoi(buff);
	if(connectionPort == 0)
		return FALSE;

	memset(buff, 0, sizeof(char) * MAX_PATH);
	// ������ ����� �������
	GetPrivateProfileString("client settings", "server_adress", "fail", buff, MAX_PATH, cmdLine);
	if(strcmp(buff, "fail") == 0)
		return FALSE;
	Adress.resize(strlen(buff));
	Adress.assign(buff);

	memset(buff, 0, sizeof(char) * MAX_PATH);
	// ������ ��� ������������
	GetPrivateProfileString("client settings", "user_login", "fail", buff, MAX_PATH, cmdLine);
	if(strcmp(buff, "fail") == 0)
		return FALSE;
	Login.resize(strlen(buff));
	Login.assign(buff);

	memset(buff, 0, sizeof(char) * MAX_PATH);
	// ������ ������ ������������
	GetPrivateProfileString("client settings", "user_password", "fail", buff, MAX_PATH, cmdLine);
	if(strcmp(buff, "fail") == 0)
		return FALSE;
	Password.resize(strlen(buff));
	Password.assign(buff);

	memset(buff, 0, sizeof(char) * MAX_PATH);
	// ������ ����
	GetPrivateProfileString("client settings", "user_key", "fail", buff, MAX_PATH, cmdLine);
	if(strcmp(buff, "fail") == 0)
		return FALSE;
	Key.resize(strlen(buff));
	Key.assign(buff);
	return TRUE;
}
// ���������� �������� � ����
BOOL Settings::Save()
{
	char buff[MAX_PATH] = {0};
	sprintf(buff, "%d", connectionPort);
	// ���������� ����
	if(!WritePrivateProfileString("client settings", "connection_port", buff, cmdLine))
		return FALSE;
	// ���������� �����
	if(!WritePrivateProfileString("client settings", "server_adress", Adress.c_str(), cmdLine))
		return FALSE;
	// ���������� �����
	if(!WritePrivateProfileString("client settings", "user_login", Login.c_str(), cmdLine))
		return FALSE;
	// ���������� ������
	if(!WritePrivateProfileString("client settings", "user_password", Password.c_str(), cmdLine))
		return FALSE;
	// ���������� ����
	if(!WritePrivateProfileString("client settings", "user_key", Key.c_str(), cmdLine))
		return FALSE;
	// ��� ������ �������
	return TRUE;
}
// ���������� �������� �� ���������
void Settings::Default()
{
	// ������������ ����
	connectionPort = 21;
	// ����� �������
	Adress.assign("localhost");
	// �����
	Login.assign("anonymous");
	// ������
	Password.assign("anonymous@mail.com");
	// ����
	Key.assign("Secret key");
}