//-----------------------------------------------------------------------------
// stdafx.h
// Created at 2017.08.26 14:00
// License: see LICENSE file
//
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
//-----------------------------------------------------------------------------
#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
// STL
#include <cassert>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <unordered_set>
#include <memory>
#include <sstream>
#include <iomanip>
#include <clocale>
#include <experimental/filesystem>

// JsonCpp, disable annoying deprecation warnings
#pragma warning(push)
#pragma warning(disable : 4996)
#include <json/json.h>
#pragma warning(pop)
