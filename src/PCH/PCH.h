#pragma once

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

#ifdef NDEBUG
#include <spdlog/sinks/basic_file_sink.h>
#else
#include <spdlog/sinks/msvc_sink.h>
#endif

using namespace std::literals;

namespace logger = SKSE::log;

namespace util
{
	using SKSE::stl::report_and_fail;

	inline auto MakeHook(REL::ID a_id, std::ptrdiff_t a_offset = 0)
	{
		return REL::Relocation<std::uintptr_t>(a_id, a_offset);
	}

	inline auto MakeHook(REL::Offset a_address, std::ptrdiff_t a_offset = 0)
	{
		return REL::Relocation<std::uintptr_t>(a_address.address() + a_offset);
	}
}

struct comp_str_cis
{
	bool operator()(const std::string& a_lhs, const std::string& a_rhs) const
	{
		return ::_stricmp(a_lhs.c_str(), a_rhs.c_str()) < 0;
	}
};

using Keyword = std::string;
template <typename V>
using KeywordMap = std::map<Keyword, V, comp_str_cis>;
using KeywordSet = std::set<Keyword, comp_str_cis>;

#ifndef SKYRIMVR
#define IF_SKYRIMSE(a_resultSE, a_resultVR) a_resultSE
#else
#define IF_SKYRIMSE(a_resultSE, a_resultVR) a_resultVR
#endif

#define MAKE_OFFSET(a_idSE, a_offsetVR) IF_SKYRIMSE(REL::ID(a_idSE), REL::Offset(a_offsetVR))

#define DLLEXPORT __declspec(dllexport)

#include "Plugin.h"
