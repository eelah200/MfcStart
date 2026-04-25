
// MfcStartDlg.h: 헤더 파일
//

#pragma once


// CMfcStartDlg 대화 상자
class CMfcStartDlg : public CDialogEx
{
// 생성입니다.
public:
	CMfcStartDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFCSTART_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEnChangeEditEdge();
	int m_nRadius; // 사용자가 입력한 점의 반지름 값
	int m_nThickness; // 사용자가 입력한 정원의 테두리 두께 값
	CPoint m_points[3]; // 사용자가 클릭한 3개의 점 좌표 배열
	int m_clickCount; // 현재 클릭된 점의 개수(0~3)
	int m_dragIndex; // 현재 드래그 중인 점의 인덱스 (드래그 중이 아니면 -1)
	bool m_isThreadRunning; // 랜덤 이동 스레드가 동작중인지 확인하는 플래그 

	bool CalculateCircle(
		CPoint p1, CPoint p2, CPoint p3, 
		double& centerX, double& centerY, double& radius
	); // 외심과 반지름 계산 함수. 세 점이 일직선이면 false 반환
	void DrawCustomCircle(double cx, double cy, double r, int thickness); // 정원 그리기 함수 
	bool IsOnCircleBoundary(
		int x, int y, 
		double cx, double cy, double r, int thickness); // bool 판별함수

	CImage m_canvas;

	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};
