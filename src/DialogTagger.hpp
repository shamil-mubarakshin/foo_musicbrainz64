#pragma once

class CDialogTagger : public CDialogImpl<CDialogTagger>, private IListControlOwnerDataSource
{
public:
	CDialogTagger(const Releases& releases, metadb_handle_list_cref handles);

	BEGIN_MSG_MAP_EX(CDialogTagger)
		CHAIN_MSG_MAP_MEMBER(m_resizer)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDOK, OnOk)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)
		NOTIFY_HANDLER_EX(IDC_LINK_URL, NM_CLICK, OnLink)
		NOTIFY_HANDLER_EX(IDC_LINK_URL, NM_RETURN, OnLink)

		COMMAND_HANDLER_EX(IDC_COMBO_PRIMARY_TYPE, CBN_SELENDOK, OnTypeChange)
		COMMAND_HANDLER_EX(IDC_COMBO_STATUS, CBN_SELENDOK, OnStatusChange)
		COMMAND_HANDLER_EX(IDC_COMBO_DISC, CBN_SELENDOK, OnDiscChange)

		COMMAND_HANDLER_EX(IDC_EDIT_ARTIST, EN_UPDATE, OnArtistUpdate)
		COMMAND_HANDLER_EX(IDC_EDIT_ALBUM, EN_UPDATE, OnAlbumUpdate)
		COMMAND_HANDLER_EX(IDC_EDIT_DATE, EN_UPDATE, OnDateUpdate)
		COMMAND_HANDLER_EX(IDC_EDIT_COUNTRY, EN_UPDATE, OnCountryUpdate)
		COMMAND_HANDLER_EX(IDC_EDIT_ORIGINAL_RELEASE_DATE, EN_UPDATE, OnOriginalDateUpdate)
		COMMAND_HANDLER_EX(IDC_EDIT_LABEL, EN_UPDATE, OnLabelUpdate)
		COMMAND_HANDLER_EX(IDC_EDIT_CATALOG, EN_UPDATE, OnCatalogUpdate)
		COMMAND_HANDLER_EX(IDC_EDIT_BARCODE, EN_UPDATE, OnBarcodeUpdate)
		COMMAND_HANDLER_EX(IDC_EDIT_SECONDARY_TYPES, EN_UPDATE, OnSecondaryTypesUpdate)
	END_MSG_MAP()

	enum { IDD = IDD_DIALOG_TAGGER };

private:
	bool listIsColumnEditable(ctx_t, size_t column) final;
	pfc::string8 listGetSubItemText(ctx_t, size_t row, size_t column) final;
	size_t listGetItemCount(ctx_t) final;
	void listSetEditField(ctx_t, size_t row, size_t column, const char* value) final;
	void listSubItemClicked(ctx_t, size_t row, size_t column) final;

private:
	enum columns
	{
		artist_column,
		release_column,
		date_column,
		country_column,
		label_column,
		catalog_column,
		media_column,
		discs_column
	};

	BOOL OnInitDialog(CWindow, LPARAM);
	LRESULT OnLink(LPNMHDR pnmh);
	std::string FilterMediaTypes(const Tracks& tracks);
	void InitReleaseInfo();
	void InitReleaseList();
	void InitTrackList();
	void OnAlbumUpdate(uint32_t, int, CWindow);
	void OnArtistUpdate(uint32_t, int, CWindow);
	void OnBarcodeUpdate(uint32_t, int, CWindow);
	void OnCancel(uint32_t, int, CWindow);
	void OnCatalogUpdate(uint32_t, int, CWindow);
	void OnCountryUpdate(uint32_t, int, CWindow);
	void OnDateUpdate(uint32_t, int, CWindow);
	void OnDiscChange(uint32_t, int, CWindow);
	void OnLabelUpdate(uint32_t, int, CWindow);
	void OnOk(uint32_t, int, CWindow);
	void OnOriginalDateUpdate(uint32_t, int, CWindow);
	void OnSecondaryTypesUpdate(uint32_t, int, CWindow);
	void OnStatusChange(uint32_t, int, CWindow);
	void OnTypeChange(uint32_t, int, CWindow);
	void UpdateRelease();
	void UpdateTracks();

	CComboBox m_combo_disc;
	CComboBox m_combo_status;
	CComboBox m_combo_primary_type;
	CDialogResizeHelper m_resizer;
	CEdit m_edit_album_artist;
	CEdit m_edit_album;
	CEdit m_edit_date;
	CEdit m_edit_original_release_date;
	CEdit m_edit_country;
	CEdit m_edit_label;
	CEdit m_edit_catalog;
	CEdit m_edit_barcode;
	CEdit m_edit_secondary_types;
	CFont m_font;
	CListControlSimple m_list_release;
	CListControlOwnerData m_list_track;
	CWindow m_link_url;
	LONG m_dpi{};
	Releases m_releases;
	fb2k::CCoreDarkModeHooks m_hooks;
	metadb_handle_list m_handles;
	size_t m_current_release{};
	size_t m_current_disc{};
	size_t m_handle_count{};
	size_t m_release_count{};
};
