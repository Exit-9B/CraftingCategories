#include "Translation.h"

#include "RE/Offset.h"

void Translation::GetCachedString(wchar_t** a_pOut, wchar_t* a_bufIn, std::uint32_t a_unused)
{
	using func_t = decltype(&Translation::GetCachedString);
	REL::Relocation<func_t> func{ RE::Offset::BSScaleformTranslator::GetCachedString };
	return func(a_pOut, a_bufIn, a_unused);
}

std::uint32_t Translation::ReadLine_w(
	RE::BSResourceNiBinaryStream* a_this,
	wchar_t* a_dst,
	std::uint32_t a_dstLen,
	std::uint32_t a_terminator)
{
	wchar_t* iter = a_dst;

	if (a_dstLen == 0)
		return 0;

	for (std::uint32_t i = 0; i < a_dstLen - 1; i++) {
		wchar_t data;

		std::uint64_t read;
		a_this->stream->DoRead(&data, sizeof(data), read);
		if (read != sizeof(data))
			break;

		if (data == a_terminator)
			break;

		*iter++ = data;
	}

	// null terminate
	*iter = 0;

	return static_cast<std::uint32_t>(iter - a_dst);
}

void Translation::ParseTranslation(const std::string& a_name)
{
	const auto scaleformManager = RE::BSScaleformManager::GetSingleton();
	const auto loader = scaleformManager ? scaleformManager->loader : nullptr;
	const auto translator = loader
		? loader->GetStateAddRef<RE::GFxTranslator>(RE::GFxState::StateType::kTranslator)
		: nullptr;

	const auto scaleformTranslator = skyrim_cast<RE::BSScaleformTranslator*>(translator);

	if (!scaleformTranslator) {
		logger::warn("Failed to import translation for {}"sv, a_name);
		return;
	}

	const auto iniSettingCollection = RE::INISettingCollection::GetSingleton();
	auto setting =
		iniSettingCollection
		? iniSettingCollection->GetSetting("sLanguage:General")
		: nullptr;

	// Construct translation filename
	std::string language = (setting && setting->GetType() == RE::Setting::Type::kString)
		? setting->data.s
		: "ENGLISH"s;

	std::string path = fmt::format("Interface\\Translations\\{}_{}.txt"sv, a_name, language);

	RE::BSResourceNiBinaryStream fileStream{ path };
	if (!fileStream.good()) {
		return;
	}
	else {
		logger::info("Reading translations from {}..."sv, path);
	}

	// Check if file is empty, if not check if the BOM is UTF-16
	std::uint16_t bom = 0;
	std::uint64_t read;
	fileStream.stream->DoRead(&bom, sizeof(std::uint16_t), read);
	if (read == 0) {
		logger::warn("Empty translation file."sv);
		return;
	}
	if (bom != 0xFEFF) {
		logger::error("BOM Error, file must be encoded in UCS-2 LE."sv);
		return;
	}

	while (true) {
		wchar_t buf[512];

		std::uint32_t len = ReadLine_w(&fileStream, buf, sizeof(buf) / sizeof(buf[0]), '\n');
		if (len == 0)  // End of file
			return;

		// at least $ + wchar_t + \t + wchar_t
		if (len < 4 || buf[0] != '$')
			continue;

		wchar_t last = buf[len - 1];
		if (last == '\r')
			len--;

		// null terminate
		buf[len] = 0;

		std::uint32_t delimIdx = 0;
		for (std::uint32_t i = 0; i < len; i++)
			if (buf[i] == '\t')
				delimIdx = i;

		// at least $ + wchar_t
		if (delimIdx < 2)
			continue;

		// replace \t by \0
		buf[delimIdx] = 0;

		wchar_t* key = nullptr;
		wchar_t* translation = nullptr;
		GetCachedString(&key, buf, 0);
		GetCachedString(&translation, &buf[delimIdx + 1], 0);
		scaleformTranslator->translator.translationMap.emplace(key, translation);
	}
}

bool Translation::Translate(const std::string& a_key, std::string& a_result)
{
	if (!a_key.starts_with('$')) {
		return false;
	}

	const auto scaleformManager = RE::BSScaleformManager::GetSingleton();
	const auto loader = scaleformManager ? scaleformManager->loader : nullptr;
	const auto translator = loader
		? loader->GetStateAddRef<RE::GFxTranslator>(RE::GFxState::StateType::kTranslator)
		: nullptr;

	if (!translator) {
		logger::warn("Failed to get Scaleform translator"sv);
		return false;
	}

	std::wstring key_utf16 = util::utf8_to_utf16(a_key).value_or(L""s);
	RE::GFxWStringBuffer result;

	RE::GFxTranslator::TranslateInfo translateInfo;
	translateInfo.key = key_utf16.c_str();
	translateInfo.result = std::addressof(result);

	translator->Translate(std::addressof(translateInfo));
	if (result.empty()) {
		return false;
	}

	std::string result_utf8 = util::utf16_to_utf8(result.c_str()).value_or(""s);

	a_result = result_utf8;
	return true;
}
