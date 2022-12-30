#include "Crafting.h"

#include "Data/CategoryManager.h"
#include "RE/Offset.h"

namespace Hooks
{
	void Crafting::Install()
	{
		LoadMoviePatch();
		CustomCategoryPatch();
	}

	void Crafting::LoadMoviePatch()
	{
		static const auto hook1 = util::MakeHook(
			RE::Offset::TESObjectREFR::ActivateCraftingWorkBench,
			0xE);

		static const auto hook2 = util::MakeHook(
			RE::Offset::CraftingMenu::Ctor,
			IF_SKYRIMSE(0xA9, 0x70));

		if (!REL::make_pattern<"E8">().match(hook1.address()) ||
			!REL::make_pattern<"E8">().match(hook2.address())) {
			util::report_and_fail("Failed to install Crafting::LoadMoviePatch"sv);
		}

		auto& trampoline = SKSE::GetTrampoline();
		_IsFurniture = trampoline.write_call<5>(hook1.address(), &Crafting::CheckFurniture);
		_LoadMovie = trampoline.write_call<5>(hook2.address(), &Crafting::LoadMovie);
	}

	void Crafting::CustomCategoryPatch()
	{
		static const auto hook = util::MakeHook(
			RE::Offset::CraftingSubMenus::ConstructibleObjectMenu::UpdateItemList,
			IF_SKYRIMSE(0x2C8, 0x22D));

		if (!REL::make_pattern<"E8">().match(hook.address())) {
			util::report_and_fail("Failed to install Crafting::CustomCategoryPatch"sv);
		}

		auto& trampoline = SKSE::GetTrampoline();
		_SetItemEntryData = trampoline.write_call<5>(hook.address(), &Crafting::SetItemEntryData);
	}

	bool Crafting::CheckFurniture(RE::TESObjectREFR* a_refr)
	{
		auto isFurniture = _IsFurniture(a_refr);
		if (isFurniture) {
			auto furniture = static_cast<RE::TESFurniture*>(a_refr->data.objectReference);
			_currentWorkBenchType = furniture->workBenchData.benchType.get();
		}

		return isFurniture;
	}

	bool Crafting::LoadMovie(
		RE::BSScaleformManager* a_scaleformManager,
		RE::IMenu* a_menu,
		RE::GPtr<RE::GFxMovieView>& a_viewOut,
		const char* a_fileName,
		RE::GFxMovieView::ScaleModeType a_mode,
		float a_backgroundAlpha)
	{
		auto fileName = a_fileName;

		switch (_currentWorkBenchType) {
		case WorkBenchType::kCreateObject:
			fileName = "ConstructibleObjectMenu";
			break;
		default:
			break;
		}

		_currentWorkBenchType = WorkBenchType::kNone;

		return _LoadMovie(
			a_scaleformManager,
			a_menu,
			a_viewOut,
			fileName,
			a_mode,
			a_backgroundAlpha);
	}

	void Crafting::SetItemEntryData(
		RE::CraftingSubMenus::ConstructibleObjectMenu* a_menu,
		RE::BSTArray<RE::CraftingSubMenus::ConstructibleObjectMenu::ItemEntry>& a_entries)
	{
		_SetItemEntryData(a_menu, a_entries);

		auto& entryList = a_menu->entryList;
		assert(entryList.GetArraySize() == a_entries.size());

		auto categoryManager = Data::CategoryManager::GetSingleton();

		categoryManager->ResetFlags();

		for (std::uint32_t i = 0; i < a_entries.size(); i++) {
			RE::GFxValue entryObject;
			entryList.GetElement(i, &entryObject);

			categoryManager->ProcessEntry(
				entryObject,
				a_entries[i].constructibleObject->createdItem);
		}

		RE::BSTArray<RE::GFxValue> categoryArgs;
		categoryManager->GetCategoryArgs(categoryArgs);

		a_menu->view->Invoke(
			"Menu.InventoryLists.SetCustomConstructCategories",
			nullptr,
			categoryArgs.data(),
			categoryArgs.size());
	}
}
