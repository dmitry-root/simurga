#include "httpservice.hpp"
#include "template.hpp"

#include <fstream>


StaticPageHttpObject::StaticPageHttpObject(const Fastcgipp::Http::Environment<char>& env)
	: HttpObject(env)
{
}

void StaticPageHttpObject::set_path(const std::string& path)
{
	HttpObject::set_path(path);
	if (!path.empty() && path[0] == '/') {
		const std::size_t pos = path.find("/", 1);
		name_ = path.substr(1, pos == std::string::npos ? pos : pos-1);
	}
	if (name_.empty())
		name_ = "lookbook";
}

void StaticPageHttpObject::get(const std::string& id)
{
	const std::string path = env_.root + "/" + name_;
	DirConfig config(path + "/.config");

	std::ifstream in((path + "/page.html").c_str());
	if (!in.good())
		throw NotFound("Index page not found");
	std::string body;
	static const std::size_t buf_size = 4096;
	char buffer[buf_size];
	while (!in.eof()) {
		const std::size_t read = in.readsome(buffer, buf_size);
		if (read == 0)
			break;
		body.append(buffer, read);
	}

	ArgumentList args;
	add_argument(args, "body", body);

	const PageTemplate* tpl = TemplateList::instance().get("static_page");
	set_page(config.data(), tpl->yield(args));
}
