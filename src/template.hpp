#pragma once

#include "common.hpp"

#include <memory>
#include <map>
#include <vector>


class TemplateArgument : public NonCopyable
{
public:
	virtual std::string to_string() const = 0;
	virtual bool is_array() const { return false; }
	virtual ~TemplateArgument() {}
};

class StringArgument : public TemplateArgument
{
public:
	StringArgument(const std::string& value = std::string()) : value_(value) {}
	virtual std::string to_string() const { return value_; }

private:
	std::string value_;
};

class ArrayArgument : public TemplateArgument
{
public:
	typedef std::map<std::string, std::string> Item;
	typedef std::shared_ptr<Item> ItemPtr;
	typedef std::vector<ItemPtr> Array;

	void add(const Item& item) { array_.push_back(std::make_shared<Item>(item)); }
	void add(ItemPtr item) { array_.push_back(item); }
	std::size_t size() const { return array_.size(); }
	const Item& get(std::size_t index) const { return *array_[index]; }
	virtual std::string to_string() const { return std::string(); }
	virtual bool is_array() const { return true; }

private:
	Array array_;
};

typedef std::shared_ptr<TemplateArgument> ArgumentPtr;
typedef std::map<std::string, ArgumentPtr> ArgumentList;

void add_argument(ArgumentList& args, const std::string& name, const std::string& value);
void add_arguments(ArgumentList& args, const std::map<std::string, std::string>& data);

class PageTemplate : public NonCopyable
{
public:
	PageTemplate(const std::string& file_name);

	std::string yield(const ArgumentList& arguments) const;

private:
	enum TokenType
	{
		Token_String,
		Token_Variable,
		Token_Foreach,
		Token_Separator,
		Token_End
	};

	struct Token
	{
		Token(TokenType type = Token_String, const std::string& data = std::string()) : type(type), data(data) {}
		TokenType type;
		std::string data;
	};
	typedef std::shared_ptr<Token> TokenPtr;
	typedef std::vector<TokenPtr> TokenList;

	struct ForeachToken : Token
	{
		ForeachToken(const std::string& data) : Token(Token_Foreach, data) {}
		TokenList body;
		TokenList sep;
	};

private:
	void parse(const std::string& data);
	TokenPtr get_next_token(const std::string& data, std::size_t& pos) const;

	void yield_token(const ArgumentList& arguments, const ArrayArgument::Item* arrayItem, const TokenPtr& token, std::string& result) const;
	void yield_tokens(const ArgumentList& arguments, const ArrayArgument::Item* arrayItem, const TokenList& tokens, std::string& result) const;
	std::string get_variable(const ArgumentList& arguments, const ArrayArgument::Item* arrayItem, const std::string& var) const;
	void yield_foreach(const ArgumentList& arguments, const TokenPtr& token, std::string& result) const;

private:
	TokenList body_;
};
typedef std::shared_ptr<PageTemplate> PageTemplatePtr;


class TemplateList : public NonCopyable
{
public:
	static TemplateList& instance();
	~TemplateList() {}

	void load(const std::string& dir);
	const PageTemplate* get(const std::string& name) const;

private:
	TemplateList() {}

private:
	typedef std::map<std::string, PageTemplatePtr> PageTemplateList;
	PageTemplateList templates_;
};
