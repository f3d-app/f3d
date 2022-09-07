/**
 * @class   F3DNSDelegate
 * @brief   The class that handles Apple events
 *
 * This class is only valid on macOS.
 * This class triggers the opening of a new file when the user opens it from the finder.
 */

#ifndef F3DNSDelegate_h
#define F3DNSDelegate_h

class F3DStarter;

class F3DNSDelegate
{
public:
  static void InitializeDelegate(F3DStarter* Starter);
};

#endif
