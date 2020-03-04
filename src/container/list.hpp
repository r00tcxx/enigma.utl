#include "__enigma.h"
#include "memory.hpp"

__ENIGMA_BEGIN__
template<class _type>
struct __list_node {
	using pointer = __list_node*;
	using value_type = _type;
	pointer next = nullptr, prev = nullptr;
	inline bool link(
		pointer prev,
		pointer next) {
		if (!next || !prev) return false;
		prev->next = this, next->prev = this;
		this->next = next, this->prev = prev;
		return true;
	}
	inline void unlink() {
		if (!next || !prev || is_head()) return;
		prev->next = next;
		next->prev = prev;
	}
	inline bool is_head() { return next == this && prev == this; }
};

template<class _type>
struct __list_node_user : public __list_node<_type> {
	using pointer = __list_node_user*;
	using value_type = _type;
	_type val;
};

template<class _Ty>
class list;

template<class _type>
class __list_iterator_base {
protected:
	using __node_type = __list_node_user<_type>;
	using __node_pointer = __node_type*;
	using __base_node_pointer = __list_node<_type>*;
	friend list<_type>;
public:
	using value_type = _type;
	using value_ref = _type&;
	using value_const_ref = const _type&;
	using value_pointer = _type*;
	using value_const_pointer = const _type*;
public:
	__list_iterator_base() {}
	__list_iterator_base(const __node_pointer node) : _ptr(node) {}
	__list_iterator_base(const __list_iterator_base& itor) { *this = itor; }
	__list_iterator_base& operator = (const __list_iterator_base& itor) {
		_ptr = itor._ptr;
		return *this;
	}

	value_ref operator *() const {
		return _ptr->val;
	}

	//user type may override & operator, we need true address of user value.
	auto operator ->() const {
		return std::pointer_traits<value_pointer>::pointer_to(this->_ptr->val);
	}

	__list_iterator_base& operator++() {
		//node ptr is struct, cannot using cpp cast.
		_ptr = (__node_pointer)this->_ptr->next;
		return *this;
	}

	__list_iterator_base operator++(int) {
		auto tmp = *this;
		this->operator++();
		return tmp;
	}

	__list_iterator_base& operator--() {
		_ptr = (__node_pointer)this->_ptr->prev;
		return *this;
	}

	__list_iterator_base operator--(int) {
		auto tmp = *this;
		this->operator--();
		return std::move(tmp);
	}

	bool operator==(const __list_iterator_base& itor) {
		return itor._ptr == _ptr;
	}

	bool operator!=(const __list_iterator_base& itor) {
		return !operator==(itor);
	}

protected:
	__node_pointer _ptr = nullptr;
};

template<class _type>
class __list_const_iterator : public __list_iterator_base<_type> {
protected:
	using __base = __list_iterator_base<_type>;
public:
	using value_type = typename __base::value_type;
	using value_ref = typename __base::value_ref;
	using value_const_ref = typename __base::value_const_ref;
	using value_pointer = typename __base::value_pointer;
	using value_const_pointer = typename __base::value_const_pointer;
	using __list_iterator_base<_type>::__list_iterator_base;
public:
	__list_const_iterator(const __base& base) : __base::__list_iterator_base(base) {}
	__list_const_iterator(const __list_const_iterator& itor) : __base::__list_iterator_base(itor->_ptr) {}
	__list_const_iterator& operator = (const __list_const_iterator& itor) {
		__base::operator=(itor);
		return *this;
	}

	value_const_ref operator*() const {
		return __base::operator*();
	}

	value_const_pointer operator ->() const {
		return (value_const_pointer)__base::operator ->();
	}

	__list_const_iterator& operator++() {
		__base::operator++();
		return *this;
	}

	__list_const_iterator&& operator++(int) {
		auto tmp = *this;
		__base::operator++(1);
		return std::move(tmp);
	}

	__list_const_iterator& operator--() {
		__base::operator--();
		return *this;
	}

	__list_const_iterator&& operator--(int) {
		auto tmp = *this;
		__base::operator--(1);
		return std::move(tmp);
	}

	bool operator==(const __list_const_iterator& itor) {
		return __base::operator==(itor._ptr);
	}

	bool operator!=(const __list_const_iterator& itor) {
		return __base::operator!=(itor._ptr);
	}

	operator bool() {
		return __base::operator bool()();
	}
};


template<class _Ty>
class list {
protected:
	using __type = typename std::decay<_Ty>::type;
	using __node_pointer = __list_node_user<__type>*;
	using __node_type = __list_node_user<__type>;
	friend __list_node_user<__type>;
public:
	using value_type = __type;
	using value_ref = __type&;
	using value_const_ref = const __type&;
	using value_pointer = const __type*;
	using value_const_pointer = const __type*;
	using value_right_ref = __type&&;
	using allocator_type = allocator;
	using size_type = size_t;
	using iterator = __list_iterator_base<__type>;
	using const_iterator = __list_const_iterator<__type>;
	friend iterator;
	friend const_iterator;
public:
	list() {
		_alloc = new allocator;
		//construct _begin and _end with allocator
		_begin.next = std::addressof(_end);
		_end.prev = std::addressof(_begin);
		//

	}
	list(const list& l) {}

	~list() {
		delete _alloc;
	}
	list& operator = (const list& l) { return *this; }

	iterator begin() noexcept {
		return iterator(reinterpret_cast<__node_pointer>(_begin.next));
	}

	const_iterator cbegin() noexcept {
		return const_iterator(reinterpret_cast<__node_pointer>(_begin.next));
	}

