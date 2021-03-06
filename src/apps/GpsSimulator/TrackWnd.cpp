// TrackWnd.cpp : implementation file
//

#include "stdafx.h"
#include "GpsSimulator.h"
#include "TrackWnd.h"

#define RADIUS 					(6371000) // 6371e3
#define PI						(3.14159265359)

#define TO_RADIAN(x)			((x)*(PI/180))
#define GET_DISTANCE(angle)		(2.0 * RADIUS * sin(TO_RADIAN((angle) / 2)))

#define ZOOM_FACTOR				(1.0)


// CTrackWnd

IMPLEMENT_DYNAMIC(CTrackWnd, CWnd)

CTrackWnd::CTrackWnd()
{
	mFront = mRear = 0;
}

CTrackWnd::~CTrackWnd()
{
}


BEGIN_MESSAGE_MAP(CTrackWnd, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
END_MESSAGE_MAP()


//

void CTrackWnd::AddPosition(double lat, double lon, double track)
{
	//
	int nNext = (mFront + 1) % MAX_POSITION;

	if (nNext == mRear)
		mRear = (mRear + 1) % MAX_POSITION;

	mPos[mFront].lat = lat;
	mPos[mFront].lon = lon;
	mPos[mFront].track = track;

	mFront = nNext;

	//
	UpdatePoints();
}

void CTrackWnd::UpdatePoints()
{
	if (mFront == mRear)
		return; // empty

	int nLast = (mFront + MAX_POSITION - 1) % MAX_POSITION;

	for (int i = mRear; i != nLast; )
	{
		mPoint[i].x = -GET_DISTANCE(mPos[nLast].lon - mPos[i].lon);
		mPoint[i].y = GET_DISTANCE(mPos[nLast].lat - mPos[i].lat);

		i = (i + 1) % MAX_POSITION;
	}

	mPoint[nLast].x = 0;
	mPoint[nLast].y = 0;
}

// CTrackWnd message handlers

BOOL CTrackWnd::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return TRUE;
}


void CTrackWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: Add your message handler code here
					   // Do not call CWnd::OnPaint() for painting messages
	CRect rc;

	this->GetClientRect(rc);
	dc.FillSolidRect(rc, RGB(255, 255, 255)); //  COLOR_WINDOW);

	if (mRear != mFront)
	{
		int nSaveDC = dc.SaveDC();
		int nLast = (mFront + MAX_POSITION - 1) % MAX_POSITION;

		CPen pen(PS_SOLID, 1, RGB(0, 0, 0));
		dc.SelectObject(&pen);

		// draw each point
		for (int i = mRear; i != mFront; )
		{
			// x1 = x * cos(theta) - y * sin(theta)
			// y1 = x * sin(theta) + y * cos(theta)
			double theta = TO_RADIAN(180 - mPos[nLast].track);

			double x0 = mPoint[i].x * ZOOM_FACTOR;
			double y0 = mPoint[i].y * ZOOM_FACTOR;

			int x1 = rc.Width() / 2 - (int)(x0 * cos(theta) - y0 * sin(theta));
			int y1 = rc.Height() / 2 - (int)(x0 * sin(theta) + y0 * cos(theta));

			dc.Ellipse(x1 - 2, y1 - 2, x1 + 2, y1 + 2);

			i = (i + 1) % MAX_POSITION;
		}

		// draw track
		{
			int x0 = rc.left + (int)(rc.Width() / 2);
			int y0 = rc.top + (int)(rc.Height() / 2);

#if 0
			int dx = (int)(60 * sin(TO_RADIAN(0)));
			int dy = (int)(60 * cos(TO_RADIAN(0)));
			
			dc.MoveTo(x0, y0);
			dc.LineTo(x0 + dx, y0 - dy);
#else
			dc.MoveTo(x0 - 20, y0);
			dc.LineTo(x0, y0 - 30);
			dc.LineTo(x0 + 20, y0);
#endif
		}


		dc.RestoreDC(nSaveDC);
	}
}
