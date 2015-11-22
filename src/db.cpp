#include <math.h>

#include "db.hpp"
#include "httpservice.hpp"


static const unsigned int connection_threads = 5;
static const unsigned int mysql_default_port = 3306;


Database::Database() :
	connection_(connection_threads)
{
	DirConfig config(get_data_dir() + "/etc/db.conf");

	const std::string host = config.get("host");
	const std::string user = config.get("user");
	const std::string passwd = config.get("passwd");
	const std::string db = config.get("db");
	const std::string port = config.get("port");
	const std::string unix_socket = config.get("unix_socket");

	connection_.connect(
		host.empty() ? 0 : host.c_str(),
		user.c_str(),
		passwd.c_str(),
		db.c_str(),
		port.empty() ? mysql_default_port : from_string<unsigned int>(port),
		unix_socket.empty() ? 0 : unix_socket.c_str(),
		0,
		"utf8"
	);
}

Database::~Database()
{
}

void Database::execute(const std::string& text, const ASql::Data::Set* parameters, const ASql::Data::Set* result_template, ASql::Data::SetContainer* results)
{
	ASql::MySQL::Statement statement(connection_, text.c_str(), text.length(),
		parameters, result_template);
	statement.execute(parameters, results, 0, 0, true, ::rand() % connection_threads);
}

Database& Database::instance()
{
	static Database db;
	return db;
}

ASql::MySQL::Connection& Database::connection()
{
	return connection_;
}

Database& db()
{
	return Database::instance();
}
