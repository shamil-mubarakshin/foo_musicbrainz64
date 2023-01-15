#include "stdafx.hpp"
#include "Preferences.hpp"

namespace prefs
{
	static constexpr wil::zstring_view default_server = "https://musicbrainz.org";

	namespace bools
	{
		cfg_bool custom_server(guids::cfg_bool_server, false);

		cfg_bool write_standard_tags(guids::cfg_bool_write_standard_tags, true);
		cfg_bool short_date(guids::cfg_bool_short_date, true);
		cfg_bool ascii_punctuation(guids::cfg_bool_ascii_punctuation, true);
		cfg_bool write_original_date(guids::cfg_bool_write_original_date, true);
		cfg_bool write_artists(guids::cfg_bool_write_artists, true);
		cfg_bool write_artist_sort(guids::cfg_bool_write_artist_sort, true);
		cfg_bool write_performer(guids::cfg_bool_write_performer, true);
		cfg_bool write_composer(guids::cfg_bool_write_composer, true);
		cfg_bool write_ids(guids::cfg_bool_write_ids, true);
		cfg_bool write_releasetype(guids::cfg_bool_write_releasetype, true);
		cfg_bool write_releasestatus(guids::cfg_bool_write_releasestatus, true);
		cfg_bool write_releasecountry(guids::cfg_bool_write_releasecountry, true);
		cfg_bool write_label_info(guids::cfg_bool_write_label_info, true);
		cfg_bool write_media(guids::cfg_bool_write_media, true);
		cfg_bool write_isrc(guids::cfg_bool_write_isrc, true);
	}

	namespace strings
	{
		cfg_string server(guids::cfg_string_server, default_server.data());
	}

	std::string get_server()
	{
		if (bools::custom_server) return strings::server.get().get_ptr();
		return default_server.data();
	}
}

class CDialogPreferences : public CDialogImpl<CDialogPreferences>, public preferences_page_instance
{
public:
	CDialogPreferences(preferences_page_callback::ptr callback) : m_callback(callback) {}

	BEGIN_MSG_MAP_EX(CDialogPreferences)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_CODE_HANDLER_EX(EN_UPDATE, OnChanged)
		COMMAND_RANGE_HANDLER_EX(IDC_CHECK_SERVER, IDC_CHECK_WRITE_ISRC, OnChanged)
	END_MSG_MAP()

	enum { IDD = IDD_DIALOG_PREFERENCES };

	uint32_t get_state() final
	{
		const auto has_changed = [&]()
		{
			for (const auto& [key, value] : m_check_map)
			{
				if (value.check.IsChecked() != *value.setting) return true;
			}

			if (pfc::getWindowText(m_edit_server) != prefs::strings::server.get()) return true;

			return false;
		}();

		uint32_t state = preferences_state::dark_mode_supported | preferences_state::resettable;
		if (has_changed) state |= preferences_state::changed;
		return state;
	}

	void apply() final
	{
		for (auto& [key, value] : m_check_map)
		{
			*value.setting = value.check.IsChecked();
		}

		prefs::strings::server = pfc::getWindowText(m_edit_server);
	}

	void reset() final
	{
		for (auto& [key, value] : m_check_map)
		{
			value.check.SetCheck(key != IDC_CHECK_SERVER);
		}

		set_window_text(m_edit_server, prefs::default_server);
		m_edit_server.EnableWindow(false);

		m_callback->on_state_changed();
	}

private:
	struct check_cfg
	{
		cfg_bool* setting{};
		CCheckBox check;
	};

	BOOL OnInitDialog(CWindow, LPARAM)
	{
		using namespace prefs;

		m_check_map[IDC_CHECK_SERVER] = { &bools::custom_server };
		m_check_map[IDC_CHECK_WRITE_STANDARD_TAGS] = { &bools::write_standard_tags };
		m_check_map[IDC_CHECK_SHORT_DATE] = { &bools::short_date };
		m_check_map[IDC_CHECK_ASCII_PUNCTUATION] = { &bools::ascii_punctuation };
		m_check_map[IDC_CHECK_WRITE_ORIGINAL_DATE] = { &bools::write_original_date };
		m_check_map[IDC_CHECK_WRITE_ARTISTS] = { &bools::write_artists };
		m_check_map[IDC_CHECK_WRITE_ARTIST_SORT] = { &bools::write_artist_sort, };
		m_check_map[IDC_CHECK_WRITE_PERFORMER] = { &bools::write_performer };
		m_check_map[IDC_CHECK_WRITE_COMPOSER] = { &bools::write_composer };
		m_check_map[IDC_CHECK_WRITE_IDS] = { &bools::write_ids };
		m_check_map[IDC_CHECK_WRITE_RELEASETYPE] = { &bools::write_releasetype };
		m_check_map[IDC_CHECK_WRITE_RELEASESTATUS] = { &bools::write_releasestatus };
		m_check_map[IDC_CHECK_WRITE_RELEASECOUNTRY] = { &bools::write_releasecountry };
		m_check_map[IDC_CHECK_WRITE_LABEL_INFO] = { &bools::write_label_info };
		m_check_map[IDC_CHECK_WRITE_MEDIA] = { &bools::write_media };
		m_check_map[IDC_CHECK_WRITE_ISRC] = { &bools::write_isrc };

		for (auto& [key, value] : m_check_map)
		{
			value.check = GetDlgItem(key);
			value.check.SetCheck(*value.setting);
		}

		m_edit_server = GetDlgItem(IDC_EDIT_SERVER);
		m_edit_server.EnableWindow(prefs::bools::custom_server);
		set_window_text(m_edit_server, prefs::strings::server.get());

		m_hooks.AddDialogWithControls(*this);
		return FALSE;
	}

	void OnChanged(uint32_t, int, CWindow)
	{
		m_edit_server.EnableWindow(m_check_map[IDC_CHECK_SERVER].check.IsChecked());
		m_callback->on_state_changed();
	}

	CEdit m_edit_server;
	fb2k::CCoreDarkModeHooks m_hooks;
	preferences_page_callback::ptr m_callback;
	std::map<int, check_cfg> m_check_map;
};

class PreferencesPageImpl : public preferences_page_impl<CDialogPreferences>
{
public:
	GUID get_guid() final
	{
		return guids::preferences_page;
	}

	GUID get_parent_guid() final
	{
		return preferences_page::guid_tools;
	}

	bool get_help_url(pfc::string_base& out) final
	{
		out = Component::home_page;
		return true;
	}

	const char* get_name() final
	{
		return Component::name.data();
	}
};

FB2K_SERVICE_FACTORY(PreferencesPageImpl);
