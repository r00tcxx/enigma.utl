#pragma once
#include <vector>
#include <map>
#include <stdexcept>
#include <functional>
#include "__enigma.h"
#include "retval.hpp"

__ENIGMA_BEGIN__
template<typename _T>
struct is_char {
	enum { value = std::is_same<_T, char>::value };
};
template<typename _T>
struct is_wchar {
	enum { value = std::is_same<_T, wchar_t>::value };
};

template<typename _Ty>
class trietree {
public:
	class _layer;
	using type = typename std::conditional<
		is_char<typename std::decay<_Ty>::type>::value, char, wchar_t>::type;
	using const_type = const type;
	using type_pointer = type*;
	using const_type_pointer = const type*;

	class trie_layer {
	public:
		using callback = std::function<bool(const_type_pointer const)>;
		using calback_param = const_type_pointer const;
	public:
		trie_layer() : _ls(nullptr) {}
		trie_layer(_layer* l) : _ls(l) {}
		trie_layer(const trie_layer& layer) : trie_layer(layer._ls) {}
		trie_layer& operator= (_layer* l) { _ls = l; }
		operator bool() { return _ls ? true : false; }
		operator _layer* () { return _ls; }

		void get_words(
			callback cb,
			const bool deep = true) {
			if (!cb) return;
			std::vector<_layer*> v;
			deep ? _ls->get_children_deep(v) : _ls->get_children(v);
			for (auto& l : v)
				if (!cb(l->get_word())) break;
		}
	private:
		_layer* _ls = nullptr;
	};

	trietree() {}
	trietree(const trietree&) = delete;
	trietree& operator=(const trietree&) = delete;
	~trietree() { clear(); }

	retval<trie_layer> query(
		const_type_pointer word,
		const size_t n) {
		if (!n) return { nullptr, "invalid array length" };
		/*
			find most match, retuan layer must be:
				same layer(length == length) or  parent layer (length - 1)
		*/
		auto finded = _root.find(word[0]);
		if (finded == _root.end()) return { nullptr };
		auto dlayer = finded->second;
		if (n == 2) return { dlayer };
		for (size_t i = 1; i < n - 1; i++) {
			auto l = query(word[i], dlayer).val();
			if (l) {
				_layer* fl = l;
				if (fl->get_length() == n && fl->word_eq(word, n))
					return { fl };
				if (fl->get_children_size()) dlayer = fl;
				else return { fl };
			}
			else
				return { dlayer };
		}
		return { nullptr };
	}

	retval<trie_layer> query(
		const_type appendChar,
		trie_layer l) {
		_layer* tlayer = l;
		if (!l) {
			auto finded = _root.find(appendChar);
			if (finded == _root.end())
				return { nullptr };
			tlayer = finded->second;
			return { tlayer };
		}
		return { tlayer->find_children(appendChar) };
	}

	retval<bool> insert(
		const_type_pointer src,
		const size_t n) {
		if (!n) return { false, "invalid length" };
		auto res = query(src, n).val();
		_layer* tlayer = res;
		if (!tlayer) {
			_layer* l = new _layer(&src[0], 2);
			auto r = _root.insert(std::make_pair(src[0], l));
			if (!r.second) return { false, "root pair insert failed" };
			if (2 == n) return { true };
			tlayer = r.first->second;
		}
		//same layer word
		if (tlayer->get_length() == n) { /*not include \0 */
			if (tlayer->word_eq(src, n)) return { true };
		}
		//tlayer's child layer
		if (tlayer->get_length() == n - 1) {
			tlayer->add_child(src, n);
			return { true };
		}
		auto dlayer = tlayer;
		for (auto i = tlayer->get_length() + 1; i <= n; i++)
			dlayer = dlayer->add_child(src, i);
		return { true };
	}

	void clear() {
		for (auto& kv : _root) {
			kv.second->clear_children();
			SAFE_DEL(kv.second);
		}
		_root.clear();
	}

public:
	class _layer {
	public:
		_layer() = delete;
		_layer(const _layer&) = delete;
		_layer& operator=(const _layer&) = delete;
		_layer(
			const_type_pointer src,
			const size_t n,
			_layer* p = nullptr) {
			if (!n) throw std::runtime_error("invalid array length");
			_word = new type[n];
			_lenth = n;
			_parent = p;
			if (2 == n) {
				_word[0] = src[0];
				_word[1] = 0x0;
			}
			else {
				memcpy(_word, src, sizeof(type) * n - 1);
				_word[n - 1] = 0x0;
			}
		}
		~_layer() { SAFE_DEL_ARRAY(_word); }

		inline auto add_child(
			const_type_pointer word,
			const size_t n) {
			auto l = new _layer(word, n, this);
			_children.push_back(l);
			return l;
		}

		inline auto get_children_size() { return _children.size(); }
		inline auto get_length() { return _lenth; }
		inline const_type_pointer const get_word() const { return _word; }

		_layer* find_children(const_type appChar) {
			if (!get_children_size()) return nullptr;
			for (auto l : _children) {
				if (l->_word[l->_lenth - 2] == appChar) return l;
			}
			return nullptr;
		}

		void get_children(std::vector<_layer*>& vec) {
			if (!_children.size()) return;
			std::for_each(
				_children.begin(), 
				_children.end(), [&](_layer* l) { vec.push_back(l); });
		}

		void get_children_deep(std::vector<_layer*>& vec) {
			if (!_children.size()) return;
			std::for_each(
				_children.begin(),
				_children.end(),
				[&](_layer* l) {
					vec.push_back(l);
					l->get_children_deep(vec); 
				});
		}

		inline bool word_eq(
			const_type_pointer src,
			const size_t n) {
			if (n != _lenth) return false;
			return !memcmp(src, _word, n);
		}

		void clear_children() {
			for (auto c : _children) {
				c->clear_children();
				SAFE_DEL(c);
			}
		}
	protected:
		type_pointer _word = nullptr;
		size_t _lenth = 0;
		std::vector<_layer*> _children;
		_layer* _parent = nullptr;
	};
protected:
	std::map<type, _layer*> _root;
};
__ENIGMA_END__
