#include <wx/app.h>
#include <wx/splitter.h> 
#include <sstream>
#include <iostream>
#include "ChatRoomManager.h"
#include "ChatFrame.h"
#include "RoomListDialog.h"

ChatRoomManager::ChatRoomManager()
{
    OutputDebugStringA("ChatRoomManager 생성\n");
    ChatClient& client = ChatClient::GetInstance();
    client.onMessageReceived = [this](const std::string& msg) 
        {
            HandleIncomingMessage(msg);
        };

	client.StartReceiving(); // 메시지 수신 시작
    OutputDebugStringA("메시지 수신 시작\n");

}

void ChatRoomManager::OpenRoom(const std::string& roomId)
{
    if (chatFrames.count(roomId))
    {
        OutputDebugStringA("채팅방이 이미 열려있습니다.\n");
        chatFrames[roomId]->Raise(); // 기존 창 띄우기
        return;
    }

    // 비밀번호 방인지 체크
	auto it = roomsInfo.find(roomId);
    if (it != roomsInfo.end())
    {
        std::string password = it->second;
        if (!password.empty())
        {
            // 비밀번호 입력 대화상자 띄우기
            wxString inputPassword = wxGetTextFromUser("비밀번호를 입력하세요:", "비밀번호 방", "", nullptr, -1, -1);
            if (inputPassword.IsEmpty() || inputPassword.ToStdString() != password)
            {
                wxMessageBox("비밀번호가 틀렸습니다.", "오류", wxOK | wxICON_ERROR);
                return;
            }
        }
    }

    ChatClient& client = ChatClient::GetInstance();
    ChatFrame* frame = new ChatFrame(client, roomId, nullptr, this);

    chatFrames[roomId] = frame;
    frame->Show();  
    OutputDebugStringA(("chatFrames[" + roomId + "] 등록 완료\n").c_str());    

    // 서버에 방 참여 메시지 전송
    client.JoinRoom(roomId);
    OutputDebugStringA(("JOIN_ROOM 메시지를 보냈습니다: " + roomId + "\n").c_str());

    OutputDebugStringA("채팅방을 열었습니다.\n");

}

void ChatRoomManager::CloseRoom(const std::string& roomId)
{
    
    auto it = chatFrames.find(roomId);
    if (it != chatFrames.end())
    {
        wxWindow* frame = it->second;
        wxTheApp->CallAfter([frame]() {
            frame->Destroy();
            OutputDebugStringA("CloseRoom-Destroy 호출됨");
            });
        OutputDebugStringA(("CloseRoom - 방 ID: " + roomId + "\n").c_str());

        chatFrames.erase(it);

        if (chatFrames.count(roomId) == 0)
        {
            char buffer[256];
            snprintf(buffer, sizeof(buffer), "roomId '%s' 없음!\n", roomId.c_str());
            OutputDebugStringA(buffer);
        }
    }
}

void ChatRoomManager::LeaveRoom(const std::string& roomId)
{
	// LEAVE_ROOM:방ID
    ChatClient& client = ChatClient::GetInstance();

    client.Send("LEAVE_ROOM:" + roomId);	// 서버에 방 퇴장 메시지 전송
	CloseRoom(roomId); // 채팅방 닫기  
}

void ChatRoomManager::MicSet(const std::string& roomId, bool status)
{
    // "VOICE_MIC:roomId:client1,1"
    ChatClient& client = ChatClient::GetInstance();

    std::string micStatus = (status == true ? "1" : "0");

    client.Send("VOICE_MIC:" + roomId + ":" + client.GetNickname() + "," + micStatus);
}

// "VOICE_HEADSET:roomId:client1,1"
void ChatRoomManager::HeadsetSet(const std::string& roomId, bool status)
{
    ChatClient& client = ChatClient::GetInstance();

    std::string headsetStatus = (status == true ? "1" : "0");
    client.Send("VOICE_HEADSET:" + roomId + ":" + client.GetNickname() + "," + headsetStatus);
}

void ChatRoomManager::HandleIncomingMessage(const std::string& rawMsg) 
{
    // 1) 메시지 끝의 개행문자(\n, \r) 제거
    std::string msg = rawMsg;
    while (!msg.empty() && (msg.back() == '\n' || msg.back() == '\r'))
        msg.pop_back();

    // 2) 빈 문자열이면 아무 처리도 안 하고 리턴
    if (msg.empty())
        return;

    OutputDebugStringA(("서버로부터: " + msg + "\n").c_str());
    if (msg.rfind("ROOMMSG:", 0) == 0)
    {
        HandleRoomMessage(msg);
    }
    else if (msg.rfind("SYSTEM:", 0) == 0)
    {
        HandleSystemMessage(msg);
    }
    else if (msg.rfind("USER_LIST:", 0) == 0)
    {
        HandleUserListMessage(msg);
    }
	else if (msg.rfind("VOICE_LIST:", 0) == 0)
	{
        HandleVoiceListMessage(msg);
	}
	else if (msg.rfind("ROOMS_INFO:", 0) == 0)
	{
		// 방 목록 수신 처리
		HandleRoomsInfoMessage(msg);
	}

}

