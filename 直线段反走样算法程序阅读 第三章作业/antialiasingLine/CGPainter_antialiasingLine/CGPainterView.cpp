// CGPainterView.cpp : implementation of the CCGPainterView class
//

#include "stdafx.h"
#include "CGPainter.h"
#include "GlobalVariables.h"
#include "math.h"

#include "CGPainterDoc.h"
#include "CGPainterView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCGPainterView

IMPLEMENT_DYNCREATE(CCGPainterView, CView)

BEGIN_MESSAGE_MAP(CCGPainterView, CView)
	//{{AFX_MSG_MAP(CCGPainterView)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCGPainterView construction/destruction

CCGPainterView::CCGPainterView()
{
	m_pMemDC=new CDC;
	m_pBitmap=new CBitmap;
	m_nMaxX=GetSystemMetrics(SM_CXSCREEN);
	m_nMaxY=GetSystemMetrics(SM_CYSCREEN);

	G_iDrawState = DRAW_NOTHING;
	G_cLineColor = RGB(0,0,0);

	G_iMouseState = NO_BUTTON;
}

CCGPainterView::~CCGPainterView()
{
	delete m_pMemDC;
	delete m_pBitmap;
}

BOOL CCGPainterView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CCGPainterView drawing

void CCGPainterView::OnDraw(CDC* pDC)
{
	CCGPainterDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	/////////////////////////////////////////
	CBitmap* pOldBitmap = m_pMemDC->SelectObject(m_pBitmap);
	pDC->BitBlt(0,0,m_nMaxX,m_nMaxY,m_pMemDC,0,0,SRCCOPY);
	m_pMemDC->SelectObject(pOldBitmap);
}

/////////////////////////////////////////////////////////////////////////////
// CCGPainterView printing

BOOL CCGPainterView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CCGPainterView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CCGPainterView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CCGPainterView diagnostics

#ifdef _DEBUG
void CCGPainterView::AssertValid() const
{
	CView::AssertValid();
}

void CCGPainterView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CCGPainterDoc* CCGPainterView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCGPainterDoc)));
	return (CCGPainterDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CCGPainterView message handlers

void CCGPainterView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	G_iMouseState = MOUSE_LEFT_BUTTON;
	m_ptStart=point;
	m_ptOld=point;
}

void CCGPainterView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	G_iMouseState = NO_BUTTON;

	if(G_iDrawState == DRAW_NOTHING)
		return;
	
	//set the drawing context
	CDC* pDC=GetDC();
	CBitmap* pOldBitmap=m_pMemDC->SelectObject(m_pBitmap);
	
	CPen pen;
	pen.CreatePen(PS_SOLID, 1, G_cLineColor);
	CPen* pOldPen=m_pMemDC->SelectObject(&pen);
	m_pMemDC->SelectObject(&pen);

	CBrush* pOldBrush=(CBrush*)m_pMemDC->SelectStockObject(NULL_BRUSH);
	m_pMemDC->SelectStockObject(NULL_BRUSH);

	//draw graph
	if(G_iDrawState == DRAW_LINE)
		DrawLine(m_pMemDC, m_ptStart, point, G_cLineColor);
	else if(G_iDrawState == DRAW_CIRCLE)
	{
		CPoint dist = point - m_ptStart;
		int radius = int(sqrt(float(dist.x*dist.x + dist.y*dist.y)) + 0.5);
		DrawCircle(m_pMemDC, m_ptStart, radius, G_cLineColor);
	}
	else if(G_iDrawState == DRAW_ANTIALIASING_LINE)
		DrawAntiLine(m_pMemDC, m_ptStart, point, G_cLineColor);


	//retrieve the old contex
	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldBrush);
	m_pMemDC->SelectObject(pOldBitmap);
	m_pMemDC->SelectObject(pOldPen);
	m_pMemDC->SelectObject(pOldBrush);
	ReleaseDC(pDC);
}

