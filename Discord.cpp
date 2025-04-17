#include "Discord.h"
#include "ChatFrame.h"
#include "ChatRoomManager.h"
#include "AudioIO.h"

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
        ChatClient& client = ChatClient::GetInstance();

        AudioIO& audioIO = AudioIO::GetInstance();
        if (!audioIO.Init())
        {
            wxMessageBox("����� �ʱ�ȭ ����", "����", wxOK | wxICON_ERROR);
            return false;
        }
        //audioIO.StartTest(); // ���� �׽�Ʈ 



        //wxString id = login.GetID();

        ChatRoomManager& roomManager = ChatRoomManager::GetInstance();
		VoiceChannelManager& voiceManager = VoiceChannelManager::GetInstance();
		
        RoomListDialog* roomList = new RoomListDialog(nullptr);
		roomManager.SetRoomListDialog(roomList); // �� ��� ��ȭ���� ����

        roomList->Show();
        return true;
    }

    
    
	return false; // ���α׷� ����
}
