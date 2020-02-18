#pragma once
#include <type_traits>
#include "__enigma.h"

__ENIGMA_BEGIN__
#define SINGLETON_CLASS(C) public singleton<C>
template<class _Ty>
class singleton {
	friend _Ty;
	using _target_type = typename std::decay<_Ty>::type;
public:
	singleton(const singleton&) = delete;
	singleton& operator=(const singleton) = delete;
	using pointer = _target_type*;
	using type = _target_type;

	template<typename ... _Args>
	inline static pointer instance(_Args... arg) {
		if (!_ptr) 	_ptr = new _target_type(std::forward<_Args>(arg)...);
		return _ptr;
	}

	inline static void release() noexcept {
		if (_ptr) SAFE_DEL(_ptr);
	}
protected:
	singleton() noexcept {}
	virtual ~singleton() {}
private:
	static _target_type* _ptr;
};
template<typename _Ty> 
typename std::decay<_Ty>::type* singleton<_Ty>::_ptr = nullptr;
__ENIGMA_END__
