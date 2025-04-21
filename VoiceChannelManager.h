#pragma once
#include <map>
#include <string>
#include <unordered_map>
#include <set>
#include "ChatFrame.h"
#include "ChatClient.h"
#include "ChatRoomManager.h"
#include "VoiceEntry.h"

// ���� ���� ä�� ���� ���¸� ����
// �� ����ä�� ���� �� ���� ���� ���� �ڵ� ����
// ��ư ���� ����ȭ(����ũ, ����) : ���� ä�ο� ���� �濡�� ��ư Ȱ��ȭ, ������ ���� ��Ȱ��ȭ
// ChatFrame UI�� �˸� : ��ư ���³� ������ ǥ�� ������Ʈ
//class AudioIO; // AudioIO Ŭ���� ���� ����
struct ParticipantAudioSettings
{
	float volume = 1.0f;
	bool muted = false;
};

class VoiceChannelManager {
public:
	VoiceChannelManager() = default; // �⺻ ������

	//void SetAudioIO(AudioIO* audio) { this->audioIO = audio; } // AudioIO ����
	bool StartAudioIO(); // ����� ����� ����
	bool EndAudioIO(); // ����� ����� ����

	void JoinVoiceChannel(ChatFrame* frame, std::string roomId, std::string clientId); // ���� ä�� ����
	void LeaveVoiceChannel(ChatFrame* frame, std::string roomId, std::string clientId); // ���� ä�� ����

	void UpdateVoiceUserList(const std::string& roomId, const std::vector<VoiceEntry>& entries); // ���� ä�� ���� ��� ������Ʈ
	std::vector<VoiceEntry> GetUsersInVoice(const std::string& roomId) const; // ���� ����ä�ο� �������� ���� ���

	static VoiceChannelManager& GetInstance(); // ��𼭵� ������ �� �ֵ��� �̱��� �������� ����

	//void CachePendingVoiceUpdate(const std::string& roomId, const std::vector<VoiceEntry>& entries); // ��� ���� ���� ä�� ������Ʈ ĳ��
	//bool HasPendingVoiceUpdate(const std::string& roomId) const; // ��� ���� ���� ä�� ������Ʈ Ȯ��
	//std::vector<std::string> ConsumePendingUpdate(const std::string& roomId); // ��� ���� ���� ä�� ������Ʈ �Һ�

	std::string GetCurrentVoiceRoomId() const { return currentVoiceRoomId; } // ���� ���� ä�� ID ��ȯ
	ChatFrame* GetCurrentVoiceFrame() const; // ���� ���� ä�� ���� ���� ������ ��ȯ

	void SetParticipantVolume(const std::string& roomId, const std::string& clientId, float vol);
	float GetParticipantVolume(const std::string& roomId, const std::string& clientId) const;
	void  SetParticipantMute(const std::string& roomId, const std::string& clientId, bool mute);
	bool  IsParticipantMuted(const std::string& roomId, const std::string& clientId) const;

private:
	//AudioIO* audioIO = nullptr; // AudioIO ��ü ������

	ChatFrame* currentVoiceFrame = nullptr; // ���� ���� ä�� ���� ���� ������
	std::string currentVoiceRoomId; // ���� ���� ä�� ID
	ChatFrame* nextVoiceFrame = nullptr;
	std::string nextVoiceRoomId;

	// roomId -> userId ��� (�� �濡 ����ä�� ���� ������)
	std::unordered_map<std::string, std::vector<VoiceEntry>> voiceParticipants;

	// userId -> ChatFrame ������ (UI ���ſ�)
	std::unordered_map<std::string, ChatFrame*> userFrames;

	std::unordered_map<std::string, std::vector<std::string>> pendingVoiceUpdates; // ��� ���� ���� ä�� ������Ʈ(�غ� �ȵż� ������Ʈ ���� �͵�)
	std::unordered_map<std::string, std::unordered_map<std::string, ParticipantAudioSettings>> participantsAudioSettings;
};
