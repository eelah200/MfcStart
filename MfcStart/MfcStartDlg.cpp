
// MfcStartDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "MfcStart.h"
#include "MfcStartDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMfcStartDlg 대화 상자



CMfcStartDlg::CMfcStartDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFCSTART_DIALOG, pParent)
	, m_nRadius(10) // Edit 변수 : 반지름 초기값
	, m_nThickness(5)// Edit 변수 : 정원 가장자리 두께
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_clickCount = 0; // 클릭한 점 개수 초깃값.
	m_dragIndex = -1; // 드래그 중인 점 인덱스 초깃값. 
	m_isThreadRunning = false; // 랜덤 이동 스레드 확인 초깃값.

	// 좌표 배열 초기화 (포인트 1,2,3 모두 (0,0)으로 초기화)
	for (int i = 0; i < 3; i++) {
		m_points[i] = CPoint(0, 0);
	}
}

void CMfcStartDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_RADIUS, m_nRadius);
	DDX_Text(pDX, IDC_EDIT_EDGE, m_nThickness);
}

BEGIN_MESSAGE_MAP(CMfcStartDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_EN_CHANGE(IDC_EDIT_EDGE, &CMfcStartDlg::OnEnChangeEditEdge)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE() 
	ON_WM_LBUTTONUP()
	ON_BN_CLICKED(IDC_BTN_INIT, &CMfcStartDlg::OnBnClickedBtnReset)
END_MESSAGE_MAP()


// CMfcStartDlg 메시지 처리기

BOOL CMfcStartDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
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

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	CRect rect;
	CWnd* pCanvas = GetDlgItem(IDC_CANVAS);
	if (pCanvas) {
		pCanvas->GetWindowRect(&rect);
		ScreenToClient(&rect);

		// 2. 실제 그림을 저장할 비트맵(종이) 생성 (중요!)
		// m_hBitmap != 0 오류를 해결하는 핵심 줄입니다.
		m_canvas.Create(rect.Width(), rect.Height(), 24);

		// 3. 종이를 깨끗하게 흰색으로 초기화
		HDC hdc = m_canvas.GetDC();
		::PatBlt(hdc, 0, 0, rect.Width(), rect.Height(), WHITENESS);
		m_canvas.ReleaseDC();
	}

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CMfcStartDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CMfcStartDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		// 1. 화면에 그리기 위한 도구(dc)를 준비합니다.
		CPaintDC dc(this);

		// 2. 도화지(m_canvas)가 만들어져 있다면 화면에 뿌려줍니다.
		if (!m_canvas.IsNull())
		{
			// 리소스 뷰에서 만든 IDC_CANVAS의 위치를 다시 계산합니다.
			CRect rect;
			CWnd* pCanvas = GetDlgItem(IDC_CANVAS);
			if (pCanvas) {
				pCanvas->GetWindowRect(&rect);
				ScreenToClient(&rect);

				// 핵심: 메모리(m_canvas)를 화면(dc)의 해당 위치에 그립니다.
				m_canvas.Draw(dc.GetSafeHdc(), rect.left, rect.top);
			}
		}
		// CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CMfcStartDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMfcStartDlg::OnEnChangeEditEdge()
{
	// TODO:  RICHEDIT 컨트롤인 경우, 이 컨트롤은
	// CDialogEx::OnInitDialog() 함수를 재지정 
	//하고 마스크에 OR 연산하여 설정된 ENM_CHANGE 플래그를 지정하여 CRichEditCtrl().SetEventMask()를 호출하지 않으면
	// ENM_CHANGE가 있으면 마스크에 ORed를 플래그합니다.

	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
}

bool CMfcStartDlg::CalculateCircle(CPoint p1, CPoint p2, CPoint p3, double& centerX, double& centerY, double& radius)
{
	double x1 = p1.x, y1 = p1.y;
	double x2 = p2.x, y2 = p2.y;
	double x3 = p3.x, y3 = p3.y;

	// 분모 계산 (세 점이 일직선인지 확인)
	double D = 2 * (x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2));

	if (D == 0) return false; // 세 점이 일직선상에 있을 경우 false 반환

	// 외심의 좌표 구하기 공식
	centerX = ((x1 * x1 + y1 * y1) * (y2 - y3) + (x2 * x2 + y2 * y2) * (y3 - y1) + (x3 * x3 + y3 * y3) * (y1 - y2)) / D;
	centerY = ((x1 * x1 + y1 * y1) * (x3 - x2) + (x2 * x2 + y2 * y2) * (x1 - x3) + (x3 * x3 + y3 * y3) * (x2 - x1)) / D;

	// 반지름 구하기 (중심점과 p1 사이의 거리)
	radius = sqrt(pow(centerX - x1, 2) + pow(centerY - y1, 2));

	return true;
}

