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

LoginResult ServerAPI::Login(const std::string& id, const std::string& pw)
{
    ChatClient& client = ChatClient::GetInstance();

    // Connect Ȯ�� �õ�
    if (!client.IsConnected()) 
    {
        if (!client.Connect(serverIP, serverPort)) 
        {
            return LoginResult::LOGIN_CONNECT_ERROR;
        }
    }

    // �α��� ��û �޽��� ������
    std::string loginMsg = "LOGIN:" + id + ":" + pw;
    client.Send(loginMsg);

    // ���� ���
    char buffer[256];
    int len = recv(client.GetSocket(), buffer, sizeof(buffer) - 1, 0);
    if (len <= 0) return LoginResult::LOGIN_CONNECT_ERROR;

    buffer[len] = '\0';
    std::string response = buffer;

    std::string debugMsg = "Login_Msg : " + response + "\n";
    OutputDebugStringA(debugMsg.c_str());

    if (response == "LOGIN_SUCCESS")
    {
        client.SetNickname(id);
        return LoginResult::LOGIN_SUCCESS;
    }
    else if (response == "LOGIN_NO_ID")
    {
        return LoginResult::LOGIN_NO_ID;
    }
    else if (response == "LOGIN_WRONG_PW")
    {
        return LoginResult::LOGIN_WRONG_PW;
    }
    else if (response == "LOGIN_FORMAT_ERROR")
    {
        return LoginResult::LOGIN_FORMAT_ERROR;
	}
	else if (response == "LOGIN_CONNECT_ERROR")
	{
		return LoginResult::LOGIN_CONNECT_ERROR;
	}
	else if (response == "LOGIN_ALREADY")
	{
		return LoginResult::LOGIN_ALREADY;
	}


    return LoginResult::LOGIN_ERROR;
}

bool ServerAPI::Register(const std::string& id, const std::string& pw) 
{
    ChatClient& client = ChatClient::GetInstance();

    if (!client.Connect(serverIP, serverPort))
        return false;

    std::string msg = "REGISTER:" + id + ":" + pw;
    client.Send(msg);
    std::string debugMsg1 = "Ŭ->���� / REGISTER:" + id + ":" + pw + "\n";
    OutputDebugStringA(debugMsg1.c_str());

    char buffer[256];
    int len = recv(client.GetSocket(), buffer, sizeof(buffer) - 1, 0);
    if (len <= 0) return false;

    buffer[len] = '\0';
    std::string response = buffer;

    std::string debugMsg2 = "����->Ŭ / REGISTER:" + response + "\n";

    OutputDebugStringA(debugMsg2.c_str());

    return response == "REGISTER_OK";
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
