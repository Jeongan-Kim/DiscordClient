#include "RoomCreateDialog.h"


wxBEGIN_EVENT_TABLE(RoomCreateDialog, wxDialog)
EVT_BUTTON(wxID_OK, RoomCreateDialog::OnRoomCreateButtonClicked)
EVT_BUTTON(wxID_CANCEL, RoomCreateDialog::OnClose)
wxEND_EVENT_TABLE()

RoomCreateDialog::RoomCreateDialog(wxWindow* parent) : wxDialog(parent, wxID_ANY, "�� ����", wxDefaultPosition, wxSize(300, 200))
{
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

	roomNameCtrl = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
	roomPasswordCtrl = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
	passwordCheckBox = new wxCheckBox(this, wxID_ANY, "��й�ȣ ����");
	createButton = new wxButton(this, wxID_OK, "�� ����");
	cancelButton = new wxButton(this, wxID_CANCEL, "���");

	sizer->Add(new wxStaticText(this, wxID_ANY, "�� �̸�:"), 0, wxALL, 5);
	sizer->Add(roomNameCtrl, 0, wxALL | wxEXPAND, 5);
	sizer->Add(passwordCheckBox, 0, wxALL, 5);

	sizer->Add(new wxStaticText(this, wxID_ANY, "��й�ȣ:"), 0, wxALL, 5);
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

	// �� �̸��� ��й�ȣ�� ������� ������ Ȯ��
	if (roomName.IsEmpty())
	{
		wxMessageBox("�� �̸��� �Է��ϼ���.", "����", wxOK | wxICON_ERROR);
		return;
	}
	if (isPasswordProtected && roomPassword.IsEmpty())
	{
		wxMessageBox("��й�ȣ�� �Է��ϼ���.", "����", wxOK | wxICON_ERROR);
		return;
	}

	// �� ���� ��û�� RoomListDialog(parent)�� ����
	RoomListDialog* parent = dynamic_cast<RoomListDialog*>(this->GetParent()); // dynamic_cast�� static_cast���� ������
	if (parent)
	{
		parent->CreateRoom(roomName, roomPassword, isPasswordProtected);
		EndModal(wxID_CANCEL);
	}
}

void RoomCreateDialog::OnClose(wxCommandEvent& event)
{
	EndModal(wxID_CANCEL); // ������ ��� ����
}
