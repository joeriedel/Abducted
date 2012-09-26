/*! \file WaypointMesh.cpp
	\copyright Copyright (c) 2010 Sunside Inc., All Rights Reserved.
	\copyright See Abducted/LICENSE for licensing terms.
	\author Joe Riedel
	\ingroup packages
*/

#include "stdafx.h"
#include "resource.h"
#include "WaypointGrid.h"
#include "TreadDoc.h"
#include "MapView.h"
#include "Abducted.h"
#include "r_gl.h"

///////////////////////////////////////////////////////////////////////////////

CWaypoint *CWaypoint::Create(CTreadDoc *doc) {
	CWaypoint *m = new CWaypoint();
	return m;
}

CWaypoint::CWaypoint() {
	R_MakeBoxMeshCmds(&m_boxMesh, -vec3(16, 16, 16), vec3(16, 16, 16));
	m_boxMesh.color2d = 0xFF00FF00;
	m_boxMesh.color3d = m_boxMesh.color2d;
	m_boxMesh.fcolor3d[0] = 0.f;
	m_boxMesh.fcolor3d[1] = 1.f;
	m_boxMesh.fcolor3d[2] = 0.f;
	m_boxMesh.fcolor3d[3] = 1.f;
	m_boxMesh.pick = this;
	m_boxMesh.solid2d = true;
	m_boxPos = vec3::zero;
}

CWaypoint::CWaypoint(const CWaypoint &m) {
}

CWaypoint::~CWaypoint() {
	m_boxMesh.FreeMesh();
}

void CWaypoint::SetObjectWorldPos( const vec3& pos, CTreadDoc* pDoc ) {
	m_pos = pos;
	CalcBounds();
	UpdateBoxMesh();
}

void CWaypoint::GetWorldMinsMaxs(vec3 *pMins, vec3 *pMaxs) {
	*pMins = m_world[0];
	*pMaxs = m_world[1];
}

void CWaypoint::GetObjectMinsMaxs(vec3 *pMins, vec3 *pMaxs) {
	*pMins = m_local[0];
	*pMaxs = m_local[1];
}

vec3 CWaypoint::GetObjectWorldPos() {
	return m_pos;
}

void CWaypoint::Nudge(const vec3 &amt, CTreadDoc *doc) {
	m_pos += amt;
	CalcBounds();
	UpdateBoxMesh();
}

bool CWaypoint::WriteToFile( CFile* pFile, CTreadDoc* pDoc, int nVersion ) {
	return CMapObject::WriteToFile(pFile, pDoc, nVersion);
}

bool CWaypoint::ReadFromFile( CFile* pFile, CTreadDoc* pDoc, int nVersion ) {
	return CMapObject::ReadFromFile(pFile, pDoc, nVersion);
}

CLinkedList<CObjProp>* CWaypoint::GetPropList( CTreadDoc* pDoc ) {
	return 0;
}

void CWaypoint::SetProp( CTreadDoc* pDoc, CObjProp* prop ) {
	CMapObject::SetProp(pDoc, prop);
}

void CWaypoint::OnAddToMap( CTreadDoc* pDoc ) {

	CMapObject::OnAddToMap(pDoc);
}

