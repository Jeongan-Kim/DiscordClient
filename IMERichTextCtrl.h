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

// �ѱ� �Է� �� ���� �Ϸ� ���� ���۷� �Ѿ�� �� �����ϱ� ���� ����� �������� �ذ� �ȵ�
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

            // 1) EM_POSFROMCHAR �� ���� �ε��� �� Ŭ���̾�Ʈ ��ǥ ���ϱ�
            POINT pt = { 0,0 };
            ::SendMessageW(GetHWND(),
                EM_POSFROMCHAR,
                reinterpret_cast<WPARAM>(&pt),
                static_cast<LPARAM>(GetInsertionPoint()));

            // 2) RichEdit ���� ��ũ�� �������� ����Ϸ���
            //    EM_GETSCROLLPOS �� ����Ͽ� scrollPos �� �̾ƿ��ŵ� �����ϴ�.
            //    ���⼱ ������ ClientToScreen �� �� �ݴϴ�.
            ::ClientToScreen(GetHWND(), &pt);

            // 3) IME â ��ġ ����
            COMPOSITIONFORM cf = {};
            cf.dwStyle = CFS_POINT;
            cf.ptCurrentPos = pt;
            ImmSetCompositionWindow(hImc, &cf);

            ImmReleaseContext(GetHWND(), hImc);

            // 4) ���� ���� �ε��� ����
            m_prevPos = GetInsertionPoint();
            m_compString.clear();
            break;
        }

        case WM_IME_COMPOSITION:
        {
            HIMC hImc = ImmGetContext(GetHWND());
            // 1) ���� �� ���ڿ�
            if (lParam & GCS_COMPSTR)
            {
                int sz = ImmGetCompositionStringW(hImc, GCS_COMPSTR, nullptr, 0);
                std::vector<wchar_t> buf(sz / 2 + 1);
                ImmGetCompositionStringW(hImc, GCS_COMPSTR, buf.data(), sz);
                buf[sz / 2] = L'\0';
                wxString comp(buf.data());

                // ���� ���� �����
                Delete(wxRichTextRange(m_prevPos,
                    m_prevPos + m_compString.Length()));

                // �� ���� ����
                SetInsertionPoint(m_prevPos);
                WriteText(comp);
                m_compString = comp;

                // ĳ�� �ڷ�
                SetInsertionPoint(m_prevPos + comp.Length());
            }
            // 2) Ȯ���� ���ڿ� (�ѱ� �ڸ� ������ ������ ��)
            if (lParam & GCS_RESULTSTR)
                m_compString.clear();

            ImmReleaseContext(GetHWND(), hImc);
            return 0;  // �⺻ ó���� ���� ����
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