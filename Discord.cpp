#include "Discord.h"
#include "ChatFrame.h"

wxIMPLEMENT_APP(Discord);

bool Discord::OnInit() 
{
    // �α��� â ����
    LoginDialog login(nullptr);
    
    if (login.ShowModal() == wxID_OK) {
        wxString nickname = login.GetNickname();
        if (!nickname.IsEmpty()) //�г����� ������ ChatFrame ����
        {
            ChatFrame* frame = new ChatFrame(nickname.ToStdString());  // nickname ����
            frame->Show(true);
            return true;
        }
    }
    return false; // ��� �� �� ����
}

