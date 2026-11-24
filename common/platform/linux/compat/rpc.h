#pragma once
#include "objbase.h"
using UUID = GUID;
using RPC_STATUS = LONG;
#ifndef RPC_S_OK
#define RPC_S_OK 0L
#endif
inline RPC_STATUS UuidCreate(UUID* u) noexcept { return SUCCEEDED(CoCreateGuid(u)) ? RPC_S_OK : 1; }
