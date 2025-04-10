#include "Discord.h"
#include "ChatFrame.h"
#include "ChatRoomManager.h"

wxIMPLEMENT_APP(Discord);

bool Discord::OnInit()
{
    wxInitAllImageHandlers(); // 이미지 핸들러 등록

	// 서버 IP 탐색 및 초기화
    if (!ServerAPI::Init())
    {
        wxMessageBox("서버를 찾을 수 없습니다.", "오류", wxOK | wxICON_ERROR);
        return false;
    }

    // 로그인 창 생성
    LoginDialog login(nullptr);

    if (login.ShowModal() == wxID_OK)
    {        
        // 로그인 성공 시
        wxString id = login.GetID();

        ChatClient& client = ChatClient::GetInstance();
        ChatRoomManager* roomManager = new ChatRoomManager(client);
        RoomListDialog* roomList = new RoomListDialog(nullptr, roomManager);
        roomList->Show();
        return true;
    }

    
    
	return false; // 프로그램 종료
}
