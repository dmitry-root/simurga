#include "httpservice.hpp"
#include "template.hpp"
#include "db.hpp"


namespace
{

	struct CollectionView
	{
		unsigned int model_id;
		std::string color_name;
		std::string title;
		unsigned int price;

		ASQL_BUILDSET(
			(model_id)
			(color_name)
			(title)
			(price)
		);
	};

}

CollectionHttpObject::CollectionHttpObject(const Fastcgipp::Http::Environment<char>& env) :
	HttpObject(env)
{
}

void CollectionHttpObject::get(const std::string& id)
{
	const std::string path = env_.root + "/collection";
	DirConfig config(path + "/.config");

	std::vector<CollectionView> collection_items;
	db().query_np<CollectionView>("select * from `collection_view_ru`", collection_items);

	std::shared_ptr<ArrayArgument> collection = std::make_shared<ArrayArgument>();
	for (const CollectionView& view : collection_items)
	{
		ArrayArgument::ItemPtr item = std::make_shared<ArrayArgument::Item>();
		(*item)["model_id"] = to_string<unsigned int>(view.model_id);
		(*item)["color_name"] = view.color_name;
		(*item)["title"] = view.title;
		(*item)["price"] = price(view.price);
		collection->add(item);
	}

	ArgumentList args;
	args["collection"] = collection;

	const PageTemplate* tpl = TemplateList::instance().get("collection");
	set_page(config.data(), tpl->yield(args));
}
