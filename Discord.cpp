#include "Discord.h"
#include "ChatFrame.h"
#include "ChatRoomManager.h"

wxIMPLEMENT_APP(Discord);

bool Discord::OnInit()
{
    // �α��� â ����
    LoginDialog login(nullptr);

    if (login.ShowModal() == wxID_OK)
    {
        wxString id = login.GetID();
        wxString pw = login.GetPassword();

        ServerAPI::Init();

        //���̵�� ��й�ȣ�� ������ ������ �α��� ��û -> ���� �� ä�ù� ����Ʈ ���̾�α� ����
        if (login.IsSignupRequested()) 
        {
            // ȸ������ ó�� ���� (���� ����)
        }
        else 
        {
            if (!id.IsEmpty() && !pw.IsEmpty()) 
            {
                if (ServerAPI::Login(id.ToStdString(), pw.ToStdString())) 
                {

                    // ChatRoomManager ���� �� �޽��� ���� �ڵ鸵 ����
					ChatClient& client = ChatClient::GetInstance();
                    ChatRoomManager* roomManager = new ChatRoomManager(client);

                    // �� ����Ʈ â ����
                    RoomListDialog* roomList = new RoomListDialog(nullptr, roomManager);
                    //roomList->Show();

                    return true;
                }
                else 
                {
                    wxMessageBox("�α��� ����!", "����", wxOK | wxICON_ERROR);
                }
            }
        }
    }
    return false;
}
