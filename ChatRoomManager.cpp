#include <wx/app.h>
#include <wx/splitter.h> 
#include <sstream>
#include <iostream>
#include "ChatRoomManager.h"
#include "ChatFrame.h"

ChatRoomManager::ChatRoomManager(ChatClient& client) : client(client) 
{
    OutputDebugStringA("ChatRoomManager 생성\n");
    this->client.onMessageReceived = [this](const std::string& msg) 
        {
        HandleIncomingMessage(msg);
        };

	this->client.StartReceiving(); // 메시지 수신 시작
    OutputDebugStringA("메시지 수신 시작\n");

}

void ChatRoomManager::OpenRoom(const std::string& roomId)
{
    if (chatFrames.count(roomId))
    {
        OutputDebugStringA("채팅방이 이미 열려있습니다.\n");
        chatFrames[roomId]->Raise(); // 기존 창 띄우기
        return;
    }

    ChatFrame* frame = new ChatFrame(client, roomId, nullptr, this);
    frame->Show();
    chatFrames[roomId] = frame;

    // 서버에 방 참여 메시지 전송
    client.JoinRoom(roomId);
    OutputDebugStringA(("JOIN_ROOM 메시지를 보냈습니다: " + roomId + "\n").c_str());

    OutputDebugStringA("채팅방을 열었습니다.\n");

}

void ChatRoomManager::CloseRoom(const std::string& roomId)
{
    auto it = chatFrames.find(roomId);
    if (it != chatFrames.end())
    {
        wxWindow* frame = it->second;
        wxTheApp->CallAfter([frame]() {
            frame->Destroy();
            OutputDebugStringA("CloseRoom-Destroy 호출됨");
            });
        OutputDebugStringA(("CloseRoom - 방 ID: " + roomId + "\n").c_str());

        chatFrames.erase(it);

        if (chatFrames.count(roomId) == 0)
        {
            char buffer[256];
            snprintf(buffer, sizeof(buffer), "roomId '%s' 없음!\n", roomId.c_str());
            OutputDebugStringA(buffer);
        }
    }
}

void ChatRoomManager::LeaveRoom(const std::string& roomId)
{
	// LEAVE_ROOM:방ID
    client.Send("LEAVE_ROOM:" + roomId);	// 서버에 방 퇴장 메시지 전송
	CloseRoom(roomId); // 채팅방 닫기  
}

void ChatRoomManager::HandleIncomingMessage(const std::string& msg) 
{
    OutputDebugStringA(("서버로부터: " + msg + "\n").c_str());
    if (msg.rfind("ROOMMSG:", 0) == 0)
    {
        HandleRoomMessage(msg);
    }
    else if (msg.rfind("SYSTEM:", 0) == 0)
    {
        HandleSystemMessage(msg);
    }
    else if (msg.rfind("USER_LIST:", 0) == 0)
    {
        HandleUserListMessage(msg);
    }
}

void ChatRoomManager::HandleRoomMessage(const std::string& msg)
{
    OutputDebugStringA(("서버로부터: " + msg + "\n").c_str());
	// 메시지 포맷 예: ROOMMSG:roomId:16:24:정안:안녕하세요~

    std::string data = msg.substr(strlen("ROOMMSG:"));
    size_t p1 = data.find(':');
    size_t p2 = data.find(':', p1 + 1);
    size_t p3 = data.find(':', p2 + 1);
    size_t p4 = data.find(':', p3 + 1);

    std::string roomId = data.substr(0, p1);
    std::string hour = data.substr(p1 + 1, p2 - p1 - 1);
    std::string minute = data.substr(p2 + 1, p3 - p2 - 1);
    std::string sender = data.substr(p3 + 1, p4 - p3 - 1);
    std::string content = data.substr(p4 + 1);

    std::string timeStr = hour + ":" + minute;
    
	std::string message = "[" + timeStr + "] " + sender + ": " + content;

    if (chatFrames.count(roomId)) // 방이 열려있다면
    {
        wxTheApp->CallAfter([=]
            {
                chatFrames[roomId]->AppendMessage(sender, message); // 메시지 추가
            });
    }

    if (!chatFrames.count(roomId)) {
        // 절대로 방을 다시 열면 안됨!
        OutputDebugStringA(("수신된 메시지인데 방이 닫혀 있음. roomId: " + roomId + "\n").c_str());
        return;
    }
}

void ChatRoomManager::HandleSystemMessage(const std::string& msg) 
{
	// 메시지 포맷 예: SYSTEM:정안님이 채팅방 [방1]에 입장하셨습니다.
	std::string content = msg.substr(7); // SYSTEM: 메시지에서 SYSTEM: 부분 제거

    // 방 이름 추출
    std::string roomName;
    size_t leftBracket = content.find('[');
    size_t rightBracket = content.find(']');
    if (leftBracket != std::string::npos && rightBracket != std::string::npos && rightBracket > leftBracket)
    {
        roomName = content.substr(leftBracket + 1, rightBracket - leftBracket - 1); // 방1, 방2 등
    }
    else
    {
        OutputDebugStringA("방 이름 추출 실패: 메시지 형식이 올바르지 않음\n");
        return;
    }

    // 해당 방의 frame에만 메시지 전달
    auto it = chatFrames.find(roomName);
    if (it != chatFrames.end())
    {
        ChatFrame* frame = it->second;
        wxTheApp->CallAfter([=] {
            frame->AppendMessage("SYSTEM", content);
            });
    }
    else
    {
        std::string debugMsg = "SYSTEM 메시지 무시됨: 방이 이미 닫혀 있음 - " + roomName + "\n";
        OutputDebugStringA(debugMsg.c_str());
    }
}

void ChatRoomManager::HandleUserListMessage(const std::string& msg)
{
    // msg 예시: USER_LIST:방ID:이름1,이름2,...

    size_t p1 = msg.find(':');
    size_t p2 = msg.find(':', p1 + 1);
    if (p1 == std::string::npos || p2 == std::string::npos) return;

    std::string roomId = msg.substr(p1 + 1, p2 - p1 - 1); // 방 ID
    std::string userListStr = msg.substr(p2 + 1); // 유저 목록 문자열

    std::istringstream ss(userListStr);
    std::string name;
    std::vector<std::string> users;

    while (std::getline(ss, name, ','))
    {
        if (!name.empty()) users.push_back(name);
    }

    if (chatFrames.count(roomId))
    {
        wxTheApp->CallAfter([=]
            {
                chatFrames[roomId]->UpdateUserList(users);
            });
    }

    if (!chatFrames.count(roomId)) {
        // 절대로 방을 다시 열면 안됨!
        OutputDebugStringA(("수신된 메시지인데 방이 닫혀 있음. roomId: " + roomId + "\n").c_str());
        return;
    }

}

//bool ChatRoomManager::IsRoomOpen(const std::string& roomId) const
//{
//    return chatFrames.count(roomId) > 0; false;
//}
