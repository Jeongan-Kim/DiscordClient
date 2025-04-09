#include <wx/app.h>
#include <wx/splitter.h> 
#include <sstream>
#include <iostream>
#include "ChatRoomManager.h"
#include "ChatFrame.h"

ChatRoomManager::ChatRoomManager(ChatClient& client) : client(client) 
{
    OutputDebugStringA("ChatRoomManager ����\n");
    this->client.onMessageReceived = [this](const std::string& msg) 
        {
        HandleIncomingMessage(msg);
        };

	this->client.StartReceiving(); // �޽��� ���� ����
    OutputDebugStringA("�޽��� ���� ����\n");

}

void ChatRoomManager::OpenRoom(const std::string& roomId)
{
    if (chatFrames.count(roomId))
    {
        OutputDebugStringA("ä�ù��� �̹� �����ֽ��ϴ�.\n");
        chatFrames[roomId]->Raise(); // ���� â ����
        return;
    }

    ChatFrame* frame = new ChatFrame(client, roomId, nullptr, this);
    frame->Show();
    chatFrames[roomId] = frame;

    // ������ �� ���� �޽��� ����
    client.JoinRoom(roomId);
    OutputDebugStringA(("JOIN_ROOM �޽����� ���½��ϴ�: " + roomId + "\n").c_str());

    OutputDebugStringA("ä�ù��� �������ϴ�.\n");

}

void ChatRoomManager::CloseRoom(const std::string& roomId)
{
    auto it = chatFrames.find(roomId);
    if (it != chatFrames.end())
    {
        wxWindow* frame = it->second;
        wxTheApp->CallAfter([frame]() {
            frame->Destroy();
            OutputDebugStringA("CloseRoom-Destroy ȣ���");
            });
        OutputDebugStringA(("CloseRoom - �� ID: " + roomId + "\n").c_str());

        chatFrames.erase(it);

        if (chatFrames.count(roomId) == 0)
        {
            char buffer[256];
            snprintf(buffer, sizeof(buffer), "roomId '%s' ����!\n", roomId.c_str());
            OutputDebugStringA(buffer);
        }
    }
}

void ChatRoomManager::LeaveRoom(const std::string& roomId)
{
	// LEAVE_ROOM:��ID
    client.Send("LEAVE_ROOM:" + roomId);	// ������ �� ���� �޽��� ����
	CloseRoom(roomId); // ä�ù� �ݱ�  
}

void ChatRoomManager::HandleIncomingMessage(const std::string& msg) 
{
    OutputDebugStringA(("�����κ���: " + msg + "\n").c_str());
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
}

void ChatRoomManager::HandleRoomMessage(const std::string& msg)
{
    OutputDebugStringA(("�����κ���: " + msg + "\n").c_str());
	// �޽��� ���� ��: ROOMMSG:roomId:16:24:����:�ȳ��ϼ���~

    std::string data = msg.substr(strlen("ROOMMSG:"));
    size_t p1 = data.find(':');
    size_t p2 = data.find(':', p1 + 1);
    size_t p3 = data.find(':', p2 + 1);
    size_t p4 = data.find(':', p3 + 1);

    std::string roomId = data.substr(0, p1);
    std::string hour = data.substr(p1 + 1, p2 - p1 - 1);
    std::string minute = data.substr(p2 + 1, p3 - p2 - 1);
    std::string sender = data.substr(p3 + 1, p4 - p3 - 1);
    std::string content = data.substr(p4 + 1);

    std::string timeStr = hour + ":" + minute;
    
	std::string message = "[" + timeStr + "] " + sender + ": " + content;

    if (chatFrames.count(roomId)) // ���� �����ִٸ�
    {
        wxTheApp->CallAfter([=]
            {
                chatFrames[roomId]->AppendMessage(sender, message); // �޽��� �߰�
            });
    }

    if (!chatFrames.count(roomId)) {
        // ����� ���� �ٽ� ���� �ȵ�!
        OutputDebugStringA(("���ŵ� �޽����ε� ���� ���� ����. roomId: " + roomId + "\n").c_str());
        return;
    }
}

void ChatRoomManager::HandleSystemMessage(const std::string& msg) 
{
	// �޽��� ���� ��: SYSTEM:���ȴ��� ä�ù� [��1]�� �����ϼ̽��ϴ�.
	std::string content = msg.substr(7); // SYSTEM: �޽������� SYSTEM: �κ� ����

    // �� �̸� ����
    std::string roomName;
    size_t leftBracket = content.find('[');
    size_t rightBracket = content.find(']');
    if (leftBracket != std::string::npos && rightBracket != std::string::npos && rightBracket > leftBracket)
    {
        roomName = content.substr(leftBracket + 1, rightBracket - leftBracket - 1); // ��1, ��2 ��
    }
    else
    {
        OutputDebugStringA("�� �̸� ���� ����: �޽��� ������ �ùٸ��� ����\n");
        return;
    }

    // �ش� ���� frame���� �޽��� ����
    auto it = chatFrames.find(roomName);
    if (it != chatFrames.end())
    {
        ChatFrame* frame = it->second;
        wxTheApp->CallAfter([=] {
            frame->AppendMessage("SYSTEM", content);
            });
    }
    else
    {
        std::string debugMsg = "SYSTEM �޽��� ���õ�: ���� �̹� ���� ���� - " + roomName + "\n";
        OutputDebugStringA(debugMsg.c_str());
    }
}

void ChatRoomManager::HandleUserListMessage(const std::string& msg)
{
    // msg ����: USER_LIST:��ID:�̸�1,�̸�2,...

    size_t p1 = msg.find(':');
    size_t p2 = msg.find(':', p1 + 1);
    if (p1 == std::string::npos || p2 == std::string::npos) return;

    std::string roomId = msg.substr(p1 + 1, p2 - p1 - 1); // �� ID
    std::string userListStr = msg.substr(p2 + 1); // ���� ��� ���ڿ�

    std::istringstream ss(userListStr);
    std::string name;
    std::vector<std::string> users;

    while (std::getline(ss, name, ','))
    {
        if (!name.empty()) users.push_back(name);
    }

    if (chatFrames.count(roomId))
    {
        wxTheApp->CallAfter([=]
            {
                chatFrames[roomId]->UpdateUserList(users);
            });
    }

    if (!chatFrames.count(roomId)) {
        // ����� ���� �ٽ� ���� �ȵ�!
        OutputDebugStringA(("���ŵ� �޽����ε� ���� ���� ����. roomId: " + roomId + "\n").c_str());
        return;
    }

}

//bool ChatRoomManager::IsRoomOpen(const std::string& roomId) const
//{
//    return chatFrames.count(roomId) > 0; false;
//}
