#include "Discord.h"
#include "ChatFrame.h"
#include "ChatRoomManager.h"
#include "AudioIO.h"

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
        ChatClient& client = ChatClient::GetInstance();

        AudioIO& audioIO = AudioIO::GetInstance();
        if (!audioIO.Init())
        {
            wxMessageBox("오디오 초기화 실패", "오류", wxOK | wxICON_ERROR);
            return false;
        }
        //audioIO.StartTest(); // 에코 테스트 



        //wxString id = login.GetID();

        ChatRoomManager& roomManager = ChatRoomManager::GetInstance();
		VoiceChannelManager& voiceManager = VoiceChannelManager::GetInstance();
		
        RoomListDialog* roomList = new RoomListDialog(nullptr);
		roomManager.SetRoomListDialog(roomList); // 방 목록 대화상자 설정

        roomList->Show();
        return true;
    }

    
    
	return false; // 프로그램 종료
}
