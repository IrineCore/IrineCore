// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#pragma warning( disable:4996 )


#define _CRT_RAND_S
#include <stdlib.h>
#define _USE_SENDCONTEXTPOOL

#include "DNLocaleHeader.h"
#include "DNDefine.h"
#include "DNHeader.h"

#include "DNPacket.h"
#include "DNProtocol.h"

#include "AssertX.h"

// boost
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/timer.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/format.hpp>

#pragma comment(lib,"Odbc32.lib")



// TODO: reference additional headers your program requires here
