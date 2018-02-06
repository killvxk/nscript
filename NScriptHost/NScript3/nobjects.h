#pragma once

#include "nscript3.h"

namespace nscript3 {

class object;

// Class that represents script variables
class variable : public object {
	value_t			_value;
public:
	variable() : _value() {}
	value_t get() { return _value; }
	void set(value_t value) { _value = value; }
	value_t create() const { return std::get<object_ptr>(_value)->create(); }
	value_t call(value_t params) { return std::get<object_ptr>(_value)->call(params); }
	value_t item(string_t item) { return std::get<object_ptr>(_value)->item(item); }
	value_t index(value_t index) {
		if(auto pobj = std::get_if<object_ptr>(&_value); pobj)	return (*pobj)->index(index);
		if(index == value_t{ 0 })	return { shared_from_this() };
		throw std::errc::invalid_argument;
	}
	string_t print() const { return std::get<object_ptr>(_value)->print(); }
};


// Class that represents arrays
class v_array : public object {
	std::vector<value_t>	_items;
public:
	v_array() {}
	template<class InputIt> v_array(InputIt first, InputIt last) : _items(first, last) {}
	v_array(std::initializer_list<value_t> items) : _items(items) {}
	value_t get() {
		if(_items.empty())		return value_t{};
		if(_items.size() == 1)	return _items.front();
		return shared_from_this();
	}
	value_t index(value_t index) {
		if(auto pi = std::get_if<int>(&index))	return std::make_shared<indexer>(std::static_pointer_cast<v_array>(shared_from_this()), *pi);
		throw std::errc::invalid_argument;
	}
	string_t print() const {
		std::stringstream ss;
		ss << '[';
		std::stringstream::pos_type pos = 0;
		for(auto& v : _items) {
			if(ss.tellp() > 1) ss << "; ";
			ss << to_string(v);
		}
		ss << ']';
		return ss.str();
	}
	std::vector<value_t>& items() { return _items; }

protected:
	class indexer : public object {
		std::shared_ptr<v_array>	_data;
		int						_index;
	public:
		indexer(std::shared_ptr<v_array> arr, int index) : _index(index), _data(arr) {};
		value_t get() { return _data->items()[_index]; }
		void set(value_t value) { _data->items()[_index] = value; }
		value_t call(value_t params)	{ return std::get<object_ptr>(_data->items()[_index])->call(params); }
		value_t item(string_t item)		{ return std::get<object_ptr>(_data->items()[_index])->item(item); }
		value_t index(value_t index)	{ return std::get<object_ptr>(_data->items()[_index])->index(index); }
	};
};

// Built-in functions
template<class FN> class builtin_function : public object {
public:
	builtin_function(int count, FN func) : _count(count), _func(func) {}
	value_t call(value_t params) {
		if(auto pa = to_array_if(params); pa) {
			if(_count >= 0 && _count != pa->size())	throw nscript_error::bad_param_count;
			return _func(*pa);
		} else if(is_empty(params) && _count <= 0)	return _func({});
		else if(_count < 0 || _count == 1)			return _func({ params });
		else										throw nscript_error::bad_param_count;
	}
protected:
	const int			_count;
	FN					_func;
};
template<class FN> object_ptr make_fn(int count, FN fn) { return std::make_shared<builtin_function<FN>>(count, fn); }

// User-defined functions
void process_args(const args_list& args, const value_t& params, NScript& script) {
	if(args.size() == 0) {
		script.add("@", params);
	} else if(args.size() == 1 && is_empty(params))	{
		script.add(args.front(), params);
	} else {
		auto a = to_array(params);
		if(args.size() != a->items().size())	throw nscript_error::bad_param_count;
		for(int i = (int)args.size() - 1; i >= 0; i--)	script.add(args[i], a->items()[i]);
	}
}

class user_function	: public object {
	const args_list		_args;
	const string_t		_body;
	const context		_context;
public:
	user_function(const args_list& args, string_view body, const context *pcontext = nullptr, const context::var_names* pcaptures = nullptr) 
		: _args(args), _body(body), _context(pcontext, pcaptures)	{}
	value_t call(value_t params) {
		NScript script(_body, &_context);
		process_args(_args, params, script);
		return script.eval({});
	}
};

// User-defined classes
class user_class : public object {
	const args_list		_args;
	const string_t		_body;
	const context		_context;
	value_t				_params;
public:
	user_class(const args_list& args, string_view body, const context *pcontext = nullptr, const context::var_names* pcaptures = nullptr)
		: _args(args), _body(body), _context(pcontext, pcaptures) {}
	value_t create() const			{ return std::make_shared<instance>(_body, &_context, _args, _params); }
	value_t call(value_t params)	{ _params = params; return shared_from_this(); }

	class instance : public object {
		NScript				_script;
	public:
		instance(string_view body, const context *pcontext, const args_list& args, value_t params) : _script(body, pcontext) {
			process_args(args, params, _script);
			_script.eval({});
		}
		value_t item(string_t item)	{ return _script.eval(item); }
	};
};

// Functional objects
class fold_function : public object
{
	object_ptr	_fun;
public:
	fold_function(object_ptr fun) : _fun(fun) {}
	value_t call(value_t params) {
		auto src = to_array(params);
		if(src->items().empty())	return value_t{};
		value_t result = src->items().front();
		for(unsigned i = 1; i < src->items().size(); i++) {
			result = _fun->call(std::make_shared<v_array>(std::initializer_list<value_t>{ result, src->items()[i] }));
		}
		return result;
	}
};

class map_function : public object
{
	object_ptr	_fun;
public:
	map_function(object_ptr fun) : _fun(fun) {}
	value_t call(value_t params) {
		auto src = to_array(params);
		auto dst = std::make_shared<v_array>();
		for(auto& i : src->items()) {
			dst->items().push_back(_fun->call(i));
		}
		return dst;
	}
};

class filter_function : public object
{
	object_ptr	_fun;
public:
	filter_function(object_ptr fun) : _fun(fun) {}
	value_t call(value_t params) {
		auto src = to_array(params);
		auto dst = std::make_shared<v_array>();
		for(auto& i : src->items()) {
			if(to_int(_fun->call(i)))	dst->items().push_back(i);
		}
		return dst;
	}
};

}