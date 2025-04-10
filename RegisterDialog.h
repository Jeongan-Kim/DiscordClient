#pragma once
#include <wx/wx.h>

class RegisterDialog : public wxDialog
{
public:
    RegisterDialog(wxWindow* parent);

    wxString GetID() const;
    wxString GetPassword() const;
    wxString GetPasswordConfirm() const;

private:
    wxTextCtrl* idCtrl;
    wxTextCtrl* pwCtrl;
    wxTextCtrl* pwConfirmCtrl;

    void OnSubmit(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
};