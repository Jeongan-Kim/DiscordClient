#include "ServerAPI.h"

// ���� �ּҿ� ��Ʈ
std::string ServerAPI::serverIP = "";
int ServerAPI::serverPort = 9000;

bool ServerAPI::Init() 
{
    ChatClient& client = ChatClient::GetInstance();  // �̱��� ��ü ����, Ŭ���̾�Ʈ ����
	serverIP = client.DiscoverServerIP();  // ���� IP Ž��

    return !serverIP.empty();
}

bool ServerAPI::Login(const std::string& id, const std::string& pw) 
{
    ChatClient& client = ChatClient::GetInstance();

    // Connect �õ�
    if (!client.Connect(serverIP, serverPort)) 
    {
        return false;
    }

    // �α��� ��û �޽��� ������
    std::string loginMsg = "LOGIN:" + id + ":" + pw;
    client.Send(loginMsg);

    // ���� ���
    char buffer[256];
    int len = recv(client.GetSocket(), buffer, sizeof(buffer) - 1, 0);
    if (len <= 0) return false;

    buffer[len] = '\0';
    std::string response = buffer;

    if (response == "LOGIN_SUCCESS")
    {
        client.SetNickname(id);  // �α��� ���� �� �г��� ����
        return true;
    }

    return false;
}

bool ServerAPI::Register(const std::string& id, const std::string& pw) 
{
    std::string msg = BuildRegisterMessage(id, pw);
    std::string response;

    return SendAndReceive(msg, response) && response == "REGISTER_OK";
}

// �޽��� ���� ��: LOGIN|���̵�|��й�ȣ
std::string ServerAPI::BuildLoginMessage(const std::string& id, const std::string& pw) {
    return "LOGIN|" + id + "|" + pw;
}

std::string ServerAPI::BuildRegisterMessage(const std::string& id, const std::string& pw) {
    return "REGISTER|" + id + "|" + pw;
}

// ������ ������ �޽����� ������ ������ ����
bool ServerAPI::SendAndReceive(const std::string& message, std::string& response) 
{
    ChatClient& client = ChatClient::GetInstance();
    if (!client.Connect(serverIP, serverPort)) return false;

    client.Send(message);

    char buffer[1024];
    int len = recv(client.GetSocket(), buffer, sizeof(buffer) - 1, 0);
    if (len <= 0) return false;

    buffer[len] = '\0';
    response = buffer;

    client.Disconnect();
    return true;
}