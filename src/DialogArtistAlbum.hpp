#pragma once

class CDialogArtistAlbum : public CDialogImpl<CDialogArtistAlbum>
{
public:
	CDialogArtistAlbum(std::string_view artist, std::string_view album) : m_artist(artist), m_album(album) {}

	BEGIN_MSG_MAP_EX(CDialogArtistAlbum)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_RANGE_HANDLER_EX(IDOK, IDCANCEL, OnCloseCmd)
		COMMAND_CODE_HANDLER_EX(EN_UPDATE, OnUpdate)
	END_MSG_MAP()

	enum { IDD = IDD_DIALOG_ARTIST_ALBUM };

	std::string m_album, m_artist;

private:
	BOOL OnInitDialog(CWindow, LPARAM)
	{
		m_btn_ok = GetDlgItem(IDOK);
		m_edit_artist = GetDlgItem(IDC_EDIT_ARTIST);
		m_edit_album = GetDlgItem(IDC_EDIT_ALBUM);

		set_window_text(m_edit_artist, m_artist);
		set_window_text(m_edit_album, m_album);

		m_hooks.AddDialogWithControls(*this);
		CenterWindow();
		return TRUE;
	}

	void OnCloseCmd(uint32_t, int nID, CWindow)
	{
		m_artist = pfc::getWindowText(m_edit_artist);
		m_album = pfc::getWindowText(m_edit_album);
		EndDialog(nID);
	}

	void OnUpdate(uint32_t, int, CWindow)
	{
		m_btn_ok.EnableWindow(m_edit_artist.GetWindowTextLengthW() > 0 && m_edit_album.GetWindowTextLengthW() > 0);
	}

	CButton m_btn_ok;
	CEdit m_edit_album, m_edit_artist;
	fb2k::CCoreDarkModeHooks m_hooks;
};
