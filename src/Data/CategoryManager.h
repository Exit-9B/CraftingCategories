#pragma once

namespace Data
{
	class CategoryManager final
	{
	public:
		using Keyword = RE::BGSKeyword*;

		struct Category
		{
			std::string Label;
			std::set<Keyword> Keywords;
		};

		struct Section
		{
			std::string Label;
			std::int32_t Priority = 50;
			std::set<Keyword> Keywords;

			std::map<std::string, Category> Categories;
			std::map<Keyword, Category*> CategoryKeywords;
		};

		static CategoryManager* GetSingleton();

		~CategoryManager() = default;
		CategoryManager(const CategoryManager&) = delete;
		CategoryManager(CategoryManager&&) = delete;
		CategoryManager& operator=(const CategoryManager&) = delete;
		CategoryManager& operator=(CategoryManager&&) = delete;

		void AddSection(
			const std::string& a_label,
			std::int32_t a_priority,
			std::set<Keyword>&& a_keywords);

		void AddCategory(
			const std::string& a_label,
			const std::string& a_section,
			std::set<Keyword>&& a_keywords);

		void ProcessEntry(RE::GFxValue& a_entryObject, RE::TESForm* a_form);

		void GetCategoryArgs(RE::BSTArray<RE::GFxValue>& a_result);

		void RemoveRedundantFilters();

		void ResetFlags();

	private:
		inline static constexpr auto AllLabel = "$ALL";
		inline static constexpr auto MiscLabel = "$MISC";
		inline static constexpr auto OtherLabel = "$Other";

		inline static constexpr std::uint32_t AllFlag = 0xFFFFFFFF;
		inline static constexpr std::uint32_t SectionMask = 0xFF000000;
		inline static constexpr std::uint32_t CategoryMask = 0x00FFFFFF;
		inline static constexpr std::uint32_t CategoryBits = 24;
		inline static constexpr std::uint32_t MiscSectionFlag = 0x01000000;
		inline static constexpr std::uint32_t OtherCategoryFlag = 0x00000001;
		inline static constexpr std::uint32_t StartIndex = 2;

		CategoryManager();

		struct CategoryData
		{
			const char* Label;
			std::int32_t Priority;
		};

		std::map<std::string, Section> _sections;
		std::map<Keyword, Section*> _sectionKeywords;

		std::uint32_t _nextSectionFlag = StartIndex;
		std::uint32_t _nextCategoryFlag = StartIndex;
		std::map<Section*, std::uint32_t> _sectionFlags;
		std::map<Category*, std::uint32_t> _categoryFlags;
		std::map<std::uint32_t, CategoryData> _currentFilters;
	};
}
