#pragma once
#include <wx/wx.h>

class LoginDialog : public wxDialog {
public:
    LoginDialog(wxWindow* parent);

    wxString GetID() const;
    wxString GetPassword() const;
    bool IsSignupRequested() const;

private:
    wxTextCtrl* idCtrl;
    wxTextCtrl* pwCtrl;
    bool signupRequested = false;
};