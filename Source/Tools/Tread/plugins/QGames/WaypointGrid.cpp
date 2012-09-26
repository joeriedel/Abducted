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

CWaypoint::CWaypoint(const CWaypoint &m) : CMapObject(m) {
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

	m_world[0] = m.m_world[0];
	m_world[1] = m.m_world[1];
	m_local[0] = m.m_local[0];
	m_local[1] = m.m_local[1];
	m_pos = m.m_pos;
	m_boxPos = vec3::zero;
	m_drag = false;
	m_tails = m.m_tails;

	for (Connection::Map::const_iterator it = m.m_connections.begin(); it != m.m_connections.end(); ++it) {
		const Connection::Ref &c = it->second;
		Connection::Ref clone(new Connection());
		clone->ctrls[0] = c->ctrls[0];
		clone->ctrls[1] = c->ctrls[1];
		clone->head = 0;
		clone->tail = 0;
		clone->headId = c->headId;
		clone->tailId = c->tailId;
		clone->InitMesh();
		m_connections[it->first] = clone;
	}

	UpdateBoxMesh();
}

CWaypoint::~CWaypoint() {
	m_boxMesh.FreeMesh();
}

void CWaypoint::SetObjectWorldPos( const vec3& pos, CTreadDoc* pDoc ) {
	m_pos = pos;
	CalcBounds();
	UpdateBoxMesh();
	NotifyMoved(pDoc);
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
	NotifyMoved(doc);
}

bool CWaypoint::WriteToFile( CFile* pFile, CTreadDoc* pDoc, int nVersion ) {
	if (!CMapObject::WriteToFile(pFile, pDoc, nVersion))
		return false;

	return true;
}

bool CWaypoint::ReadFromFile( CFile* pFile, CTreadDoc* pDoc, int nVersion ) {
	if (!CMapObject::ReadFromFile(pFile, pDoc, nVersion))
		return false;

	return true;
}

CLinkedList<CObjProp>* CWaypoint::GetPropList( CTreadDoc* pDoc ) {
	return 0;
}

void CWaypoint::SetProp( CTreadDoc* pDoc, CObjProp* prop ) {
	CMapObject::SetProp(pDoc, prop);
}

void CWaypoint::OnAddToMap( CTreadDoc* pDoc ) {
	if (!IsAttached()) {
		NotifyAttach(pDoc);
		NotifyMoved(pDoc);
	}

	DeleteGizmos(pDoc);
	CMapObject::OnAddToMap(pDoc);
}

void CWaypoint::OnAddToSelection( CTreadDoc* pDoc ) {
	CreateGizmos(pDoc);
	CMapObject::OnAddToSelection(pDoc);
}

