#include "stdafx.hpp"
#include "DialogTagger.hpp"
#include "ReleaseParser.hpp"
#include "TagWriter.hpp"

static const CDialogResizeHelper::Param resize_data[] =
{
	{ IDC_HEADER_RELEASE, 0, 0, 1, 0},
	{ IDC_HEADER_RELEASE_INFO, 1, 0, 1, 0},
	{ IDC_HEADER_TRACK, 0, 0, 1, 0},
	{ IDC_LIST_RELEASE, 0, 0, 1, 0},
	{ IDC_LIST_TRACK, 0, 0, 1, 1},
	{ IDC_LABEL_ARTIST, 1, 0, 1, 0 },
	{ IDC_EDIT_ARTIST, 1, 0, 1, 0 },
	{ IDC_LABEL_ALBUM, 1, 0, 1, 0 },
	{ IDC_EDIT_ALBUM, 1, 0, 1, 0 },
	{ IDC_LABEL_DATE, 1, 0, 1, 0 },
	{ IDC_EDIT_DATE, 1, 0, 1, 0 },
	{ IDC_LABEL_COUNTRY, 1, 0, 1, 0 },
	{ IDC_EDIT_COUNTRY, 1, 0, 1, 0 },
	{ IDC_LABEL_ORIGINAL_RELEASE_DATE, 1, 0, 1, 0 },
	{ IDC_EDIT_ORIGINAL_RELEASE_DATE, 1, 0, 1, 0 },
	{ IDC_LABEL_LABEL, 1, 0, 1, 0 },
	{ IDC_EDIT_LABEL, 1, 0, 1, 0 },
	{ IDC_LABEL_CATALOG, 1, 0, 1, 0 },
	{ IDC_EDIT_CATALOG, 1, 0, 1, 0 },
	{ IDC_LABEL_BARCODE, 1, 0, 1, 0 },
	{ IDC_EDIT_BARCODE, 1, 0, 1, 0 },
	{ IDC_LABEL_PRIMARY_TYPE, 1, 0, 1, 0 },
	{ IDC_COMBO_PRIMARY_TYPE, 1, 0, 1, 0 },
	{ IDC_LABEL_SECONDARY_TYPES, 1, 0, 1, 0 },
	{ IDC_EDIT_SECONDARY_TYPES, 1, 0, 1, 0 },
	{ IDC_LABEL_STATUS, 1, 0, 1, 0 },
	{ IDC_COMBO_STATUS, 1, 0, 1, 0 },
	{ IDC_LINK, 0, 1, 1, 1 },
	{ IDOK, 1, 1, 1, 1 },
	{ IDCANCEL, 1, 1, 1, 1 },
};

static const CRect resize_min_max(760, 440, 0, 0);
cfg_window_placement_v2 g_window_placement(guids::window_placement);

CDialogTagger::CDialogTagger(const Releases& releases, metadb_handle_list_cref handles)
	: m_releases(releases)
	, m_handles(handles)
	, m_resizer(resize_data, resize_min_max)
	, m_list_track(this)
	, m_dpi(QueryScreenDPIEx(*this).cx)
	, m_current_release(0)
	, m_current_disc(0)
	, m_handle_count(handles.get_count())
	, m_release_count(releases.size())
{
	CSeparator::Register();
}

BOOL CDialogTagger::OnInitDialog(CWindow, LPARAM)
{
	SetIcon(ui_control::get()->get_main_icon());

	CreatePreferencesHeaderFont(m_font, *this);
	GetDlgItem(IDC_HEADER_RELEASE).SetFont(m_font.m_hFont);
	GetDlgItem(IDC_HEADER_RELEASE_INFO).SetFont(m_font.m_hFont);
	GetDlgItem(IDC_HEADER_TRACK).SetFont(m_font.m_hFont);
	m_combo_disc = GetDlgItem(IDC_COMBO_DISC);
	m_link = GetDlgItem(IDC_LINK);

	InitTrackList();
	InitReleaseInfo();
	InitReleaseList();

	m_hooks.AddDialogWithControls(*this);
	g_window_placement.apply_to_window(*this, false);
	ShowWindow(SW_SHOW);
	return TRUE;
}

