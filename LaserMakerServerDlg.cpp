
// LaserMakerServerDlg.cpp : implementation file
//
#include "stdafx.h"
#include "LaserMakerServer.h"
#include "LaserMakerServerDlg.h"
#include "afxdialogex.h"

#include <afxsock.h>

std::random_device rd;
std::mt19937 gen(rd());

int random(int low, int high)
{
	std::uniform_int_distribution<> dist(low, high);
	return dist(gen);
}

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define		 STX			0x02
#define		 ETX			0x03
#define		 ACK			0x06
#define		 NAK			0x15
#define		 ENQ			0x05
#define		 EOT			0x04
#define		 LF				0x0A
#define		 CR				0x0D

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CLaserMakerServerDlg dialog



CLaserMakerServerDlg::CLaserMakerServerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CLaserMakerServerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CLaserMakerServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_START_SERVER, m_btnStart);
	DDX_Control(pDX, IDC_STOP_SERVER, m_btnStop);
	DDX_Control(pDX, IDC_LOG, m_editLog);
	DDX_Control(pDX, IDC_EDIT_PORT_NUM, m_ctrlPortNum);
	DDX_Control(pDX, IDC_IPADDRESS, m_ctrlIpAddr);
	DDX_Control(pDX, IDC_STATIC_LBL_STS, m_ctrStatus);
}

BEGIN_MESSAGE_MAP(CLaserMakerServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_START_SERVER, &CLaserMakerServerDlg::OnBnClickedStartServer)
	ON_BN_CLICKED(IDC_STOP_SERVER, &CLaserMakerServerDlg::OnBnClickedStopServer)
END_MESSAGE_MAP()


// CLaserMakerServerDlg message handlers