void CWaypoint::OnRemoveFromMap( CTreadDoc* pDoc ) {
	DeleteGizmos(pDoc);
	NotifyDetach(pDoc);
	CMapObject::OnRemoveFromMap(pDoc);
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

CMapObject* CWaypoint::Clone() {
	return new CWaypoint(*this);
}

int CWaypoint::GetNumRenderMeshes( CMapView* pView ) {
	m_meshIt = m_connections.begin();
	return 1 + (int)m_connections.size();
}

CRenderMesh* CWaypoint::GetRenderMesh( int num, CMapView* pView ) {
	if (num == 0)
		return &m_boxMesh;
	if (m_meshIt != m_connections.end()) {
		const Connection::Ref &c = m_meshIt->second;
		++m_meshIt;
		return &c->mesh;
	}
	return 0;
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

void CWaypoint::NotifyAttach(CTreadDoc *doc) {
	for (Connection::Map::const_iterator it = m_connections.begin(); it != m_connections.end(); ++it) {
		int id = it->first;
		it->second->head = this;
		CWaypoint *waypoint = static_cast<CWaypoint*>(doc->ObjectForUID(id));
		if (waypoint)
			NotifyAttach(doc, *waypoint);
	}
	for (IntSet::const_iterator it = m_tails.begin(); it != m_tails.end(); ++it) {
		CWaypoint *waypoint = static_cast<CWaypoint*>(doc->ObjectForUID(*it));
		if (waypoint)
			waypoint->NotifyAttach(doc, *this);
	}
}

void CWaypoint::NotifyDetach(CTreadDoc *doc) {
	for (Connection::Map::const_iterator it = m_connections.begin(); it != m_connections.end(); ++it) {
		int id = it->first;
		CWaypoint *waypoint = static_cast<CWaypoint*>(doc->ObjectForUID(id));
		if (waypoint)
			NotifyDetach(doc, *waypoint);
	}
	for (IntSet::const_iterator it = m_tails.begin(); it != m_tails.end(); ++it) {
		CWaypoint *waypoint = static_cast<CWaypoint*>(doc->ObjectForUID(*it));
		if (waypoint)
			waypoint->NotifyDetach(doc, *this);
	}
}

void CWaypoint::NotifyAttach(CTreadDoc *doc, CWaypoint &src) {
	Connection::Map::const_iterator it = m_connections.find(src.GetUID());
	if (it != m_connections.end()) {
		const Connection::Ref &c = it->second;
		c->tail = &src;
	}
}

void CWaypoint::NotifyDetach(CTreadDoc *doc, CWaypoint &src) {
	Connection::Map::const_iterator it = m_connections.find(src.GetUID());
	if (it != m_connections.end()) {
		const Connection::Ref &c = it->second;
		c->tail = 0;
	}
}

void CWaypoint::NotifyMoved(CTreadDoc *doc) {
	for (Connection::Map::const_iterator it = m_connections.begin(); it != m_connections.end(); ++it) {
		const Connection::Ref &c = it->second;
		if (c->tail)
			c->UpdateMesh();
	}
	for (IntSet::const_iterator it = m_tails.begin(); it != m_tails.end(); ++it) {
		CWaypoint *waypoint = static_cast<CWaypoint*>(doc->ObjectForUID(*it));
		if (waypoint)
			waypoint->NotifyMoved(doc, *this);
	}
}

void CWaypoint::NotifyMoved(CTreadDoc *doc, CWaypoint &src) {
	Connection::Map::const_iterator it = m_connections.find(src.GetUID());
	if (it != m_connections.end()) {
		const Connection::Ref &c = it->second;
		c->UpdateMesh();
	}
}

bool CWaypoint::OnPopupMenu( CMapView* pView, int nMX, int nMY, int nButtons, CPickObject* pSrc) {
	AbductedUserData *ud = static_cast<AbductedUserData*>(pView->GetDocument()->UserData());
	ud->MakeWaypointMenu(pView->GetDocument());
	ud->m_WaypointMenu.view = pView;
	ud->m_WaypointMenu.waypoint = this;
	
	Sys_DisplayObjectMenu(pView, nMX, nMY, &ud->m_WaypointMenu);
	return true;
}

void CWaypoint::CreateGizmos(CTreadDoc *doc) {
	for (Connection::Map::const_iterator it = m_connections.begin(); it != m_connections.end(); ++it) {
		const Connection::Ref &c = it->second;
		if (c->tail && c->tail->IsSelected())
			c->CreateGizmos(doc);
	}

	for (IntSet::const_iterator it = m_tails.begin(); it != m_tails.end(); ++it) {
		CWaypoint *waypoint = static_cast<CWaypoint*>(doc->ObjectForUID(*it));
		if (waypoint)
			waypoint->CreateGizmos(doc, *this);
	}
}

void CWaypoint::DeleteGizmos(CTreadDoc *doc) {
	for (Connection::Map::const_iterator it = m_connections.begin(); it != m_connections.end(); ++it) {
		const Connection::Ref &c = it->second;
		c->DeleteGizmos(doc);
	}

	for (IntSet::const_iterator it = m_tails.begin(); it != m_tails.end(); ++it) {
		CWaypoint *waypoint = static_cast<CWaypoint*>(doc->ObjectForUID(*it));
		if (waypoint)
			waypoint->DeleteGizmos(doc, *this);
	}
}

void CWaypoint::CreateGizmos(CTreadDoc *doc, CWaypoint &src) {
	if (!IsSelected())
		return;

	Connection::Map::const_iterator it = m_connections.find(src.GetUID());
	if (it != m_connections.end()) {
		const Connection::Ref &c = it->second;
		c->CreateGizmos(doc);
	}
}

void CWaypoint::DeleteGizmos(CTreadDoc *doc, CWaypoint &src) {
	Connection::Map::const_iterator it = m_connections.find(src.GetUID());
	if (it != m_connections.end()) {
		const Connection::Ref &c = it->second;
		c->DeleteGizmos(doc);
	}
}

void CWaypoint::PopupMenu_OnConnectWaypoints(CMapView *view) {
	CLinkedList<CMapObject> *selection = view->GetDocument()->GetSelectedObjectList();

	CMapObject *xNext;
	for (CMapObject *x = selection->ResetPos(); x; x = xNext) {
		selection->SetPosition(x);
		xNext = selection->GetNextItem();

		CWaypoint *src = dynamic_cast<CWaypoint*>(x);
		if (!src)
			continue;

		for (CMapObject *y = xNext; y; y = selection->GetNextItem()) {
			CWaypoint *dst = dynamic_cast<CWaypoint*>(y);
			if (!dst)
				continue;
			Connect(view->GetDocument(), *src, *dst);
		}
	}
	
	view->GetDocument()->Prop_UpdateSelection();
	Sys_RedrawWindows();
}

void CWaypoint::PopupMenu_OnDisconnectWaypoints(CMapView *view) {
	CLinkedList<CMapObject> *selection = view->GetDocument()->GetSelectedObjectList();

	CMapObject *xNext;
	for (CMapObject *x = selection->ResetPos(); x; x = xNext) {
		selection->SetPosition(x);
		xNext = selection->GetNextItem();

		CWaypoint *src = dynamic_cast<CWaypoint*>(x);
		if (!src)
			continue;

		for (CMapObject *y = xNext; y; y = selection->GetNextItem()) {
			CWaypoint *dst = dynamic_cast<CWaypoint*>(y);
			if (!dst)
				continue;
			Disconnect(view->GetDocument(), *src, *dst, false);
		}
	}
	
	view->GetDocument()->Prop_UpdateSelection();
	Sys_RedrawWindows();
}

void CWaypoint::Connect(CTreadDoc *doc, CWaypoint &src, CWaypoint &dst) {
	// is there a connection in src with dst in it?
	Connection::Map::iterator it = src.m_connections.find(dst.GetUID());
	if (it != src.m_connections.end())
		return;

	doc->GenericUndoRedoFromSelection()->SetTitle("Connect Waypoints");

	Connection::Ref c(new Connection());
	c->head = &src;
	c->headId = src.GetUID();
	c->tail = &dst;
	c->tailId = dst.GetUID();

	vec3 mid = src.m_pos + ((dst.m_pos - src.m_pos) / 2.f);
	c->ctrls[0] = mid;
	c->ctrls[1] = mid;

	c->InitMesh();
	c->UpdateMesh();
	c->CreateGizmos(doc);
	c->Select(true);

	src.m_connections[dst.GetUID()] = c;
	dst.m_tails.insert(src.GetUID());
}

void CWaypoint::Disconnect(CTreadDoc *doc, CWaypoint &src, CWaypoint &dst, bool flip) {
	Connection::Map::iterator it = src.m_connections.find(dst.GetUID());
	if (it == src.m_connections.end()) {
		if (!flip)
			Disconnect(doc, dst, src, true); // maybe connected this way
		return;
	}
	doc->GenericUndoRedoFromSelection()->SetTitle("Disconnect Waypoints");
	it->second->DeleteGizmos(doc);
	src.m_connections.erase(it);
	dst.m_tails.erase(src.GetUID());
}

///////////////////////////////////////////////////////////////////////////////

CWaypoint::ControlPointGizmo3D::ControlPointGizmo3D() {
	size = 64.f;
	color = 0xFF00FF00;
	hlcolor = 0xFF4DE6F2;
}

CWaypoint::ControlPointGizmo3D::~ControlPointGizmo3D() {
}

void CWaypoint::ControlPointGizmo3D::OnMouseDown( CMapView* pView, int nMX, int nMY, int nButtons, CPickObject* pSrc ) {
	moved = false;
	CVec3D_Manipulator::OnMouseDown( pView, nMX, nMY, nButtons, pSrc );
}

bool CWaypoint::ControlPointGizmo3D::OnDrag( CMapView* pView, int nButtons, const vec3& move ) {
	vec3 t;

	if( pView->GetGridSnap() )
	{
		t = Sys_SnapVec3( move, pView->GetGridSize() );
		if( equals( t, vec3::zero, 0.0001f ) )
			return false;
	}
	else
	{
		t = move;
	}

	if( !moved )
	{
		pView->GetDocument()->GenericUndoRedoFromSelection()->SetTitle( "Control Point Drag" );
		moved = true;
	}

	*(src->pos) += t;
	src->connection->UpdateMesh();

	pView->GetDocument()->BuildSelectionBounds();
	Sys_RedrawWindows();

	return true;
}

///////////////////////////////////////////////////////////////////////////////

CWaypoint::ControlPointGizmo::ControlPointGizmo() {
	size = 10.f;
	color = 0xFF00FF00;
	hlcolor = 0xFF4DE6F2;
	hover = false;
	SetViewFlags(VIEW_FLAG_MAP);
}

CWaypoint::ControlPointGizmo::~ControlPointGizmo() {
}

void CWaypoint::ControlPointGizmo::OnMouseDown( CMapView* pView, int nMX, int nMY, int nButtons, CPickObject* pSrc ) {

	if (pView->GetViewType() == VIEW_TYPE_3D)
		return;

	pView->GetDocument()->ClearSelectedManipulators();
	pView->GetDocument()->AddManipulatorToSelection(this);

	drag = true;
	snap = false;
	moved = false;
	Sys_SetMouseCapture( pView );
}

void CWaypoint::ControlPointGizmo::OnMouseUp( CMapView* pView, int nMX, int nMY, int nButtons, CPickObject* pSrc ) {
	hover = false;
	drag = false;
	Sys_SetMouseCapture( 0 );
	pView->GetDocument()->ClearAllTrackPicks();
	pView->GetDocument()->BuildSelectionBounds();
	Sys_RedrawWindows();
}

void CWaypoint::ControlPointGizmo::OnMouseMove( CMapView* pView, int nMX, int nMY, int nButtons, CPickObject* pSrc ) {

	if (!drag)
		return;

	float mx, my;
	float dx, dy;
	int x_axis, y_axis;

	pView->WinXYToMapXY( nMX, nMY, &mx, &my );

	//
	// snap the sucker.
	//
	if( pView->GetGridSnap() )
	{
		mx = Sys_Snapf( mx, pView->GetGridSize() );
		my = Sys_Snapf( my, pView->GetGridSize() );

		if( snap )
		{
			if( mx == lastx && my == lasty )
				return;// no movement.
		}

		snap = true;
		lastx = mx;
		lasty = my;
	}

	x_axis = QUICK_AXIS( pView->View.or2d.lft );
	y_axis = QUICK_AXIS( pView->View.or2d.up );

	dx = mx - (*pos)[x_axis];
	dy = my - (*pos)[y_axis];

	if( (dx != 0 || dy != 0) && !moved)
	{
		moved = true;
		pView->GetDocument()->GenericUndoRedoFromSelection()->SetTitle("Control Point Drag");
	}

	(*pos)[x_axis] = mx;
	(*pos)[y_axis] = my;
	connection->UpdateMesh();

	pView->GetDocument()->BuildSelectionBounds();
	Sys_RedrawWindows();
}

void CWaypoint::ControlPointGizmo::OnMouseEnter( CMapView* pView, int nMX, int nMY, int nButtons, CPickObject* pSrc ) {
	hover = true;

	if( IsSelected() ) {
		pView->GetDocument()->AddManipulatorToSelection( this ); // this little trick pushes it forward to be visible in all views.
	} else {
		pView->GetDocument()->AddManipulatorToMap( this ); // this little trick pushes it forward to be visible in all views.
	}

	Sys_RedrawWindows();
}

void CWaypoint::ControlPointGizmo::OnMouseLeave( CMapView* pView, int nMX, int nMY, int nButtons, CPickObject* pSrc ) {
	hover = false;
	Sys_RedrawWindows();
}

void CWaypoint::ControlPointGizmo::OnDraw( CMapView* pView ) {

	float s;
	vec3 mins, maxs;

	if( pView->GetViewType() != VIEW_TYPE_3D )
	{
		s = size*pView->View.fInvScale;
	}
	else
	{
		s = size;
	}

	if( IsSelected() )
	{
		glColor4ub( 0xFF, 0, 0, 0xFF );
	}
	else
	if( hover )
	{
		glColor4ubv( (GLubyte*)&hlcolor );
	}
	else
	{
		glColor4ubv( (GLubyte*)&color );
	}

	mins = (*pos) - (s*0.5f);
	maxs = (*pos) + (s*0.5f);

	R_DrawBox( mins, maxs );

	// draw a line

	glLoadName( 0 );

	glColor4f( 0, 0, 1, 1 );
	glBegin( GL_LINES );
	glVertex3fv( &waypoint->m_pos.x );
	glVertex3fv( &pos->x );
	glEnd();
		
	glLoadName( (GLint)this );

}

///////////////////////////////////////////////////////////////////////////////

CWaypoint::Connection::Connection() {
	for (int i = 0; i < 2; ++i) {
		gizmos[i] = 0;
		gizmos3D[i][0] = 0;
		gizmos3D[i][1] = 0;
		gizmos3D[i][2] = 0;
	}
}

CWaypoint::Connection::~Connection() {
	delete [] mesh.xyz;
	delete [] mesh.tris;
}

void CWaypoint::Connection::InitMesh() {
	mesh.cmds = GL_LINES;
	mesh.num_pts = kNumPoints;
	mesh.num_tris = kNumPoints;
	mesh.xyz = new vec3[kNumPoints];
	mesh.tris = new unsigned int[kNumPoints];
	for (int i = 0; i < kNumPoints; ++i) {
		mesh.tris[i] = i;
	}
	mesh.allow_selected = false;
	mesh.allow_wireframe = false;
	mesh.solid2d = false;

	Select(false);
}

void CWaypoint::Connection::UpdateMesh() {

	float nummids = 1.0f/(float)(kNumPoints-1);

	float u = 0;

	vec3 ctrls[4];
	ctrls[0] = head->m_pos;
	ctrls[1] = this->ctrls[0];
	ctrls[2] = this->ctrls[1];
	ctrls[3] = tail->m_pos;
	
	vec3 a = -ctrls[0] + 3*ctrls[1] - 3*ctrls[2] + ctrls[3];
	vec3 b = 3*ctrls[0] - 6*ctrls[1] + 3*ctrls[2];
	vec3 c = -3*ctrls[0] + 3*ctrls[1];
	vec3 d = ctrls[0];

	for(int i = 0; i < kNumPoints; ++i, u+=nummids)
	{
		float u_sq = u*u;
		float u_cb = u*u_sq;

		mesh.xyz[i] = a*u_cb + b*u_sq + c*u + d;
	}
}

void CWaypoint::Connection::Select(bool select) {
	if (select) {
		mesh.color2d = mesh.color3d = 0xFF100030;
		mesh.fcolor3d[0] = 48.f/255.f;
		mesh.fcolor3d[1] = 0.0f;
		mesh.fcolor3d[2] = 16.f/255.f;
		mesh.fcolor3d[3] = 1.0f;
		mesh.line_size = 2.f;
	} else {
		mesh.color2d = mesh.color3d = 0xFF10FF20;
		mesh.fcolor3d[0] = 32.f/255.f;
		mesh.fcolor3d[1] = 1.0f;
		mesh.fcolor3d[2] = 16.f/255.f;
		mesh.fcolor3d[3] = 1.0f;
		mesh.line_size = 1.f;
	}
}

void CWaypoint::Connection::Bind(CTreadDoc *doc) {
	head = static_cast<CWaypoint*>(doc->ObjectForUID(headId));
	tail = static_cast<CWaypoint*>(doc->ObjectForUID(tailId));
}

void CWaypoint::Connection::CreateGizmos(CTreadDoc *doc) {

	Select(true);

	const unsigned int hlcolor = 0xFF4DE6F2;

	for (int i = 0; i < 2; ++i) {
		ControlPointGizmo *gizmo = new ControlPointGizmo();
		gizmo->pos = &ctrls[i];
		gizmo->waypoint = (i == 0) ? head : tail;
		gizmo->connection = this;
		doc->AddManipulatorToMap(gizmo);
		gizmos[i] = gizmo;

		ControlPointGizmo3D *gizmo3D = new ControlPointGizmo3D();
		gizmo3D->color = 0xFF0000FF;
		gizmo3D->hlcolor = hlcolor;
		gizmo3D->vec = sysAxisX;
		gizmo3D->x_in = gizmo->pos;
		gizmo3D->y_in = gizmo->pos;
		gizmo3D->z_in = gizmo->pos;
		gizmo3D->string = "drag x";
		gizmo3D->src = gizmo;
		doc->AddManipulatorToMap(gizmo3D);
		gizmos3D[i][0] = gizmo3D;

		gizmo3D = new ControlPointGizmo3D();
		gizmo3D->color = 0xFF00FF00;
		gizmo3D->hlcolor = hlcolor;
		gizmo3D->vec = sysAxisY;
		gizmo3D->x_in = gizmo->pos;
		gizmo3D->y_in = gizmo->pos;
		gizmo3D->z_in = gizmo->pos;
		gizmo3D->string = "drag y";
		gizmo3D->src = gizmo;
		doc->AddManipulatorToMap(gizmo3D);
		gizmos3D[i][1] = gizmo3D;

		gizmo3D = new ControlPointGizmo3D();
		gizmo3D->color = 0xFFFF0000;
		gizmo3D->hlcolor = hlcolor;
		gizmo3D->vec = sysAxisZ;
		gizmo3D->x_in = gizmo->pos;
		gizmo3D->y_in = gizmo->pos;
		gizmo3D->z_in = gizmo->pos;
		gizmo3D->string = "drag z";
		gizmo3D->src = gizmo;
		doc->AddManipulatorToMap(gizmo3D);
		gizmos3D[i][2] = gizmo3D;
	}
}

void CWaypoint::Connection::DeleteGizmos(CTreadDoc *doc) {

	Select(false);

	for (int i = 0; i < 2; ++i) {
		if (!gizmos[i])
			return;
		doc->DetachManipulator(gizmos[i]);
		delete gizmos[i];
		gizmos[i] = 0;
		doc->DetachManipulator(gizmos3D[i][0]);
		doc->DetachManipulator(gizmos3D[i][1]);
		doc->DetachManipulator(gizmos3D[i][2]);
		delete gizmos3D[i][0];
		delete gizmos3D[i][1];
		delete gizmos3D[i][2];
	}
}

///////////////////////////////////////////////////////////////////////////////


CWaypoint::ContextMenu::ContextMenu() {
}

CWaypoint::ContextMenu::~ContextMenu() {
}

void CWaypoint::ContextMenu::OnUpdateCmdUI(int id, CCmdUI *ui) {

	int numObjs = view->GetDocument()->GetSelectedObjectCount();
	int numNodes = view->GetDocument()->GetSelectedObjectCount(MAPOBJ_CLASS_WAYPOINT, MAPOBJ_SUBCLASS_NONE);

	switch (id) {
	case kConnect:
	case kDisconnect:
		ui->Enable((numNodes > 1) && (numObjs == numNodes));
	default:
		break;
	}
}

void CWaypoint::ContextMenu::OnMenuItem(int id) {

	switch (id) {
	case kConnect:
		waypoint->PopupMenu_OnConnectWaypoints(view);
		break;
	case kDisconnect:
		waypoint->PopupMenu_OnDisconnectWaypoints(view);
		break;
	default:
		break;
	}
}
