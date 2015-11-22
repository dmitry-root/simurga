#include "httpservice.hpp"
#include "template.hpp"
#include "db.hpp"


namespace
{

	struct ModelParameters
	{
		unsigned int model_id;
		std::string color_name;

		ASQL_BUILDSET(
			(model_id)
			(color_name)
		);
	};

	struct ModelView
	{
		unsigned int model_id;
		std::string color_name;
		std::string color_desc;
		std::string model_title;
		unsigned int num_images;
		unsigned int sizes;
		ASql::Data::Nullable<std::string> material_desc;
		unsigned int price;
		std::string desc;

		ASQL_BUILDSET(
			(model_id)
			(color_name)
			(color_desc)
			(model_title)
			(num_images)
			(sizes)
			(material_desc)
			(price)
			(desc)
		);
	};

	const std::string model_query =
		"select * from `model_view_ru` where `model_id` = ? and `color_name` = ? limit 1";

	inline bool is_letter(char c)
	{
		return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
	}

	inline bool is_digit(char c)
	{
		return '0' <= c && c <= '9';
	}

	inline bool is_special(char c)
	{
		return c == '-' || c == '_';
	}

	void parse_id(const std::string& id, unsigned int& model_id, std::string& color_name)
	{
		size_t pos = id.find('-');
		if (pos == std::string::npos)
			throw NotFound();

		model_id = from_string<unsigned int>(id.substr(0, pos));
		color_name = id.substr(pos+1);

		if (color_name.empty())
			throw NotFound();

		for (size_t i = 0, n = color_name.length(); i < n; i++)
		{
			const char c = color_name[i];
			if (!is_letter(c) && !is_digit(c) && !is_special(c))
				throw NotFound();
		}
	}

	enum ImageType
	{
		Image_Big,
		Image_Preview
	};

	inline std::string image_path(ImageType image_type, unsigned int num)
	{
		std::ostringstream ss;
		ss << (image_type == Image_Big ? "big_" : "preview_") << num << ".jpg";
		return ss.str();
	}

	enum Size
	{
		Size_XS   = 1 << 3,
		Size_S    = 1 << 4,
		Size_M    = 1 << 5,
		Size_L    = 1 << 6,
		Size_XL   = 1 << 7,
		Size_XXL  = 1 << 8,
		Size_XXXL = 1 << 9
	};

	inline std::string size_to_string(unsigned int sizes)
	{
		std::string result;
		// FIXME
		for (unsigned int size = Size_XS; size <= Size_XXL; size <<= 1)
		{
			if ((sizes & size) == 0)
				continue;
			if (!result.empty())
				result += ", ";
			switch (size)
			{
				case Size_XS  : result += "38-40"; break;
				case Size_S   : result += "42-44"; break;
				case Size_M   : result += "44-46"; break;
				case Size_L   : result += "46-48"; break;
				case Size_XL  : result += "48-50"; break;
				case Size_XXL : result += "50-52"; break;
				case Size_XXXL: result += "52-54"; break;
			}
		}
		return result;
	}

}


ModelHttpObject::ModelHttpObject(const Fastcgipp::Http::Environment<char>& env) :
	HttpObject(env)
{
}

void ModelHttpObject::get(const std::string& id)
{
	const std::string path = env_.root + "/model";
	DirConfig config(path + "/.config");

	ModelParameters params;
	parse_id(id, params.model_id, params.color_name);
	const unsigned int model_id = params.model_id;
	const std::string color_name = params.color_name;

	std::vector<ModelView> model_items;
	db().query<ModelParameters, ModelView>(model_query, params, model_items);
	if (model_items.empty())
		throw NotFound();

	const ModelView& model = model_items.front();
	const std::string title = model.model_title;

	ArgumentList args;
	add_argument(args, "model_id", to_string<unsigned int>(model.model_id));
	add_argument(args, "color_name", model.color_name);
	add_argument(args, "color_desc", model.color_desc);
	add_argument(args, "model_title", model.model_title);
	add_argument(args, "sizes", size_to_string(model.sizes));
	add_argument(args, "material_desc", model.material_desc.nullness ? std::string() : model.material_desc.object);
	add_argument(args, "price", price(model.price));
	add_argument(args, "desc", model.desc);
	add_argument(args, "default_image", image_path(Image_Big, 0));

	std::shared_ptr<ArrayArgument> images = std::make_shared<ArrayArgument>();
	for (unsigned int i = 0; i < model.num_images; i++)
	{
		ArrayArgument::ItemPtr item = std::make_shared<ArrayArgument::Item>();
		(*item)["big"] = image_path(Image_Big, i);
		(*item)["preview"] = image_path(Image_Preview, i);
		(*item)["index"] = to_string<unsigned int>(i + 1);
		images->add(item);
	}

	args["previews"] = images;

	const PageTemplate* tpl = TemplateList::instance().get("model");
	set_page(title, tpl->yield(args));
}
