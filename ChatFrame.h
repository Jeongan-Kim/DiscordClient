#pragma once
#include "ChatClient.h"
#include <wx/wx.h>

class ChatRoomManager; // 전방 선언
// ChatRoomManager에서 받은 메시지를 단순히UI에 출력하는 역할만 담당
class ChatFrame : public wxFrame
{
public:
    ChatFrame(ChatClient& clientInst, const std::string& roomId, wxWindow* parent, ChatRoomManager* manager);

    void AppendMessage(const std::string& sender, const std::string& text);
    void UpdateUserList(const std::vector<std::string>& users);

private:
    ChatClient& client;
	ChatRoomManager* roomManager; // 방 관리 객체
    std::string roomId; // 방 ID

    wxTextCtrl* chatDisplay; // 채팅창
    wxTextCtrl* inputBox;  // 메시지 박스
    wxListBox* participantList; // 참여자 목록 창
    wxStaticText* participantCountText; // 참여자 인원 수

    void OnSend(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);
    wxTextAttr GetStyleForMessage(const std::string& sender, const std::string& myNickname, const std::string& part); // 글씨 색 설정 함수
};
