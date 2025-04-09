#pragma once
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "ChatFrame.h"
#include "ChatClient.h"


// ChatFrame을 생성/ 관리함
// ChatClient에서 수신한 메시지를 ChatFrame에 전달하는 역할

class ChatRoomManager 
{
public:
    ChatRoomManager(ChatClient& client);

	void OpenRoom(const std::string& roomId);   // 채팅방 입장, ChatFrame 생성
	void CloseRoom(const std::string& roomId);  // 채팅방 종료, ChatFrame 삭제
	void LeaveRoom(const std::string& roomId);  // 채팅방 퇴장 요청 서버로 전송
    void HandleIncomingMessage(const std::string& msg);

    void HandleSystemMessage(const std::string& msg);
    void HandleRoomMessage(const std::string& msg);
    void HandleUserListMessage(const std::string& msg);

    //bool IsRoomOpen(const std::string& roomId) const;
private:

    ChatClient& client;  // 메시지 수신을 연결하기 위해 참조 보유
    std::map<std::string, ChatFrame*> chatFrames;  // roomId -> ChatFrame
};
