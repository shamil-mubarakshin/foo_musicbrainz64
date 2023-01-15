#pragma once

class CDialogAlbumID : public CDialogImpl<CDialogAlbumID>
{
public:
	CDialogAlbumID(wil::zstring_view albumid) : m_albumid(albumid)
	{
		m_url = fmt::format("{}/release/", prefs::get_server());
	}

	BEGIN_MSG_MAP_EX(CDialogAlbumID)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_RANGE_HANDLER_EX(IDOK, IDCANCEL, OnCloseCmd)
		COMMAND_CODE_HANDLER_EX(EN_UPDATE, OnUpdate)
	END_MSG_MAP()

	enum { IDD = IDD_DIALOG_ALBUMID };

	std::string m_albumid;

private:
	BOOL OnInitDialog(CWindow, LPARAM)
	{
		m_btn_ok = GetDlgItem(IDOK);
		m_edit_albumid = GetDlgItem(IDC_EDIT_ALBUMID);

		set_window_text(m_edit_albumid, m_albumid);

		m_hooks.AddDialogWithControls(*this);
		CenterWindow();
		return TRUE;
	}

	void OnCloseCmd(uint32_t, int nID, CWindow)
	{
		m_albumid = pfc::getWindowText(m_edit_albumid);
		EndDialog(nID);
	}

	void OnUpdate(uint32_t, int, CWindow)
	{
		const std::string str = pfc::getWindowText(m_edit_albumid).get_ptr();

		if (str.starts_with(m_url))
		{
			set_window_text(m_edit_albumid, str.substr(m_url.length()));
			return;
		}

		m_btn_ok.EnableWindow(is_uuid(str.c_str()));
	}

	CButton m_btn_ok;
	CEdit m_edit_albumid;
	fb2k::CCoreDarkModeHooks m_hooks;
	std::string m_url;
};
