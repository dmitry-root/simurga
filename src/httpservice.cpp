#include "httpservice.hpp"
#include "jobqueue.hpp"
#include "template.hpp"
#include <stdexcept>
#include <new>
#include <memory>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <algorithm>
#define _BSD_SOURCE 1
#include <dirent.h>

using namespace Fastcgipp;

void HttpObject::handle(const std::string& id)
{
	using namespace Fastcgipp;
	try
	{
		switch (env_.requestMethod)
		{
			case Http::HTTP_METHOD_HEAD:
				head(id);
				break;

			case Http::HTTP_METHOD_GET:
				get(id);
				break;

			case Http::HTTP_METHOD_PUT:
				create(id);
				break;

			case Http::HTTP_METHOD_POST:
				update(id);
				break;

			case Http::HTTP_METHOD_DELETE:
				remove(id);
				break;

			default:
				throw InvalidMethod();
		}
	}
	catch (const HttpException& e)
	{
		content_type_ = "text/plain";
		data_ = e.what();
		status_ = e.status();
	}
	catch (const std::exception& e)
	{
		// TODO log this action
		content_type_ = "text/plain";
		status_ = "500 Internal Server Error";
		data_ = e.what();
	}
}

void HttpObject::set_page(const std::string& title, const std::string& body)
{
	ArgumentList args;
	add_argument(args, "title", title);
	add_argument(args, "body", body);
	add_argument(args, "base_uri", path_ + "/");
	set_data(TemplateList::instance().get("main")->yield(args));
}

void HttpObject::set_page(const std::map<std::string, std::string>& config, const std::string& body)
{
	ArgumentList args;
	add_argument(args, "base_uri", path_ + "/");
	add_arguments(args, config);
	add_argument(args, "body", body);
	set_data(TemplateList::instance().get("main")->yield(args));
}

void HttpObject::add_custom_header(const std::string& header, const std::string& value)
{
	custom_headers_ += header + ": " + value + "\r\n";
}

void HttpObject::redirect(const std::string& url)
{
	content_type_.clear();
	status_ = "302 Found";
	add_custom_header("Location", url);
}


HttpObjectList& HttpObjectList::instance()
{
	static HttpObjectList the_instance;
	return the_instance;
}

HttpObjectList::HttpObjectList()
{
	add_standard_objects();
}

HttpObjectList::~HttpObjectList()
{
}

void HttpObjectList::add(const std::string& path, bool acceptsId, ObjectAllocProc allocator)
{
	table_[path] = Item(acceptsId, allocator);
}

HttpObjectPtr HttpObjectList::allocate(const Http::Environment<char>& env, std::string& id) const
{
	// parse request uri: remove all up to service path, remove query parameters
	std::string path = env.requestUri;
	size_t pos = path.find("?");
	if (pos != std::string::npos)
		path.replace(pos, path.length(), "");
	if (!path.empty() && path[path.length()-1] == '/')
		path.replace(path.length()-1, 1, "");

	// now it's 2 variants: use the entire path as a key, or remove last path item and use it as id
	// the 1st variant is preferred, try it first
	std::unique_ptr<char[]> decoded_path_buf( new char[path.length() + 1] );
	size_t decoded_path_len = Http::percentEscapedToRealBytes(path.c_str(), decoded_path_buf.get(), path.length());
	const std::string decoded_path( decoded_path_buf.get(), decoded_path_len );
	decoded_path_buf.reset();

	ObjectTable::const_iterator i = table_.find(decoded_path);
	if (i != table_.end()) {
		HttpObjectPtr result = i->second.allocator(env);
		result->set_path(decoded_path);
		return result;
	}

	// 2nd case: try to get id from path
	// NOTE we don't use already decoded path as decoded id may contain "/"
	pos = path.find_last_of("/");
	if (pos == std::string::npos)
		return HttpObjectPtr(); // not found
	id = path.substr(pos+1);
	path.replace(pos, path.length(), "");
	decoded_path_buf.reset( new char[ std::max(path.length(), id.length()) + 1] );
	decoded_path_len = Http::percentEscapedToRealBytes(path.c_str(), decoded_path_buf.get(), path.length());
	path.assign(decoded_path_buf.get(), decoded_path_len);
	decoded_path_len = Http::percentEscapedToRealBytes(id.c_str(), decoded_path_buf.get(), id.length());
	id.assign(decoded_path_buf.get(), decoded_path_len);
	decoded_path_buf.reset(0);
	
	// sanity check
	if (id.empty() || id.find_first_of("\\/|.&#") != std::string::npos)
		return HttpObjectPtr();

	i = table_.find(path);
	if (i != table_.end() && i->second.accepts_id)
	{
		const HttpObjectPtr result = i->second.allocator(env);
		result->set_path(path + "/" + id);
		return result;
	}
	return HttpObjectPtr(); // not found
}

void HttpObjectList::add_standard_objects()
{
	add<StaticPageHttpObject>("/phylosophy", false);
	add<StaticPageHttpObject>("/contacts", false);
	add<StaticPageHttpObject>("/howtobuy", false);
	add<LookbookHttpObject>("/lookbook", false);
	add<LookbookHttpObject>("", false);
	add<CollectionHttpObject>("/collection", false);
	add<ModelHttpObject>("/model", true);
}


DirConfig::DirConfig(const std::string& file_name)
{
	exists_ = false;
	std::ifstream in(file_name.c_str());
	if (!in.good())
		return;
	while (!in.eof())
	{
		std::string line;
		std::getline(in, line);
		line = trim(line);
		if (line.empty() || line[0] == '#')
			continue;
		const std::size_t pos = line.find("=");
		if (pos == std::string::npos)
			continue;
		const std::string key = trim(line.substr(0, pos));
		const std::string value = trim(line.substr(pos+1));
		data_[key] = value;
	}
	exists_ = true;
}

std::string DirConfig::get(const std::string& key)
{
	std::map<std::string, std::string>::const_iterator i = data_.find(key);
	return i == data_.end() ? std::string() : i->second;
}


static
bool enum_items(const std::string& path, const std::string& ext, unsigned char d_type, std::vector<std::string>& result, bool sort)
{
	std::vector<std::string> items;
	DIR* dir = ::opendir(path.c_str());
	if (dir == 0)
		return false;
	struct ::dirent myentry, *entry = 0;
	while (::readdir_r(dir, &myentry, &entry) == 0 && entry != 0) {
		const std::string id = entry->d_name;
		if (id.empty() || id[0] == '.')
			continue;
		if (entry->d_type != d_type)
			continue;
		if (!ext.empty() &&
		    (id.length() < ext.length() || !std::equal(id.begin() + (id.length() - ext.length()), id.end(), ext.begin()))
		   )
		{
			continue;
		}
		items.push_back(id);
	}
	::closedir(dir);
	if (items.empty())
		return false;

	if (sort)
		std::sort(items.begin(), items.end());
	result.swap(items);
	return true;
}

bool enum_files(const std::string& path, const std::string& ext, std::vector<std::string>& result, bool sort)
{
	return enum_items(path, ext, DT_REG, result, sort);
}

bool enum_folders(const std::string& path, std::vector<std::string>& result, bool sort)
{
	return enum_items(path, std::string(), DT_DIR, result, sort);
}