void ChatRoomManager::HandleRoomMessage(const std::string& msg)
{
    OutputDebugStringA(("서버로부터: " + msg + "\n").c_str());
	// 메시지 포맷 예: ROOMMSG:roomId:16:24:정안:안녕하세요~

    std::string data = msg.substr(strlen("ROOMMSG:"));
    size_t p1 = data.find(':');
    size_t p2 = data.find(':', p1 + 1);
    size_t p3 = data.find(':', p2 + 1);
    size_t p4 = data.find(':', p3 + 1);

    std::string roomId = data.substr(0, p1);
    std::string hour = data.substr(p1 + 1, p2 - p1 - 1);
    std::string minute = data.substr(p2 + 1, p3 - p2 - 1);
    std::string sender = data.substr(p3 + 1, p4 - p3 - 1);
    std::string contentUtf8 = data.substr(p4 + 1);

    //std::string timeStr = hour + ":" + minute;    
	//std::string message = "[" + timeStr + "] " + sender + ": " + content;

    if (chatFrames.count(roomId)) // 방이 열려있다면
    {
        wxString content = wxString::FromUTF8(contentUtf8);
        wxTheApp->CallAfter([=]
            {
                chatFrames[roomId]->AppendMessage(hour, minute, sender, content); // 메시지 추가
            });
    }

    if (!chatFrames.count(roomId)) {
        // 절대로 방을 다시 열면 안됨!
        OutputDebugStringA(("수신된 메시지인데 방이 닫혀 있음. roomId: " + roomId + "\n").c_str());
        return;
    }
}

void ChatRoomManager::HandleSystemMessage(const std::string& msg) 
{
	// 메시지 포맷 예: SYSTEM:정안님이 채팅방 [방1]에 입장하셨습니다.
	std::string content = msg.substr(7); // SYSTEM: 메시지에서 SYSTEM: 부분 제거

    // 방 이름 추출
    std::string roomName;
    size_t leftBracket = content.find('[');
    size_t rightBracket = content.find(']');
    if (leftBracket != std::string::npos && rightBracket != std::string::npos && rightBracket > leftBracket)
    {
        roomName = content.substr(leftBracket + 1, rightBracket - leftBracket - 1); // 방1, 방2 등
    }
    else
    {
        OutputDebugStringA("방 이름 추출 실패: 메시지 형식이 올바르지 않음\n");
        return;
    }

    // 해당 방의 frame에만 메시지 전달
    auto it = chatFrames.find(roomName);
    if (it != chatFrames.end())
    {
        ChatFrame* frame = it->second;
        wxTheApp->CallAfter([=] {
            frame->AppendMessage("", "", "SYSTEM", content);
            });
    }
    else
    {
        std::string debugMsg = "SYSTEM 메시지 무시됨: 방이 이미 닫혀 있음 - " + roomName + "\n";
        OutputDebugStringA(debugMsg.c_str());
    }
}

void ChatRoomManager::HandleUserListMessage(const std::string& msg)
{
    // msg 예시: USER_LIST:방ID:이름1,이름2,...

    size_t p1 = msg.find(':');
    size_t p2 = msg.find(':', p1 + 1);
    if (p1 == std::string::npos || p2 == std::string::npos) return;

    std::string roomId = msg.substr(p1 + 1, p2 - p1 - 1); // 방 ID
    std::string userListStr = msg.substr(p2 + 1); // 유저 목록 문자열

    std::istringstream ss(userListStr);
    std::string name;
    std::vector<std::string> users;


    while (std::getline(ss, name, ','))
    {
        if (!name.empty()) users.push_back(name);
    }

    OutputDebugStringA(("users.size() = " + std::to_string(users.size()) + "\n").c_str());
    for (const auto& u : users) {
        OutputDebugStringA(("user = [" + u + "]\n").c_str());
    }

    if (chatFrames.count(roomId))
    {
        wxTheApp->CallAfter([=]
            {
                chatFrames[roomId]->UpdateUserList(users);
            });
    }

    if (!chatFrames.count(roomId)) {
        // 절대로 방을 다시 열면 안됨!
        OutputDebugStringA(("수신된 메시지인데 방이 닫혀 있음. roomId: " + roomId + "\n").c_str());
        return;
    }

}

