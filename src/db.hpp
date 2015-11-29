#pragma once

#include "common.hpp"

#include <asql/mysql.hpp>

class Database : NonCopyable
{
public:
	static Database& instance();
	ASql::MySQL::Connection& connection();

	template <typename Parameter, typename Result>
	void query(const std::string& text, const Parameter& parameter, std::vector<Result>& results)
	{
		ASql::Data::SetRefBuilder<Parameter> in_set(parameter);
		ASql::Data::SetBuilder<Result> dummy;
		ASql::Data::STLSetRefContainer< std::vector<Result> > out_set(results);
		execute(text, &in_set, &dummy, &out_set);
	}

	template <typename Result>
	void query_np(const std::string& text, std::vector<Result>& results)
	{
		ASql::Data::SetBuilder<Result> dummy;
		ASql::Data::STLSetRefContainer< std::vector<Result> > out_set(results);
		execute(text, 0, &dummy, &out_set);
	}

private:
	Database();
	~Database();

	void connect();

	void execute(const std::string& text, const ASql::Data::Set* parameters, const ASql::Data::Set* result_template, ASql::Data::SetContainer* results);

private:
	ASql::MySQL::Connection connection_;
};

Database& db();

// Utility for prices
inline std::string price(unsigned int value)
{
	std::ostringstream ss;
	const unsigned int cent = value % 100;
	ss << (value/100);
	if (cent == 0)
		return ss.str();
	ss << ".";
	if (cent < 10)
		ss << "0";
	ss << cent;
	return ss.str();
}
