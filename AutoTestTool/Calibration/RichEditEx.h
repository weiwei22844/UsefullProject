#pragma once

class CRichEditEx : public CRichEditCtrl
{
	DECLARE_DYNAMIC(CRichEditEx)

public:
	CRichEditEx();
	virtual ~CRichEditEx();    

protected:
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnCopy() { Copy(); }
    afx_msg void OnSelectall() { SetSel(0, -1); }
    afx_msg void OnClearAll();
};


