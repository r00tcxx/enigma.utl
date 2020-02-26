#pragma once
#include "__enigma.h"
#include "singleton.h"
#include "memory.hpp"
#include <thread>

__ENIGMA_BEGIN__
class log : public singleton<log> {
	friend singleton<log>;
public:
	enum class level { null = 0, info = 1, warning = 2, error = 3, fatal = 4 };

	class loging_stream {

	protected:
		allocater* _alloc = nullptr;
	};
	
	
	log(const log&) = delete;
	log(log&&) = delete;
	log& operator= (const log&) = delete;
	log(const size_t allocater_wl,
		const char* prefix_format,
		const size_t len) noexcept {
		size_t wl = allocater_wl <= 512 ? 512 : allocater_wl;
		_alloc = new allocater(
			std::bind(&log::_alloc_ofm, this),
			wl);
		for (size_t i = 1; i <= (size_t)level::fatal; i++) {
			auto r = _alloc->constructor<loging_stream>().m_val();
		}
	}


protected:
	void _alloc_ofm() { }


protected:
	allocater::ofm_func _ofm = nullptr;
	allocater* _alloc = nullptr;
	allocater_ptr<loging_stream*> _ss[4];
	std::thread _th;
};
__ENIGMA_END__