void CWaypoint::OnMouseDown( CMapView* pView, int nMX, int nMY, int nButtons, CPickObject* pSrc ) {

	bool redraw = false;
	m_drag = false;

	if (IsSelected()) {
		if( nButtons&MS_CONTROL )
		{
			pView->GetDocument()->MakeUndoDeselectAction();
			Deselect( pView->GetDocument() );
			pView->GetDocument()->Prop_UpdateSelection();
			pView->GetDocument()->UpdateSelectionInterface();
			redraw = true;
		}
		else
		{
			if( pView->GetViewType() != VIEW_TYPE_3D )
			{
				m_drag = true;
				Sys_BeginDragSel( pView, nMX, nMY, nButtons );
			}
			else
			{
				pView->GetDocument()->MakeUndoDeselectAction();
				pView->GetDocument()->ClearSelection();
				Select( pView->GetDocument() );
				pView->GetDocument()->Prop_UpdateSelection();
				pView->GetDocument()->UpdateSelectionInterface();
				redraw = true;
			}
		}
	} else {
		if( !(nButtons&MS_CONTROL) )
		{
			pView->GetDocument()->MakeUndoDeselectAction();
			pView->GetDocument()->ClearSelection();
		}

		Select( pView->GetDocument(), pSrc );
		pView->GetDocument()->Prop_UpdateSelection();
		redraw = true;

		pView->GetDocument()->UpdateSelectionInterface();
			
		if( pView->GetViewType() != VIEW_TYPE_3D )
		{
			m_drag = true;
			Sys_BeginDragSel( pView, nMX, nMY, nButtons );
		}
	}

	if( redraw )
		Sys_RedrawWindows();
}

void CWaypoint::OnMouseUp( CMapView* pView, int nMX, int nMY, int nButtons, CPickObject* pSrc ) {
	if( m_drag )
	{
		Sys_EndDragSel( pView, nMX, nMY, nButtons );
		pView->GetDocument()->UpdateSelectionInterface();
	}
}

void CWaypoint::OnMouseMove( CMapView* pView, int nMX, int nMY, int nButtons, CPickObject* pSrc ) {
	if( m_drag )
		Sys_DragSel( pView, nMX, nMY, nButtons );
}

void CWaypoint::OnAddToSelection( CTreadDoc* pDoc ) {
	CMapObject::OnAddToSelection(pDoc);
}

void CWaypoint::OnRemoveFromMap( CTreadDoc* pDoc ) {
	CMapObject::OnRemoveFromMap(pDoc);
}

CMapObject* CWaypoint::Clone() {
	return new CWaypoint(*this);
}

int CWaypoint::GetNumRenderMeshes( CMapView* pView ) {
	return 1;
}

CRenderMesh* CWaypoint::GetRenderMesh( int num, CMapView* pView ) {
	return &m_boxMesh;
}

const char* CWaypoint::GetRootName() {
	return "Waypoint Mesh";
}

void CWaypoint::CalcBounds() {
	m_local[1] = vec3(8.f, 8.f, 8.f);
	m_local[0] = -m_local[1];

	m_world[0] = m_local[0] + m_pos;
	m_world[1] = m_local[1] + m_pos;
}

void CWaypoint::UpdateBoxMesh() {
	vec3 t = m_pos - m_boxPos;
	for (int i = 0; i < m_boxMesh.num_pts; ++i)
		m_boxMesh.xyz[i] += t;
	m_boxPos = m_pos;
}

bool CWaypoint::OnPopupMenu( CMapView* pView, int nMX, int nMY, int nButtons, CPickObject* pSrc) {
	AbductedUserData *ud = static_cast<AbductedUserData*>(pView->GetDocument()->UserData());
	ud->MakeWaypointMenu(pView->GetDocument());
	ud->m_WaypointMenu.view = pView;
	ud->m_WaypointMenu.waypoint = this;
	
	Sys_DisplayObjectMenu(pView, nMX, nMY, &ud->m_WaypointMenu);
	return true;
}

void CWaypoint::PopupMenu_OnConnectWaypoints(CMapView *view) {
}

void CWaypoint::PopupMenu_OnDisconnectWaypoints(CMapView *view) {
}

///////////////////////////////////////////////////////////////////////////////

CWaypoint::ContextMenu::ContextMenu() {
}

CWaypoint::ContextMenu::~ContextMenu() {
}

void CWaypoint::ContextMenu::OnUpdateCmdUI(int id, CCmdUI *ui) {

	switch (id) {
	default:
		break;
	}
}

void CWaypoint::ContextMenu::OnMenuItem(int id) {

	switch (id) {
	default:
		break;
	}
}
