#pragma once
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "ChatFrame.h"

// ChatFrame을 생성/ 관리함
// ChatClient에서 수신한 메시지를 ChatFrame에 전달하는 역할

class ChatRoomManager 
{
public:
    ChatRoomManager();

	void OpenRoom(const std::string& roomId);   // 채팅방 입장, ChatFrame 생성
	void CloseRoom(const std::string& roomId);  // 채팅방 종료, ChatFrame 삭제
	void LeaveRoom(const std::string& roomId);  // 채팅방 퇴장 요청 서버로 전송
    void HandleIncomingMessage(const std::string& msg);

    void HandleSystemMessage(const std::string& msg);
    void HandleRoomMessage(const std::string& msg);
    void HandleUserListMessage(const std::string& msg);
    void HandleVoiceListMessage(const std::string& msg);

    static ChatRoomManager& GetInstance(); // 어디서든 접근할 수 있도록 싱글톤 패턴으로 구현

	std::map<std::string, ChatFrame*> GetChatFrames() const { return chatFrames; } // 열려있는 채팅방 목록 반환

private:
    std::map<std::string, ChatFrame*> chatFrames;  // roomId -> ChatFrame
};
