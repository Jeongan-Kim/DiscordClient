#include "LoginDialog.h"

LoginDialog::LoginDialog(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, "로그인", wxDefaultPosition, wxSize(300, 150)), signupRequested(false)
{
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

	//닉네임 입력창
    sizer->Add(new wxStaticText(this, wxID_ANY, "아이디:"), 0, wxALL, 5);
    idCtrl = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    sizer->Add(idCtrl, 0, wxEXPAND | wxALL, 5);

	// 비밀번호 입력창
    sizer->Add(new wxStaticText(this, wxID_ANY, "비밀번호:"), 0, wxALL, 5);
    pwCtrl = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD | wxTE_PROCESS_ENTER);
    sizer->Add(pwCtrl, 0, wxEXPAND | wxALL, 5);

	// 버튼 레이아웃
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* loginBtn = new wxButton(this, wxID_OK, "로그인");
    wxButton* signupBtn = new wxButton(this, wxID_ANY, "회원가입");

	// 버튼 크기 조정
    btnSizer->Add(loginBtn, 1, wxALL, 5);
    btnSizer->Add(signupBtn, 1, wxALL, 5);
    sizer->Add(btnSizer, 0, wxALIGN_CENTER);

	// 레이아웃 설정
    SetSizerAndFit(sizer);
	// 레이아웃 중앙 정렬
    Centre();

    // Enter 키로 로그인
    idCtrl->Bind(wxEVT_TEXT_ENTER, [=](wxCommandEvent&) {
        EndModal(wxID_OK);
        });
    pwCtrl->Bind(wxEVT_TEXT_ENTER, [=](wxCommandEvent&) {
        EndModal(wxID_OK);
        });

    // 회원가입 버튼 처리
    signupBtn->Bind(wxEVT_BUTTON, [=](wxCommandEvent&) {
        signupRequested = true;
        EndModal(wxID_OK);
        });
}

wxString LoginDialog::GetID() const
{
    return idCtrl->GetValue();
}

wxString LoginDialog::GetPassword() const
{
    return pwCtrl->GetValue();

}

bool LoginDialog::IsSignupRequested() const
{
    return signupRequested;
}

//wxString LoginDialog::GetNickname() const {
//    return nicknameCtrl->GetValue();
//}
