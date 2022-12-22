#include "Crafting.h"

#include "RE/Offset.h"

namespace Hooks
{
	void Crafting::Install()
	{
		LoadMoviePatch();
	}

	void Crafting::LoadMoviePatch()
	{
		static const auto hook1 = REL::Relocation<std::uintptr_t>(
			RE::Offset::TESObjectREFR::ActivateCraftingWorkBench,
			0xE);

		static const auto hook2 = REL::Relocation<std::uintptr_t>(
			RE::Offset::CraftingMenu::Ctor,
			0xA9);

		if (!REL::make_pattern<"E8">().match(hook1.address()) ||
			!REL::make_pattern<"E8">().match(hook2.address())) {
			util::report_and_fail("Failed to install Crafting::LoadMoviePatch"sv);
		}

		auto& trampoline = SKSE::GetTrampoline();
		_IsFurniture = trampoline.write_call<5>(hook1.address(), &Crafting::CheckFurniture);
		_LoadMovie = trampoline.write_call<5>(hook2.address(), &Crafting::LoadMovie);
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
}
