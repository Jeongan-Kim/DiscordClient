#pragma once
#include <map>
#include <string>
#include <unordered_map>
#include <set>
#include "ChatFrame.h"

// ���� ���� ä�� ���� ���¸� ����
// �� ����ä�� ���� �� ���� ���� ���� �ڵ� ����
// ��ư ���� ����ȭ(����ũ, ����) : ���� ä�ο� ���� �濡�� ��ư Ȱ��ȭ, ������ ���� ��Ȱ��ȭ
// ChatFrame UI�� �˸� : ��ư ���³� ������ ǥ�� ������Ʈ

class VoiceChannelManager {
public:
	void JoinVoiceChannel(ChatFrame* frame); // ���� ä�� ����
	void LeaveVoiceChannel(ChatFrame* frame); // ���� ä�� ����
	ChatFrame* GetCurrentVoiceFrame() const; // ���� ���� ä�� ���� ���� ������ ��ȯ
	std::vector<std::string> GetUsersInVoice(const std::string& roomId) const; // ���� ����ä�ο� �������� ���� ���

	static VoiceChannelManager& GetInstance(); // ��𼭵� ������ �� �ֵ��� �̱��� �������� ����

private:
	VoiceChannelManager() = default;
	ChatFrame* currentVoiceFrame = nullptr; // ���� ���� ä�� ���� ���� ������

	// roomId -> userId ��� (�� �濡 ����ä�� ���� ������)
	std::unordered_map<std::string, std::set<std::string>> voiceParticipants;

	// userId -> ChatFrame ������ (UI ���ſ�)
	std::unordered_map<std::string, ChatFrame*> userFrames;



	//std::string GetUserIdFromFrame(ChatFrame* frame) const;
};
