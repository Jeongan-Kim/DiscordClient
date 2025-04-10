// VoiceChannelManager.cpp
#include "VoiceChannelManager.h"
#include "ChatFrame.h"

void VoiceChannelManager::JoinVoiceChannel(ChatFrame* frame) {
    if (currentVoiceFrame == frame) return; // 이미 참가 중이면 무시

    // 기존에 참가중인 채널이 있으면 나가기
    if (currentVoiceFrame) {
        currentVoiceFrame->OnVoiceChannelLeftByManager();
    }

    currentVoiceFrame = frame;
    frame->OnVoiceChannelJoinedByManager(); // 새로운 채널에 참가
}

void VoiceChannelManager::LeaveVoiceChannel(ChatFrame* frame) {
    if (currentVoiceFrame == frame) {
        frame->OnVoiceChannelLeftByManager();
        currentVoiceFrame = nullptr;
    }
}

ChatFrame* VoiceChannelManager::GetCurrentVoiceFrame() const {
    return currentVoiceFrame;
}

std::vector<std::string> VoiceChannelManager::GetUsersInVoice(const std::string& roomId) const
{
    std::vector<std::string> result;

    auto it = voiceParticipants.find(roomId);
    if (it != voiceParticipants.end()) {
        result.assign(it->second.begin(), it->second.end());
    }

    return result;
}

VoiceChannelManager& VoiceChannelManager::GetInstance()
{
    static VoiceChannelManager instance;
    return instance;
}
