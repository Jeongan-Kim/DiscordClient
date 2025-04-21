#pragma once
#include <map>
#include <string>
#include <unordered_map>
#include <set>
#include "ChatFrame.h"
#include "ChatClient.h"
#include "ChatRoomManager.h"
#include "VoiceEntry.h"

// 현재 음성 채널 참가 상태를 추적
// 새 음성채널 참가 시 기존 음성 연결 자동 종료
// 버튼 상태 동기화(마이크, 헤드셋) : 음성 채널에 속한 방에는 버튼 활성화, 나머지 방은 비활성화
// ChatFrame UI에 알림 : 버튼 상태나 참가자 표시 업데이트
//class AudioIO; // AudioIO 클래스 전방 선언
struct ParticipantAudioSettings
{
	float volume = 1.0f;
	bool muted = false;
};

class VoiceChannelManager {
public:
	VoiceChannelManager() = default; // 기본 생성자

	//void SetAudioIO(AudioIO* audio) { this->audioIO = audio; } // AudioIO 설정
	bool StartAudioIO(); // 오디오 입출력 시작
	bool EndAudioIO(); // 오디오 입출력 종료

	void JoinVoiceChannel(ChatFrame* frame, std::string roomId, std::string clientId); // 음성 채널 참가
	void LeaveVoiceChannel(ChatFrame* frame, std::string roomId, std::string clientId); // 음성 채널 퇴장

	void UpdateVoiceUserList(const std::string& roomId, const std::vector<VoiceEntry>& entries); // 음성 채널 유저 목록 업데이트
	std::vector<VoiceEntry> GetUsersInVoice(const std::string& roomId) const; // 현재 음성채널에 참여중인 유저 목록

	static VoiceChannelManager& GetInstance(); // 어디서든 접근할 수 있도록 싱글톤 패턴으로 구현

	//void CachePendingVoiceUpdate(const std::string& roomId, const std::vector<VoiceEntry>& entries); // 대기 중인 음성 채널 업데이트 캐시
	//bool HasPendingVoiceUpdate(const std::string& roomId) const; // 대기 중인 음성 채널 업데이트 확인
	//std::vector<std::string> ConsumePendingUpdate(const std::string& roomId); // 대기 중인 음성 채널 업데이트 소비

	std::string GetCurrentVoiceRoomId() const { return currentVoiceRoomId; } // 현재 음성 채널 ID 반환
	ChatFrame* GetCurrentVoiceFrame() const; // 현재 음성 채널 참가 중인 프레임 반환

	void SetParticipantVolume(const std::string& roomId, const std::string& clientId, float vol);
	float GetParticipantVolume(const std::string& roomId, const std::string& clientId) const;
	void  SetParticipantMute(const std::string& roomId, const std::string& clientId, bool mute);
	bool  IsParticipantMuted(const std::string& roomId, const std::string& clientId) const;

private:
	//AudioIO* audioIO = nullptr; // AudioIO 객체 포인터

	ChatFrame* currentVoiceFrame = nullptr; // 현재 음성 채널 참가 중인 프레임
	std::string currentVoiceRoomId; // 현재 음성 채널 ID
	ChatFrame* nextVoiceFrame = nullptr;
	std::string nextVoiceRoomId;

	// roomId -> userId 목록 (각 방에 음성채널 참여 유저들)
	std::unordered_map<std::string, std::vector<VoiceEntry>> voiceParticipants;

	// userId -> ChatFrame 포인터 (UI 갱신용)
	std::unordered_map<std::string, ChatFrame*> userFrames;

	std::unordered_map<std::string, std::vector<std::string>> pendingVoiceUpdates; // 대기 중인 음성 채널 업데이트(준비 안돼서 업데이트 못한 것들)
	std::unordered_map<std::string, std::unordered_map<std::string, ParticipantAudioSettings>> participantsAudioSettings;
};
