#include "RegisterDialog.h"

wxBEGIN_EVENT_TABLE(RegisterDialog, wxDialog)
EVT_BUTTON(wxID_OK, RegisterDialog::OnSubmit)
wxEND_EVENT_TABLE()

RegisterDialog::RegisterDialog(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, "회원가입", wxDefaultPosition, wxSize(300, 200))
{
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    idCtrl = new wxTextCtrl(this, wxID_ANY);
    pwCtrl = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
    pwConfirmCtrl = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);

    sizer->Add(new wxStaticText(this, wxID_ANY, "아이디:"), 0, wxALL, 5);
    sizer->Add(idCtrl, 0, wxALL | wxEXPAND, 5);
    sizer->Add(new wxStaticText(this, wxID_ANY, "비밀번호:"), 0, wxALL, 5);
    sizer->Add(pwCtrl, 0, wxALL | wxEXPAND, 5);
    sizer->Add(new wxStaticText(this, wxID_ANY, "비밀번호 확인:"), 0, wxALL, 5);
    sizer->Add(pwConfirmCtrl, 0, wxALL | wxEXPAND, 5);

    sizer->Add(new wxButton(this, wxID_OK, "가입"), 0, wxALL | wxALIGN_CENTER, 10);

    SetSizerAndFit(sizer);
}

void RegisterDialog::OnSubmit(wxCommandEvent& event)
{
    if (GetPassword() != GetPasswordConfirm()) {
        wxMessageBox("비밀번호가 일치하지 않습니다.", "오류", wxOK | wxICON_ERROR);
        return;
    }

    EndModal(wxID_OK);
}

wxString RegisterDialog::GetID() const { return idCtrl->GetValue(); }
wxString RegisterDialog::GetPassword() const { return pwCtrl->GetValue(); }
wxString RegisterDialog::GetPasswordConfirm() const { return pwConfirmCtrl->GetValue(); }