bool CMfcStartDlg::IsOnCircleBoundary(int x, int y, double cx, double cy, double r, int thickness)
{
	double dx = x - cx;
	double dy = y - cy;
	double distSq = dx * dx + dy * dy; // 현재 픽셀의 거리 제곱

	double innerR = r - (thickness / 2.0);
	double outerR = r + (thickness / 2.0);

	// Min^2 <= D^2 <= Max^2
	return (distSq >= innerR * innerR && distSq <= outerR * outerR);
}

void CMfcStartDlg::DrawCustomCircle(double cx, double cy, double r, int thickness)
{
	// 1. 도화지(m_canvas)가 제대로 있는지 확인
	if (m_canvas.IsNull()) return;

	// 메모리의 첫 주소(fm)와 한 줄의 크기(nPitch)를 얻어옵니다.
	unsigned char* fm = (unsigned char*)m_canvas.GetBits();
	int nPitch = m_canvas.GetPitch();
	int nBpp = m_canvas.GetBPP() / 8; // 24비트면 3바이트(B,G,R)를 의미함

	// 2. 탐색 영역 설정 (Bounding Box)
	int startX = (int)(cx - r - thickness);
	int endX = (int)(cx + r + thickness);
	int startY = (int)(cy - r - thickness);
	int endY = (int)(cy + r + thickness);

	// 3. 이중 for문 탐색
	for (int y = startY; y <= endY; y++) {
		for (int x = startX; x <= endX; x++) {

			// 영역 이탈 방지 (에러 방어 코드)
			if (x < 0 || x >= m_canvas.GetWidth() || y < 0 || y >= m_canvas.GetHeight())
				continue;

			// 4. 판별 함수 호출 (테두리 두께 계산)
			if (IsOnCircleBoundary(x, y, cx, cy, r, thickness)) {

				// 5. [메모리 직접 렌더링] 
				// 영상의 fm[j*nPitch + i] 방식을 24비트 컬러에 맞게 변형
				int index = y * nPitch + x * nBpp;

				// 검은색 테두리로 칠하기 (B=0, G=0, R=0)
				fm[index + 0] = 0;   // Blue
				fm[index + 1] = 0; // Green
				fm[index + 2] = 0; // Red
			}
		}
	}
}

void CMfcStartDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// 1. IDC_CANVAS 영역의 위치 구하기
	CRect rect;
	GetDlgItem(IDC_CANVAS)->GetWindowRect(&rect);
	ScreenToClient(&rect); // 모니터 좌표를 다이얼로그 내부 좌표로 변환

	// 2. 사용자가 클릭한 곳이 캔버스 '안쪽'인지 확인
	if (rect.PtInRect(point))
	{
		// 3. 좌표 오프셋 보정 (다이얼로그 좌표 -> 캔버스 도화지 좌표)
		CPoint imgPoint;
		imgPoint.x = point.x - rect.left;
		imgPoint.y = point.y - rect.top;

		// --- 여기서부터 3단계 핵심 로직 시작 ---

		if (m_clickCount < 3)
		{
			// [1] 점 좌표 저장
			m_points[m_clickCount] = imgPoint;

			// [2] 현재 찍힌 점의 순서에 맞춰서 글자 업데이트
			CString str;
			str.Format(_T("점%d(%d, %d)"), m_clickCount + 1, imgPoint.x, imgPoint.y);

			if (m_clickCount == 0)      SetDlgItemText(IDC_POINT_1, str);
			else if (m_clickCount == 1) SetDlgItemText(IDC_POINT_2, str);
			else if (m_clickCount == 2) SetDlgItemText(IDC_POINT_3, str);

			// [3] 점 그리기
			DrawPoint(imgPoint);

			// [4] 점 개수 증가 (이 위치가 제일 중요합니다!)
			m_clickCount++;

			// [5] 방금 찍은 게 3번째 점이라면 바로 원 그리기
			if (m_clickCount == 3) {
				double cx, cy, r;
				if (CalculateCircle(m_points[0], m_points[1], m_points[2], cx, cy, r)) {
					DrawCustomCircle(cx, cy, r, m_nThickness);
				}
				else {
					AfxMessageBox(_T("세 점이 일직선이라 원을 그릴 수 없습니다. 다시 시도해주세요."));
					m_clickCount = 2; // 일직선이면 다시 세 번째 점을 찍을 수 있게 되돌림
				}
			}
		}
		else
		{
			// [6] 이미 점이 3개인 상태 -> 드래그할 점이 있는지 검사
			for (int i = 0; i < 3; i++) {
				double dx = imgPoint.x - m_points[i].x;
				double dy = imgPoint.y - m_points[i].y;
				double dist = sqrt(dx * dx + dy * dy);

				// 마우스 클릭 위치가 점의 중심에서 15픽셀 이내라면 "잡았다!"
				if (dist <= 15.0) {
					m_dragIndex = i;
					break;
				}
			}
		}

		// 화면 갱신 (OnPaint 호출)
		Invalidate();
	}

	CDialogEx::OnLButtonDown(nFlags, point);
}