void ChatRoomManager::HandleVoiceListMessage(const std::string& msg)
{
    OutputDebugStringA("HandleVoiceListMessage 호출 완료\n");
    // VOICE_LIST:roomId:user1,1,1;user2,1,0; ( 이름, 마이크, 헤드셋)
    size_t p1 = msg.find(':'); 
    size_t p2 = msg.find(':', p1 + 1);
    if (p1 == std::string::npos) return;

    std::string roomId;
    std::string userListStr;

    if (p2 == std::string::npos)
    {
        // "VOICE_LIST:sd" 형태면 roomId만 있고 리스트는 빈 상태
        roomId = msg.substr(p1 + 1);
        userListStr.clear();
    }
    else 
    {
        // "VOICE_LIST:sd:...." 형태
        roomId = msg.substr(p1 + 1, p2 - p1 - 1);
        userListStr = msg.substr(p2 + 1);
    }

    // 끝에 "\n"이나 ";" 있으면 제거
    while (!userListStr.empty() &&
        (userListStr.back() == '\n' ||  userListStr.back() == '\r' ||   userListStr.back() == ';'))
    {
        userListStr.pop_back();
    }

    // 디버깅용
    std::istringstream ss(userListStr);
    std::string name;
    std::vector<std::string> users;

    int i = 0;
    while (std::getline(ss, name, ','))
    {
        OutputDebugStringA(("user[" + std::to_string(i++) + "] = [" + name + "]\n").c_str());
        if (!name.empty()) 
            users.push_back(name);
    }
    //

    // 2) ';' 로 split 해서 각 "userId, mic, headset" 토큰으로 분리
    std::vector<VoiceEntry> entries;

    if (!userListStr.empty())
    {
        std::istringstream es(userListStr);
        std::string item;
        while (std::getline(es, item, ';')) {
            if (item.empty()) continue;

            // 3) 각 항목 내부를 ',' 로 split
            std::istringstream fs(item);
            std::string id, mic, head;
            if (std::getline(fs, id, ',') &&
                std::getline(fs, mic, ',') &&
                std::getline(fs, head, ','))
            {
                entries.push_back({
                    id,
                    mic == "1",  // "1" 이면 on
                    head == "1"
                    });
            }
        }
    } 

    // VoiceChannelManager에 유저 리스트 전달

    ChatRoomManager& roomManager = ChatRoomManager::GetInstance();
    auto& chatFrames = roomManager.GetChatFrames();
    auto it = chatFrames.find(roomId);
    if (it == chatFrames.end())
    {
        OutputDebugStringA("it == roomManager.GetChatFrames().end()\n");
        roomId;
        OutputDebugStringA(("roomManager address: " + std::to_string((uintptr_t)&roomManager) + "\n").c_str());
        it == roomManager.GetChatFrames().end();
        //VoiceChannelManager::GetInstance().CachePendingVoiceUpdate(roomId, entries);
        return;
    }
    if (!it->second->IsReady())
    {
        OutputDebugStringA("ChatFrame not ready, caching voice update.\n");
        //VoiceChannelManager::GetInstance().CachePendingVoiceUpdate(roomId, entries);
        return;
    }

    //// UI 준비 완료 상태에서만 반영
    VoiceChannelManager::GetInstance().UpdateVoiceUserList(roomId, entries);
}

void ChatRoomManager::HandleRoomsInfoMessage(const std::string& msg)
{
	OutputDebugStringA("HandleRoomsInfoMessage 호출 완료\n");

	// 방 목록 수신 처리
	// msg 예시: ROOMS_INFO:(roomName, password):(roomName, password) ...
    // 이전 데이터를 지우고
    roomsInfo.clear();

	std::string msgData = msg.substr(strlen("ROOMS_INFO:"));

    if (!msgData.empty() && msgData.back() == ':')
        msgData.pop_back();

    // 2) ':' 로 split
    std::vector<std::string> tokens;
    {
        std::istringstream iss(msgData);
        std::string tk;
        while (std::getline(iss, tk, ':'))
            tokens.push_back(tk);
    }

    // 짝수‑인덱스=방이름, 다음 인덱스=비밀번호
    size_t i = 0;
    for (; i + 1 < tokens.size(); i += 2)
    {
        roomsInfo[tokens[i]] = tokens[i + 1];  // 빈 문자열도 OK
    }
    // 토큰 개수가 홀수면 마지막 토큰은 방이름, 빈 비밀번호
    if (i < tokens.size())
    {
        roomsInfo[tokens[i]] = "";
    }
	//while (msgData.empty() == false)
	//{
	//	size_t p1 = msgData.find(':');
	//	size_t p2 = msgData.find(':', p1 + 1);

	//	if (p1 == std::string::npos) break;

	//	std::string roomName = msgData.substr(0, p1); // 방 이름
	//	std::string password = msgData.substr(p1 + 1, p2 - p1 - 1); // 방 비밀번호
	//	roomsInfo[roomName] = password; // 방 이름과 비밀번호 저장
	//	msgData.erase(0, p2); // 다음 메시지로 이동
	//}

	if (roomListDialog)
	{
		roomListDialog->RefreshRoomList(roomsInfo);		
	}
}

ChatRoomManager& ChatRoomManager::GetInstance()
{
    static ChatRoomManager instance;
    return instance;
}


