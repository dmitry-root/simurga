#include "template.hpp"

#include <exception>
#include <stdexcept>
#include <fstream>

#include <dirent.h>

void add_argument(ArgumentList& args, const std::string& name, const std::string& value)
{
	args[name] = std::make_shared<StringArgument>(value);
}

void add_arguments(ArgumentList& args, const std::map<std::string, std::string>& data)
{
	for (auto value : data)
		args[value.first] = std::make_shared<StringArgument>(value.second);
}

PageTemplate::PageTemplate(const std::string& file_name)
{
	static const std::size_t buf_size = 4096;
	std::ifstream in(file_name.c_str());
	std::string body;
	char buffer[buf_size];
	while (!in.eof())
	{
		const std::size_t read = in.readsome(buffer, buf_size);
		if (read == 0)
			break;
		body.append(buffer, read);
	}
	in.close();

	parse(body);
}

std::string PageTemplate::yield(const ArgumentList& arguments) const
{
	std::string result;
	yield_tokens(arguments, 0, body_, result);
	return result;
}

void PageTemplate::yield_tokens(const ArgumentList& arguments, const ArrayArgument::Item* arrayItem, const TokenList& tokens, std::string& result) const
{
	for (auto token : tokens)
		yield_token(arguments, arrayItem, token, result);
}

void PageTemplate::yield_token(const ArgumentList& arguments, const ArrayArgument::Item* arrayItem, const TokenPtr& token, std::string& result) const
{
	switch (token->type)
	{
		case Token_String:
			result.append(token->data);
			break;

		case Token_Variable:
			result.append(get_variable(arguments, arrayItem, token->data));
			break;

		case Token_Foreach:
			yield_foreach(arguments, token, result);
			break;

		case Token_If:
			yield_if(arguments, arrayItem, token, result);
			break;

		default:
			throw std::runtime_error("invalid template structure");
	}
}

std::string PageTemplate::get_variable(const ArgumentList& arguments, const ArrayArgument::Item* arrayItem, const std::string& var) const
{
	if (arrayItem != 0)
	{
		ArrayArgument::Item::const_iterator i = arrayItem->find(var);
		if (i != arrayItem->end())
			return i->second;
	}
	ArgumentList::const_iterator i = arguments.find(var);
	return i == arguments.end() ? std::string() : i->second->to_string();
}

void PageTemplate::yield_foreach(const ArgumentList& arguments, const TokenPtr& token, std::string& result) const
{
	const ForeachToken* ftoken = static_cast<const ForeachToken*>(token.get());
	ArgumentList::const_iterator i = arguments.find(ftoken->data);
	if (i == arguments.end() || !i->second->is_array())
		return;
	const ArrayArgument* array = static_cast<const ArrayArgument*>(i->second.get());
	for (std::size_t i = 0, n = array->size(); i < n; i++)
	{
		if (i > 0)
			yield_tokens(arguments, 0, ftoken->sep, result);
		yield_tokens(arguments, &array->get(i), ftoken->body, result);
	}
}

void PageTemplate::yield_if(const ArgumentList& arguments, const ArrayArgument::Item* arrayItem, const TokenPtr& token, std::string& result) const
{
	const IfToken* iftoken = static_cast<const IfToken*>(token.get());
	const std::string& value = get_variable(arguments, arrayItem, iftoken->data);
	if (value.empty())
		yield_tokens(arguments, arrayItem, iftoken->if_false, result);
	else
		yield_tokens(arguments, arrayItem, iftoken->if_true, result);
}

void PageTemplate::parse(const std::string& data)
{
	std::size_t pos = 0, len = data.length();
	while (pos < len)
		body_.push_back(get_next_token(data, pos));
}

PageTemplate::TokenPtr PageTemplate::get_next_token(const std::string& data, std::size_t& pos) const
{
	std::size_t spec_pos = data.find("<%", pos);
	if (spec_pos == std::string::npos)
		spec_pos = data.length();
	if (spec_pos != pos)
	{
		TokenPtr result = std::make_shared<Token>(Token_String, data.substr(pos, spec_pos - pos));
		pos = spec_pos;
		return result;
	}

	spec_pos += 2;
	const std::size_t spec_end = data.find("%>", spec_pos);
	if (spec_end == std::string::npos)
		throw std::runtime_error("invalid template");
	pos = spec_end + 2;

	const std::string body = trim( data.substr(spec_pos, spec_end - spec_pos) );
	if (body.empty())
		throw std::runtime_error("invalid template");

	if (body[0] != '#')
		return std::make_shared<Token>(Token_Variable, body);

	if (body == "#end")
		return std::make_shared<Token>(Token_End);

	if (body == "#sep")
		return std::make_shared<Token>(Token_Separator);

	if (body == "#else")
		return std::make_shared<Token>(Token_Else);

	if (body.substr(0, 3) == "#if")
	{
		std::shared_ptr<IfToken> result = std::make_shared<IfToken>(trim(body.substr(4)));
		const TokenType end = parse_subtokens(data, pos, result->if_true, Token_End, Token_Else);
		if (end == Token_Else)
			parse_subtokens(data, pos, result->if_false, Token_End);
		return std::static_pointer_cast<Token, IfToken>(result);
	}

	if (body.substr(0, 4) == "#for")
	{
		std::shared_ptr<ForeachToken> result = std::make_shared<ForeachToken>(trim(body.substr(5)));
		const TokenType end = parse_subtokens(data, pos, result->body, Token_End, Token_Separator);
		if (end == Token_Separator)
			parse_subtokens(data, pos, result->sep, Token_End);
		return std::static_pointer_cast<Token, ForeachToken>(result);
	}

	throw std::runtime_error("invalid template special tag");
}

PageTemplate::TokenType PageTemplate::parse_subtokens(const std::string& data, std::size_t& pos, TokenList& result, TokenType end1, TokenType end2) const
{
	while (pos < data.length())
	{
		TokenPtr token = get_next_token(data, pos);
		if (token->type == end1 || token->type == end2)
			return token->type;
		result.push_back(token);
	}
	throw std::runtime_error("unexpected eof");
}


TemplateList& TemplateList::instance()
{
	static TemplateList result;
	return result;
}

void TemplateList::load(const std::string& dir)
{
	DIR* dh = opendir(dir.c_str());
	if (dh == 0)
		return;
	std::vector<std::string> files;
	while (struct ::dirent* entry = readdir(dh))
		files.push_back(entry->d_name);
	closedir(dh);

	for (const std::string& file : files)
	{
		if (file.length() < 4 || file.rfind(".tpl") != file.length()-4)
			continue;
		const std::string name = file.substr(0, file.length()-4);
		templates_[name] = std::make_shared<PageTemplate>(dir + "/" + file);
	}
}

const PageTemplate* TemplateList::get(const std::string& name) const
{
	PageTemplateList::const_iterator i = templates_.find(name);
	if (i == templates_.end())
		throw std::runtime_error("Template " + name + " not found");
	return i->second.get();
}
