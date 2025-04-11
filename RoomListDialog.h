#pragma once
#include <wx/wx.h>
#include <wx/listbox.h>
#include "ChatClient.h"
#include "ChatRoomManager.h"

//wxFrame : wxWidgets의 기본 프레임 클래스, 각 채팅방을 위한 독립된 창
//wxDialog : 대화상자, 사용자와 상호작용하는 창, 일시적인 입력을 받는 창에 주로 사용

class RoomListDialog : public wxFrame
{
public:
	// 생성자
    RoomListDialog(wxWindow* parent);
   
    wxString GetSelectedRoom() const;

    void OnClose(wxCloseEvent& event);

private:
    void OnJoinClicked(wxCommandEvent& event);

    wxListBox* roomListBox;
    wxButton* joinButton;
    wxString selectedRoom;

    wxDECLARE_EVENT_TABLE();
};
