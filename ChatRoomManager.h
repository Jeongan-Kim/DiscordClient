#pragma once
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "ChatFrame.h"
#include "ChatClient.h"


// ChatFrame�� ����/ ������
// ChatClient���� ������ �޽����� ChatFrame�� �����ϴ� ����

class ChatRoomManager 
{
public:
    ChatRoomManager(ChatClient& client);

	void OpenRoom(const std::string& roomId);   // ä�ù� ����, ChatFrame ����
	void CloseRoom(const std::string& roomId);  // ä�ù� ����, ChatFrame ����
	void LeaveRoom(const std::string& roomId);  // ä�ù� ���� ��û ������ ����
    void HandleIncomingMessage(const std::string& msg);

    void HandleSystemMessage(const std::string& msg);
    void HandleRoomMessage(const std::string& msg);
    void HandleUserListMessage(const std::string& msg);

    //bool IsRoomOpen(const std::string& roomId) const;
private:

    ChatClient& client;  // �޽��� ������ �����ϱ� ���� ���� ����
    std::map<std::string, ChatFrame*> chatFrames;  // roomId -> ChatFrame
};
