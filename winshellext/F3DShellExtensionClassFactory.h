#pragma once

#include "F3DThumbnailProvider.h"

#include <new>

#include <shlwapi.h>
#include <unknwn.h>
#include <windows.h>

extern long g_cDllRef;

// This class is a basic implementation of a COM ClassFactory.
// It just create the F3DThumbnailProvider objects
class F3DShellExtensionClassFactory : public IClassFactory
{
public:
  F3DShellExtensionClassFactory()
    : m_cRef(1)
  {
    ::InterlockedIncrement(&g_cDllRef);
  }

  // IUnknown
  IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv)
  {
    static const QITAB qit[] = {
      QITABENT(F3DShellExtensionClassFactory, IClassFactory),
      { 0 },
    };
    return QISearch(this, qit, riid, ppv);
  }

  IFACEMETHODIMP_(ULONG) AddRef() { return ::InterlockedIncrement(&m_cRef); }

  IFACEMETHODIMP_(ULONG) Release()
  {
    ULONG cRef = ::InterlockedDecrement(&m_cRef);
    if (cRef == 0)
    {
      delete this;
    }
    return cRef;
  }

  // IClassFactory
  IFACEMETHODIMP CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppv)
  {
    if (pUnkOuter != nullptr)
    {
      return CLASS_E_NOAGGREGATION;
    }

    // Create the COM component.
    HRESULT hr = E_OUTOFMEMORY;
    F3DThumbnailProvider* pExt = new (std::nothrow) F3DThumbnailProvider();
    if (pExt)
    {
      hr = pExt->QueryInterface(riid, ppv);
      pExt->Release();
    }

    return hr;
  }

  IFACEMETHODIMP LockServer(BOOL fLock)
  {
    if (fLock)
    {
      ::InterlockedIncrement(&g_cDllRef);
    }
    else
    {
      ::InterlockedDecrement(&g_cDllRef);
    }
    return S_OK;
  }

protected:
  ~F3DShellExtensionClassFactory() { ::InterlockedDecrement(&g_cDllRef); }

  long m_cRef;
};
