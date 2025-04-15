#pragma once
#include <wx/wx.h>
#include <wx/listbox.h>
#include "ChatClient.h"
#include "ChatRoomManager.h"
#include "RoomCreateDialog.h"

//wxFrame : wxWidgets의 기본 프레임 클래스, 각 채팅방을 위한 독립된 창
//wxDialog : 대화상자, 사용자와 상호작용하는 창, 일시적인 입력을 받는 창에 주로 사용

class RoomListDialog : public wxFrame
{
public:
    RoomListDialog(wxWindow* parent);
   
    wxString GetSelectedRoom() const;

    void RefreshRoomList(std::unordered_map<std::string, std::string> roomsInfo); // 방 목록 갱신
    void OnClose(wxCloseEvent& event);

	void CreateRoom(wxString roomName, wxString password, bool isPasswordProtected);

private:
    void OnJoinClicked(wxCommandEvent& event);
    void OnRoomCreateButtonClicked(wxCommandEvent& event);
	void OnRefreshButtonClicked(wxCommandEvent& event);

    wxListBox* roomListBox;
	//wxListCtrl* roomListBox; // 방 목록을 표시하는 리스트 박스
    wxButton* joinButton;
    wxButton* createButton;
	wxButton* refreshButton;
    wxString selectedRoom;

    wxDECLARE_EVENT_TABLE();
};
