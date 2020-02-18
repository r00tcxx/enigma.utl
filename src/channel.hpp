#pragma once
#include <list>
#include <queue>
#include <thread>
#include <mutex>
#include <vector>
#include <condition_variable>
#include "__enigma.h"
#include "retval.hpp"

__ENIGMA_BEGIN__
template<typename _Ty>
class channel {
public:
	using type = typename std::decay<_Ty>::type;
	using type_pointer = type*;
	using const_type = const type;
	using const_type_ref = const_type&;
	using ref = channel&;
	using const_ref = const ref;

protected:
	class _io {
	public:
		_io(channel<_Ty>* m) : _mother(m) {}
		_io(const _io&) = delete;
		_io(_io&&) = delete;
		_io& operator=(const _io&) = delete;
	protected:
		channel<_Ty>* _mother;
	};

public:
	class consumer : public _io {
		friend channel;
	public:
		using _io::_io;
		consumer() = delete;
		consumer(const consumer&) = delete;
		consumer(consumer&&) = delete;
		consumer& operator=(const consumer&) = delete;

		retval<const_type_ref> consume() {
			std::unique_lock<std::mutex> l(_m);
			_cv.wait(l, [this] {return !_records.empty(); });
			auto m = _records.front();
			_records.pop();
			return { *m, nullptr };
		}

	protected:
		void _event(type_pointer msg) {
			std::lock_guard<std::mutex> l(_m);
			_records.push(msg);
			_cv.notify_one();
		}

	protected:
		std::mutex _m;
		std::condition_variable _cv;
		std::queue<type_pointer> _records;
	};

	class producer : public _io {
		friend channel;
	public:
		producer(channel<_Ty>* m) : _io(m) {
			for (auto& c : m->_cs)
				_cs.push_back(c);
		}

		producer() = delete;
		producer(const producer&) = delete;
		producer(producer&&) = delete;
		producer& operator=(const producer&) = delete;

		retval<bool> publish(const_type_ref msg) {
			_records.emplace_back(msg);
			for (auto& c : _cs) 
				c->_event(&(*_records.rbegin()));
			return { true };
		}

	protected:
		std::list<type> _records;
		std::list<consumer*> _cs;
	};

public:
	channel() : channel(1, 1) {}
	channel(
		const size_t n_producer,
		const size_t n_consumer) : _np(n_producer), _nc(n_consumer), _ps(_np), _cs(_nc) {
		for (auto i = 0; i < _nc; i++)
			_cs.emplace_back(new consumer(this));
		for (auto i = 0; i < _np; i++)
			_ps.emplace_back(new producer(this));
	}
	inline const size_t get_producer_number() const { return _np; }
	inline const size_t get_consumer_number() const { return _nc; }

	inline retval<producer*> new_producer() {
		std::lock_guard<std::mutex> l(_mutex);
		static std::vector<producer*>::iterator it = _ps.begin();
		return it == _ps.end() ? retval<producer*>(nullptr, "producter full") : retval<producer*>(*(it++));
	}

	inline retval<consumer*> new_consumer() {
		std::lock_guard<std::mutex> l(_mutex);
		static std::vector<consumer*>::iterator it = _cs.begin();
		return it == _cs.end() ? retval<consumer*>(nullptr, "producter full") : retval<consumer*>(*(it++));
	}


protected:
	std::vector<producer*> _ps;
	std::vector<consumer*> _cs;
	std::mutex _mutex;
	const size_t _np, _nc;
};
__ENIGMA_END__


/*
1. 加入ack机制
2. p&c 多线程检测
3. 优化
4. 检查一下拷贝问题
*/