#pragma once
#include <string>
#include <stdio.h>
#include "__enigma.h"
#include "retval.hpp"
#if _WIN32
#	define WIN32_LEAN_AND_MEAN
#	include <Windows.h>
#	include <objbase.h>
#endif

__ENIGMA_BEGIN__
class OS {
public:
	static const char* get_run_path() {
#if _WIN32
		static char p[MAX_PATH]{ 0 };
		if (!p)
			auto hmod = ::GetModuleFileNameA(
				NULL,
				p,
				MAX_PATH);
		return p;
#else
		//todo linux
#endif
	}

	static std::string&& create_uuid() {
		char buf[64]{ 0 };
#if _WIN32
		GUID guid{ 0 };
		auto hr = CoCreateGuid(&guid);
		if (S_OK != hr) return nullptr;
		snprintf(buf,
			sizeof(buf),
			"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
			guid.Data1, guid.Data2, guid.Data3,
			guid.Data4[0], guid.Data4[1],
			guid.Data4[2], guid.Data4[3],
			guid.Data4[4], guid.Data4[5],
			guid.Data4[6], guid.Data4[7]);
#else
#endif
		return std::string(buf);
	}

private:
	OS() {}
	~OS() {}
};

__ENIGMA_END__
