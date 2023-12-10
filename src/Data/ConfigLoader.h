#pragma once

#include <json/json.h>

namespace fs = std::filesystem;

namespace Data
{
	class ConfigLoader final
	{
	public:
		static ConfigLoader* GetSingleton();

		~ConfigLoader() = default;
		ConfigLoader(const ConfigLoader&) = delete;
		ConfigLoader(ConfigLoader&&) = delete;
		ConfigLoader& operator=(const ConfigLoader&) = delete;
		ConfigLoader& operator=(ConfigLoader&&) = delete;

		void LoadConfigs();

		void LoadConfig(const fs::path& a_path);

	private:
		ConfigLoader() = default;

		KeywordSet GetKeywords(Json::Value a_json);
	};
}
