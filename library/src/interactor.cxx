#include "interactor.h"

namespace f3d
{
//----------------------------------------------------------------------------
interactor::already_exists_exception::already_exists_exception(const std::string& what)
  : exception(what)
{
}

//----------------------------------------------------------------------------
interactor::does_not_exists_exception::does_not_exists_exception(const std::string& what)
  : exception(what)
{
}

//----------------------------------------------------------------------------
interactor::command_runtime_exception::command_runtime_exception(const std::string& what)
  : exception(what)
{
}
}
