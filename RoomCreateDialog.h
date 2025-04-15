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

	void OnRoomCreateButtonClicked(wxCommandEvent& event); // 방 생성 버튼 클릭 이벤트 핸들러
	void OnClose(wxCommandEvent& event);

private:
	wxButton* createButton;			// 방 생성 버튼
	wxButton* cancelButton;			// 방 생성 취소 버튼
	wxCheckBox* passwordCheckBox;	// 비밀번호 설정 여부 체크박스
	wxTextCtrl* roomNameCtrl;		// 방 이름 입력창
	wxTextCtrl* roomPasswordCtrl;	// 방 비밀번호 입력창	

	wxDECLARE_EVENT_TABLE();

};

