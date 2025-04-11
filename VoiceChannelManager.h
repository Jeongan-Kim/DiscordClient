#pragma once
#include <map>
#include <string>
#include <unordered_map>
#include <set>
#include "ChatFrame.h"
#include "ChatClient.h"
#include "ChatRoomManager.h"

// ���� ���� ä�� ���� ���¸� ����
// �� ����ä�� ���� �� ���� ���� ���� �ڵ� ����
// ��ư ���� ����ȭ(����ũ, ����) : ���� ä�ο� ���� �濡�� ��ư Ȱ��ȭ, ������ ���� ��Ȱ��ȭ
// ChatFrame UI�� �˸� : ��ư ���³� ������ ǥ�� ������Ʈ

class VoiceChannelManager {
public:
	VoiceChannelManager() = default; // �⺻ ������

	void JoinVoiceChannel(ChatFrame* frame, std::string roomId, std::string clientId); // ���� ä�� ����
	void LeaveVoiceChannel(ChatFrame* frame, std::string roomId, std::string clientId); // ���� ä�� ����

	void UpdateVoiceUserList(const std::string& roomId, const std::vector<std::string>& users); // ���� ä�� ���� ��� ������Ʈ
	ChatFrame* GetCurrentVoiceFrame() const; // ���� ���� ä�� ���� ���� ������ ��ȯ
	std::vector<std::string> GetUsersInVoice(const std::string& roomId) const; // ���� ����ä�ο� �������� ���� ���

	static VoiceChannelManager& GetInstance(); // ��𼭵� ������ �� �ֵ��� �̱��� �������� ����

	void CachePendingVoiceUpdate(const std::string& roomId, const std::vector<std::string> users); // ��� ���� ���� ä�� ������Ʈ ĳ��
	bool HasPendingVoiceUpdate(const std::string& roomId) const; // ��� ���� ���� ä�� ������Ʈ Ȯ��
	std::vector<std::string> ConsumePendingUpdate(const std::string& roomId); // ��� ���� ���� ä�� ������Ʈ �Һ�


private:
	ChatFrame* currentVoiceFrame = nullptr; // ���� ���� ä�� ���� ���� ������
	std::string currentVoiceRoomId; // ���� ���� ä�� ID
	ChatFrame* nextVoiceFrame = nullptr;
	std::string nextVoiceRoomId;

	// roomId -> userId ��� (�� �濡 ����ä�� ���� ������)
	std::unordered_map<std::string, std::set<std::string>> voiceParticipants;

	// userId -> ChatFrame ������ (UI ���ſ�)
	std::unordered_map<std::string, ChatFrame*> userFrames;

	std::unordered_map<std::string, std::vector<std::string>> pendingVoiceUpdates; // ��� ���� ���� ä�� ������Ʈ(�غ� �ȵż� ������Ʈ ���� �͵�)
};
