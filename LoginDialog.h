#pragma once
#include <wx/wx.h>
#include "RegisterDialog.h"
#include "ServerAPI.h"

/// �α��� ���̾�α� Ŭ����
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