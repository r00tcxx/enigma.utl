#include "__enigma.h"
#include "retval.hpp"
#include <functional>
#include <cassert>
#ifdef __ENIGMA_DEBUG
#	include <iostream>
#endif

__ENIGMA_BEGIN__
template<class _target, class _baseof>
class union_ptr {
public:
	using target_type = typename std::decay<_target>::type;
	using based_type = typename std::decay<_baseof>::type;
	using target_pointer_type =
		typename std::enable_if<std::is_pointer<target_type>::value, target_type>::type;
	using based_pointer_type =
		typename std::enable_if<std::is_pointer<based_type>::value, based_type>::type;
	union_ptr(const union_ptr& src) { *this = const_cast<union_ptr&>(src); }
	union_ptr() = default;
	union_ptr(nullptr_t null) {};
	union_ptr(
		_target t,
		_baseof b) : _t(t), _b(b) {}
	union_ptr& operator = (const union_ptr& src) {
		auto& s = const_cast<union_ptr&>(src);
		_b = src._b;
		_t = src._t;
		s._b = nullptr;
		s._t = nullptr;
		return *this;
	}
	operator bool() { return _t && _b; }
	union_ptr& operator = (nullptr_t null) { _t = nullptr; _b = nullptr; return *this; }
	inline void set_baseof(const based_pointer_type b) { _b = b; }
	inline void set_target(const target_pointer_type t) { _t = t; }
	inline based_pointer_type baseof() { return _b; }
	inline target_pointer_type target() { return _t; }
	operator target_pointer_type() { return target(); }
	//operator based_pointer_type() { return baseof(); }
protected:
	target_pointer_type _t = nullptr;
	based_pointer_type _b = nullptr;
};


class allocater {
public:
	using ofm_func = std::function<void()>;
protected:
	class _chain {
	public:
		enum { c = 0x01, o = 0x02, u = 0x04, ul = 0x08 };
	protected:
		void* _mem = nullptr;
		size_t _length = 0, _pow = 0;
		_chain* _next = nullptr, * _prev = nullptr;
		char _flag = ul;
	public:
		_chain(const _chain&) = delete;
		_chain(_chain&&) = delete;
		_chain& operator=(const _chain&) = delete;
		_chain(
			const size_t pow,
			allocater* a_,
			const size_t rsize,
			_chain* prev) noexcept(false) : _length(rsize), _pow(pow), _prev(prev) {
			assert(a_ && _length > 0 && pow > 0);
			_mem = static_cast<char*>(a_->__alloc(pow));
		}

		virtual ~_chain() {
			operator delete[](_mem, _pow);
			_mem = nullptr, _next = nullptr, _prev = nullptr;
		}

		operator void* () const { return _mem; }
		inline char flag() const { return _flag; }
		inline auto max_length() const { return _pow; }
		inline auto length() const { return _length; }
		inline auto is_free() { return  bool(_flag & ul); }
		inline void* address() const { return _mem; }
		inline _chain* next() const { return _next; }
		inline void add_next(_chain* next) { _next = next; }
		inline void disuse() { 
			_flag = 0x0; _flag |= ul; 
		}

		bool reuse(
			const size_t new_length,
			const bool obj) {
			if (ul & _flag) {
				_length = new_length;
				_flag = 0, _flag |= u, _flag |= obj ? o : c;
				return true;
			}
			return false;
		}
	};

	struct _chain_obj : public _chain {
		using _chain::_chain;
		std::function<void()> obj_destructor = nullptr;
		virtual ~_chain_obj() {
			if (_flag & u)
				if (obj_destructor) obj_destructor();
		}
	};

	class _block {
		friend allocater;
	public:
		_block(const _block&) = delete;
		_block(_block&&) = delete;
		_block& operator=(const _block&) = delete;
		_block(
			allocater* a_,
			const size_t pow_) : _a(a_), _pow(pow_) {}
	protected:
		allocater* _a = nullptr;
		size_t _length = 0, _pow = 0;
		_chain* _c = nullptr;
	public:
		_chain* get_free_chain(
			const size_t rsize,
			const bool obj) {
			_chain* rs = nullptr;
			if (!_c) {
				_c = _new_chain(rsize, obj, nullptr);
				rs = _c;
			}
			else {
				rs = _c;
				while (1) {
					if (rs->is_free()) break;
					if (!rs->next()) {
						auto nc = _new_chain(rsize, obj, rs);
						rs->add_next(nc);
						rs = nc;
						break;
					}
					rs = rs->next();
				}
			}
			rs->reuse(rsize, obj);
			return rs;
		}

		_chain* _new_chain(
			const size_t rsize,
			const bool obj,
			_chain* prev) {
			_length += rsize;
			if (obj) {
				auto p = new(_a->__alloc(sizeof(_chain_obj))) _chain_obj(_pow, _a, rsize, prev);
				return dynamic_cast<_chain*>(p);
			}
			return new(_a->__alloc(sizeof(_chain))) _chain(_pow, _a, rsize, prev);
		}
	};

