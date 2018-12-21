#include "Headers.h"

class Settings
{
public:
	Settings();
	Settings(const Settings&);
	~Settings();
	
	BOOL Load();
	BOOL Save();
	void Default();
	
	Settings &Settings::operator=(const Settings &href);
	
	int connectionPort;
	string Adress;
	string Login;
	string Password;
	string Key;
private:
	char *cmdLine;	
};