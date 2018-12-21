#pragma once

#include "Settings.h"

// Конструктор
Settings::Settings()
{
	// Используемый порт
	connectionPort = 21;
	Adress.reserve(MAX_PATH);
	Login.reserve(MAX_PATH);
	Password.reserve(MAX_PATH);
	Key.reserve(MAX_PATH);
	// Адрес сервера
	Adress = "";
	// Логин
	Login = "";
	// Пароль
	Password = "";
	// Ключ
	Key = "";
	// Путь к файлу настроек
	cmdLine = new char[MAX_PATH];
	// Получаем путь
	GetModuleFileName(GetModuleHandle(NULL), cmdLine, MAX_PATH);
	// Отсекаем лишние символы
    *strrchr(cmdLine, '\\') = NULL;
	// Формируем строку пути к файлу настроек
	strcat(cmdLine, "\\client.ini");
}
// Конструктор копии
Settings::Settings(const Settings &href)
{
	// Используемый порт
	connectionPort = href.connectionPort;
	// Адрес сервера
	Adress.resize(href.Adress.size());
	Adress.assign(href.Adress);
	// Логин
	Login.resize(href.Login.size());
	Login.assign(href.Login);
	// Пароль
	Password.resize(href.Password.size());
	Password.assign(href.Password);
	// Ключ
	Key.resize(href.Key.size());
	Key.assign(href.Key);
	// Путь к файлу настроек
	cmdLine = new char[MAX_PATH];
	// Получаем путь
	GetModuleFileName(GetModuleHandle(NULL), cmdLine, MAX_PATH);
	// Отсекаем лишние символы
    *strrchr(cmdLine, '\\') = NULL;
	// Формируем строку пути к файлу настроек
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
	
	// Используемый порт
	connectionPort = href.connectionPort;
	// Адрес сервера
	Adress.resize(href.Adress.size());
	Adress.assign(href.Adress);
	// Логин
	Login.resize(href.Login.size());
	Login.assign(href.Login);
	// Пароль
	Password.resize(href.Password.size());
	Password.assign(href.Password);
	// Ключ
	Key.resize(href.Key.size());
	Key.assign(href.Key);

	delete cmdLine;
	cmdLine = new char[strlen(href.cmdLine) + 1];
	strcpy(cmdLine, href.cmdLine);
	return *this;
}
// Загрузка настроек из файла
BOOL Settings::Load()
{
	char buff[MAX_PATH] = {0};
	memset(buff, 0, sizeof(char) * MAX_PATH);
	// Читаем используемый порт
	GetPrivateProfileString("client settings", "connection_port", "fail", buff, MAX_PATH, cmdLine);
	if(strcmp(buff, "fail") == 0)
		return FALSE;
	// Получаем порт для соединения с сервером
	connectionPort = atoi(buff);
	if(connectionPort == 0)
		return FALSE;

	memset(buff, 0, sizeof(char) * MAX_PATH);
	// Читаем адрес сервера
	GetPrivateProfileString("client settings", "server_adress", "fail", buff, MAX_PATH, cmdLine);
	if(strcmp(buff, "fail") == 0)
		return FALSE;
	Adress.resize(strlen(buff));
	Adress.assign(buff);

	memset(buff, 0, sizeof(char) * MAX_PATH);
	// Читаем имя пользователя
	GetPrivateProfileString("client settings", "user_login", "fail", buff, MAX_PATH, cmdLine);
	if(strcmp(buff, "fail") == 0)
		return FALSE;
	Login.resize(strlen(buff));
	Login.assign(buff);

	memset(buff, 0, sizeof(char) * MAX_PATH);
	// Читаем пароль пользователя
	GetPrivateProfileString("client settings", "user_password", "fail", buff, MAX_PATH, cmdLine);
	if(strcmp(buff, "fail") == 0)
		return FALSE;
	Password.resize(strlen(buff));
	Password.assign(buff);

	memset(buff, 0, sizeof(char) * MAX_PATH);
	// Читаем ключ
	GetPrivateProfileString("client settings", "user_key", "fail", buff, MAX_PATH, cmdLine);
	if(strcmp(buff, "fail") == 0)
		return FALSE;
	Key.resize(strlen(buff));
	Key.assign(buff);
	return TRUE;
}
// Сохранение настроек в файл
BOOL Settings::Save()
{
	char buff[MAX_PATH] = {0};
	sprintf(buff, "%d", connectionPort);
	// Записываем порт
	if(!WritePrivateProfileString("client settings", "connection_port", buff, cmdLine))
		return FALSE;
	// Записываем адрес
	if(!WritePrivateProfileString("client settings", "server_adress", Adress.c_str(), cmdLine))
		return FALSE;
	// Записываем логин
	if(!WritePrivateProfileString("client settings", "user_login", Login.c_str(), cmdLine))
		return FALSE;
	// Записываем пароль
	if(!WritePrivateProfileString("client settings", "user_password", Password.c_str(), cmdLine))
		return FALSE;
	// Записываем ключ
	if(!WritePrivateProfileString("client settings", "user_key", Key.c_str(), cmdLine))
		return FALSE;
	// Все прошло успешно
	return TRUE;
}
// Применение настроек по умолчанию
void Settings::Default()
{
	// Используемый порт
	connectionPort = 21;
	// Адрес сервера
	Adress.assign("localhost");
	// Логин
	Login.assign("anonymous");
	// Пароль
	Password.assign("anonymous@mail.com");
	// Ключ
	Key.assign("Secret key");
}