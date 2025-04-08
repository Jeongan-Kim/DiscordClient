#include "Discord.h"
#include "ChatFrame.h"

wxIMPLEMENT_APP(Discord);

bool Discord::OnInit() 
{
    // 로그인 창 실행
    LoginDialog login(nullptr);
    
    if (login.ShowModal() == wxID_OK) {
        wxString nickname = login.GetNickname();
        if (!nickname.IsEmpty()) //닉네임을 받으면 ChatFrame 띄우기
        {
            ChatFrame* frame = new ChatFrame(nickname.ToStdString());  // nickname 전달
            frame->Show(true);
            return true;
        }
    }
    return false; // 취소 시 앱 종료
}

