#pragma once
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "ChatFrame.h"
#include "VoiceEntry.h"
//#include "RoomListDialog.h"

// ChatFrame�� ����/ ������
// ChatClient���� ������ �޽����� ChatFrame�� �����ϴ� ����

class RoomListDialog; // ���� ����


class ChatRoomManager 
{
public:
    ChatRoomManager();

	void SetRoomListDialog(RoomListDialog* dialog) { roomListDialog = dialog; } // �� ��� ��ȭ���� ����

	void OpenRoom(const std::string& roomId);   // ä�ù� ����, ChatFrame ����
	void CloseRoom(const std::string& roomId);  // ä�ù� ����, ChatFrame ����
	void LeaveRoom(const std::string& roomId);  // ä�ù� ���� ��û ������ ����

    void MicSet(const std::string& roomId, bool status);      //����ũ ���� ������ ����
    void HeadsetSet(const std::string& roomId, bool status);  //���� ���� ������ ����

    void HandleIncomingMessage(const std::string& msg);

    void HandleSystemMessage(const std::string& msg);
    void HandleRoomMessage(const std::string& msg);
    void HandleUserListMessage(const std::string& msg);
    void HandleVoiceListMessage(const std::string& msg);
    void HandleRoomsInfoMessage(const std::string& msg);

    static ChatRoomManager& GetInstance(); // ��𼭵� ������ �� �ֵ��� �̱��� �������� ����

	const std::map<std::string, ChatFrame*>& GetChatFrames() const { return chatFrames; } // �����ִ� ä�ù� ��� ��ȯ

private:
    std::map<std::string, ChatFrame*> chatFrames;  // roomId -> ChatFrame
    std::unordered_map<std::string, std::string> roomsInfo;           //�� �̸� -> �� ��й�ȣ 
	RoomListDialog* roomListDialog = nullptr; // �� ��� ��ȭ����
};
