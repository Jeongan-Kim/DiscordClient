// VoiceChannelManager.cpp
#include "VoiceChannelManager.h"
#include "ChatFrame.h"
#include "AudioIO.h"

bool VoiceChannelManager::StartAudioIO()
{
    AudioIO& audioIO = AudioIO::GetInstance();

    if (!audioIO.StartCapture())  // → 마이크 열고 서버로 전송 시작(UDP 50505 송신)
    {
        OutputDebugStringA("[VoiceChannelManager] 마이크 입력 시작 실패\n");
        return false;
    }

    if (!audioIO.StartPlayback())  // → 서버로부터 수신 시작해서 스피커로 출력(UDP 50506 수신)
    {
        OutputDebugStringA("[VoiceChannelManager] 스피커 출력 시작 실패\n");

        return false;
    }

    OutputDebugStringA("[VoiceChannelManager] 음성 채널에 성공적으로 참여함!\n");

    return true;

}

bool VoiceChannelManager::EndAudioIO()
{
    AudioIO& audioIO = AudioIO::GetInstance();

	if (!audioIO.StopCapture()) // 마이크 종료
    {
        OutputDebugStringA("[VoiceChannelManager] 마이크 입력 종료 실패\n");

        return false;
    }

	if (!audioIO.StopPlayback()) // 스피커 종료
    {
        OutputDebugStringA("[VoiceChannelManager] 스피커 출력 종료 실패\n");

        return false;
    }

    OutputDebugStringA("[VoiceChannelManager] 음성 채널에서 성공적으로 퇴장함!\n");

    return true;
}

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

	StartAudioIO(); // 오디오 테스트 시작
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

        EndAudioIO();
    }
}

void VoiceChannelManager::UpdateVoiceUserList(const std::string& roomId, const std::vector<VoiceEntry>& entries)
{
    OutputDebugStringA("UpdateVoiceUserList: 시작\n");
    // 1) 받은 entries 그대로 저장
    voiceParticipants[roomId] = entries;

    // 2) ChatFrame 찾아서 UI 갱신
    ChatRoomManager& roomManager = ChatRoomManager::GetInstance();
    auto& chatFrames = roomManager.GetChatFrames();
    auto it = chatFrames.find(roomId);

    ChatFrame* targetFrame = it->second;

    // 3) UI 스레드에서 안전하게 호출
    targetFrame->CallAfter([=]() {
        if (targetFrame && targetFrame->IsReady()) {
            OutputDebugStringA("CallAfter: UpdateVoiceParticipantList 호출\n");

            targetFrame->UpdateVoiceParticipantList();
        }
        //else {
        //    // 준비 안 됐으면 캐시
        //    VoiceChannelManager::GetInstance()
        //        .CachePendingVoiceUpdate(roomId, entries);
        //}
        });

	//voiceParticipants[roomId].clear(); // 기존 유저 목록 초기화
	//voiceParticipants[roomId].insert(users.begin(), users.end()); // 유저 목록 업데이트

//    ChatClient& client = ChatClient::GetInstance();
//
//    // client가 참여한 채팅방들 중에 roomId가 있는지 확인
//	// 있다면 해당 채팅방(frame)에서 음성채널 참가자 목록 갱신
//        
//	ChatRoomManager& roomManager = ChatRoomManager::GetInstance();
//
//    OutputDebugStringA(("현재 roomId : [" + roomId + "] \n").c_str());
//
//	auto& chatFrames = roomManager.GetChatFrames();
//	auto it = chatFrames.find(roomId);
//	if (it != roomManager.GetChatFrames().end())
//	{        
//        ChatFrame* targetFrame = it->second;
//        OutputDebugStringA("CallAfter 등록 시작\n");
//		wxTheApp->CallAfter([=]
//			{
//                OutputDebugStringA("CallAfter 람다 진입\n");
//
//				if (targetFrame->IsReady())
//				{
//                    OutputDebugStringA("프레임 준비됨\n");
//                    targetFrame->UpdateVoiceParticipantList();
//				}
///*                else
//                {
//                    OutputDebugStringA("프레임 준비 안 됨. 캐시로 저장\n");
//
//                    VoiceChannelManager::GetInstance().CachePendingVoiceUpdate(roomId, users);
//                }	*/			
//			});
//	}    
}

ChatFrame* VoiceChannelManager::GetCurrentVoiceFrame() const {
    return currentVoiceFrame;
}

std::vector<VoiceEntry> VoiceChannelManager::GetUsersInVoice(const std::string& roomId) const
{
    std::vector<VoiceEntry> result;

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

//void VoiceChannelManager::CachePendingVoiceUpdate(const std::string& roomId, const std::vector<VoiceEntry>& entries)
//{
//    pendingVoiceUpdates[roomId] = users;
//}
//
//bool VoiceChannelManager::HasPendingVoiceUpdate(const std::string& roomId) const
//{
//	auto it = pendingVoiceUpdates.find(roomId);
//    if (it != pendingVoiceUpdates.end()) return true; // 대기 중인 음성 채널 업데이트가 있으면 true 반환
//
//    return false;
//}
//
//std::vector<std::string> VoiceChannelManager::ConsumePendingUpdate(const std::string& roomId)
//{
//    auto it = pendingVoiceUpdates.find(roomId);
//    if (it != pendingVoiceUpdates.end())
//    {
//        std::vector<std::string> result = std::move(it->second);
//        pendingVoiceUpdates.erase(it);
//        return result;
//    }
//	
//    return {};
//
//}
