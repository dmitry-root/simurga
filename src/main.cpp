#include <iostream>
#include <algorithm>
#include <fastcgi++/manager.hpp>
#include <fastcgi++/request.hpp>
#include <thread>

#include "jobqueue.hpp"
#include "logs.hpp"
#include "httpservice.hpp"
#include "template.hpp"
#include "db.hpp"

#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

#ifndef PREFIX
#define PREFIX "/usr/local"
#endif


static
std::string get_env(const std::string& key)
{
	char* envp = ::getenv(key.c_str());
	return std::string( envp == 0 ? "" : envp );
}

std::string get_data_dir()
{
	const std::string datadir = get_env("SIMURGA_DATA");
	return datadir.empty() ? PREFIX : datadir;
}

std::string trim(const std::string& s)
{
	const std::size_t start = s.find_first_not_of(" \t\n\r");
	if (start == std::string::npos)
		return std::string();
	const std::size_t end = s.find_last_not_of(" \t\n\r");
	return s.substr(start, end-start+1);
}

static
int setup_socket(const std::string& path)
{
	::unlink(path.c_str());
	int fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd == -1)
		throw std::runtime_error( std::string("socket: ") + ::strerror(errno) );

	size_t ssize = sizeof(struct ::sockaddr_un) + path.length() + 1;
	struct ::sockaddr_un* addr = (struct ::sockaddr_un*)::malloc(ssize);
	::memset(addr, 0, ssize);
	addr->sun_family = AF_UNIX;
	::strcpy(addr->sun_path, path.c_str());
	
	mode_t mask = ::umask(0);
	if (::bind(fd, (struct ::sockaddr*)addr, sizeof(struct ::sockaddr_un)) == -1)
	{
		::free(addr);
		::close(fd);
		::umask(mask);
		throw std::runtime_error( std::string("bind: ") + ::strerror(errno) );
	}
	::free(addr);
	if (::listen(fd, 40) == -1)
	{
		::close(fd);
		::umask(mask);
		throw std::runtime_error( std::string("listen: ") + ::strerror(errno) );
	}
	::umask(mask);
	
	return fd;
}


class Request : public Fastcgipp::Request<char>
{
public:
	enum State { Start, Finish };
	Request() : state_(Start) {}

	bool response();
	bool response_start();
	void response_finish();
	
private:
	State state_;
	HttpObjectPtr http_object_;
	std::string id_;
};


bool Request::response()
{
	if (state_ == Start)
	{
		if (response_start())
			return true;
		state_ = Finish;
		return false;
	}
	else
	{
		response_finish();
		return true;
	}
}

bool Request::response_start()
{
	try
	{
		// create http object for request
		http_object_ = HttpObjectList::instance().allocate(environment(), id_);
		if (http_object_ == 0)
		{
			warn("http request to '%s': not found", environment().requestUri.c_str());
			out << "Status: 404 Not Found\r\n";
			out << "Content-Type: text/plain\r\n\r\n";
			out << "Invalid url\r\n";
			return true;
		}

		info("http request to '%s'", environment().requestUri.c_str());
		// http object created, schedule it for execution
		Fastcgipp::Message msg;
		msg.type = 1;
		JobQueue::instance().add(
			boost::bind( &HttpObject::handle, http_object_.get(), id_ ),  // job to run
			boost::bind( callback(), msg )   // what to run on completion of the job
		);
		return false;
	}
	catch (const std::exception& e)
	{
		// handle all exceptions
		::err("http: exception '%s'", e.what());
		err << e.what();
		out << "Status: 500 Internal Server Error\r\n";
		out << "Content-Type: text/plain\r\n\r\n";
		out << "Internal server error\r\n";
		return true;
	}
}

void Request::response_finish()
{
	// request processing complete, send it
	out << "Status: " << http_object_->status() << "\r\n";
	if (!http_object_->content_type().empty())
		out << "Content-Type: " << http_object_->content_type() << "\r\n";
	out << "Content-Length: " << http_object_->data().size() << "\r\n";
	out << http_object_->custom_headers();
	out << "\r\n";
	if (environment().requestMethod != Fastcgipp::Http::HTTP_METHOD_HEAD)
		out << http_object_->data();
}


int main(int argc, char* argv[])
{
	try
	{
		Database::instance();
	}
	catch (const ASql::Error& e)
	{
		err("connect to database: error %d: %s", e.erno, e.msg);
		return 1;
	}
	catch (const std::exception& e)
	{
		err("connect to database: %s", e.what());
		return 1;
	}

	std::string socket_path = get_env("SIMURGA_SOCKET");
	if (socket_path.empty())
		socket_path = "/tmp/simurga.sock";

	int fd = 0;
	try
	{
		fd = setup_socket(socket_path);
	}
	catch (const std::exception& e)
	{
		err("init socket: %s", e.what());
		return 1;
	}

	try
	{
		TemplateList::instance().load(get_data_dir() + "/templates");
		JobQueue::instance(); // create job queue first
		Fastcgipp::Manager<Request> fcgi(fd);
		std::thread t( &JobQueue::run, &JobQueue::instance() ); // main running thread
		fcgi.handler(); // fcgi server thread; manages proper signals itself; exits on signal
		
		// upon fcgi exit
		JobQueue::instance().stop();
		t.join();
	}
	catch (const std::exception& e)
	{
		err("%s", e.what());
		return 2;
	}

	if (!socket_path.empty())
	{
		::close(fd);
		::unlink(socket_path.c_str());
	}
	return 0;
}
