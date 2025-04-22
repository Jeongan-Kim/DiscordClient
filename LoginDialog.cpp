#include "LoginDialog.h"

LoginDialog::LoginDialog(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, "로그인", wxDefaultPosition, wxSize(300, 150))/*, signupRequested(false)*/
{
    Bind(wxEVT_CLOSE_WINDOW, &LoginDialog::OnClose, this);

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
    idCtrl->Bind(wxEVT_TEXT_ENTER, [=](wxCommandEvent&) { AttemptLogin(); });
    pwCtrl->Bind(wxEVT_TEXT_ENTER, [=](wxCommandEvent&) { AttemptLogin(); });
    loginBtn->Bind(wxEVT_BUTTON, [=](wxCommandEvent&) { AttemptLogin(); });

    // 회원가입 버튼 처리
    signupBtn->Bind(wxEVT_BUTTON, [=](wxCommandEvent&) { AttemptRegister(); });

    idCtrl->SetFocus();
    // 한영키 토글
    keybd_event(VK_HANGUL, 0, 0, 0);
    keybd_event(VK_HANGUL, 0, KEYEVENTF_KEYUP, 0);
}

void LoginDialog::AttemptLogin()
{
    wxString id = idCtrl->GetValue();
    wxString pw = pwCtrl->GetValue();

    if (id.IsEmpty() || pw.IsEmpty()) {
        wxMessageBox("아이디와 비밀번호를 입력하세요.", "오류", wxOK | wxICON_WARNING);
        return;
    }

    switch (ServerAPI::Login(id.ToStdString(), pw.ToStdString()))
    {
    case LoginResult::LOGIN_SUCCESS:
        EndModal(wxID_OK);
        break;
    case LoginResult::LOGIN_NO_ID:
        wxMessageBox("존재하지 않는 아이디입니다.", "로그인 실패", wxOK | wxICON_WARNING);
        break;
    case LoginResult::LOGIN_WRONG_PW:
        wxMessageBox("비밀번호가 틀렸습니다.", "로그인 실패", wxOK | wxICON_WARNING);
        break;
    case LoginResult::LOGIN_FORMAT_ERROR:
        wxMessageBox("로그인 요청 형식이 틀렸습니다.", "로그인 실패", wxOK | wxICON_WARNING);
        break;
    case LoginResult::LOGIN_CONNECT_ERROR:
        wxMessageBox("서버와의 연결이 끊겼습니다.", "로그인 실패", wxOK | wxICON_WARNING);
        break;
    case LoginResult::LOGIN_ALREADY:
        wxMessageBox("이미 로그인 중인 아이디입니다.", "로그인 실패", wxOK | wxICON_WARNING);
        break;
    default:
        wxMessageBox("로그인에 실패했습니다.", "로그인 실패", wxOK | wxICON_ERROR);
        break;
    }
}

void LoginDialog::AttemptRegister()
{
    RegisterDialog regDialog(this); // 부모를 로그인창으로 설정
    if (regDialog.ShowModal() == wxID_OK)
    {
        wxString id = regDialog.GetID();
        wxString pw = regDialog.GetPassword();

        if (ServerAPI::Register(id.ToStdString(), pw.ToStdString()))
        {
            wxMessageBox("회원가입 성공!", "알림", wxOK | wxICON_INFORMATION);
        }
        else
        {
            wxMessageBox("회원가입 실패!", "오류", wxOK | wxICON_ERROR);
        }
    }
}

wxString LoginDialog::GetID() const
{
    return idCtrl->GetValue();
}

wxString LoginDialog::GetPassword() const
{
    return pwCtrl->GetValue();

}

//bool LoginDialog::IsSignupRequested() const
//{
//    return signupRequested;
//}

void LoginDialog::OnClose(wxCloseEvent& event)
{
    ChatClient::GetInstance().Disconnect(); // 프로그램 종료 시에만

    EndModal(wxID_CANCEL); // 강제로 모달 종료
}