LRESULT CDialogTagger::OnLink(LPNMHDR pnmh)
{
	const auto link = reinterpret_cast<PNMLINK>(pnmh);
	ShellExecuteW(nullptr, L"open", link->item.szUrl, nullptr, nullptr, SW_SHOW);
	return 0;
}

bool CDialogTagger::listIsColumnEditable(ctx_t, size_t column)
{
	return column > 1 && prefs::bools::write_standard_tags;
}

pfc::string8 CDialogTagger::listGetSubItemText(ctx_t, size_t row, size_t column)
{
	const size_t track_idx = row + (m_current_disc * m_handle_count);
	auto& release = m_releases[m_current_release];
	auto& track = release.tracks[track_idx];

	switch (column)
	{
	case 0:
		return pfc::string_filename_ext(m_handles[row]->get_path());
	case 1:
		return fmt::format("{}.{}", track.discnumber, track.tracknumber);
	case 2:
		if (track.tracknumber == 1 && release.totaldiscs > 1)
		{
			return track.subtitle;
		}
		return "";
	case 3:
		return track.title;
	case 4:
		return track.artist;
	default:
		return "";
	}
}

std::string CDialogTagger::FilterMediaTypes(const Tracks& tracks)
{
	StringSet set;
	Strings ffs;
	for (auto&& track : tracks)
	{
		if (set.emplace(track.media).second)
		{
			ffs.emplace_back(track.media);
		}
	}
	return fmt::format("{}", fmt::join(ffs, ", "));
}

size_t CDialogTagger::listGetItemCount(ctx_t)
{
	return m_handle_count;
}

void CDialogTagger::InitReleaseInfo()
{
	m_edit_album_artist = GetDlgItem(IDC_EDIT_ARTIST);
	m_edit_album = GetDlgItem(IDC_EDIT_ALBUM);
	m_edit_date = GetDlgItem(IDC_EDIT_DATE);
	m_edit_country = GetDlgItem(IDC_EDIT_COUNTRY);
	m_edit_original_release_date = GetDlgItem(IDC_EDIT_ORIGINAL_RELEASE_DATE);
	m_edit_label = GetDlgItem(IDC_EDIT_LABEL);
	m_edit_catalog = GetDlgItem(IDC_EDIT_CATALOG);
	m_edit_barcode = GetDlgItem(IDC_EDIT_BARCODE);
	m_edit_secondary_types = GetDlgItem(IDC_EDIT_SECONDARY_TYPES);

	m_combo_primary_type = GetDlgItem(IDC_COMBO_PRIMARY_TYPE);
	m_combo_status = GetDlgItem(IDC_COMBO_STATUS);

	for (auto&& type : primary_types)
	{
		m_combo_primary_type.AddString(pfc::wideFromUTF8(type.data()));
	}

	for (auto&& status : release_statuses)
	{
		m_combo_status.AddString(pfc::wideFromUTF8(status.data()));
	}

	// Don't allow editing if disabled in Preferences
	if (!prefs::bools::write_standard_tags)
	{
		m_edit_album_artist.EnableWindow(FALSE);
		m_edit_album.EnableWindow(FALSE);
		m_edit_date.EnableWindow(FALSE);
		m_edit_original_release_date.EnableWindow(FALSE);
	}

	if (!prefs::bools::write_label_info)
	{
		m_edit_label.EnableWindow(FALSE);
		m_edit_catalog.EnableWindow(FALSE);
		m_edit_barcode.EnableWindow(FALSE);
	}

	if (!prefs::bools::write_releasetype)
	{
		m_combo_primary_type.EnableWindow(FALSE);
		m_edit_secondary_types.EnableWindow(FALSE);
	}

	m_edit_country.EnableWindow(prefs::bools::write_releasecountry);
	m_combo_status.EnableWindow(prefs::bools::write_releasestatus);
}