	iterator end() noexcept {
		return iterator(reinterpret_cast<__node_pointer>(
			std::pointer_traits<decltype(_end)*>::pointer_to(_end)));
	}

	const_iterator cend() noexcept {
		return iterator(reinterpret_cast<__node_pointer>(
			std::pointer_traits<decltype(_end)*>::pointer_to(_end)));
	}


	value_ref front() noexcept {
		return *begin();
	}

	value_const_ref front() const noexcept {
		return *begin();
	}

	value_ref back()  noexcept {
		return *--end();
	}

	value_const_ref back() const  noexcept {
		return *--end();
	}

	void push_back(value_right_ref val) noexcept {
		__emplace(end(), std::move(val));
	}

	void push_back(value_const_ref val) noexcept {
		__emplace(end(), val);
	}

	void push_front(value_right_ref val) noexcept {
		__emplace(begin(), std::move(val));
	}

	void push_front(value_const_ref val) noexcept {
		__emplace(begin(), val);
	}

	iterator insert(const_iterator where, value_right_ref val) {
		return iterator(__emplace(where, std::move(val)));
	}

	iterator insert(const_iterator where, value_const_ref val) {
		return iterator(__emplace(where, val));
	}

	iterator insert(const_iterator where, const size_type count, value_const_ref val) {
		iterator forward = where;
		__node_pointer result = nullptr;
		for (size_type n = 0; n < count; ++n, ++forward) {
			forward = __emplace(forward, val);
			if (!n) result = forward._ptr;
			}
		return iterator(result);
	}

	iterator insert(const_iterator where, const_iterator first, const_iterator last) {
		__node_pointer result = nullptr;
		iterator begin = nullptr == first._ptr->prev ?
			reinterpret_cast<__node_pointer>(first._ptr->next) : 
			reinterpret_cast<__node_pointer>(first._ptr);
		iterator forward = where;

		for (iterator itor = begin; itor != last; ++itor, ++forward) {
			forward = __emplace(forward, *itor);
			if (itor == begin) result = forward._ptr;
		}
		return iterator(result);
	}

	void assign(const size_type count, value_const_ref val) {
		if (count < _size) resize(count);	//erase left
		for (auto itor = begin(); itor != end(); ++itor)
			*itor = val;
		if (size_type diff = count - _size != 0)
			insert(end(), diff, val);
	}

	void assign(const_iterator first, const_iterator last) {
		auto desitor = begin();
		for (auto itor = first; itor != last; ++itor, ++desitor) {
			if (end() == desitor) {
				insert(itor, last);
				break;	
			}
			*desitor = *itor;
		}
	}

	void merge(list<__type>&& l) {
		//to do operator =
	}

	void merge(list<__type>& l) {

	}

	void swap(list<__type>& l) {
		std::swap(l._alloc, this->_alloc);
		std::swap(l._begin, this->_begin);
		std::swap(l.end, this->_end);
	}

	iterator erase(const_iterator where) {
		if (where == cend()) return end();
		iterator itor(reinterpret_cast<__node_pointer>(where._ptr->next));
		where._ptr->unlink();
		__destory(where);
		--_size;
		return itor;
	}
	 
	iterator erase(const_iterator first, const_iterator last) {
		if (first == cend()) return end();
		iterator _last = last == end() ? --end() : last, forward = first;
		while (1) {
			forward = erase(forward++);
			if (forward == _last) break;
		}
		return forward;
	}

	void remove(value_const_ref val) {
		remove_if([val](value_const_ref other) { return val == other; });
	}

	void remove_if(std::function<bool(value_const_ref)> la) {
		for (auto itor = begin(); itor != end();) {
			if (la(*itor)) itor = erase(itor);
			else ++itor;
		}
	}

	void resize(const size_type new_size) {
		constexpr bool is_scalar = std::is_scalar<__type>::value;
		resize(new_size, is_scalar ? 0 : __type());
	}

	void resize(const size_type new_size, value_const_ref new_val) {
		if (new_size < _size)
			while (_size > new_size) pop_back();
		else {
			const size_t differ = new_size - _size;
			for (size_t n = 0; n < differ; ++n)
				__emplace(end(), new_val);
		}
	}

	inline void clear() {
		erase(cbegin(), cend());
		_size = 0;
	}

	inline bool empty() const {
		return !_size;
	}

	inline size_t size() const {
		return _size;
	}

	inline void pop_front() noexcept {
		if (!_size)return;
		erase(begin());
	}
	
	inline void pop_back() noexcept {
		if (!_size) return;
		erase(--end());
	}

	void sort() {

	}

	void reverse() {

	}

	void unique() {

	}

protected:
	template<class _ur>
	__node_pointer __emplace(iterator where, _ur&& val) {
		auto itor = where;
		__node_pointer n = _alloc->construct<__node_type>();
		n->val = std::forward<_ur>(val);
		if (dynamic_cast<__list_node<__type>*>(where._ptr) == _begin.next)
			_begin.link(nullptr, n);
		else if (dynamic_cast<__list_node<__type>*>(where._ptr) == _end.prev)
			_end.link(n, nullptr);
		n->link((--itor)._ptr, (where)._ptr);
		++_size;
		return n;
	}

	void __destory(iterator where) {
		_alloc->destruct<__node_type>(where._ptr);
	}

protected:
	allocator* _alloc = nullptr;;
	size_type _size = 0;
	__list_node<__type> _begin, _end;
};
__ENIGMA_END__
