// XTreeCtrl.cpp : implementation file
// Download by http://www.codefans.net

#include "stdafx.h"
#include "Calibration.h"
#include "XTreeCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define   DRAG_DELAY   60
/////////////////////////////////////////////////////////////////////////////
// CXTreeCtrl

CXTreeCtrl::CXTreeCtrl()
{
	m_bDragging = false;
}

CXTreeCtrl::~CXTreeCtrl()
{
    ;
}


BEGIN_MESSAGE_MAP(CXTreeCtrl, CTreeCtrl)
	//{{AFX_MSG_MAP(CXTreeCtrl)
	ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnBegindrag)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
	ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT, &CXTreeCtrl::OnTvnEndlabeledit)
    ON_NOTIFY_REFLECT(TVN_BEGINLABELEDIT, &CXTreeCtrl::OnTvnBeginlabeledit)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CXTreeCtrl message handlers

void CXTreeCtrl::OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	*pResult = 0;

	//如果是无意拖曳，则放弃操作
	if( (GetTickCount() - m_dwDragStart) < DRAG_DELAY )
		return;

	m_hItemDragS = pNMTreeView->itemNew.hItem;
	m_hItemDragD = NULL;

	//得到用于拖动时显示的图象列表
	m_pDragImage = CreateDragImage( m_hItemDragS );
	if( !m_pDragImage )
		return;

	m_bDragging = true;
	m_pDragImage->BeginDrag ( 0,CPoint(8,8) );
	CPoint  pt = pNMTreeView->ptDrag;
	ClientToScreen( &pt );
	m_pDragImage->DragEnter ( this,pt );  //"this"将拖曳动作限制在该窗口
	SetCapture();

	m_nScrollTimerID = SetTimer( 2,40,NULL );
}

void CXTreeCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	HTREEITEM  hItem;
	UINT       flags;

	//检测鼠标敏感定时器是否存在,如果存在则删除,删除后再定时
	if( m_nHoverTimerID )
	{
		KillTimer( m_nHoverTimerID );
		m_nHoverTimerID = 0;
	}
	m_nHoverTimerID = SetTimer( 1,800,NULL );  //定时为 0.8 秒则自动展开
	m_HoverPoint = point;

	if( m_bDragging )
	{
		CPoint  pt = point;
		CImageList::DragMove( pt );

		//鼠标经过时高亮显示
		CImageList::DragShowNolock( false );  //避免鼠标经过时留下难看的痕迹
		if( (hItem = HitTest(point,&flags)) != NULL )
		{
			SelectDropTarget( hItem );
			m_hItemDragD = hItem;
		}
		CImageList::DragShowNolock( true );

		//当条目被拖曳到左边缘时，将条目放在根下
		CRect  rect;
		GetClientRect( &rect );
		if( point.x < rect.left + 20 )
			m_hItemDragD = NULL;
	}

	CTreeCtrl::OnMouseMove(nFlags, point);
}

void CXTreeCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CTreeCtrl::OnLButtonUp(nFlags, point);

	if( m_bDragging )
	{
		m_bDragging = FALSE;
		CImageList::DragLeave( this );
		CImageList::EndDrag();
		ReleaseCapture();
		delete m_pDragImage;

		SelectDropTarget( NULL );
		if( m_hItemDragS == m_hItemDragD )
		{
			KillTimer( m_nScrollTimerID );
			return;
		}

		Expand( m_hItemDragD,TVE_EXPAND );
#if 0
        HTREEITEM  htiParent = m_hItemDragD;
		while( (htiParent = GetParentItem(htiParent)) != NULL )
		{
			if( htiParent == m_hItemDragS )
			{
				HTREEITEM  htiNewTemp = CopyBranch( m_hItemDragS,NULL,TVI_LAST );
				HTREEITEM  htiNew = CopyBranch( htiNewTemp,m_hItemDragD,TVI_LAST );
				DeleteItem( htiNewTemp );
				SelectItem( htiNew );
				KillTimer( m_nScrollTimerID );
				return;
			}
		}

		HTREEITEM  htiNew = CopyBranch( m_hItemDragS,m_hItemDragD,TVI_LAST );
		DeleteItem( m_hItemDragS );
		SelectItem( htiNew );
		KillTimer( m_nScrollTimerID );
#else                                           // by ZWW 不再支持子节点，拖动时直接向前插入
        HTREEITEM hSiblingItem = GetPrevSiblingItem(m_hItemDragD);
        if(!hSiblingItem){
            hSiblingItem = TVI_FIRST;
        }
        TV_INSERTSTRUCT  tvstruct;
        HTREEITEM        hNewItem;
        CString          sText;

        //得到源条目的信息
        tvstruct.item.hItem = m_hItemDragS;
        tvstruct.item.mask  = TVIF_CHILDREN|TVIF_HANDLE|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
        GetItem( &tvstruct.item );
        sText = GetItemText( m_hItemDragS );
        tvstruct.item.cchTextMax = sText.GetLength ();
        tvstruct.item.pszText    = sText.LockBuffer ();

        //将条目插入到合适的位置
        tvstruct.hParent         = TVI_ROOT;
        tvstruct.hInsertAfter    = hSiblingItem;
        tvstruct.item.mask       = TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_TEXT;
        hNewItem = InsertItem( &tvstruct );
        sText.ReleaseBuffer ();

        //限制拷贝条目数据和条目状态
        SetItemData( hNewItem,GetItemData(m_hItemDragS) );
        SetItemState( hNewItem,GetItemState(m_hItemDragS,TVIS_STATEIMAGEMASK),TVIS_STATEIMAGEMASK);

        DeleteItem( m_hItemDragS );
        KillTimer( m_nScrollTimerID );
#endif
	}
}

