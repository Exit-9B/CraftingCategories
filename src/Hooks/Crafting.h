#pragma once

namespace Hooks
{
	class Crafting
	{
	public:
		using WorkBenchType = RE::TESFurniture::WorkBenchData::BenchType;

		Crafting() = delete;

		static void Install();

		static void LoadMoviePatch();

		static void CustomCategoryPatch();

	private:
		static bool CheckFurniture(RE::TESObjectREFR* a_refr);

		static bool LoadMovie(
			RE::BSScaleformManager* a_scaleformManager,
			RE::IMenu* a_menu,
			RE::GPtr<RE::GFxMovieView>& a_viewOut,
			const char* a_fileName,
			RE::GFxMovieView::ScaleModeType a_mode,
			float a_backgroundAlpha);

		static void SetItemEntryData(
			RE::CraftingSubMenus::ConstructibleObjectMenu* a_menu,
			RE::BSTArray<RE::CraftingSubMenus::ConstructibleObjectMenu::ItemEntry>& a_entries);

		inline static WorkBenchType _currentWorkBenchType = WorkBenchType::kNone;

		inline static REL::Relocation<decltype(&CheckFurniture)> _IsFurniture;
		inline static REL::Relocation<decltype(&LoadMovie)> _LoadMovie;
		inline static REL::Relocation<decltype(&SetItemEntryData)> _SetItemEntryData;
	};
}