void CDialogTagger::InitReleaseList()
{
	m_list_release.CreateInDialog(*this, IDC_LIST_RELEASE);
	m_list_release.SetSelectionModeSingle();
	m_list_release.onSelChange = [this]
	{
		const size_t idx = m_list_release.GetFocusItem();
		if (idx != m_current_release && idx < m_release_count)
		{
			m_current_release = idx;
			UpdateRelease();
		}
	};

	m_list_release.AddColumnAutoWidth("Artist");
	m_list_release.AddColumnAutoWidth("Release");
	m_list_release.AddColumn("Date", MulDiv(80, m_dpi, 96));
	m_list_release.AddColumn("Country", MulDiv(60, m_dpi, 96));
	m_list_release.AddColumnAutoWidth("Label");
	m_list_release.AddColumnAutoWidth("Cat#");
	m_list_release.AddColumn("Media", MulDiv(100, m_dpi, 96));
	m_list_release.AddColumn("Discs", MulDiv(40, m_dpi, 96));

	m_list_release.SetItemCount(m_release_count);

	for (const size_t i : std::views::iota(0U, m_release_count))
	{
		auto& release = m_releases[i];
		m_list_release.SetItemText(i, artist_column, release.album_artist.c_str());
		m_list_release.SetItemText(i, release_column, release.title.c_str());
		m_list_release.SetItemText(i, date_column, release.date.c_str());
		m_list_release.SetItemText(i, country_column, release.country.c_str());
		m_list_release.SetItemText(i, label_column, release.label.c_str());
		m_list_release.SetItemText(i, catalog_column, release.catalog.c_str());
		m_list_release.SetItemText(i, media_column, FilterMediaTypes(release.tracks).c_str());
		m_list_release.SetItemText(i, discs_column, pfc::format_uint(release.totaldiscs));
	}

	UpdateRelease();
}

void CDialogTagger::InitTrackList()
{
	m_list_track.CreateInDialog(*this, IDC_LIST_TRACK);
	m_list_track.SetSelectionModeNone();
	m_list_track.AddColumn("Filename", MulDiv(230, m_dpi, 96));
	m_list_track.AddColumn("#", MulDiv(40, m_dpi, 96), HDF_RIGHT);
	m_list_track.AddColumn("Disc Subtitle", MulDiv(120, m_dpi, 96));
	m_list_track.AddColumnAutoWidth("Title");
}

void CDialogTagger::OnAlbumUpdate(uint32_t, int, CWindow)
{
	m_releases[m_current_release].title = pfc::getWindowText(m_edit_album);
	m_list_release.SetItemText(m_current_release, release_column, m_releases[m_current_release].title.c_str());
}

void CDialogTagger::OnArtistUpdate(uint32_t, int, CWindow)
{
	m_releases[m_current_release].album_artist = pfc::getWindowText(m_edit_album_artist);
	m_list_release.SetItemText(m_current_release, artist_column, m_releases[m_current_release].album_artist.c_str());
}

void CDialogTagger::OnBarcodeUpdate(uint32_t, int, CWindow)
{
	m_releases[m_current_release].barcode = pfc::getWindowText(m_edit_barcode);
}

void CDialogTagger::OnCancel(uint32_t, int, CWindow)
{
	g_window_placement.read_from_window(*this);
	DestroyWindow();
}

void CDialogTagger::OnCatalogUpdate(uint32_t, int, CWindow)
{
	m_releases[m_current_release].catalog = pfc::getWindowText(m_edit_catalog);
	m_list_release.SetItemText(m_current_release, catalog_column, m_releases[m_current_release].catalog.c_str());
}

void CDialogTagger::OnCountryUpdate(uint32_t, int, CWindow)
{
	m_releases[m_current_release].country = pfc::getWindowText(m_edit_country);
	m_list_release.SetItemText(m_current_release, country_column, m_releases[m_current_release].country.c_str());
}

void CDialogTagger::OnDateUpdate(uint32_t, int, CWindow)
{
	m_releases[m_current_release].date = pfc::getWindowText(m_edit_date);
	m_list_release.SetItemText(m_current_release, date_column, m_releases[m_current_release].date.c_str());
}

void CDialogTagger::OnDiscChange(uint32_t, int, CWindow)
{
	m_current_disc = m_combo_disc.GetCurSel();
	UpdateTracks();
}

void CDialogTagger::OnLabelUpdate(uint32_t, int, CWindow)
{
	m_releases[m_current_release].label = pfc::getWindowText(m_edit_label);
	m_list_release.SetItemText(m_current_release, label_column, m_releases[m_current_release].label.c_str());
}

void CDialogTagger::OnOk(uint32_t, int, CWindow)
{
	auto& release = m_releases[m_current_release];
	auto view = release.tracks | std::views::drop(m_current_disc * m_handle_count) | std::views::take(m_handle_count);
	release.tracks = std::ranges::to<Tracks>(view);
	TagWriter(m_handles, release).write();

	g_window_placement.read_from_window(*this);
	DestroyWindow();
}

