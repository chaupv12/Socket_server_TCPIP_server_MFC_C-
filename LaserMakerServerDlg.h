
// LaserMakerServerDlg.h : header file
//
#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <random>

using namespace std;

// CLaserMakerServerDlg dialog
class CLaserMakerServerDlg : public CDialogEx
{
// Construction
public:
	CLaserMakerServerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_LASERMAKERSERVER_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedStartServer();
	afx_msg void OnBnClickedStopServer();

private:
	CButton m_btnStart;
	CButton m_btnStop;
	CEdit m_editLog;

	SOCKET m_serverSocket;
	static UINT ServerThread(LPVOID pParam);

	CEdit m_ctrlPortNum;
	CIPAddressCtrl m_ctrlIpAddr;
	CStatic m_ctrStatus;

public:
	vector<string> Split2Arr(const string& str, const char& spliter);
	void KillThreadReadWrite(void);

	CWinThread				*m_pThreadReadWriteRun;
	HANDLE					m_hThreadReadWriteRun;
};
