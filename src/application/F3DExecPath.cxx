#include "F3DExecPath.h"

#ifdef _WIN32
#include <Windows.h>
#endif

namespace F3DExecPath
{
  std::string getExecPath(char* argv0)
  {
	  std::string execPath = argv0;
#ifdef _WIN32
	  if (execPath.empty())
	  {
		  WCHAR winPath[MAX_PATH];
		  GetModuleFileNameW(NULL, winPath, MAX_PATH);

		  //convert from wide char to narrow char array
		  char ch[MAX_PATH];
		  WideCharToMultiByte(CP_ACP, 0, winPath, -1, ch, MAX_PATH, nullptr, nullptr);

		  //A std:string  using the char* constructor.
		  execPath = ch;
	  }
#endif
	return execPath;
  }
};

