#include "RoomCreateDialog.h"


wxBEGIN_EVENT_TABLE(RoomCreateDialog, wxDialog)
EVT_BUTTON(wxID_OK, RoomCreateDialog::OnRoomCreateButtonClicked)
EVT_BUTTON(wxID_CANCEL, RoomCreateDialog::OnClose)
wxEND_EVENT_TABLE()

RoomCreateDialog::RoomCreateDialog(wxWindow* parent) : wxDialog(parent, wxID_ANY, "방 생성", wxDefaultPosition, wxSize(300, 200))
{
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

	roomNameCtrl = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
	roomPasswordCtrl = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
	passwordCheckBox = new wxCheckBox(this, wxID_ANY, "비밀번호 설정");
	createButton = new wxButton(this, wxID_OK, "방 생성");
	cancelButton = new wxButton(this, wxID_CANCEL, "취소");

	sizer->Add(new wxStaticText(this, wxID_ANY, "방 이름:"), 0, wxALL, 5);
	sizer->Add(roomNameCtrl, 0, wxALL | wxEXPAND, 5);
	sizer->Add(passwordCheckBox, 0, wxALL, 5);

	sizer->Add(new wxStaticText(this, wxID_ANY, "비밀번호:"), 0, wxALL, 5);
	sizer->Add(roomPasswordCtrl, 0, wxALL | wxEXPAND, 5);

	sizer->Add(createButton, 1, wxALL | wxALIGN_CENTER, 5);
	sizer->Add(cancelButton, 1, wxALL | wxALIGN_CENTER, 5);

	SetSizerAndFit(sizer);

	Show();
}

void RoomCreateDialog::OnRoomCreateButtonClicked(wxCommandEvent& event)
{
	wxString roomName = roomNameCtrl->GetValue();
	wxString roomPassword = roomPasswordCtrl->GetValue();
	bool isPasswordProtected = passwordCheckBox->IsChecked();

	// 방 이름과 비밀번호가 비어있지 않은지 확인
	if (roomName.IsEmpty())
	{
		wxMessageBox("방 이름을 입력하세요.", "오류", wxOK | wxICON_ERROR);
		return;
	}
	if (isPasswordProtected && roomPassword.IsEmpty())
	{
		wxMessageBox("비밀번호를 입력하세요.", "오류", wxOK | wxICON_ERROR);
		return;
	}

	// 방 생성 요청을 RoomListDialog(parent)에 전송
	RoomListDialog* parent = dynamic_cast<RoomListDialog*>(this->GetParent()); // dynamic_cast가 static_cast보다 안전함
	if (parent)
	{
		parent->CreateRoom(roomName, roomPassword, isPasswordProtected);
		EndModal(wxID_CANCEL);
	}
}

void RoomCreateDialog::OnClose(wxCommandEvent& event)
{
	EndModal(wxID_CANCEL); // 강제로 모달 종료
}
