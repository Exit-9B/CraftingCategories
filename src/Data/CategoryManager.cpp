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
		KeywordSet&& a_keywords,
		const std::string& a_iconSource,
		const std::string& a_iconLabel)
	{
		if (a_label == AllLabel) {
			if (!a_iconSource.empty()) {
				_iconSourceAll = a_iconSource;
			}

			if (!a_iconLabel.empty()) {
				_iconLabelAll = a_iconLabel;
			}

			return;
		}
		else if (a_label == MiscLabel) {
			if (!a_iconSource.empty()) {
				_sections[""].IconSource = a_iconSource;
			}

			if (!a_iconLabel.empty()) {
				_sections[""].IconLabel = a_iconLabel;
			}

			return;
		}

		auto& section = _sections[a_label];
		section.Label = a_label;

		if (a_priority >= 0) {
			section.Priority = a_priority;
		}

		section.Keywords.merge(a_keywords);

		if (!a_iconSource.empty()) {
			section.IconSource = a_iconSource;
		}

		if (!a_iconLabel.empty()) {
			section.IconLabel = a_iconLabel;
		}

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
		KeywordSet&& a_keywords)
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
			auto hasKeyword = [keywordForm](std::string_view keywordString)
			{
				const auto keywordA = RE::TESForm::LookupByEditorID<RE::BGSKeyword>(keywordString);
				if (keywordA) {
					return keywordForm->HasKeyword(keyword);
				}
				if (static auto tesDataHandler = RE::TESDataHandler::GetSingleton(); tesDataHandler) {
					auto& keywordArray = tesDataHandler->GetFormArray<RE::BGSKeyword>();
					for (auto& keywordB : keywordArray) {
						if (::_stricmp(keywordB->formEditorID.data(), keywordString.data()) == 0) {
							return keywordForm->HasKeyword(keywordB);
						}
					}
				}
				return false;
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

		_currentFilters[sectionFlag] = CategoryData{
			.Label = assignedSection->Label.data(),
			.Priority = assignedSection->Priority,
			.IconSource = assignedSection->IconSource.data(),
			.IconLabel = assignedSection->IconLabel.data(),
		};

		_currentFilters[sectionFlag | categoryFlag] = CategoryData{
			.Label = assignedCategory ? assignedCategory->Label.data() : OtherLabel,
			.Priority = assignedSection->Priority,
		};

		RE::GFxValue priority = assignedSection->Priority;
		a_entryObject.SetMember("priority", priority);
		RE::GFxValue flag = sectionFlag | categoryFlag;
		a_entryObject.SetMember("filterFlag", flag);
	}

	void CategoryManager::GetCategoryArgs(RE::BSTArray<RE::GFxValue>& a_result)
	{
		RemoveRedundantFilters();

		constexpr std::uint32_t stride = 5;

		const std::uint32_t numCategories = 1 + static_cast<std::uint32_t>(_currentFilters.size());
		const std::uint32_t numArgs = numCategories * stride;
		a_result.resize(numArgs);

		std::uint32_t i = 0;
		a_result[i++] = AllLabel;
		a_result[i++] = AllFlag;
		a_result[i++] = -1;
		a_result[i++] = _iconSourceAll;
		a_result[i++] = _iconLabelAll;

		for (auto& [flag, data] : _currentFilters) {
			a_result[i++] = data.Label;
			a_result[i++] = flag;
			a_result[i++] = data.Priority;
			a_result[i++] = data.IconSource;
			a_result[i++] = data.IconLabel;
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
