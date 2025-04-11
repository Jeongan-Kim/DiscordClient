#pragma once
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "ChatFrame.h"

// ChatFrame�� ����/ ������
// ChatClient���� ������ �޽����� ChatFrame�� �����ϴ� ����

class ChatRoomManager 
{
public:
    ChatRoomManager();

	void OpenRoom(const std::string& roomId);   // ä�ù� ����, ChatFrame ����
	void CloseRoom(const std::string& roomId);  // ä�ù� ����, ChatFrame ����
	void LeaveRoom(const std::string& roomId);  // ä�ù� ���� ��û ������ ����
    void HandleIncomingMessage(const std::string& msg);

    void HandleSystemMessage(const std::string& msg);
    void HandleRoomMessage(const std::string& msg);
    void HandleUserListMessage(const std::string& msg);
    void HandleVoiceListMessage(const std::string& msg);

    static ChatRoomManager& GetInstance(); // ��𼭵� ������ �� �ֵ��� �̱��� �������� ����

	std::map<std::string, ChatFrame*> GetChatFrames() const { return chatFrames; } // �����ִ� ä�ù� ��� ��ȯ

private:
    std::map<std::string, ChatFrame*> chatFrames;  // roomId -> ChatFrame
};
