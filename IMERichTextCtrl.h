#pragma once
#ifdef __WXMSW__
    #include <wx/msw/wrapwin.h>
    #include <imm.h>
    #pragma comment(lib, "imm32.lib")
#endif

#include "wx/wx.h"
#include <wx/richtext/richtextctrl.h>
#include <vector>
#include <string>

// 한글 입력 시 조합 완료 전에 버퍼로 넘어가는 걸 제거하기 위해 만들어 보았지만 해결 안됨
class IMERichTextCtrl : public wxRichTextCtrl
{
public:
    IMERichTextCtrl(wxWindow* parent, wxWindowID id = wxID_ANY,
        const wxString& v = wxEmptyString,
        const wxPoint& p = wxDefaultPosition,
        const wxSize& s = wxDefaultSize,
        long st = wxRE_MULTILINE | wxRE_READONLY)
        : wxRichTextCtrl(parent, id, v, p, s, st), m_prevPos(0) {
    }

protected:
    WXLRESULT MSWWindowProc(WXUINT msg, WXWPARAM wParam, WXLPARAM lParam) override
    {
#ifdef __WXMSW__
        switch (msg)
        {
        case WM_IME_STARTCOMPOSITION:
        {
            HIMC hImc = ImmGetContext(GetHWND());

            // 1) EM_POSFROMCHAR 로 문자 인덱스 → 클라이언트 좌표 구하기
            POINT pt = { 0,0 };
            ::SendMessageW(GetHWND(),
                EM_POSFROMCHAR,
                reinterpret_cast<WPARAM>(&pt),
                static_cast<LPARAM>(GetInsertionPoint()));

            // 2) RichEdit 내부 스크롤 오프셋을 고려하려면
            //    EM_GETSCROLLPOS 을 사용하여 scrollPos 를 뽑아오셔도 좋습니다.
            //    여기선 간단히 ClientToScreen 만 해 줍니다.
            ::ClientToScreen(GetHWND(), &pt);

            // 3) IME 창 위치 설정
            COMPOSITIONFORM cf = {};
            cf.dwStyle = CFS_POINT;
            cf.ptCurrentPos = pt;
            ImmSetCompositionWindow(hImc, &cf);

            ImmReleaseContext(GetHWND(), hImc);

            // 4) 조합 시작 인덱스 저장
            m_prevPos = GetInsertionPoint();
            m_compString.clear();
            break;
        }

        case WM_IME_COMPOSITION:
        {
            HIMC hImc = ImmGetContext(GetHWND());
            // 1) 조합 중 문자열
            if (lParam & GCS_COMPSTR)
            {
                int sz = ImmGetCompositionStringW(hImc, GCS_COMPSTR, nullptr, 0);
                std::vector<wchar_t> buf(sz / 2 + 1);
                ImmGetCompositionStringW(hImc, GCS_COMPSTR, buf.data(), sz);
                buf[sz / 2] = L'\0';
                wxString comp(buf.data());

                // 이전 조합 지우기
                Delete(wxRichTextRange(m_prevPos,
                    m_prevPos + m_compString.Length()));

                // 새 조합 쓰기
                SetInsertionPoint(m_prevPos);
                WriteText(comp);
                m_compString = comp;

                // 캐럿 뒤로
                SetInsertionPoint(m_prevPos + comp.Length());
            }
            // 2) 확정된 문자열 (한글 자모 조합이 끝났을 때)
            if (lParam & GCS_RESULTSTR)
                m_compString.clear();

            ImmReleaseContext(GetHWND(), hImc);
            return 0;  // 기본 처리는 하지 않음
        }

        case WM_IME_ENDCOMPOSITION:
            m_compString.clear();
            break;
        }
#endif
        return wxRichTextCtrl::MSWWindowProc(msg, wParam, lParam);
    }

private:
    long     m_prevPos;
    wxString m_compString;
};