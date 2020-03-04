#pragma once
#include "__enigma.h"
#include <cassert>
#include <functional>

__ENIGMA_BEGIN__
class allocator {

protected:
	using __obj_destructor = void(*)();
	struct __block {
		__block* prev = nullptr;
		__block* next = nullptr;
		__block* destructor = nullptr;
		char flag = 0;
		size_t size = 0;
		char* mem = nullptr;

		bool unlink() {
			if (!next && !prev) return false;
			else if (is_head()) return false;
			prev->next = next;
			next->prev = prev;
			return true;
		}

		void link(
			 __block* prev, 
			 __block* next) {
			if (!prev || !next) return;
			prev->next = this;
			next->prev = this;
			this->next = next;
			this->prev = prev;
		}
		inline bool is_head() { return next == this && prev == this; }
	};
	using __blocks_list = __block**;
public:
	using oom_func = std::function<void()>;
	allocator(): allocator(256) {}
	allocator(const allocator&) = delete;
	allocator& operator=(const allocator&) = delete;
	allocator(
		const size_t wl,
		oom_func oom = nullptr) noexcept : _wl(wl < 256 ? 256 : wl), _oom(oom)  {
		auto index = (size_t)ceil(log2(_wl));
		_bl = new (__alloc(sizeof(__block*) * index)) __block* [index];
		_fbl = new (__alloc(sizeof(__block*) * index)) __block * [index];
		for (size_t i = 1; i <= index; i++) {
			_bl[i - 1] = nullptr;
			_fbl[i - 1] = nullptr;
		}
		_max_index = index;
	}
	
	~allocator() {
		auto clear_blocks = [this](__block* b) {
			if (!b)return;
			auto head = b;
			auto cur = b->next;
			while (cur && cur->next != cur) {
				auto next = cur->next;
				__dealloc(cur);
				cur = next;
				if (cur == head) break;
			}
			__dealloc(head);
		};

		for (size_t i = 0; i < _max_index; i++) {
			if (_bl[i])
				clear_blocks(_bl[i]);
			if (_fbl[i])
				clear_blocks(_fbl[i]);
		}
		_fbl = nullptr, _bl = nullptr;
	}

	void* alloc(const size_t size) {
		assert(size != 0);
		if (size <= 0) return nullptr;
		else if (size > _wl) return ::operator new(size);
		const size_t pow = (size_t)ceil(log2(size));
		if (pow > _max_index) return nullptr;
		return __get_block(size, pow);
	}

	void free(
		void* const ptr,
		const size_t size) {	
		assert(ptr);
		if (size > _wl) {
			::operator delete(ptr, size);
			return;
		}
		__block* block = reinterpret_cast<__block*>((size_t)ptr - sizeof(__block));
		__unuse_block(block);
	}

	template<class _Ty, typename ..._Args>
	typename std::decay<_Ty>::type* construct(_Args&& ...args) {
		static_assert(!std::is_pointer<_Ty>::value, "_Ty cannot be a pointer");
		using type = typename std::decay<_Ty>::type;
		if (sizeof(type) >= _wl) return  new type(std::forward<_Args>(args)...);
		const size_t size = sizeof(type);
		const size_t pow = (size_t)ceil(log2(size));
		if (pow > _max_index) return nullptr;
		auto p = __get_block(size, pow);
		type* obj = new(p) type(std::forward<_Args>(args)...);
		__block* b = reinterpret_cast<__block*>((size_t)p - sizeof(__block));

		//catch destruct
		__block* desblock = __balloc(sizeof(std::function<void()>));
		auto desfun =  new (desblock->mem) std::function<void()>(nullptr);
		desblock->flag = 0x01;
		*desfun = [obj] { obj->~type(); };
		b->destructor = desblock;
		return obj;
	}

	template<class _Ty>
	void destruct(typename std::decay<_Ty>::type* ptr) {
		static_assert(!std::is_pointer<_Ty>::value, "_Ty cannot be a pointer");
		using type = typename std::decay<_Ty>::type;
		if (sizeof(type) >= _wl) {
			delete ptr;
			return;
		}
		__block* b = reinterpret_cast<__block*>((size_t)ptr - sizeof(__block));
		if (b->destructor) (*(std::function<void()>*)b->destructor->mem)();
		__unuse_block(b->destructor);
		__unuse_block(b);
	}

protected:
	inline __block* __balloc(const size_t size) {
		return reinterpret_cast<__block*>((size_t)alloc(size) - sizeof(__block));
	}

	void* __alloc(const size_t bytes) {
		void* p = nullptr;
		bool btry = false;
		try {
			p = ::operator new (bytes);
		}
		catch (...) { btry = true; }
		while (btry) {
			if (_oom) _oom();
			else throw std::bad_alloc();
			try {
				p = operator new (bytes);
			}
			catch (...) {}
		}
		return p;
	}

	void __dealloc(__block* b) {
		if (!b) return;
		if (0x01 == b->flag)	//when 0x01 == flag, this block used for obj destructor,
			return;				//can not free before obj.
		if (b->destructor) {
			auto des = (std::function<void()>*)b->destructor->mem;
			(*des)();
			b->destructor->flag = 0x02; //skip line 161 check, now can be free.
			__dealloc(b->destructor);
		}
		__unuse_block(b, false);
		const size_t power = (size_t)::pow(2, ceil(log2(b->size)));
		operator delete[](b, power + sizeof(__block));
		b = nullptr;
	}

	void* __get_block(
		const size_t rsize,
		const size_t pow) {
		__block* fb = _fbl[pow - 1];
		__block*& ub = _bl[pow - 1];
		if (!fb) {
			auto nb = __new_block(rsize, pow);
			nb->link(!ub ? nb : ub->prev, !ub ? nb : ub);
			ub = !ub ? nb : ub; 
			return nb->mem;
		}
		if (!fb->is_head()) fb->unlink();
		else _fbl[pow - 1] = nullptr;
		if (!ub) {
			ub = fb;
			ub->link(ub, ub);
		}
		else  fb->link(ub->prev, ub);
		return fb->mem;
	}

	__block* __new_block(
		const size_t rsize,
		const size_t index) {
		auto p = __alloc(sizeof(__block) + (size_t)::pow(2, index));
		assert(p);
		auto block = new (p) __block;
		block->mem = (char*)((size_t)&block->mem + sizeof(block->mem));
		block->size = rsize;
		return block;
	}

	void __unuse_block(
		__block* b,
		const bool add2free = true) {
		const size_t pow = (size_t)ceil(log2(b->size));
		memset(b->mem, 0, b->size);
		if (b->is_head()) {
			_bl[pow - 1] = nullptr;
		}
		else {
			if (_bl[pow - 1] == b) 
				_bl[pow - 1] = b->next;
			b->unlink();
		}
		if (add2free) {
			b->destructor = nullptr;
			auto& fb = _fbl[pow - 1];
			if (!fb) {
				b->link(b, b);
				fb = b;
			}
			else b->link(fb->prev, fb);
		}
	}

protected:
	__blocks_list _bl;
	__blocks_list _fbl;
	size_t _wl, _max_index = 0;
	oom_func _oom = nullptr;
};
__ENIGMA_END__