//拷贝条目
HTREEITEM CXTreeCtrl::CopyItem(HTREEITEM hItem, HTREEITEM htiNewParent, HTREEITEM htiAfter)
{
	TV_INSERTSTRUCT  tvstruct;
	HTREEITEM        hNewItem;
	CString          sText;

	//得到源条目的信息
	tvstruct.item.hItem = hItem;
	tvstruct.item.mask  = TVIF_CHILDREN|TVIF_HANDLE|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
	GetItem( &tvstruct.item );
	sText = GetItemText( hItem );
	tvstruct.item.cchTextMax = sText.GetLength ();
	tvstruct.item.pszText    = sText.LockBuffer ();

	//将条目插入到合适的位置
	tvstruct.hParent         = htiNewParent;
	tvstruct.hInsertAfter    = htiAfter;
	tvstruct.item.mask       = TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_TEXT;
	hNewItem = InsertItem( &tvstruct );
	sText.ReleaseBuffer ();

	//限制拷贝条目数据和条目状态
	SetItemData( hNewItem,GetItemData(hItem) );
	SetItemState( hNewItem,GetItemState(hItem,TVIS_STATEIMAGEMASK),TVIS_STATEIMAGEMASK);

	return hNewItem;
}

//拷贝分支
HTREEITEM CXTreeCtrl::CopyBranch(HTREEITEM htiBranch, HTREEITEM htiNewParent, HTREEITEM htiAfter)
{
	HTREEITEM  hChild;
	HTREEITEM  hNewItem = CopyItem( htiBranch,htiNewParent,htiAfter );
	hChild = GetChildItem( htiBranch );

	while( hChild != NULL )
	{
		CopyBranch( hChild,hNewItem,htiAfter );
		hChild = GetNextSiblingItem( hChild );
	}

	return hNewItem;
}

void CXTreeCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	//处理无意拖曳
	m_dwDragStart = GetTickCount();
	
	CTreeCtrl::OnLButtonDown(nFlags, point);
}

void CXTreeCtrl::OnTimer(UINT nIDEvent) 
{
	//鼠标敏感节点
	if( nIDEvent == m_nHoverTimerID )
	{
		KillTimer( m_nHoverTimerID );
		m_nHoverTimerID = 0;
		HTREEITEM  trItem = 0;
		UINT  uFlag = 0;
		trItem = HitTest( m_HoverPoint,&uFlag );
		if( trItem && m_bDragging )
		{
			SelectItem( trItem );
			Expand( trItem,TVE_EXPAND );
		}
	}
	//处理拖曳过程中的滚动问题
	else if( nIDEvent == m_nScrollTimerID )
	{
		m_TimerTicks++;
		CPoint  pt;
		GetCursorPos( &pt );
		CRect  rect;
		GetClientRect( &rect );
		ClientToScreen( &rect );

		HTREEITEM  hItem = GetFirstVisibleItem();
		if( pt.y < rect.top +10 )
		{
			//向上滚动
			int  slowscroll = 6 - (rect.top + 10 - pt.y )/20;
			if( 0 == (m_TimerTicks % ((slowscroll > 0) ? slowscroll : 1)) )
			{
				CImageList::DragShowNolock ( false );
				SendMessage( WM_VSCROLL,SB_LINEUP );
				SelectDropTarget( hItem );
				m_hItemDragD = hItem;
				CImageList::DragShowNolock ( true );
			}
		}
		else if( pt.y > rect.bottom - 10 )
		{
			//向下滚动
			int  slowscroll = 6 - (pt.y - rect.bottom + 10)/20;
			if( 0 == (m_TimerTicks % ((slowscroll > 0) ? slowscroll : 1)) )
			{
				CImageList::DragShowNolock ( false );
				SendMessage( WM_VSCROLL,SB_LINEDOWN );
				int  nCount = GetVisibleCount();
				for( int i=0 ; i<nCount-1 ; i++ )
					hItem = GetNextVisibleItem( hItem );
				if( hItem )
					SelectDropTarget( hItem );
				m_hItemDragD = hItem;
				CImageList::DragShowNolock ( true );
			}
		}
	}
	else
		CTreeCtrl::OnTimer(nIDEvent);
}


void CXTreeCtrl::OnTvnEndlabeledit(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVDISPINFO pTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;

	CString csText;
	GetEditControl()->GetWindowText(csText);
	HTREEITEM hItem = GetSelectedItem();
    if(hItem){
        if(csText.GetLength() > MODULENAMEMAXLEN-1){
            MessageBox("不支持过长名称");
            SetItemText(hItem, m_csCurEditText);
            return;
        }
        if(csText.IsEmpty())
            SetItemText(hItem, m_csCurEditText);
        else
	        SetItemText(hItem, csText);

        ModuleData *pModuleData = NULL;
        pModuleData = (ModuleData *)GetItemData(hItem);
        if(pModuleData){
            strcpy(pModuleData->cName, csText.GetBuffer(0));
        }
    }
}

void CXTreeCtrl::OnTvnBeginlabeledit(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMTVDISPINFO pTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>(pNMHDR);
    // TODO: Add your control notification handler code here
    *pResult = 0;
    m_csCurEditText = pTVDispInfo->item.pszText;
}

