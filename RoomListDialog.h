#pragma once
#include <wx/wx.h>
#include <wx/listbox.h>
#include "ChatClient.h"
#include "ChatRoomManager.h"

//wxFrame : wxWidgets�� �⺻ ������ Ŭ����, �� ä�ù��� ���� ������ â
//wxDialog : ��ȭ����, ����ڿ� ��ȣ�ۿ��ϴ� â, �Ͻ����� �Է��� �޴� â�� �ַ� ���

class RoomListDialog : public wxFrame
{
public:
	// ������
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