void CCGPainterView::OnMouseMove(UINT nFlags, CPoint point) 
{
	if(G_iDrawState == DRAW_NOTHING)
		return;
	
	if(G_iMouseState != MOUSE_LEFT_BUTTON)
		return;

	//set drawing context
	CDC* pDC=GetDC();
	CBitmap* pOldBitmap = m_pMemDC->SelectObject(m_pBitmap);
	
	CPen pen;
	pen.CreatePen(PS_SOLID, 1, G_cLineColor);
	CPen* pOldPen=m_pMemDC->SelectObject(&pen);
	pDC->SelectObject(&pen);
	
	CBrush* pOldBrush=(CBrush*)m_pMemDC->SelectStockObject(NULL_BRUSH);
	pDC->SelectStockObject(NULL_BRUSH);

	//wipe the screen. This is used to realize the "elastic band" effect.
	CRect wipeRect(m_ptStart, m_ptOld);
	int radius;
	if(G_iDrawState == DRAW_CIRCLE)		//if drawing circle, the wipeRect is larger
	{
		CPoint dist = m_ptOld - m_ptStart;
		radius = int(sqrt(float(dist.x*dist.x + dist.y*dist.y)) + 0.5);

		CPoint ptLeftTop(m_ptStart.x - radius, m_ptStart.y - radius);
		CPoint ptRightBottom(m_ptStart.x + radius, m_ptStart.y + radius);
		wipeRect = CRect(ptLeftTop, ptRightBottom);
	}
	wipeRect.NormalizeRect();
	wipeRect.InflateRect(2, 2);
	pDC->BitBlt(wipeRect.left, wipeRect.top,
				wipeRect.Width(), wipeRect.Height(),
				m_pMemDC,
				wipeRect.left,wipeRect.top,
				SRCCOPY);

	//draw graph
	if(G_iDrawState == DRAW_LINE)
		DrawLine(pDC, m_ptStart, point, G_cLineColor);
	else if(G_iDrawState == DRAW_CIRCLE)
	{
		CPoint dist = point - m_ptStart;
		int radius = int(sqrt(float(dist.x*dist.x + dist.y*dist.y)) + 0.5);
		DrawCircle(pDC, m_ptStart, radius, G_cLineColor);
	}
	else if(G_iDrawState == DRAW_ANTIALIASING_LINE)
		DrawAntiLine(pDC, m_ptStart, point, G_cLineColor);
	
	//retrieve the old contex
	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldBrush);
	m_pMemDC->SelectObject(pOldBitmap);
	m_pMemDC->SelectObject(pOldPen);
	m_pMemDC->SelectObject(pOldBrush);
	ReleaseDC(pDC);
	m_ptOld=point;
}

void CCGPainterView::DrawLine(CDC *pDC, CPoint ptStartPoint, CPoint ptEndPoint, COLORREF cLineColor)
{
	pDC->MoveTo(ptStartPoint);
	pDC->LineTo(ptEndPoint);

	//write the Bresenham' line algorithm for drawing the line
	//use function:
	//pDC->SetPixelV(point, cLineColor); to drawing a pixel
	




}

int CCGPainterView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	CDC* pDC=GetDC();
	m_pMemDC->CreateCompatibleDC(pDC);
	m_pBitmap->CreateCompatibleBitmap(pDC,m_nMaxX,m_nMaxY); 
	CBitmap* pOldBitmap=m_pMemDC->SelectObject(m_pBitmap);
	CBrush brush;
	brush.CreateStockObject(WHITE_BRUSH);
	CRect rect(-1,-1,m_nMaxX,m_nMaxY);
	m_pMemDC->FillRect(rect,&brush);
	
	m_pMemDC->SelectObject(pOldBitmap);
	ReleaseDC(pDC);
	
	return 0;
}

void CCGPainterView::DrawCircle(CDC *pDC, CPoint ptOrigin, int iRadius, COLORREF cLineColor)
{
	CPoint ptLeftTop(ptOrigin.x - iRadius, ptOrigin.y - iRadius);
	CPoint ptRightBottom(ptOrigin.x + iRadius, ptOrigin.y + iRadius);
	CRect circleRect(ptLeftTop, ptRightBottom);
	pDC->Ellipse(circleRect);

	//write the Midpoint circle algorithm for drawing the circle
	//use function:
	//pDC->SetPixelV(point, cLineColor); to drawing a pixel

}

