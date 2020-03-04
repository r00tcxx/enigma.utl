#include "container/list.hpp"
#include <list>
#include <iostream>
#include <time.h>
using namespace enigma;

int main(int argc, char* argv[]) {

	list<int> l;

	for (int n = 1; n <= 5; n++) {
		l.push_back(n);
	}

	int k = 0;
	l.push_front(k);
	std::cout << *l.begin() << std::endl;

	l.insert(l.end(), 6);
	std::cout << *--l.end() << std::endl;

	//std::cout << *l.insert(l.end(), 3, 9) << std::endl;
	//for (auto itor = l.begin(); itor != l.end(); ++itor)
	//	std::cout << *itor;
	//std::cout << std::endl;

	list<int> l2;
	l2.push_back(8);
	l2.insert(l2.end(), l.begin(), l.end());
	for (auto itor = l2.begin(); itor != l2.end(); ++itor)
		std::cout << *itor;

	std::cout << std::endl;
	l2.erase(l2.begin());
	//l2.erase(l2.begin(), --l2.end());
	for (auto itor = l2.begin(); itor != l2.end(); ++itor)
		std::cout << *itor;
	std::cout << std::endl;
	l2.resize(10);
	for (auto itor = l2.begin(); itor != l2.end(); ++itor)
		std::cout << *itor;
	std::cout << std::endl;

	l2.remove(0);
	for (auto itor = l2.begin(); itor != l2.end(); ++itor)
		std::cout << *itor;

#if 1


	std::cout << std::endl;
	std::list<int> sl{ 0, 1, 2, 3, 4 };
	sl.erase(--sl.end());
	for (auto& i : sl)
		std::cout << i;
	std::cout << std::endl;
	//std::cout << sl.remove(2);
	for (auto& i : sl)
		std::cout << i;
	sl.resize(10);
	for (auto& i : sl)
		std::cout << i;
	sl.merge()
	sl.assign()
	sl.reverse();
	sl.sort();
	sl.unique();
#endif
	//std::list<int>::iterator itor;
	//std::cout << *(itor = l2.insert(l2.end(), 3, 5));
	//std::cout << *++itor;

	//l.insert(l.end(), 5);
	//l.insert(l.end(), 3, 6);
	//std::cout << *l.insert(l.end(), 2) << std::endl;
	//std::cout << *l.begin() << *++l.begin() << *++++l.begin() << std::endl;
	//std::cout << *--l.end();


	//for (auto itor = l.begin(); itor != l.end(); ++itor) {
	//	std::cout << *itor;
	//}
	return 0;
}

/*
template<class _Ty>
class __list_iterator_base {
public:
	using self = __list_iterator_base;
public:
	__list_iterator_base() = delete;
	__list_iterator_base(nullptr_t) : _ptr(nullptr) {}
	__list_iterator_base(const __list_iterator_base& l) : _ptr(l->_ptr) {}
	__list_iterator_base(__list_node_user<_Ty>::self_pointer p) : _ptr(p) {}
protected:
	__list_node_val<_Ty>::self_pointer _ptr = nullptr;
};


template<class _Ty>
class __list_iterator : public __list_iterator_base<_Ty> {
public:
	using self = __list_iterator;
	using const_self = const __list_iterator;
	using reference = __list_iterator&;
	using const_reference = const __list_iterator&;
	using pointer = _Ty*;
	using __list_iterator_base::__list_iterator_base;
	__list_iterator() : __list_iterator_base(nullptr) {}

	reference operator=(const_reference l) { _ptr = l->_ptr; return *this; }
	reference operator++() { _ptr = _ptr->next; return *this; }
	reference operator--() { _ptr = _ptr->prev; return *this; }
	bool operator==(const_reference l) { return l->_ptr == _ptr; }
	bool operator!=(const_reference l) { return l->_ptr != _ptr; }
	pointer operator->() { return _ptr; }
	pointer operator*() { return _ptr; }
	pointer operator pointer() { return _ptr; }
	bool operator bool() { return _ptr != nullptr; }
};

template<class _Ty>
class __list_const_iterator : public __list_iterator<_Ty> {
public:
	using self = __list_const_iterator;
	using const_self = const __list_const_iterator;
	using reference = __list_const_iterator&;
	using const_reference = const __list_const_iterator&;
	using pointer = _Ty*;
	using const_pointer = const _Ty*;
	using __list_iterator::__list_iterator;
	__list_const_iterator() = delete;

	void operator=(const_reference l) { _ptr = l->_ptr; return *this; }
	void operator++() { _ptr = _ptr->next; return *this; }
	void operator--() { _ptr = _ptr->prev; return *this; }
	const_pointer operator* () const { return _cptr; }
	const_pointer operator-> () const { return _cptr; }
	bool operator==(const_reference l) const { return l->_ptr == _ptr; }
	bool operator!=(const_reference l) const { return l->_ptr != _ptr; }
	bool operator bool() const { return _ptr != nullptr; }
protected:
	__list_const_iterator(__list_node_val<_Ty>::self_pointer p) : __list_iterator_base(p) {}
};
*/