BOOL CLaserMakerServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	
	m_pThreadReadWriteRun = NULL;
	// Initialize Winsock
	AfxSocketInit();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CLaserMakerServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CLaserMakerServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CLaserMakerServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CLaserMakerServerDlg::OnBnClickedStartServer()
{
	CString sPortNum, sIP;
	m_ctrlPortNum.GetWindowText(sPortNum);
	m_ctrlIpAddr.GetWindowText(sIP);
	string cIp = sIP;
	string cPortNum = sPortNum;

	struct hostent *hostentry;
	hostentry = gethostbyname(cIp.c_str());
	char *pipaddr = inet_ntoa(*(struct in_addr *)*hostentry->h_addr_list);
	int portno = atoi(cPortNum.c_str());

	m_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_serverSocket == INVALID_SOCKET)
	{
		//AfxMessageBox(_T("Failed to create socket"));
		int error = WSAGetLastError();
		CString errorMsg;
		errorMsg.Format(_T("Failed to create socket. Error code: %d"), error);
		AfxMessageBox(errorMsg);
		return;
	}

	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr(pipaddr); //htonl(INADDR_ANY);
	serverAddr.sin_port = htons(portno); //htons(2000);

	if (bind(m_serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		AfxMessageBox(_T("Failed to bind socket"));
		closesocket(m_serverSocket);
		return;
	}

	if (listen(m_serverSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		AfxMessageBox(_T("Error in listening on socket"));
		closesocket(m_serverSocket);
		return;
	}

	if (m_pThreadReadWriteRun == NULL)
	{
		m_pThreadReadWriteRun = AfxBeginThread(ServerThread, this);

		m_ctrStatus.SetWindowText("Client connected");
	}
}

void CLaserMakerServerDlg::OnBnClickedStopServer()
{
	if (m_serverSocket != INVALID_SOCKET)
	{
		closesocket(m_serverSocket);
		m_serverSocket = INVALID_SOCKET;
		m_editLog.SetWindowText(_T("Server stopped."));
		m_ctrStatus.SetWindowText("Client disconnected");
		KillThreadReadWrite();
	}
}

vector<string> CLaserMakerServerDlg::Split2Arr(const string& str, const char& spliter) {
	string chr;
	vector<string> result;

	for (string::const_iterator it = str.begin(); it != str.end(); it++) {
		if (*it == spliter) {
			if (!chr.empty()) {
				result.push_back(chr);
				chr.clear();
			}
		}
		else {
			chr += *it;
		}
	}
	if (!chr.empty())
		result.push_back(chr);
	return result;
}

UINT CLaserMakerServerDlg::ServerThread(LPVOID pParam)
{
	CLaserMakerServerDlg* pDlg = reinterpret_cast<CLaserMakerServerDlg*>(pParam);
	SOCKET clientSocket;
	CString strRecvData,curTxt;

	CString keyHead, header;
	vector<string> sArrResult;
	string head;

	while ((clientSocket = accept(pDlg->m_serverSocket, NULL, NULL)) != INVALID_SOCKET)
	{
		char buffer[1024];
		int bytesRead;

		pDlg->m_editLog.SetWindowText(_T("Client connected.\r\n"));

		do
		{
			/*bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
			if (bytesRead > 0)
			{
				buffer[bytesRead] = '\0';
				pDlg->m_editLog.ReplaceSel(CString(buffer) + _T("\r\n"));
			}*/

			bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
			if (bytesRead > 0)
			{
				buffer[bytesRead] = '\0';
				pDlg->m_editLog.ReplaceSel(CString(buffer) + _T("\r\n"));
			}

			//send
			strRecvData.Format("%s\r\n \r\n%s\r\n\r\n", (const char*)CTime::GetCurrentTime().Format("%B %d, %Y %H:%M:%S"),buffer);
			keyHead.Format("%s", buffer);
			header = keyHead.Mid(1, keyHead.GetLength() - 1);

			head = header;
			sArrResult = pDlg->Split2Arr(head, '_');

			string mdataVec[] = { "S16XT1200M / SPD25 / F22102402043 / H23440201L034",
				"S16XT1200M / SPD25 / F22102402043 / H23440201L018",
				"S16XT1200M / SPD25 / F22102402043 / H23440201L027",
				"S16XT1200M / SPD25 / F22102402043 / H23440201L027",
				"S16XT1200M / SPD25 / F22102402043 / H23440201L016",
				"S16XT1200M / SPD25 / F22102402043 / H23440201L001",
				"S16XT1200M / SPD25 / F22102402043 / H23440201L006",
				"S16XT1200M / SPD25 / F22102402043 / H23440201L037",
				"S16XT1200M / SPD25 / F22102402043 / H23440201L026",
				"S16XT1200M / SPD25 / F22102402043 / H23440201L033",
				"S16XT1200M / SPD25 / F22102402043 / H23440201L003",
				"S16XT1200M / SPD25 / F22102402043 / H23440201L028",
				"S16XT1200M / SPD25 / F22102402043 / H23440201L015",
				"S16XT1200M / SPD25 / F22102402043 / H23440201L024",
				"S16XT1200M / SPD25 / F22102402043 / H23440201L010" };


			int num = random(0, 14);
			string rep;
			CString val;

			if (sArrResult[0] == "$RecipeUpload"){
				val.Format("%c$Receive_OK_Upload%c", STX, ETX);
				rep = val;
				send(clientSocket, rep.c_str(), rep.length(), 0);
			}
			else if (sArrResult[0] == "$RecipeDownload"){
				val.Format("%c$Receive_OK_Download%c", STX, ETX);
				rep = val;
				send(clientSocket, rep.c_str(), rep.length(), 0);
			}
			else if (sArrResult[0] == "$RecipeChange"){
				val.Format("%c$Receive_OK_Change%c", STX, ETX);
				rep = val;
				send(clientSocket, rep.c_str(), rep.length(), 0);
			}
			else if (sArrResult[0] == "$RecipeDelete"){
				val.Format("%c$Receive_OK_Delete%c", STX, ETX);
				rep = val;
				send(clientSocket, rep.c_str(), rep.length(), 0);
			}
			else
				send(clientSocket, mdataVec[num].c_str(), mdataVec[num].length(), 0);

		} while (bytesRead > 0);

		closesocket(clientSocket);
		pDlg->m_editLog.ReplaceSel(_T("Client disconnected.\r\n"));
	}

	return 0;
}

void CLaserMakerServerDlg::KillThreadReadWrite(void)
{
	if (m_pThreadReadWriteRun != NULL)
	{
		m_pThreadReadWriteRun = NULL;
	}
}