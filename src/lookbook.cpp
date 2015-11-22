#include "httpservice.hpp"
#include "template.hpp"


LookbookHttpObject::LookbookHttpObject(const Fastcgipp::Http::Environment<char>& env) :
	HttpObject(env)
{
}

void LookbookHttpObject::get(const std::string& id)
{
	const std::string path = env_.root + "/lookbook";
	DirConfig config(path + "/.config");

	std::vector<std::string> images;
	enum_files(path, ".jpg", images);

	std::shared_ptr<ArrayArgument> image_list = std::make_shared<ArrayArgument>();
	for (auto image : images)
	{
		ArrayArgument::ItemPtr item = std::make_shared<ArrayArgument::Item>();
		item->insert( std::make_pair("image", image) );
		image_list->add(item);
	}

	ArgumentList args;
	args["images"] = image_list;

	const PageTemplate* tpl = TemplateList::instance().get("lookbook");
	set_page(config.data(), tpl->yield(args));
}
