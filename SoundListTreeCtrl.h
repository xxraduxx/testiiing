#pragma once


// SoundListTreeCtrl

class SoundListTreeCtrl : public CXTTreeCtrl
{
	DECLARE_DYNAMIC(SoundListTreeCtrl)

public:
	SoundListTreeCtrl();
	virtual ~SoundListTreeCtrl();

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnDropItem(WPARAM, LPARAM);
	afx_msg LRESULT OnDropMoveItem(WPARAM, LPARAM);
public:
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
};


