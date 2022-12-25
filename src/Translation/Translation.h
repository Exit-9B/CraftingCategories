#pragma once

namespace Translation
{
	void GetCachedString(wchar_t** a_pOut, wchar_t* a_bufIn, std::uint32_t a_unused);

	std::uint32_t ReadLine_w(
		RE::BSResourceNiBinaryStream* a_this,
		wchar_t* a_dst,
		std::uint32_t a_dstLen,
		std::uint32_t a_terminator);

	void ParseTranslation(const std::string& a_name);

	bool Translate(const std::string& a_key, std::string& a_result);
}
