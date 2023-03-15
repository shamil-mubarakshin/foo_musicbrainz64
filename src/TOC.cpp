#include "stdafx.hpp"
#include "TOC.hpp"
#include "SHA1.hpp"

TOC::TOC(const pfc::array_t<metadb_v2::rec_t>& recs) : m_count(recs.get_count()), m_tracks(100, 0)
{
	const file_info_impl first = recs[0].info->info();
	const uint32_t bits_per_sector = first.info_get_int("samplerate") == 48000 ? 640 : 588;
	calc_pregap(first);

	for (const size_t i : std::views::iota(0U, m_count))
	{
		const uint32_t samples = static_cast<uint32_t>(recs[i].info->info().info_get_length_samples());
		const uint32_t sectors = samples / bits_per_sector;
		const uint32_t sectors_running_total = m_tracks[i + 1] + sectors;

		if (i == m_count - 1)
		{
			m_tracks[0] = sectors_running_total;
		}
		else
		{
			m_tracks[i + 2] = sectors_running_total;
		}
	}
}

std::string TOC::get_discid()
{
	SHA1Context context(m_count, m_tracks);
	return context.RFC822_Binary();
}

std::string TOC::get_url()
{
	auto view = m_tracks | std::views::take(m_count + 1);
	return fmt::format("{}/cdtoc/attach?toc=1 {} {}", prefs::get_server(), m_count, fmt::join(view, " "));
}

void TOC::calc_pregap(const file_info& info)
{
	const char* msf = info.info_get("pregap");
	const auto rx = std::regex("^\\d\\d:\\d\\d:\\d\\d$");
	uint32_t pregap = 150;

	if (msf != nullptr && std::regex_match(msf, rx))
	{
		const Strings parts = split_string(msf, ":");
		const auto m = std::stoul(parts[0]) * 75 * 60;
		const auto s = std::stoul(parts[1]) * 75;
		const auto f = std::stoul(parts[2]);
		pregap += m + s + f;
	}
	m_tracks[1] = pregap;
}