void CCGPainterView::DrawAntiLine(CDC *pDC, CPoint ptStartPoint, CPoint ptEndPoint, COLORREF cLineColor)
{
	float accumulate;
	int i;	int dx, dy;
	int x_acc, y_acc;
	int xx, yy;
	int interchange;
	float slope;
	int x0 = ptStartPoint.x;
	int y0 = ptStartPoint.y;
	int x1 = ptEndPoint.x;
	int y1 = ptEndPoint.y;
	
	if(x0==x1)
	{
		if(y1>=y0)
			for(i=y0; i<=y1; i++)
				pDC->SetPixelV(x0, i, cLineColor);
		else
			for(i=y0; i>=y1; i--)
				pDC->SetPixelV(x0, i, cLineColor);
		
		return;
	}
	
	if(y0==y1)
	{
		if(x1>=x0)
			for(i=x0; i<=x1; i++)
				pDC->SetPixelV(i, y0, cLineColor);
		else
			for(i=x0; i>=x1; i--)
				pDC->SetPixelV(i, y0, cLineColor);
		
		return;
	}
//////////////////////////////////
	if(y1>y0)
	{
		dy=y1-y0;
		y_acc=1;
	}
	else
	{
		dy=y0-y1;
		y_acc=-1;
	}

	if(x1>x0)
	{
		dx=x1-x0;
		x_acc=1;
	}
	else
	{
		dx=x0-x1;
		x_acc=-1;
	}
//////////////////////////////////////
	if(dx==dy)
	{
		xx=x0; yy=y0;
		for(i=0; i<=dx; i++)
		{
			pDC->SetPixelV(xx, yy, cLineColor);
			xx+=x_acc;   yy+=y_acc;
		}
	}
//////////////////////////////////////
	accumulate=0;
	xx=x0; yy=y0;

	if(dx>=dy)
	{
		interchange=0;
		slope=dy/(float)dx;
	}
	else
	{
		interchange=1;
		slope=dx/(float)dy;
		int temp=dx;
		dx=dy;
		dy=temp;
	}
/////////COLORREF's order is A,B,G,R.
	COLORREF color;
	float red, green, blue;
	red=(float)(cLineColor&0x000000ff);
	green=(float)((cLineColor>>8)&0x000000ff);
	blue=(float)(cLineColor>>16);
	pDC->SetPixelV(xx, yy, cLineColor);
	BYTE red1, green1, blue1;
	//background's color;
	COLORREF bkcolor;
	BYTE bkred, bkgreen, bkblue;

	for(i=1; i<=dx; i++)
	{
		
		if(interchange==0)
		{
			xx+=x_acc;
			accumulate+=slope;
			if(accumulate>=1)
			{
				yy+=y_acc;
				accumulate-=1;
			}
			//get the bk's color.
			bkcolor = pDC->GetPixel(xx, yy);
			bkred=(BYTE)(bkcolor&0x000000ff);
			bkgreen=(BYTE)((bkcolor>>8)&0x000000ff);
			bkblue=(BYTE)(bkcolor>>16);

			color=0;
			red1=(BYTE)(red*(1-accumulate)+bkred*accumulate);
			green1=(BYTE)(green*(1-accumulate)+bkgreen*accumulate);
			blue1=(BYTE)(blue*(1-accumulate)+bkblue*accumulate);
			color=RGB(red1, green1, blue1);
		//	color=color|(int)(red*(1-accumulate)+255*accumulate);
		//	color=(color<<8)|(int)(green*(1-accumulate)+255*accumulate);
		//	color=(color<<8)|(int)(blue*(1-accumulate)+255*accumulate);
			pDC->SetPixelV(xx, yy, color);

			//get the bk's color.
			bkcolor = pDC->GetPixel(xx, yy+y_acc);
			bkred=(BYTE)(bkcolor&0x000000ff);
			bkgreen=(BYTE)((bkcolor>>8)&0x000000ff);
			bkblue=(BYTE)(bkcolor>>16);

			color=0;
			red1=(BYTE)(red*accumulate+bkred*(1-accumulate));
			green1=(BYTE)(green*accumulate+bkgreen*(1-accumulate));
			blue1=(BYTE)(blue*accumulate+bkblue*(1-accumulate));
			color=RGB(red1, green1, blue1);
	//		color=color|(int)(red*accumulate+255*(1-accumulate));
	//		color=(color<<8)|(int)(green*accumulate+255*(1-accumulate));
	//		color=(color<<8)|(int)(blue*accumulate+255*(1-accumulate));
			pDC->SetPixelV(xx, yy+y_acc, color);
		}
		else
		{
			yy+=y_acc;
			accumulate+=slope;
			if(accumulate>=1)
			{
				xx+=x_acc;
				accumulate-=1;
			}
			
			//get the bk's color.
			bkcolor = pDC->GetPixel(xx, yy);
			bkred=(BYTE)(bkcolor&0x000000ff);
			bkgreen=(BYTE)((bkcolor>>8)&0x000000ff);
			bkblue=(BYTE)(bkcolor>>16);
			
			color=0;
//			color=color|(int)(red*(1-accumulate));
//			color=(color<<8)|(int)(green*(1-accumulate));
//			color=(color<<8)|(int)(blue*(1-accumulate));

			color=color|(int)(red*(1-accumulate)+bkred*accumulate);
			color=color|((int)(green*(1-accumulate)+bkgreen*accumulate)<<8);
			color=color|((int)(blue*(1-accumulate)+bkblue*accumulate)<<16);
			pDC->SetPixelV(xx, yy, color);
		
			//get the bk's color.
			bkcolor = pDC->GetPixel(xx+x_acc, yy);
			bkred=(BYTE)(bkcolor&0x000000ff);
			bkgreen=(BYTE)((bkcolor>>8)&0x000000ff);
			bkblue=(BYTE)(bkcolor>>16);

			color=0;
//			color=color|(int)(red*accumulate);
//			color=(color<<8)|(int)(green*accumulate);
//			color=(color<<8)|(int)(blue*accumulate);

			color=color|(int)(red*accumulate+bkred*(1-accumulate));
			color=color|((int)(green*accumulate+bkgreen*(1-accumulate))<<8);
			color=color|((int)(blue*accumulate+bkblue*(1-accumulate))<<16);
			pDC->SetPixelV(xx+x_acc, yy, color);
		}
	}
}
