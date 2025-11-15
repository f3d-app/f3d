#include <iostream>
#include <string>

static bool IsASCII(const std::string& str)
{
  for (unsigned char c : str)
  {
    if (c > 127)
    {
      return false;
    }
  }
  return true;
}

int main()
{
  std::string a = "hello.glb";        // ASCII
  std::string b = "模型.glb";         // Non-ASCII
  std::string c = "café.obj";        // Non-ASCII (é)

  std::cout << "hello.glb: " << IsASCII(a) << std::endl;
  std::cout << "模型.glb: " << IsASCII(b) << std::endl;
  std::cout << "café.obj: " << IsASCII(c) << std::endl;

  return 0;
}