void CMfcStartDlg::DrawPoint(CPoint p)
{
	// 3-3. 클릭한 위치에 눈에 잘 띄게 검은색 정원 점 찍기 (피드백)
	int pointRadius = 7; // 점의 크기 조절
	for (int y = -pointRadius; y <= pointRadius; y++) {
		for (int x = -pointRadius; x <= pointRadius; x++) {
			// 원의 방정식 (x^2 + y^2 <= r^2)을 이용해 내부를 채웁니다.
			if (x * x + y * y <= pointRadius * pointRadius) {
				int drawX = p.x + x;
				int drawY = p.y + y;

				// 캔버스 범위를 벗어나지 않을 때만 그리기
				if (drawX >= 0 && drawX < m_canvas.GetWidth() &&
					drawY >= 0 && drawY < m_canvas.GetHeight()) {
					m_canvas.SetPixel(drawX, drawY, RGB(0, 0, 0)); // 검은색 점
				}
			}
		}
	}
}

void CMfcStartDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_dragIndex != -1 && (nFlags & MK_LBUTTON)) {
		// 1. 좌표 보정
		CRect rect;
		GetDlgItem(IDC_CANVAS)->GetWindowRect(&rect);
		ScreenToClient(&rect);

		CPoint imgPoint;
		imgPoint.x = point.x - rect.left;
		imgPoint.y = point.y - rect.top;

		// 2. 잡고 있는 점의 좌표 업데이트
		m_points[m_dragIndex] = imgPoint;

		// 3. 캔버스 초기화 (이전 그림 지우기)
		HDC hdc = m_canvas.GetDC();
		::PatBlt(hdc, 0, 0, rect.Width(), rect.Height(), WHITENESS);
		m_canvas.ReleaseDC();

		// 4. 세 점 및 원 다시 그리기
		for (int i = 0; i < 3; i++) {
			// (점 그리는 함수 혹은 로직 호출)
			DrawPoint(m_points[i]);
		}

		double cx, cy, r;
		if (CalculateCircle(m_points[0], m_points[1], m_points[2], cx, cy, r)) {
			DrawCustomCircle(cx, cy, r, m_nThickness);
		}

		// 5. 화면 갱신
		Invalidate();
	}
	CDialogEx::OnMouseMove(nFlags, point);
}

void CMfcStartDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_dragIndex = -1; // 드래그 종료
	CDialogEx::OnLButtonUp(nFlags, point);
}

void CMfcStartDlg::OnBnClickedBtnReset()
{
	// 1. 클릭 상태 및 좌표 배열 초기화
	m_clickCount = 0;
	m_dragIndex = -1;
	for (int i = 0; i < 3; i++) {
		m_points[i] = CPoint(0, 0);
	}

	// 2. 랜덤 이동 스레드 상태 초기화 (다음 5단계를 위한 대비)
	m_isThreadRunning = false;

	// 3. 화면 상단의 좌표 텍스트 초기화
	SetDlgItemText(IDC_POINT_1, _T("점1(0, 0)"));
	SetDlgItemText(IDC_POINT_2, _T("점2(0, 0)"));
	SetDlgItemText(IDC_POINT_3, _T("점3(0, 0)"));

	// 4. 도화지(m_canvas)를 깨끗한 흰색으로 지우기
	if (!m_canvas.IsNull()) {
		CRect rect;
		GetDlgItem(IDC_CANVAS)->GetWindowRect(&rect);

		HDC hdc = m_canvas.GetDC();
		// PatBlt 함수와 WHITENESS 옵션을 사용해 도화지 전체를 흰색으로 덮습니다.
		::PatBlt(hdc, 0, 0, rect.Width(), rect.Height(), WHITENESS);
		m_canvas.ReleaseDC();
	}

	// 5. 화면 새로고침 (지워진 도화지를 모니터에 반영)
	Invalidate();
}
