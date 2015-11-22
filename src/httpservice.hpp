/**
 * \brief Interface for HTTP handling classes
 */

#include "common.hpp"
#include <functional>
#include <fastcgi++/request.hpp>
#include <fastcgi++/http.hpp>
#include <map>

//! Special exception classes for returning HTTP status
class HttpException : public std::runtime_error
{
public:
	HttpException() : std::runtime_error("Unknown server error") {}
	HttpException(const std::string& desc) : std::runtime_error(desc) {}
	virtual const char* status() const { return "500 Internal Server Error"; }
};

class NotFound : public HttpException
{
public:
	NotFound() : HttpException("Object not found") {}
	NotFound(const std::string& desc) : HttpException(desc) {}
	virtual const char* status() const { return "404 Not Found"; }
};

class InvalidMethod : public HttpException
{
public:
	InvalidMethod() : HttpException("Method is not allowed for this object") {}
	virtual const char* status() const { return "405 Method Not Allowed"; }
};

/**
 * \brief Pure virtual base class for HTTP object
 *
 * Children of this class handle a specific HTTP request to a particular URI.
 * \note The request processing MUST be atomic, i.e. the job may be done or not done, but not "partially" done.
 */
class HttpObject
{
public:
	HttpObject(const Fastcgipp::Http::Environment<char>& env) : env_(env), status_("200 OK"), content_type_("text/html; charset=utf-8"), data_("OK") {}
	virtual ~HttpObject() {}

	const Fastcgipp::Http::Environment<char>& environment() const { return env_; }
	const std::string& status() const { return status_; }
	const std::string& content_type() const { return content_type_; }
	const std::string& custom_headers() const { return custom_headers_; }
	const std::string& data() const { return data_; }

	virtual void set_path(const std::string& path) { path_ = path; }

	//! This function should be called from the main thread to run the job
	void handle(const std::string& id);

protected:
	const Fastcgipp::Http::Environment<char>& env_;
	std::string status_;  //!< Status code; filled automatically by handling exceptions from run()
	std::string path_;

	// Result fields to be filled by child objects
	std::string content_type_;   //!< Content-Type returned by job ("text/plain" by default)
	std::string custom_headers_; //!< Custom header list
	std::string data_;           //!< Output data returned by job

	virtual void head(const std::string& id) { get(id); }
	//! GET handler
	virtual void get(const std::string& id) { throw InvalidMethod(); }
	//! PUT handler
	virtual void create(const std::string& id) { throw InvalidMethod(); }
	//! POST handler
	virtual void update(const std::string& id) { throw InvalidMethod(); }
	//! DELETE handler
	virtual void remove(const std::string& id) { throw InvalidMethod(); }
	
	void set_content_type(const std::string& value) { content_type_ = value; }
	void set_data(const std::string& data) { data_ = data; }
	void set_page(const std::string& title, const std::string& body);
	void set_page(const std::map<std::string, std::string>& config, const std::string& body);
	void add_custom_header(const std::string& header, const std::string& value);
	void redirect(const std::string& url);

	//! Set content type to application/xml
	void set_xml() { content_type_ = "application/xml"; }
};

typedef std::shared_ptr<HttpObject> HttpObjectPtr;


template<class T>
inline HttpObjectPtr create_http_object(const Fastcgipp::Http::Environment<char>& env)
{
	return HttpObjectPtr( static_cast<HttpObject*>(new T(env)) );
}

/**
 * \brief Table of HTTP classes by request path
 */
class HttpObjectList : public NonCopyable
{
	friend std::shared_ptr<HttpObjectList> std::make_shared<HttpObjectList>();
public:
	typedef std::function< HttpObjectPtr(const Fastcgipp::Http::Environment<char>&) > ObjectAllocProc;

	template<class T>
	static HttpObjectPtr create_http_object(const Fastcgipp::Http::Environment<char>& env)
	{
		return std::static_pointer_cast<HttpObject>( std::make_shared<T>(env) );
	}

	static HttpObjectList& instance();
	~HttpObjectList();

	void add(const std::string& path, bool accepts_id, ObjectAllocProc allocator);
	
	template<class T>
	void add(const std::string& path, bool accepts_id)
	{
		add(path, accepts_id, create_http_object<T>);
	}

	HttpObjectPtr allocate(const Fastcgipp::Http::Environment<char>& env, std::string& id) const;

private:
	HttpObjectList();

	struct Item
	{
		bool accepts_id;
		ObjectAllocProc allocator;
		Item(bool acceptsId = false, ObjectAllocProc allocator = ObjectAllocProc()) : accepts_id(acceptsId), allocator(allocator) {}
	};
	typedef std::map<std::string, Item> ObjectTable;
	ObjectTable table_;
	
	void add_standard_objects();
};


class DirConfig : public NonCopyable
{
public:
	DirConfig(const std::string& fileName);
	bool exists() const { return exists_; }
	const std::map<std::string, std::string>& data() const { return data_; }
	std::string get(const std::string& key);

private:
	bool exists_;
	std::map<std::string, std::string> data_;
};

bool enum_files(const std::string& path, const std::string& ext, std::vector<std::string>& result, bool sort = true);
bool enum_folders(const std::string& path, std::vector<std::string>& result, bool sort = true);


class StaticPageHttpObject : public HttpObject
{
public:
	StaticPageHttpObject(const Fastcgipp::Http::Environment<char>& env);
	virtual void set_path(const std::string& path);
	virtual void get(const std::string& id);

private:
	std::string name_;
};

class LookbookHttpObject : public HttpObject
{
public:
	LookbookHttpObject(const Fastcgipp::Http::Environment<char>& env);
	virtual void get(const std::string& id);
};

class CollectionHttpObject : public HttpObject
{
public:
	CollectionHttpObject(const Fastcgipp::Http::Environment<char>& env);
	virtual void get(const std::string& id);
};

class ModelHttpObject : public HttpObject
{
public:
	ModelHttpObject(const Fastcgipp::Http::Environment<char>& env);
	virtual void get(const std::string& id);
};

