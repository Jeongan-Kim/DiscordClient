#pragma once
#include <wx/wx.h>
#include "ChatClient.h"

class ChatFrame : public wxFrame {
public:
    ChatFrame(const std::string& nickname);
    ~ChatFrame();
    void OnClose(wxCloseEvent& event);

private:
    wxTextCtrl* chatDisplay; // ä��â
    wxTextCtrl* inputBox;   // �޽��� �ڽ�
    ChatClient client;
    wxListBox* participantList; // ������ ��� â
    wxStaticText* participantCountText; // ������ �ο� ��

    void OnSend(wxCommandEvent& event);
    void AppendMessage(const std::string& msg);
    wxTextAttr GetStyleForMessage(const std::string& msg, const std::string& myNickname); // �۾� �� ���� �Լ�

};
