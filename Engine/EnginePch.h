#pragma once

// WinApi 프레임워크
#include <Windows.h>

// WinApi img32 라이브러리
#pragma comment(lib, "MsImg32.lib")

// FMOD 사운드
#include "FMOD/inc/fmod.hpp"
#pragma comment(lib, "FMOD/lib/fmod_vc.lib")

// JSON for Modern C++
#include "JSON/inc/json.hpp"

using namespace FMOD;

// C++ 표준 라이브러리
#include <cassert>
#include <chrono>
#include <string>
#include <array>
#include <map>
#include <list>
#include <queue>
#include <unordered_map>

using namespace std;

// Util
#include "Util/SingleTon.h"
#include "Util/Struct.h"
#include "Util/Logger.h"
#include "Util/StringHelper.h"
#include "Core/Component.h"