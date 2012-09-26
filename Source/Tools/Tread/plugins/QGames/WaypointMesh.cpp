/*! \file WaypointMesh.cpp
	\copyright Copyright (c) 2010 Sunside Inc., All Rights Reserved.
	\copyright See Abducted/LICENSE for licensing terms.
	\author Joe Riedel
	\ingroup packages
*/

#include "stdafx.h"
#include "resource.h"
#include "WaypointMesh.h"
#include "TreadDoc.h"
#include "MapView.h"
#include "Abducted.h"

///////////////////////////////////////////////////////////////////////////////

CWaypointMesh *CWaypointMesh::Create(CTreadDoc *doc) {
	CWaypointMesh *m = new CWaypointMesh();
	m->m_creating = true;
	return m;
}

int CWaypointMesh::EnterEditMode(CMapObject* p, void* parm, void* parm2) {
	CWaypointMesh *m = dynamic_cast<CWaypointMesh*>(p);
	if (m)
		m->EnableManipulators(Sys_GetActiveDocument(), true);
	return 0;
}

int CWaypointMesh::ExitEditMode(CMapObject* p, void* parm, void* parm2) {
	CWaypointMesh *m = dynamic_cast<CWaypointMesh*>(p);
	if (m)
		m->EnableManipulators(Sys_GetActiveDocument(), false);
	return 0;
}

CWaypointMesh::CWaypointMesh() : m_nextWaypointId(0), m_creating(false), m_pos(vec3::zero), m_drag(false) {
}

CWaypointMesh::CWaypointMesh(const CWaypointMesh &m) {
}

CWaypointMesh::~CWaypointMesh() {
}

void CWaypointMesh::SetObjectWorldPos( const vec3& pos, CTreadDoc* pDoc ) {
	if (m_creating) {
		m_pos = pos;
		m_creating = false;
		AddWaypoint(pos);
		CalcBounds();
		MakeManipulators(pDoc);
		EnableManipulators(pDoc, false);
		return;
	}
	Move(pos - m_pos);
	CalcBounds();
	m_pos = pos;
}

void CWaypointMesh::Move(const vec3 &delta) {
	for (Waypoint::List::const_iterator it = m_waypoints.begin(); it != m_waypoints.end(); ++it) {
		const Waypoint::Ref &waypoint = *it;
		waypoint->pos += delta;
	}
}

void CWaypointMesh::GetWorldMinsMaxs(vec3 *pMins, vec3 *pMaxs) {
	*pMins = m_world[0];
	*pMaxs = m_world[1];
}

void CWaypointMesh::GetObjectMinsMaxs(vec3 *pMins, vec3 *pMaxs) {
	if (m_creating) {
		*pMins = vec3(-64, -64, -64);
		*pMaxs = vec3( 64,  64,  64);
		return;
	}

	*pMins = m_local[0];
	*pMaxs = m_local[1];
}

vec3 CWaypointMesh::GetObjectWorldPos() {
	return m_pos;
}

void CWaypointMesh::Nudge(const vec3 &amt, CTreadDoc *doc) {
}

bool CWaypointMesh::WriteToFile( CFile* pFile, CTreadDoc* pDoc, int nVersion ) {
	return CMapObject::WriteToFile(pFile, pDoc, nVersion);
}

bool CWaypointMesh::ReadFromFile( CFile* pFile, CTreadDoc* pDoc, int nVersion ) {
	return CMapObject::ReadFromFile(pFile, pDoc, nVersion);
}

CLinkedList<CObjProp>* CWaypointMesh::GetPropList( CTreadDoc* pDoc ) {
	return 0;
}

void CWaypointMesh::SetProp( CTreadDoc* pDoc, CObjProp* prop ) {
	CMapObject::SetProp(pDoc, prop);
}

void CWaypointMesh::OnAddToMap( CTreadDoc* pDoc ) {

	if (!IsAttached())
		MakeManipulators(pDoc);

	EnableManipulators(pDoc, false);
	CMapObject::OnAddToMap(pDoc);
}

