#pragma once

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

#include <Thor/Input.hpp>
#include <Thor/Math.hpp>
#include <Thor/Vectors.hpp>

#include <TGUI/TGUI.hpp>

#include "Raknet/WindowsIncludes.h"
#include "Raknet/RakPeerInterface.h"
#include "Raknet/BitStream.h"
#include "Raknet/RakWString.h"
#include "Raknet/MessageIdentifiers.h"
#include "Raknet/PacketLogger.h"
#include "Raknet/GetTime.h"


#pragma warning(push)
#pragma warning(disable : 4820)
#pragma warning(disable : 4619)
#pragma warning(disable : 4548)
#pragma warning(disable : 4668)
#pragma warning(disable : 4365)
#pragma warning(disable : 4710)
#pragma warning(disable : 4371)
#pragma warning(disable : 4826)
#pragma warning(disable : 4061)
#pragma warning(disable : 4640)
#include <string>
#include <vector>
#include <deque>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <climits>
#include <optional>
#include <filesystem>
#pragma warning(pop)