// VoiceChannelManager.cpp
#include "VoiceChannelManager.h"

#include "ChatFrame.h"

void VoiceChannelManager::JoinVoiceChannel(ChatFrame* frame, std::string roomId, std::string clientId) 
{
    if (currentVoiceFrame == frame) return; // 이미 참가 중이면 무시

    ChatClient& client = ChatClient::GetInstance();
    // 기존에 참가중인 채널이 있으면 나가기
    if (currentVoiceFrame) 
    {
        OutputDebugStringA("기존방 나가기\n");
		LeaveVoiceChannel(currentVoiceFrame, currentVoiceRoomId, clientId); // 기존 방 나가기
    }

    currentVoiceFrame = frame; // 현재 참가중인 방 프레임 저장
	currentVoiceRoomId = roomId; // 현재 참가중인 방 ID 저장

	currentVoiceFrame->OnVoiceChannelJoinedByManager(); // UI 참가 상태 표시
    OutputDebugStringA("currentVoiceFrame을 이동할 방으로 변경 완료\n");

    OutputDebugStringA("음성채널 참가했다고 서버에 알리기\n");
	client.Send("VOICE_JOIN:" + roomId + ":" + clientId); // 서버에 참가 메시지 전송
}

void VoiceChannelManager::LeaveVoiceChannel(ChatFrame* frame, std::string roomId, std::string clientId)
{
    if (currentVoiceFrame == frame)
    {
        //OutputDebugStringA("음성채널 나갔다고 서버에 알리기\n");

        ChatClient& client = ChatClient::GetInstance();
        client.Send("VOICE_LEAVE:" + roomId + ":" + clientId); // 서버에 퇴장 메시지 전송
        currentVoiceFrame->OnVoiceChannelLeftByManager();    // UI 퇴장 상태 표시
        OutputDebugStringA("기존방 나간다고 서버에 알림\n");
        currentVoiceFrame = nullptr;
        currentVoiceRoomId.clear();
    }
}

void VoiceChannelManager::UpdateVoiceUserList(const std::string& roomId, const std::vector<std::string>& users)
{
    OutputDebugStringA("UpdateVoiceUserList: 시작\n");
	voiceParticipants[roomId].clear(); // 기존 유저 목록 초기화
	voiceParticipants[roomId].insert(users.begin(), users.end()); // 유저 목록 업데이트

    ChatClient& client = ChatClient::GetInstance();

    // client가 참여한 채팅방들 중에 roomId가 있는지 확인
	// 있다면 해당 채팅방(frame)에서 음성채널 참가자 목록 갱신
        
	ChatRoomManager& roomManager = ChatRoomManager::GetInstance();

    OutputDebugStringA(("현재 roomId : [" + roomId + "] \n").c_str());

	auto& chatFrames = roomManager.GetChatFrames();
	auto it = chatFrames.find(roomId);
	if (it != roomManager.GetChatFrames().end())
	{        
        ChatFrame* targetFrame = it->second;
        OutputDebugStringA("CallAfter 등록 시작\n");
		wxTheApp->CallAfter([=]
			{
                OutputDebugStringA("CallAfter 람다 진입\n");

				if (targetFrame->IsReady())
				{
                    OutputDebugStringA("프레임 준비됨\n");
                    targetFrame->UpdateVoiceParticipantList();
				}
                else
                {
                    OutputDebugStringA("프레임 준비 안 됨. 캐시로 저장\n");

                    VoiceChannelManager::GetInstance().CachePendingVoiceUpdate(roomId, users);
                }				
			});
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

void VoiceChannelManager::CachePendingVoiceUpdate(const std::string& roomId, const std::vector<std::string> users)
{
    pendingVoiceUpdates[roomId] = users;
}

bool VoiceChannelManager::HasPendingVoiceUpdate(const std::string& roomId) const
{
	auto it = pendingVoiceUpdates.find(roomId);
    if (it != pendingVoiceUpdates.end()) return true; // 대기 중인 음성 채널 업데이트가 있으면 true 반환

    return false;
}

std::vector<std::string> VoiceChannelManager::ConsumePendingUpdate(const std::string& roomId)
{
    auto it = pendingVoiceUpdates.find(roomId);
    if (it != pendingVoiceUpdates.end())
    {
        std::vector<std::string> result = std::move(it->second);
        pendingVoiceUpdates.erase(it);
        return result;
    }
	
    return {};

}
