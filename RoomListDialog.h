#pragma once
#include <wx/wx.h>
#include <wx/listbox.h>
#include "ChatClient.h"
#include "ChatRoomManager.h"
#include "RoomCreateDialog.h"

//wxFrame : wxWidgets�� �⺻ ������ Ŭ����, �� ä�ù��� ���� ������ â
//wxDialog : ��ȭ����, ����ڿ� ��ȣ�ۿ��ϴ� â, �Ͻ����� �Է��� �޴� â�� �ַ� ���

class RoomListDialog : public wxFrame
{
public:
    RoomListDialog(wxWindow* parent);
   
    wxString GetSelectedRoom() const;

    void RefreshRoomList(std::unordered_map<std::string, std::string> roomsInfo); // �� ��� ����
    void OnClose(wxCloseEvent& event);

	void CreateRoom(wxString roomName, wxString password, bool isPasswordProtected);

private:
    void OnJoinClicked(wxCommandEvent& event);
    void OnRoomCreateButtonClicked(wxCommandEvent& event);
	void OnRefreshButtonClicked(wxCommandEvent& event);

    wxListBox* roomListBox;
	//wxListCtrl* roomListBox; // �� ����� ǥ���ϴ� ����Ʈ �ڽ�
    wxButton* joinButton;
    wxButton* createButton;
	wxButton* refreshButton;
    wxString selectedRoom;

    wxDECLARE_EVENT_TABLE();
};