void CWaypointMesh::OnMouseDown( CMapView* pView, int nMX, int nMY, int nButtons, CPickObject* pSrc ) {

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

void CWaypointMesh::OnMouseUp( CMapView* pView, int nMX, int nMY, int nButtons, CPickObject* pSrc ) {
	if( m_drag )
	{
		Sys_EndDragSel( pView, nMX, nMY, nButtons );
		pView->GetDocument()->UpdateSelectionInterface();
	}
}

void CWaypointMesh::OnMouseMove( CMapView* pView, int nMX, int nMY, int nButtons, CPickObject* pSrc ) {
	if( m_drag )
		Sys_DragSel( pView, nMX, nMY, nButtons );
}

void CWaypointMesh::OnAddToSelection( CTreadDoc* pDoc ) {
	if (pDoc->IsEditingWaypoints() && !pDoc->IsInTrackAnimationMode()) {
		EnableManipulators(pDoc, true);
	}
}

void CWaypointMesh::OnRemoveFromMap( CTreadDoc* pDoc ) {
	FreeManipulators(pDoc);
	CMapObject::OnRemoveFromMap(pDoc);
}

CMapObject* CWaypointMesh::Clone() {
	return new CWaypointMesh(*this);
}

int CWaypointMesh::GetNumRenderMeshes( CMapView* pView ) {
	m_currentRenderMeshIdx = 0;
	return (int)m_connections.size();
}

CRenderMesh* CWaypointMesh::GetRenderMesh( int num, CMapView* pView ) {
	if (m_currentRenderMeshIdx > num) {
		m_currentRenderMeshIdx = 0;
		m_currentRenderMesh = m_connections.begin();
	}
	while (m_currentRenderMeshIdx < num) {
		++m_currentRenderMeshIdx;
		++m_currentRenderMesh;
	}

	OS_ASSERT(m_currentRenderMesh != m_connections.end());

	return m_currentRenderMesh->second->mesh.get();
}

const char* CWaypointMesh::GetRootName() {
	return "Waypoint Mesh";
}

CWaypointMesh::Waypoint* CWaypointMesh::AddWaypoint(vec3 pos) {
	Waypoint::Ref w(new Waypoint(m_nextWaypointId++));
	w->name.Format("Waypoint %d", w->id);
	w->pos = pos;
	m_waypoints.push_back(w);
	w->it = --(m_waypoints.end());
	return w.get();
}

void CWaypointMesh::DeleteWaypoint(Waypoint *waypoint) {
	Waypoint::Ref w = *waypoint->it;
	
	while (!w->connections.empty()) {
		Connection *c = w->connections.begin()->second;
		Disconnect(*c->pair.waypoints[0], *c->pair.waypoints[1]);
	}

	m_waypoints.erase(w->it);
}

bool CWaypointMesh::IsConnected(Waypoint &a, Waypoint &b) {
	WaypointPair p;
	p.waypoints[0] = &a;
	p.waypoints[1] = &b;
	return m_connections.find(p) != m_connections.end();
}

bool CWaypointMesh::Connect(Waypoint &a, Waypoint &b) {
	if (IsConnected(a, b))
		return false;
	
	Connection::Ref c(new Connection());
	c->pair.waypoints[0] = &a;
	c->pair.waypoints[1] = &b;

	c->UpdateRenderMesh();
	a.connections[&b] = c.get();
	b.connections[&a] = c.get();

	m_connections[c->pair] = c;

	return true;
}

void CWaypointMesh::Disconnect(Waypoint &a, Waypoint &b) {
	WaypointPair p;
	p.waypoints[0] = &a;
	p.waypoints[1] = &b;

	Connection::Map::iterator it = m_connections.find(p);
	if (it == m_connections.end()) {
		p.waypoints[0] = &b;
		p.waypoints[1] = &a;
		it = m_connections.find(p);
		if (it == m_connections.end())
			return;
	}

	Connection::Ref r = it->second;
	m_connections.erase(it);
	a.connections.erase(&b);
	b.connections.erase(&a);
}

void CWaypointMesh::CalcBounds() {
	m_world[0] = vec3::bogus_max;
	m_world[1] = vec3::bogus_min;

	for (Waypoint::List::const_iterator it = m_waypoints.begin(); it != m_waypoints.end(); ++it) {
		const Waypoint::Ref &waypoint = *it;
		m_world[0] = vec_mins(m_world[0], waypoint->pos);
		m_world[1] = vec_maxs(m_world[1], waypoint->pos);
	}

	m_local[1] = ((m_world[1]-m_world[2]) / 2.f);
	m_local[0] = -m_local[1];
}

void CWaypointMesh::UpdateRenderMeshes() {
}

void CWaypointMesh::MakeManipulators(CTreadDoc *doc) {
	for (Waypoint::List::const_iterator it = m_waypoints.begin(); it != m_waypoints.end(); ++it) {
		const Waypoint::Ref &waypoint = *it;
		if (!waypoint->gizmo) {
			waypoint->gizmo = WaypointGizmo::Create(waypoint.get(), this);
			OS_ASSERT(waypoint->gizmo);
			doc->AddManipulatorToMap(waypoint->gizmo);
		}
	}
}

void CWaypointMesh::FreeManipulators(CTreadDoc *doc) {
	doc->ClearAllTrackPicks();
	for (Waypoint::List::const_iterator it = m_waypoints.begin(); it != m_waypoints.end(); ++it) {
		const Waypoint::Ref &waypoint = *it;
		if (waypoint->gizmo) {
			doc->DetachManipulator(waypoint->gizmo);
			delete waypoint->gizmo;
			waypoint->gizmo = 0;
		}
	}
}

void CWaypointMesh::EnableManipulators(CTreadDoc *doc, bool enabled) {
	for (Waypoint::List::const_iterator it = m_waypoints.begin(); it != m_waypoints.end(); ++it) {
		const Waypoint::Ref &waypoint = *it;
		if (waypoint->gizmo) {
			waypoint->gizmo->SetParent(enabled ? 0 : this);
			waypoint->gizmo->dragging = false;
			waypoint->gizmo->hover = false;
			doc->AddManipulatorToMap(waypoint->gizmo);
		}
	}
}

void CWaypointMesh::OnWaypointDrag(Waypoint &waypoint) {
	// rebuild all connections with this waypoint in it.
	for (Waypoint::ConnectionMap::const_iterator it = waypoint.connections.begin(); it != waypoint.connections.end(); ++it) {
		Connection *c = it->second;
		c->UpdateRenderMesh();
	}
	CalcBounds();
}

void CWaypointMesh::SelectManipulators(CTreadDoc *doc, bool select) {
	for (Waypoint::List::const_iterator it = m_waypoints.begin(); it != m_waypoints.end(); ++it) {
		const Waypoint::Ref &waypoint = *it;
		if (waypoint->gizmo) {
			if (select) {
				doc->AddManipulatorToSelection(waypoint->gizmo);
			} else {
				doc->AddManipulatorToMap(waypoint->gizmo);
			}
		}
	}
}

int CWaypointMesh::NumSelectedManipulators() {
	int numSel = 0;
	for (Waypoint::List::const_iterator it = m_waypoints.begin(); it != m_waypoints.end(); ++it) {
		const Waypoint::Ref &waypoint = *it;
		if (waypoint->gizmo) {
			if (waypoint->gizmo->IsSelected())
				++numSel;
		}
	}

	return numSel;
}

bool CWaypointMesh::OnPopupMenu( CMapView* pView, int nMX, int nMY, int nButtons, CPickObject* pSrc) {
	WaypointGizmo *gizmo = dynamic_cast<WaypointGizmo*>(pSrc);
	if (!gizmo)
		return false;
	if (!gizmo->IsSelected()) {
		pView->GetDocument()->AddManipulatorToSelection(gizmo);
	}
	AbductedUserData *ud = static_cast<AbductedUserData*>(pView->GetDocument()->UserData());
	ud->MakeWaypointMenu(pView->GetDocument());
	ud->m_WaypointMenu.view = pView;
	ud->m_WaypointMenu.mesh = this;
	ud->m_WaypointMenu.waypoint = gizmo->waypoint;

	Sys_DisplayObjectMenu(pView, nMX, nMY, &ud->m_WaypointMenu);
	return true;
}

void CWaypointMesh::PopupMenu_OnAddWaypoint(CMapView *view, Waypoint *context) {

	vec3 pos = context->pos;

	if (view->GetViewType() == VIEW_TYPE_3D) {
	} else {
		int x_axis = QUICK_AXIS( view->View.or2d.lft );
		int y_axis = QUICK_AXIS( view->View.or2d.up );

		vec3 pos;
		pos[x_axis] += 64.f;
		pos[y_axis] += 64.f;
	}

	Waypoint *waypoint = AddWaypoint(pos);
	MakeManipulators(view->GetDocument());
	EnableManipulators(view->GetDocument(), true);
	SelectManipulators(view->GetDocument(), false);
	view->GetDocument()->AddManipulatorToSelection(waypoint->gizmo);
}

void CWaypointMesh::PopupMenu_OnConnectWaypoints(CMapView *view, Waypoint *context) {
}

void CWaypointMesh::PopupMenu_OnDisconnectWaypoints(CMapView *view, Waypoint *context) {
}

void CWaypointMesh::PopupMenu_OnDeleteSelectedWaypoints(CMapView *view, Waypoint *context) {
}

void CWaypointMesh::PopupMenu_OnEditWaypoint(CMapView *view, Waypoint *context) {
}

///////////////////////////////////////////////////////////////////////////////

int CWaypointMesh::WaypointPair::Compare(const WaypointPair &p) const {
	if (waypoints[0] < p.waypoints[0])
		return -1;
	if (waypoints[0] > p.waypoints[0])
		return -1;
	if (waypoints[1] < p.waypoints[1])
		return -1;
	if (waypoints[1] > p.waypoints[1])
		return 1;
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

CWaypointMesh::Connection::~Connection() {
	if (mesh)
		mesh->FreeMesh();
}

void CWaypointMesh::Connection::UpdateRenderMesh() {
	if (!mesh) {
		mesh.reset(new CRenderMesh());

		mesh->cmds = GL_LINES;
		mesh->num_pts = 2;
		mesh->xyz = new vec3[2];
		mesh->color2d = 0xff400040;
		mesh->color3d = 0xff400040;
		mesh->wireframe3d = 0xffdddddd;
		mesh->fcolor3d[0] = 0.f;
		mesh->fcolor3d[1] = 1.f;
		mesh->fcolor3d[2] = 0.f;
		mesh->fcolor3d[3] = 1.f;
		mesh->num_tris = 2;
		mesh->tris = new unsigned int[2];
		mesh->allow_selected = true;
		mesh->line_size = 4;
	}

	mesh->tris[0] = 0;
	mesh->tris[1] = 1;
	mesh->xyz[0] = pair.waypoints[0]->pos;
	mesh->xyz[1] = pair.waypoints[1]->pos;
}

///////////////////////////////////////////////////////////////////////////////

CWaypointMesh::Waypoint::Waypoint(int _id) : id(_id), gizmo(0) {
	name.Format("Node %d", id);
}

///////////////////////////////////////////////////////////////////////////////

// Manipulates the waypoints in the 2D views.

CWaypointMesh::WaypointGizmo::WaypointGizmo() : 
waypoint(0), 
mesh(0), 
size(10.f), 
color(0xff00ff00), 
hlcolor(0xff4de6f2), 
dragging(false) {
	SetViewFlags(VIEW_FLAG_MAP);
}

CWaypointMesh::WaypointGizmo::~WaypointGizmo() {
}

CWaypointMesh::WaypointGizmo *CWaypointMesh::WaypointGizmo::Create(Waypoint *waypoint, CWaypointMesh *mesh) {
	WaypointGizmo *gizmo = new WaypointGizmo();
	gizmo->waypoint = waypoint;
	gizmo->mesh = mesh;
	return gizmo;
}

void CWaypointMesh::WaypointGizmo::OnMouseDown( CMapView* pView, int nMX, int nMY, int nButtons, CPickObject* pSrc ) {
	
	snap = false;
	dragging = false;
	moved = false;

	if(!IsSelected()) {
		if( !(nButtons&MS_CONTROL) ) {
			pView->GetDocument()->ClearSelectedManipulators();
			dragging = true;
		}

		pView->GetDocument()->AddManipulatorToSelection( this );
		pView->GetDocument()->UpdateSelectionInterface();
		Sys_RedrawWindows();

	} else {
		if( (nButtons&MS_CONTROL) ) {
			pView->GetDocument()->AddManipulatorToMap( this );
			pView->GetDocument()->UpdateSelectionInterface();
			Sys_RedrawWindows();
		} else {
			dragging = true;
		}
	}

	dragging = dragging && (pView->GetViewType() != VIEW_TYPE_3D);

	if (dragging) {
		Sys_SetMouseCapture(pView);
	}
}

void CWaypointMesh::WaypointGizmo::OnMouseUp( CMapView* pView, int nMX, int nMY, int nButtons, CPickObject* pSrc ) {
	Sys_SetMouseCapture( 0 );
	pView->GetDocument()->ClearAllTrackPicks();
	pView->GetDocument()->BuildSelectionBounds();
	Sys_RedrawWindows();
}

void CWaypointMesh::WaypointGizmo::OnMouseMove( CMapView* pView, int nMX, int nMY, int nButtons, CPickObject* pSrc ) {

	if (!dragging)
		return;

	float mx, my;
	float dx, dy;
	int x_axis, y_axis;

	pView->WinXYToMapXY( nMX, nMY, &mx, &my );

	// snap to grid
	if( pView->GetGridSnap() ) {
		mx = Sys_Snapf( mx, pView->GetGridSize() );
		my = Sys_Snapf( my, pView->GetGridSize() );

		if(snap && mx == lastx && my == lasty)
			return;
		
		snap = true;
		lastx = mx;
		lasty = my;
	}

	x_axis = QUICK_AXIS( pView->View.or2d.lft );
	y_axis = QUICK_AXIS( pView->View.or2d.up );

	dx = mx - waypoint->pos[x_axis];
	dy = my - waypoint->pos[y_axis];

	if (!moved && (dx != 0 || dy != 0)) {
		moved = true;
		pView->GetDocument()->GenericUndoRedoFromSelection()->SetTitle("Waypoint Drag");
	}

	//
	// translate the remaining vertices.
	//
	vec3 drag = vec3::zero;

	drag[x_axis] = dx;
	drag[y_axis] = dy;

	pView->GetDocument()->GetSelectedManipulatorList()->WalkList( Drag, &drag );
	pView->GetDocument()->BuildSelectionBounds();

	Sys_RedrawWindows();
}

void CWaypointMesh::WaypointGizmo::OnMouseEnter( CMapView* pView, int nMX, int nMY, int nButtons, CPickObject* pSrc ) {

	hover = true;

	if(IsSelected()) {
		pView->GetDocument()->AddManipulatorToSelection( this ); // this little trick pushes it forward to be visible in all views.
	} else {
		pView->GetDocument()->AddManipulatorToMap( this ); // this little trick pushes it forward to be visible in all views.
	}

	Sys_RedrawWindows();
}

void CWaypointMesh::WaypointGizmo::OnMouseLeave( CMapView* pView, int nMX, int nMY, int nButtons, CPickObject* pSrc ) {
	hover = false;
	Sys_RedrawWindows();
}

bool CWaypointMesh::WaypointGizmo::OnPopupMenu( CMapView* pView, int nMX, int nMY, int nButtons, CPickObject* pSrc) {
	return mesh->OnPopupMenu(pView, nMX, nMY, nButtons, pSrc);
}

void CWaypointMesh::WaypointGizmo::OnDraw( CMapView* pView ) {
	float s;
	vec3 mins, maxs;

	if(pView->GetViewType() != VIEW_TYPE_3D) {
		s = size*pView->View.fInvScale;
	} else {
		s = size;
	}

	if(IsSelected()) {
		glColor4ub( 0xff, 0, 0, 0xff );
	} else if( hover ) {
		glColor4ubv( (GLubyte*)&hlcolor );
	} else if( !pView->GetDocument()->IsEditingWaypoints() && mesh->IsSelected() ) {
		glColor4ub( 0x7f, 0, 0, 0xff  );
	} else {
		glColor4ubv( (GLubyte*)&color );
	}

	mins = waypoint->pos - (s*0.5f);
	maxs = waypoint->pos + (s*0.5f);

	R_DrawBox( mins, maxs );
}

int CWaypointMesh::WaypointGizmo::Drag(CManipulator *m, void *p, void *p2) {
	WaypointGizmo* gizmo = dynamic_cast<WaypointGizmo*>(m);
	if(gizmo && gizmo->IsSelected()) {
		gizmo->waypoint->pos += *((vec3*)p);
		gizmo->mesh->OnWaypointDrag(*gizmo->waypoint);
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////

CWaypointMesh::ContextMenu::ContextMenu() {
}

CWaypointMesh::ContextMenu::~ContextMenu() {
}

void CWaypointMesh::ContextMenu::OnUpdateCmdUI(int id, CCmdUI *ui) {

	int editing = view->GetDocument()->IsEditingWaypoints();

	switch (id) {
	case kEditWaypoints:
		ui->Enable(TRUE);
		ui->SetCheck(editing);
		break;
	case kAddWaypoint:
		ui->Enable(editing);
		break;
	case kDeleteSelected:
		ui->Enable(editing);
		break;
	case kEditWaypoint:
		ui->Enable(editing);
		break;
	default:
		break;
	}
}

void CWaypointMesh::ContextMenu::OnMenuItem(int id) {

	int editing = view->GetDocument()->IsEditingWaypoints();

	switch (id) {
	case kEditWaypoints:
		editing = !editing;
		view->GetDocument()->SetEditingWaypoints(editing);
		view->GetDocument()->GamePlugin()->EnterWaypointMode(view->GetDocument(), editing != 0);
		view->GetDocument()->UpdateSelectionInterface();
		Sys_RedrawWindows();
		break;
	case kAddWaypoint:
		mesh->PopupMenu_OnAddWaypoint(view, waypoint);
		view->GetDocument()->UpdateSelectionInterface();
		Sys_RedrawWindows();
		break;
	case kConnectWaypoints:
		mesh->PopupMenu_OnConnectWaypoints(view, waypoint);
		view->GetDocument()->UpdateSelectionInterface();
		Sys_RedrawWindows();
		break;
	case kDisconnectWaypoints:
		mesh->PopupMenu_OnDisconnectWaypoints(view, waypoint);
		view->GetDocument()->UpdateSelectionInterface();
		Sys_RedrawWindows();
		break;
	case kEditWaypoint:
		mesh->PopupMenu_OnEditWaypoint(view, waypoint);
		break;
	case kDeleteSelected:
		mesh->PopupMenu_OnDeleteSelectedWaypoints(view, waypoint);
		view->GetDocument()->UpdateSelectionInterface();
		Sys_RedrawWindows();
		break;
	default:
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////

// Manipulates the waypoints in the 3D views.

CWaypointDrag3DGizmo::CWaypointDrag3DGizmo() : CVec3D_Manipulator() {
}

CWaypointDrag3DGizmo::~CWaypointDrag3DGizmo() {
}

void CWaypointDrag3DGizmo::OnMouseDown( CMapView* pView, int nMX, int nMY, int nButtons, CPickObject* pSrc ) {
	m_bMoved = false;
	CVec3D_Manipulator::OnMouseDown( pView, nMX, nMY, nButtons, pSrc );
}

void CWaypointDrag3DGizmo::OnMouseUp( CMapView* pView, int nMX, int nMY, int nButtons, CPickObject* pSrc ) {
	CVec3D_Manipulator::OnMouseUp( pView, nMX, nMY, nButtons, pSrc );
	pView->GetDocument()->ClearAllTrackPicks();
	pView->GetDocument()->BuildSelectionBounds();
	Sys_RedrawWindows();
}

bool CWaypointDrag3DGizmo::OnDrag( CMapView* pView, int nButtons, const vec3& move ) {
	vec3 t = move;

	if( pView->GetGridSnap() ) {
		t = Sys_SnapVec3( move, pView->GetGridSize() );
		if( equals( t, vec3::zero, 0.00001f ) )
			return false;
	}

	if( !m_bMoved ) {
		m_bMoved = true;
		pView->GetDocument()->GenericUndoRedoFromSelection()->SetTitle("Waypoint Drag");
	}

	CManipulator* m;
	CWaypointMesh::WaypointGizmo* gizmo;

	for( m = pView->GetDocument()->GetSelectedManipulatorList()->ResetPos(); m; m = pView->GetDocument()->GetSelectedManipulatorList()->GetNextItem() ) {
		gizmo = dynamic_cast<CWaypointMesh::WaypointGizmo*>(m);
		if( gizmo ) {
			gizmo->waypoint->pos += t;
			gizmo->mesh->OnWaypointDrag(*gizmo->waypoint);
		}
	}

	pView->GetDocument()->BuildSelectionBounds();
	Sys_RedrawWindows();
	return true;
}

void CWaypointDrag3DGizmo::OnDraw( CMapView* pView ) {
	if( pView->GetDocument()->GetSelectedManipulatorList()->IsEmpty() )
		return;
	CVec3D_Manipulator::OnDraw( pView );
}