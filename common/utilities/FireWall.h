#pragma  once

#if defined(_WIN32)
#include <windows.h>
#include <crtdbg.h>
#include "netfw.h"
#include <objbase.h>
#include <oleauto.h>
#include <stdio.h>
#else
#include <windows.h>
#include <stdio.h>
struct INetFwProfile {};
enum NET_FW_IP_PROTOCOL { NET_FW_IP_PROTOCOL_TCP = 6, NET_FW_IP_PROTOCOL_UDP = 17, NET_FW_IP_PROTOCOL_ANY = 256 };
#endif

HRESULT WindowsFirewallInitialize(OUT INetFwProfile** fwProfile);
void WindowsFirewallCleanup(IN INetFwProfile* fwProfile);
HRESULT WindowsFirewallIsOn(IN INetFwProfile* fwProfile, OUT BOOL* fwOn);
HRESULT WindowsFirewallTurnOn(IN INetFwProfile* fwProfile);
HRESULT WindowsFirewallTurnOff(IN INetFwProfile* fwProfile);
HRESULT WindowsFirewallAppIsEnabled( IN INetFwProfile* fwProfile, IN const wchar_t* fwProcessImageFileName, OUT BOOL* fwAppEnabled );
HRESULT WindowsFirewallAddApp( IN INetFwProfile* fwProfile, IN const wchar_t* fwProcessImageFileName, IN const wchar_t* fwName );
HRESULT WindowsFirewallPortIsEnabled( IN INetFwProfile* fwProfile, IN LONG portNumber, IN NET_FW_IP_PROTOCOL ipProtocol, OUT BOOL* fwPortEnabled );
HRESULT WindowsFirewallPortAdd( IN INetFwProfile* fwProfile, IN LONG portNumber, IN NET_FW_IP_PROTOCOL ipProtocol, IN const wchar_t* name );
