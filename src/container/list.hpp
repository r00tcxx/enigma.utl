#include "__enigma.h"
#include "memory.hpp"

__ENIGMA_BEGIN__

template<class _Ty>
class list {
protected:
	using __type = typename std::decay<_Ty>::type;
	struct __list_node {
		__type val;
		__type* next = nullptr, * prev = nullptr;
	};



public:
	using value_type = __type;
	using reference = __type&;
	using const_reference = const __type&;
	using allocator_type = allocator;
	using size_type = size_t;
	using pointer = __type*;
	using const_pointer = const __type*;
	using self = list&;
	using const_self = const list&;
public:
	list() {}
	list(const_self l) {}
	list& operator(const_self l) { return *this; }
	~list() {}

protected:
	allocator _alloc;
	__list_node* head = nullptr;
};
__ENIGMA_END__