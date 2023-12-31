/***************************************************************************
 *
 * PROJECT: The Dark Mod - Updater
 * $Revision: 4379 $
 * $Date: 2010-12-22 09:49:40 -0500 (Wed, 22 Dec 2010) $
 * $Author: greebo $
 *
 ***************************************************************************/

#pragma once

#include "afxwin.h"
#include "TraceLog.h"
#include <boost/shared_ptr.hpp>

// LogViewer dialog

using namespace tdm;

class LogViewer : 
	public CDialog,
	public ILogWriter
{
	DECLARE_DYNAMIC(LogViewer)

	HICON m_hIcon;

public:
	LogViewer(CWnd* pParent = NULL);   // standard constructor
	virtual ~LogViewer();

	// Dialog Data
	enum { IDD = IDD_LOGVIEWER_DIALOG };

	// ILogWriter impl.
	void WriteLog(LogClass lc, const std::string& str);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	CEdit _logView;
	afx_msg void OnBnClickedOk();
};
typedef boost::shared_ptr<LogViewer> LogViewerPtr;
