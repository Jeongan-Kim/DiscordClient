#pragma once
#include <wx/wx.h>
#include "ChatClient.h"

class ChatFrame : public wxFrame {
public:
    ChatFrame(const std::string& nickname);
    ~ChatFrame();
    void OnClose(wxCloseEvent& event);

private:
    wxTextCtrl* chatDisplay; // 채팅창
    wxTextCtrl* inputBox;   // 메시지 박스
    ChatClient client;
    wxListBox* participantList; // 참여자 목록 창
    wxStaticText* participantCountText; // 참여자 인원 수

    void OnSend(wxCommandEvent& event);
    void AppendMessage(const std::string& msg);
    wxTextAttr GetStyleForMessage(const std::string& msg, const std::string& myNickname); // 글씨 색 설정 함수

};
