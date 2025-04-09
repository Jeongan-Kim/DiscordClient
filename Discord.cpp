#include "Discord.h"
#include "ChatFrame.h"
#include "ChatRoomManager.h"

wxIMPLEMENT_APP(Discord);

bool Discord::OnInit()
{
    // 로그인 창 실행
    LoginDialog login(nullptr);

    if (login.ShowModal() == wxID_OK)
    {
        wxString id = login.GetID();
        wxString pw = login.GetPassword();

        ServerAPI::Init();

        //아이디와 비밀번호를 받으면 서버에 로그인 요청 -> 성공 시 채팅방 리스트 다이얼로그 실행
        if (login.IsSignupRequested()) 
        {
            // 회원가입 처리 로직 (추후 구현)
        }
        else 
        {
            if (!id.IsEmpty() && !pw.IsEmpty()) 
            {
                if (ServerAPI::Login(id.ToStdString(), pw.ToStdString())) 
                {

                    // ChatRoomManager 생성 및 메시지 수신 핸들링 시작
					ChatClient& client = ChatClient::GetInstance();
                    ChatRoomManager* roomManager = new ChatRoomManager(client);

                    // 방 리스트 창 열기
                    RoomListDialog* roomList = new RoomListDialog(nullptr, roomManager);
                    //roomList->Show();

                    return true;
                }
                else 
                {
                    wxMessageBox("로그인 실패!", "오류", wxOK | wxICON_ERROR);
                }
            }
        }
    }
    return false;
}
