// VoiceChannelManager.cpp
#include "VoiceChannelManager.h"
#include "ChatFrame.h"

void VoiceChannelManager::JoinVoiceChannel(ChatFrame* frame) {
    if (currentVoiceFrame == frame) return; // �̹� ���� ���̸� ����

    // ������ �������� ä���� ������ ������
    if (currentVoiceFrame) {
        currentVoiceFrame->OnVoiceChannelLeftByManager();
    }

    currentVoiceFrame = frame;
    frame->OnVoiceChannelJoinedByManager(); // ���ο� ä�ο� ����
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
