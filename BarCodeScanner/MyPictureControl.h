#pragma once


// MyPictureControl

class MyPictureControl : public CStatic
{
	DECLARE_DYNAMIC(MyPictureControl)

public:
	MyPictureControl();
	virtual ~MyPictureControl();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDropFiles(HDROP hDropInfo);
};


