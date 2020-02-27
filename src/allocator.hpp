#pragma once
#include "__enigma.h"
#include "memory.hpp"
#define __NODISCARD 

//DO NOT USE THIS FILE, IN WINDOWS STL NO FUCKING WORK 
//WHY THE FUCK VC STL WILL DESTORY ALLOCATOR BEFORE USING IT
//I HAVE NO IDEA HOW TO SOLVE THIS ISSUE.

__ENIGMA_BEGIN__
template<class _Ty>
constexpr size_t __get_size_of_n(const size_t n) {
	constexpr size_t ts = sizeof(_Ty), max_possible = size_t(-1) / ts;
	if constexpr (!ts) return 0;
	return n > max_possible ? 0 : n * ts;
};

template<class _Ty>
class std_allocator {
public:
	using value_type = _Ty;
	using size_type = size_t;
	using difference_type = ptrdiff_t;

	template<class _Other>
	struct rebind {
		using other = std_allocator<_Other>;
	};

	__NODISCARD constexpr std_allocator() {}
	__NODISCARD constexpr std_allocator(const std_allocator&) {
		int k = 0;
	}
	template<class _other>
	__NODISCARD constexpr std_allocator(const std_allocator<_other>& o) noexcept {
		int k = 0;
	}
	__NODISCARD ~std_allocator() {
		int k = 0;
	}
	__NODISCARD _Ty* address(_Ty& _val) const noexcept { return std::addressof(_val); }
	__NODISCARD const _Ty address(const _Ty _val) const noexcept { return std::addressof(_val); }
	__NODISCARD size_t max_size() const noexcept { return static_cast<size_t>(-1) / sizeof(_Ty); }

	__NODISCARD void deallocate(
		_Ty* const ptr,
		const size_t count) {
		_allocator.free(ptr, sizeof(_Ty) * count);
	}

	__NODISCARD
#ifdef _WIN32
		__declspec(allocator)
#endif
		_Ty* allocate(const size_t count) {
		return static_cast<_Ty*>(_allocator.alloc(__get_size_of_n<_Ty>(count)));
	}

	__NODISCARD
#ifdef _WIN32
		__declspec(allocator)
#endif
		_Ty* allocate(
			const size_t count,
			const void*) {
		return allocate(count);
	}

	template<class _Ty, class ... _Args>
	void construct(
		_Ty* ptr,
		_Args&& ...args) {
		ptr = _allocator.construct<_Ty>(std::forward<_Args>(args)...);
	}

	template<class _Ty>
	void destroy(_Ty* const ptr) {
		_allocator.destruct<_Ty>(ptr);
	}

protected:
	allocator _allocator;
};
__ENIGMA_END__
