#include "CategoryManager.h"

namespace Data
{
	CategoryManager::CategoryManager()
	{
		auto& miscSection = _sections[""];
		miscSection.Label = MiscLabel;
		miscSection.Priority = 200;
	}

	CategoryManager* CategoryManager::GetSingleton()
	{
		static CategoryManager singleton{};
		return &singleton;
	}

	void CategoryManager::AddSection(
		const std::string& a_label,
		std::int32_t a_priority,
		std::set<Keyword>&& a_keywords)
	{
		auto& section = _sections[a_label];
		section.Label = a_label;
		section.Priority = a_priority;
		section.Keywords.merge(a_keywords);

		for (auto& keyword : a_keywords) {
			if (auto i = _sectionKeywords.find(keyword); i != _sectionKeywords.end()) {
				i->second->Keywords.erase(keyword);
			}

			_sectionKeywords[keyword] = &section;
		}
	}

	void CategoryManager::AddCategory(
		const std::string& a_label,
		const std::string& a_section,
		std::set<Keyword>&& a_keywords)
	{
		auto& section = _sections[a_section];
		auto& category = section.Categories[a_label];
		category.Label = a_label;
		category.Keywords.merge(a_keywords);

		for (auto& keyword : a_keywords) {
			if (auto i = section.CategoryKeywords.find(keyword);
				i != section.CategoryKeywords.end()) {
				i->second->Keywords.erase(keyword);
			}

			section.CategoryKeywords[keyword] = &category;
		}
	}

	void CategoryManager::ProcessEntry(RE::GFxValue& a_entryObject, RE::TESForm* a_form)
	{
		const auto keywordForm = skyrim_cast<RE::BGSKeywordForm*>(a_form);

		const Section* assignedSection = &_sections[""];
		const Category* assignedCategory = nullptr;
		std::uint32_t sectionFlag = MiscSectionFlag;
		std::uint32_t categoryFlag = OtherCategoryFlag;

		_sectionFlags.try_emplace(assignedSection, MiscSectionFlag);

		if (keywordForm) {
			auto hasKeyword = [keywordForm](RE::BGSKeyword* keyword)
			{
				return keywordForm->HasKeyword(keyword);
			};

			for (auto s = _sections.rbegin(); s != _sections.rend(); ++s) {
				auto& [label, section] = *s;

				if (std::ranges::any_of(section.Keywords, hasKeyword)) {
					assignedSection = &section;
					if (auto i = _sectionFlags.find(&section); i != _sectionFlags.end()) {
						sectionFlag = i->second;
					}
					else {
						sectionFlag = _nextSectionFlag++ << CategoryBits;
						_sectionFlags[&section] = sectionFlag;
					}

					break;
				}
			}

			for (auto c = assignedSection->Categories.rbegin();
				 c != assignedSection->Categories.rend();
				 ++c) {
				auto& [label, category] = *c;

				if (std::ranges::any_of(category.Keywords, hasKeyword)) {
					assignedCategory = &category;
					if (auto i = _categoryFlags.find(&category); i != _categoryFlags.end()) {
						categoryFlag = i->second;
					}
					else {
						categoryFlag = _nextCategoryFlag;
						_categoryFlags[&category] = _nextCategoryFlag++;
					}

					break;
				}
			}
		}

		_currentFilters[sectionFlag] = {
			assignedSection->Label.data(),
			assignedSection->Priority
		};

		_currentFilters[sectionFlag | categoryFlag] = {
			assignedCategory ? assignedCategory->Label.data() : OtherLabel,
			assignedSection->Priority
		};

		RE::GFxValue priority = assignedSection->Priority;
		a_entryObject.SetMember("priority", priority);
		RE::GFxValue flag = sectionFlag | categoryFlag;
		a_entryObject.SetMember("filterFlag", flag);
	}

	void CategoryManager::GetCategoryArgs(RE::BSTArray<RE::GFxValue>& a_result)
	{
		RemoveRedundantFilters();

		constexpr std::uint32_t rowLen = 3;

		const std::uint32_t numCategories = 1 + static_cast<std::uint32_t>(_currentFilters.size());
		const std::uint32_t numArgs = numCategories * rowLen;
		a_result.resize(numArgs);

		a_result[0] = AllLabel;
		a_result[1] = AllFlag;
		a_result[2] = -1;

		std::uint32_t i = rowLen;
		for (auto& [flag, data] : _currentFilters) {
			a_result[i++] = data.Label;
			a_result[i++] = flag;
			a_result[i++] = data.Priority;
		}
	}

	void CategoryManager::RemoveRedundantFilters()
	{
		std::vector<std::uint32_t> redundantFilters;

		for (auto& [section, flag] : _sectionFlags) {
			bool hasCategory = false;
			for (auto& [label, category] : section->Categories) {
				if (_categoryFlags.contains(&category)) {
					hasCategory = true;
					break;
				}
			}

			if (!hasCategory) {
				auto otherFilter = flag | OtherCategoryFlag;
				redundantFilters.push_back(otherFilter);
			}
		}

		for (auto& filter : redundantFilters) {
			_currentFilters.erase(filter);
		}
	}

	void CategoryManager::ResetFlags()
	{
		_nextSectionFlag = StartIndex;
		_nextCategoryFlag = StartIndex;
		_sectionFlags.clear();
		_categoryFlags.clear();
		_currentFilters.clear();
	}
}