void CDialogTagger::OnOriginalDateUpdate(uint32_t, int, CWindow)
{
	m_releases[m_current_release].original_release_date = pfc::getWindowText(m_edit_original_release_date);
}

void CDialogTagger::OnSecondaryTypesUpdate(uint32_t, int, CWindow)
{
	m_releases[m_current_release].secondary_types = pfc::getWindowText(m_edit_secondary_types);
}

void CDialogTagger::OnStatusChange(uint32_t, int, CWindow)
{
	m_releases[m_current_release].status = get_status_str(m_combo_status.GetCurSel());
}

void CDialogTagger::OnTypeChange(uint32_t, int, CWindow)
{
	m_releases[m_current_release].primary_type = get_primary_type_str(m_combo_primary_type.GetCurSel());
}

void CDialogTagger::UpdateRelease()
{
	auto& release = m_releases[m_current_release];

	set_window_text(m_edit_album_artist, release.album_artist);
	set_window_text(m_edit_album, release.title);
	set_window_text(m_edit_date, release.date);
	set_window_text(m_edit_country, release.country);
	set_window_text(m_edit_original_release_date, release.original_release_date);
	set_window_text(m_edit_label, release.label);
	set_window_text(m_edit_catalog, release.catalog);
	set_window_text(m_edit_barcode, release.barcode);
	set_window_text(m_edit_secondary_types, release.secondary_types);

	m_combo_primary_type.SetCurSel(get_primary_type_index(release.primary_type));
	m_combo_status.SetCurSel(get_status_index(release.status));

	m_combo_disc.ResetContent();

	if (release.partial_lookup_matches > 0)
	{
		for (const size_t i : std::views::iota(0U, release.partial_lookup_matches))
		{
			auto& track = release.tracks[i * m_handle_count];
			const std::string tmp = fmt::format("Disc {} of {}", track.discnumber, release.totaldiscs);
			m_combo_disc.AddString(pfc::wideFromUTF8(tmp.data()));
		}
	}
	else
	{
		if (release.totaldiscs == 1)
		{
			m_combo_disc.AddString(L"Showing the only disc");
		}
		else
		{
			const std::string tmp = fmt::format("Showing all {} discs", release.totaldiscs);
			m_combo_disc.AddString(pfc::wideFromUTF8(tmp.data()));
		}
	}

	m_current_disc = 0;
	m_combo_disc.SetCurSel(static_cast<int>(m_current_disc));

	if (release.is_various)
	{
		if (m_list_track.GetColumnCount() == 4)
		{
			m_list_track.AddColumn("Track Artist", MulDiv(250, m_dpi, 96));
		}
	}
	else if (m_list_track.GetColumnCount() == 5)
	{
		m_list_track.DeleteColumn(4, false);
	}

	UpdateTracks();

	const std::string url = fmt::format("{}/release/{}", prefs::get_server(), release.albumid);
	const std::string link_str = fmt::format("<a href=\"{0}\">{0}</a>", url);
	set_window_text(m_link, link_str);
}

void CDialogTagger::UpdateTracks()
{
	if (m_list_track.TableEdit_IsActive())
	{
		m_list_track.TableEdit_Abort(false);
	}
	m_list_track.ReloadData();
}

void CDialogTagger::listSetEditField(ctx_t, size_t row, size_t column, const char* value)
{
	const size_t track_idx = row + (m_current_disc * m_handle_count);
	auto& track = m_releases[m_current_release].tracks[track_idx];

	switch (column)
	{
	case 2:
		track.subtitle = value;
		break;
	case 3:
		track.title = value;
		break;
	case 4:
		track.artist = value;
		break;
	}
}

void CDialogTagger::listSubItemClicked(ctx_t, size_t row, size_t column)
{
	const size_t track_idx = row + (m_current_disc * m_handle_count);
	auto& release = m_releases[m_current_release];
	auto& track = release.tracks[track_idx];

	const bool subtitle_edit = prefs::bools::write_standard_tags && column == 2 && release.totaldiscs > 1 && track.tracknumber == 1;
	const bool standard_edit = prefs::bools::write_standard_tags && column > 2;
	if (subtitle_edit || standard_edit)
	{
		m_list_track.TableEdit_Start(row, column);
	}
}