	size_t _cl = 0, _wl = 0, _mi;
	_block** _fls = nullptr;
	ofm_func _ofm = nullptr;
public:
	allocater(const allocater&) = delete;
	allocater(allocater&&) = delete;
	allocater& operator=(const allocater&) = delete;
	allocater(
		ofm_func ofm,
		const size_t wl = 256) noexcept(false) {
		_wl = wl < 256 ? 256 : wl, _cl = 0, _ofm = ofm;
		auto index = (size_t)ceil(log2(wl));
		_fls = new (__alloc(sizeof(_block*) * index)) _block * [index];
		for (size_t i = 1; i <= index; i++)
			_fls[i - 1] = static_cast<_block*>(new (__alloc(sizeof(_block))) _block(this, (size_t)pow(2, i)));
		_mi = index;
	}

	~allocater() {
		__enum_chain([&](_chain*& c) {
			if (c->flag() & _chain::o) {
				_chain_obj* co = (_chain_obj*)c;
				SAFE_DEL(co);
			}
			else SAFE_DEL(c);
			});
	}

	retval<union_ptr<void*, _chain*>> alloc(const size_t bytes) {
		assert(bytes > 0);
		if (bytes >= _wl) return { { ::operator new(bytes), nullptr } }; //£¡
		const size_t pow = (size_t)ceil(log2(bytes));
		if (pow > _mi) return { nullptr, "out of block" };
		auto c = _fls[pow - 1]->get_free_chain(bytes, false);
		return { { c->address(), c } };
	}

	retval_b free(union_ptr<void*, _chain*>& ptr) {
		assert(ptr);
		if (!ptr) return { false, "empty ptr" };
		auto a = ptr.baseof();
		_chain* c = ptr.baseof();
		if (!c || c->is_free())  return { false, "invalid ptr" };
		c->disuse();
		ptr = nullptr;
		return { true };
	}

	template<class _Ty, typename ..._Arg>
	retval<union_ptr<typename std::decay<_Ty>::type*, _chain*>> constructor(_Arg&&... args) {
		using type = typename std::decay<_Ty>::type;
		if (sizeof(type) >= _wl) return { { new type(std::forward<_Arg>(args)...), nullptr } };
		const size_t bytes = sizeof(type);
		const size_t pow = (size_t)ceil(log2(bytes));
		if (pow > _mi) return { nullptr, "out of block" };
		_chain_obj* co = (_chain_obj*)_fls[pow - 1]->get_free_chain(bytes, true);
		auto ptr = new (co->address()) type(std::forward<_Arg>(args)...);
		co->obj_destructor = [=] { ptr->~type(); };
		return { { ptr, dynamic_cast<_chain*>(co) } };
	}

	template<class _Ty>
	retval_b destructor(
		union_ptr<typename std::decay<typename std::remove_pointer<_Ty>::type>::type*, _chain*>& ptr) {
		using type = typename std::decay<typename std::remove_pointer<_Ty>::type>::type;
		assert(ptr);
		auto c = ptr.baseof();
		if (!c || c->is_free())  return { false, "invalid ptr" };
		else if (c->length() != sizeof(type)) return { false, "invalid class pointer" };
		ptr.target()->~type();
		c->disuse();
		ptr = nullptr;
		return { true };
	}

#ifdef __ENIGMA_DEBUG
	void debug_check_chain() {
		using namespace std;
		cout << "free list length:" << _mi << endl;
		for (size_t i = 0; i < _mi; i++) {
			cout << "chain max length:[" << _fls[i]->_length << "]" << endl;
			cout << "chain pow:[" << _fls[i]->_pow << "]" << endl;
			cout << "chain: {" << endl;
			if (!_fls[i]->_c) cout << "\t null block.\n }" << endl;
			else {
				auto c = _fls[i]->_c;
				while (c) {
					cout << "\t {"
						<< " data length ="
						<< c->length()
						<< " free ="
						<< c->is_free()
						<< " address ="
						<< (void*)c->address()
						<< " }" << endl;
					c = c->next();
				}
				cout << "}" << endl;
			}
			cout << endl << endl;
		}
	}
#endif

protected:
	void __enum_chain(std::function<void(_chain*&)> cb) {
		if (!_fls) return;
		for (size_t i = 0; i < _mi; i++) {
			auto b = _fls[i];
			auto c = b->_c;
			while (c) {
				auto next = c->next();
				cb(c);
				c = next;
			}
		}
	}

	void* __alloc(const size_t bytes) {
		void* p = nullptr;
		bool btry = false;
		try {
			p = ::operator new (bytes);
			_cl += bytes;
		}
		catch (...) { btry = true; }
		while (btry) {
			if (_ofm) _ofm();
			else throw std::bad_alloc();
			try {
				p = operator new (bytes);
				_cl += bytes;
			}
			catch (...) {}
		}
		return p;
	}
};

template<typename _Ty>
struct allocater_ptr : public union_ptr<_Ty, allocater::_chain*>{
	using type = union_ptr<_Ty, allocater::_chain*>;
	using union_ptr<_Ty, allocater::_chain*>::union_ptr;
};
__ENIGMA_END__