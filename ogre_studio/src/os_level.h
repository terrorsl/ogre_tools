#ifndef OS_LEVEL_FILE
#define OS_LEVEL_FILE

#include<string>

class OgreStudioLevel
{
public:
	OgreStudioLevel(std::string& name);
	bool Load(std::string &fname);
	bool Save();
private:
	std::string filename;
};
#endif