#pragma once
#include <wx/wx.h>
#include "RegisterDialog.h"
#include "ServerAPI.h"

/// 로그인 다이얼로그 클래스
class LoginDialog : public wxDialog {
public:
    LoginDialog(wxWindow* parent);

	void AttemptLogin();
    void AttemptRegister();

    wxString GetID() const;
    wxString GetPassword() const;
    //bool IsSignupRequested() const;

    void OnClose(wxCloseEvent& event);

private:
    wxTextCtrl* idCtrl;
    wxTextCtrl* pwCtrl;
    //bool signupRequested = false;
};