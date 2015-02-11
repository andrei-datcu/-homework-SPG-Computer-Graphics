
// ImageProcessingDlg.h : header file
//

#pragma once


// CImageProcessingDlg dialog
class CImageProcessingDlg : public CDialogEx
{
// Construction
public:
	CImageProcessingDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_IMAGEPROCESSING_DIALOG };

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
	afx_msg void OnBnClickedExit();
	afx_msg void OnBnClickedSaveas();
	afx_msg void OnBnClickedBlur();
	afx_msg void OnBnClickedGrayscale();
	afx_msg void OnBnClickedOpenfile();
    afx_msg void OnBnClickedGrayscale2();
};
