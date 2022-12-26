#include "ConfigLoader.h"

#include "Data/CategoryManager.h"

namespace Data
{
	ConfigLoader* ConfigLoader::GetSingleton()
	{
		static ConfigLoader singleton{};
		return &singleton;
	}

	RE::BSResourceNiBinaryStream& operator>>(
		RE::BSResourceNiBinaryStream& a_sin,
		Json::Value& a_root)
	{
		Json::CharReaderBuilder fact;
		std::unique_ptr<Json::CharReader> const reader{ fact.newCharReader() };

		auto size = a_sin.stream->totalSize;
		auto buffer = std::make_unique<char[]>(size);
		a_sin.read(buffer.get(), size);

		auto begin = buffer.get();
		auto end = begin + size;

		std::string errs;
		bool ok = reader->parse(begin, end, &a_root, &errs);

		if (!ok) {
			throw std::runtime_error(errs);
		}

		return a_sin;
	}

	void ConfigLoader::LoadConfigs()
	{
		const auto dataHandler = RE::TESDataHandler::GetSingleton();
		if (!dataHandler)
			return;

		for (auto& file : dataHandler->files) {
			if (!file)
				continue;

			auto fileName = fs::path(file->fileName);
			fileName.replace_extension("json"sv);
			auto directory = fs::path(fmt::format("SKSE/Plugins/{}"sv, Plugin::NAME));
			auto configFile = directory / fileName;

			LoadConfig(configFile);
		}
	}

	void ConfigLoader::LoadConfig(const fs::path& a_path)
	{
		RE::BSResourceNiBinaryStream fileStream{ a_path.string() };

		if (!fileStream.good())
			return;

		Json::Value root;
		try {
			fileStream >> root;
		}
		catch (...) {
			logger::error("Parse errors in file: {}"sv, a_path.filename().string());
		}

		if (!root.isObject())
			return;

		auto categoryManager = CategoryManager::GetSingleton();

		Json::Value sections = root["sections"];
		if (sections.isObject()) {
			for (auto& label : sections.getMemberNames()) {
				Json::Value section = sections[label];

				std::int32_t priority = -1;
				if (section.isMember("priority")) {
					priority = std::min(std::max(section["priority"].asInt(), 0), 100);
				}

				auto keywords = GetKeywords(section["keywords"]);

				std::string iconSource;
				std::string iconLabel;

				Json::Value icon = section["icon"];
				if (icon.isObject()) {
					iconSource = icon["source"].asString();
					iconLabel = icon["label"].asString();
				}

				categoryManager->AddSection(
					label,
					priority,
					std::move(keywords),
					iconSource,
					iconLabel);
			}
		}

		Json::Value categories = root["categories"];
		if (categories.isObject()) {
			for (auto& label : categories.getMemberNames()) {
				Json::Value category = categories[label];

				std::string section = category["section"].asString();

				auto keywords = GetKeywords(category["keywords"]);

				categoryManager->AddCategory(label, section, std::move(keywords));
			}
		}
	}

	std::set<RE::BGSKeyword*> ConfigLoader::GetKeywords(Json::Value a_json)
	{
		std::set<RE::BGSKeyword*> keywords;
		if (a_json.isArray()) {
			for (auto& id : a_json) {
				auto keyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>(id.asString());
				if (keyword) {
					keywords.insert(keyword);
				}
				else {
					logger::warn("Failed to lookup keyword: {}"sv, id.asString());
				}
			}
		}

		return keywords;
	}
}
