#include "LoginDialog.h"

LoginDialog::LoginDialog(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, "로그인", wxDefaultPosition, wxSize(300, 150)) {
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    sizer->Add(new wxStaticText(this, wxID_ANY, "닉네임을 입력하세요:"), 0, wxALL, 10);

    nicknameCtrl = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);

    sizer->Add(nicknameCtrl, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    wxButton* okBtn = new wxButton(this, wxID_OK, "확인");
    sizer->Add(okBtn, 0, wxALIGN_CENTER | wxALL, 10);

    nicknameCtrl->Bind(wxEVT_TEXT_ENTER, [=](wxCommandEvent&)
        {
            EndModal(wxID_OK); // 다이얼로그 닫고 OK 반환
        });

    SetSizer(sizer);
    Centre();
}

wxString LoginDialog::GetNickname() const {
    return nicknameCtrl->GetValue();
}
