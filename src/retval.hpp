#pragma once
#include <functional>
#include <tuple>
#include <utility>
#include "__enigma.h"
#include <iostream>
#include <optional>

__ENIGMA_BEGIN__

template<class ..._tuple_arg>
class result {
	using xtuple = std::tuple<_tuple_arg...>;
public:
	result(xtuple&& t) : _t(t) {}


public:
	xtuple _t;
	const char* _e = nullptr;
};

template<class _void> 
struct result<_void> {
	enum { check_type = std::is_same<void, _void>::value };
	template<class ..._args>
	static std::tuple<_args...>&& make(_args&& ...arg) {
		return std::move(std::tuple<_args...>(std::forward<_args>(arg)...));
	}
};

using resule_void = result<void>;

template<typename _Ty>
class retval {
public:
	using type = _Ty;
	using const_type = const type;
	using const_type_ref = const_type&;
	using checkFunc = std::function<bool(const_type_ref)>;
	retval() :retval(0, nullptr) {}
	retval(const _Ty& _r, const char* _i = nullptr) : r(_r), i(_i) {}
	retval(const retval& _r) : retval(_r.r, _r.i) {}
	retval(retval&& _lr) : retval(_lr.r, _lr.i) {}
	retval& operator= (const retval& _r) { r = _r.r; i = _r.i; }
	operator type() { return r; }

	const_type_ref val() { return r; } const
	type&& m_val() { return std::move(r); }
	const char* error() { return i; } const 
	bool check(checkFunc cb) { return cb ? cb(r) : false; }
protected:
	const_type r;
	const char* i = nullptr;
};

class retval_i : public retval<int> {
public:
	using retval<int>::retval;
	bool is_success() { return !r; }
};

class retval_b : public retval<bool> {
public:
	using retval<bool>::retval;
	bool is_success() { return r; }
};

__ENIGMA_END__
