#pragma once
#include "__enigma.h"
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <list>

__ENIGMA_BEGIN__
template<class _Ty>
class squeue {
public:
	using value_type = typename std::decay<_Ty>::type;
public:
	squeue() : squeue(0) {}
	squeue(const size_t max_size) : _max(max_size), _stop(false) {}
	squeue(const squeue& src) { *this = src; }
	squeue& operator = (const squeue& src) {
		std::lock_guard<std::mutex> l(_m);
		std::lock_guard<std::mutex> sl(src._m);
		_q.insert(_q.end(), src._q.begin(), src._q.end());
		return *this;
	}
	~squeue() { stop(); }

	bool push(value_type& elem) { return __add(std::move(elem)); }
	bool push(value_type&& relem) { return __add(std::forward<_Ty>(relem)); }

	void pop(value_type& out) {
		std::unique_lock<std::mutex> l(_m);
		_tcv.wait(l, [this]()->bool { return  _stop || !_q.empty(); });
		if (_stop) return;
		out = std::move(_q.front());
		_q.pop_front();
		_pcv.notify_one();
	}

	bool full() {
		std::lock_guard<std::mutex> l(_m);
		return _q.size() == _max;
	}

	bool empty() {
		std::lock_guard<std::mutex> l(_m);
		return _q.empty();
	}

	inline auto max_size() const { return _max; }

	inline auto size() {
		std::lock_guard<std::mutex> l(_m);
		return _q.size();
	}

	inline void clear() {
		std::lock_guard<std::mutex> l(_m);
		_q.clear();
	}

	inline void stop() {
		_stop = true; 
		clear();
		_pcv.notify_all();
		_tcv.notify_all();
	}

protected:
	bool __add(value_type&& elem) {
		std::unique_lock<std::mutex> l(_m);
		_pcv.wait(l, [this]()->bool { return _stop || _max ? _q.size() <= _max : true; });
		if (_stop) return true;
		_q.emplace_back(elem);
		_tcv.notify_one();
		return true;
	}
protected:
	size_t _max = 0;
	std::condition_variable _pcv, _tcv;
	std::mutex _m;
	std::list<value_type> _q;
	std::atomic<bool> _stop = false;
};
__ENIGMA_END__
