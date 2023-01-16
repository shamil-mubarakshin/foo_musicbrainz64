#pragma once
#define _WIN32_WINNT _WIN32_WINNT_WIN7
#define WINVER _WIN32_WINNT_WIN7

#include <map>
#include <ranges>
#include <regex>
#include <set>
#include <string>

#define FMT_HEADER_ONLY
#include <fmt/format.h>

#define JSON_USE_IMPLICIT_CONVERSIONS 0
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <helpers/foobar2000+atl.h>
#include <helpers/atl-misc.h>
#include <helpers/window_placement_helper.h>
#include <SDK/coreDarkMode.h>
#include <libPPUI/CDialogResizeHelper.h>
#include <libPPUI/CListControlOwnerData.h>
#include <libPPUI/CListControlSimple.h>
#include <libPPUI/Controls.h>
#include <pfc/string-conv-lite.h>

#include <wil/stl.h>

#include "foo_musicbrainz64.hpp"
#include "GUIDS.hpp"
#include "Resource.hpp"
#include "Helpers.hpp"
#include "Preferences.hpp"
