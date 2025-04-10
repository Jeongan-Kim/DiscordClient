#pragma once
#include <map>
#include <string>
#include <unordered_map>
#include <set>
#include "ChatFrame.h"

// 현재 음성 채널 참가 상태를 추적
// 새 음성채널 참가 시 기존 음성 연결 자동 종료
// 버튼 상태 동기화(마이크, 헤드셋) : 음성 채널에 속한 방에는 버튼 활성화, 나머지 방은 비활성화
// ChatFrame UI에 알림 : 버튼 상태나 참가자 표시 업데이트

class VoiceChannelManager {
public:
	void JoinVoiceChannel(ChatFrame* frame); // 음성 채널 참가
	void LeaveVoiceChannel(ChatFrame* frame); // 음성 채널 퇴장
	ChatFrame* GetCurrentVoiceFrame() const; // 현재 음성 채널 참가 중인 프레임 반환
	std::vector<std::string> GetUsersInVoice(const std::string& roomId) const; // 현재 음성채널에 참여중인 유저 목록

	static VoiceChannelManager& GetInstance(); // 어디서든 접근할 수 있도록 싱글톤 패턴으로 구현

private:
	VoiceChannelManager() = default;
	ChatFrame* currentVoiceFrame = nullptr; // 현재 음성 채널 참가 중인 프레임

	// roomId -> userId 목록 (각 방에 음성채널 참여 유저들)
	std::unordered_map<std::string, std::set<std::string>> voiceParticipants;

	// userId -> ChatFrame 포인터 (UI 갱신용)
	std::unordered_map<std::string, ChatFrame*> userFrames;



	//std::string GetUserIdFromFrame(ChatFrame* frame) const;
};
