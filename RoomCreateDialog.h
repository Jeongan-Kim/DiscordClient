#pragma once
#include <wx/wx.h>
#include <wx/listbox.h>
#include "ChatClient.h"
#include "ChatRoomManager.h"
#include "RoomListDialog.h"

class RoomCreateDialog : public wxDialog
{
public:
	RoomCreateDialog(wxWindow* parent);

	void OnRoomCreateButtonClicked(wxCommandEvent& event); // �� ���� ��ư Ŭ�� �̺�Ʈ �ڵ鷯
	void OnClose(wxCommandEvent& event);

private:
	wxButton* createButton;			// �� ���� ��ư
	wxButton* cancelButton;			// �� ���� ��� ��ư
	wxCheckBox* passwordCheckBox;	// ��й�ȣ ���� ���� üũ�ڽ�
	wxTextCtrl* roomNameCtrl;		// �� �̸� �Է�â
	wxTextCtrl* roomPasswordCtrl;	// �� ��й�ȣ �Է�â	

	wxDECLARE_EVENT_TABLE();

};

