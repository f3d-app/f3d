#include "interactor.h"

namespace f3d
{
//----------------------------------------------------------------------------
bool interaction_bind_t::operator<(const interaction_bind_t& bind) const
{
  return this->mod < bind.mod || (this->mod == bind.mod && this->inter < bind.inter);
}

//----------------------------------------------------------------------------
bool interaction_bind_t::operator==(const interaction_bind_t& bind) const
{
  return this->mod == bind.mod && this->inter == bind.inter;
}

//----------------------------------------------------------------------------
std::string interaction_bind_t::format() const
{
  switch (this->mod)
  {
    case ModifierKeys::CTRL_SHIFT:
      return "Ctrl+Shift+" + this->inter;
    case ModifierKeys::CTRL:
      return "Ctrl+" + this->inter;
    case ModifierKeys::SHIFT:
      return "Shift+" + this->inter;
    case ModifierKeys::ANY:
      return "Any+" + this->inter;
    default:
      // No need to check for NONE
      return this->inter;
  }
}

//----------------------------------------------------------------------------
interaction_bind_t interaction_bind_t::parse(const std::string& str)
{
  interaction_bind_t bind;
  auto plusIt = str.find_last_of('+');
  if (plusIt == std::string::npos)
  {
    bind.inter = str;
  }
  else
  {
    bind.inter = str.substr(plusIt + 1);

    std::string modStr = str.substr(0, plusIt);
    if (modStr == "Ctrl+Shift")
    {
      bind.mod = ModifierKeys::CTRL_SHIFT;
    }
    else if (modStr == "Shift")
    {
      bind.mod = ModifierKeys::SHIFT;
    }
    else if (modStr == "Ctrl")
    {
      bind.mod = ModifierKeys::CTRL;
    }
    else if (modStr == "Any")
    {
      bind.mod = ModifierKeys::ANY;
    }
    else if (modStr == "None")
    {
      bind.mod = ModifierKeys::NONE;
    }
    else
    {
      f3d::log::warn("Invalid modifier: ", modStr, ", ignoring modifier");
    }
  }
  return bind;
}

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
