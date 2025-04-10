#include "Discord.h"
#include "ChatFrame.h"
#include "ChatRoomManager.h"

wxIMPLEMENT_APP(Discord);

bool Discord::OnInit()
{
    wxInitAllImageHandlers(); // �̹��� �ڵ鷯 ���

	// ���� IP Ž�� �� �ʱ�ȭ
    if (!ServerAPI::Init())
    {
        wxMessageBox("������ ã�� �� �����ϴ�.", "����", wxOK | wxICON_ERROR);
        return false;
    }

    // �α��� â ����
    LoginDialog login(nullptr);

    if (login.ShowModal() == wxID_OK)
    {        
        // �α��� ���� ��
        wxString id = login.GetID();

        ChatClient& client = ChatClient::GetInstance();
        ChatRoomManager* roomManager = new ChatRoomManager(client);
        RoomListDialog* roomList = new RoomListDialog(nullptr, roomManager);
        roomList->Show();
        return true;
    }

    
    
	return false; // ���α׷� ����
}
