#pragma once
#include "ChatClient.h"
#include <wx/wx.h>

class ChatRoomManager; // ���� ����
// ChatRoomManager���� ���� �޽����� �ܼ���UI�� ����ϴ� ���Ҹ� ���
class ChatFrame : public wxFrame
{
public:
    ChatFrame(ChatClient& clientInst, const std::string& roomId, wxWindow* parent, ChatRoomManager* manager);

    void AppendMessage(const std::string& sender, const std::string& text);
    void UpdateUserList(const std::vector<std::string>& users);

private:
    ChatClient& client;
	ChatRoomManager* roomManager; // �� ���� ��ü
    std::string roomId; // �� ID

    wxTextCtrl* chatDisplay; // ä��â
    wxTextCtrl* inputBox;  // �޽��� �ڽ�
    wxListBox* participantList; // ������ ��� â
    wxStaticText* participantCountText; // ������ �ο� ��

    void OnSend(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);
    wxTextAttr GetStyleForMessage(const std::string& sender, const std::string& myNickname, const std::string& part); // �۾� �� ���� �Լ�
};
