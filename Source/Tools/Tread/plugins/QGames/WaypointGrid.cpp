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

enum {
	kWaypointSaveVersion = 3
};

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
	m_inAddSelection = false;
	m_beingSelected = false;
	InitProps();
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
		clone->headId = c->headId;
		clone->tailId = c->tailId;
		clone->InitMesh();
		m_connections[it->first] = clone;
	}

	UpdateBoxMesh();
	InitProps();
	m_inAddSelection = false;
	m_beingSelected = false;

	m_props[0].SetValue(&m.m_props[0]);
	m_props[1].SetValue(&m.m_props[1]);
}

CWaypoint::~CWaypoint() {
	m_boxMesh.FreeMesh();
	m_propList.ReleaseList();
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

	MAP_WriteInt(pFile, kWaypointSaveVersion);
	MAP_WriteVec3(pFile, &m_world[0]);
	MAP_WriteVec3(pFile, &m_world[1]);
	MAP_WriteVec3(pFile, &m_local[0]);
	MAP_WriteVec3(pFile, &m_local[1]);
	MAP_WriteVec3(pFile, &m_pos);
		
	MAP_WriteInt(pFile, (int)m_connections.size());
	for (Connection::Map::const_iterator it = m_connections.begin(); it != m_connections.end(); ++it) {
		const Connection::Ref &c = it->second;
		MAP_WriteInt(pFile, c->headId);
		MAP_WriteInt(pFile, c->tailId);
		MAP_WriteVec3(pFile, &c->ctrls[0]);
		MAP_WriteVec3(pFile, &c->ctrls[1]);
		MAP_WriteString(pFile, c->props[Connection::kProp_FwdStart].GetString());
		MAP_WriteString(pFile, c->props[Connection::kProp_FwdEnd].GetString());
		MAP_WriteString(pFile, c->props[Connection::kProp_BackStart].GetString());
		MAP_WriteString(pFile, c->props[Connection::kProp_BackEnd].GetString());
		MAP_WriteInt(pFile, c->props[Connection::kProp_Flags].GetInt());
	}

	MAP_WriteInt(pFile, (int)m_tails.size());
	for (IntSet::const_iterator it = m_tails.begin(); it != m_tails.end(); ++it) {
		MAP_WriteInt(pFile, *it);
	}

	MAP_WriteString(pFile, m_props[kProp_Name].GetString());
	MAP_WriteString(pFile, m_props[kProp_Id].GetString());
	MAP_WriteString(pFile, m_props[kProp_Floor].GetString());
	MAP_WriteInt(pFile, m_props[kProp_Flags].GetInt());

	return true;
}

bool CWaypoint::ReadFromFile( CFile* pFile, CTreadDoc* pDoc, int nVersion ) {
	if (!CMapObject::ReadFromFile(pFile, pDoc, nVersion))
		return false;

	int version = MAP_ReadInt(pFile);
	MAP_ReadVec3(pFile, &m_world[0]);
	MAP_ReadVec3(pFile, &m_world[1]);
	MAP_ReadVec3(pFile, &m_local[0]);
	MAP_ReadVec3(pFile, &m_local[1]);
	MAP_ReadVec3(pFile, &m_pos);

	int num = MAP_ReadInt(pFile);
	for (int i = 0; i < num; ++i) {
		Connection::Ref c(new Connection());
		c->headId = MAP_ReadInt(pFile);
		c->head = this;
		c->tailId = MAP_ReadInt(pFile);
		c->InitMesh();
		MAP_ReadVec3(pFile, &c->ctrls[0]);
		MAP_ReadVec3(pFile, &c->ctrls[1]);

		if (version >= 3) {
			c->props[Connection::kProp_FwdStart].SetString(MAP_ReadString(pFile));
			c->props[Connection::kProp_FwdEnd].SetString(MAP_ReadString(pFile));
			c->props[Connection::kProp_BackStart].SetString(MAP_ReadString(pFile));
			c->props[Connection::kProp_BackEnd].SetString(MAP_ReadString(pFile));
			c->props[Connection::kProp_Flags].SetInt(MAP_ReadInt(pFile));
		}

		m_connections[c->tailId] = c;
	}

	num = MAP_ReadInt(pFile);
	for (int i = 0; i < num; ++i) {
		m_tails.insert(MAP_ReadInt(pFile));
	}

	if (version >= 3) {
		m_props[kProp_Name].SetString(MAP_ReadString(pFile));
		m_props[kProp_Id].SetString(MAP_ReadString(pFile));
		m_props[kProp_Floor].SetString(MAP_ReadString(pFile));
		m_props[kProp_Flags].SetInt(MAP_ReadInt(pFile));
	} else if (version == 2) {
		MAP_ReadString(pFile);
		MAP_ReadString(pFile);
	}

	UpdateBoxMesh();

	return true;
}

CLinkedList<CObjProp>* CWaypoint::GetPropList( CTreadDoc* doc ) {
	return &m_propList;
}

void CWaypoint::AddSelectedConnectionProps(CTreadDoc* doc) {

	if (m_inAddSelection)
		return;

	m_inAddSelection = true;

	m_propList.ReleaseList();
	m_propList.AddItem(&m_props[kProp_Name]);
	m_propList.AddItem(&m_props[kProp_Id]);
	m_propList.AddItem(&m_props[kProp_Floor]);
	m_propList.AddItem(&m_props[kProp_Flags]);

	for (Connection::Map::const_iterator it = m_connections.begin(); it != m_connections.end(); ++it) {
		const Connection::Ref &c = it->second;
		if (c->tail && (c->tail->IsSelected() || c->tail->m_beingSelected)) {
			AddConnectionProps(*c);
			c->tail->AddSelectedConnectionProps(doc);
		}
	}

	for (IntSet::const_iterator it = m_tails.begin(); it != m_tails.end(); ++it) {
		CWaypoint *waypoint = static_cast<CWaypoint*>(doc->ObjectForUID(*it));
		if (waypoint && (waypoint->IsSelected() || waypoint->m_beingSelected)) {
			for (Connection::Map::const_iterator it = waypoint->m_connections.begin(); it != waypoint->m_connections.end(); ++it) {
				const Connection::Ref &c = it->second;
				if (c->tail == this) {
					AddConnectionProps(*c);
					waypoint->AddSelectedConnectionProps(doc);
				}
			}
		}
	}

	m_inAddSelection = false;
}

void CWaypoint::SetProp( CTreadDoc* pDoc, CObjProp* prop ) {
	const char *name = prop->GetName();

	for (int i = kProp_First; i < kProp_Num; ++i) {
		if (!strcmp(name, m_props[i].GetName()))
			m_props[i].SetValue(prop);
	}
	
	SetSelectedConnectionProp(prop);
}

void CWaypoint::SetSelectedConnectionProp(CObjProp *prop) {
	for (Connection::Map::const_iterator it = m_connections.begin(); it != m_connections.end(); ++it) {
		const Connection::Ref &c = it->second;
		if (c->tail && c->tail->IsSelected()) {
			SetConnectionProp(*c, prop);
		}
	}
}

void CWaypoint::SetConnectionProp(Connection &c, CObjProp *p) {
	const char *name = p->GetName();

	for (int i = Connection::kProp_First; i < Connection::kProp_Num; ++i) {
		if (!strcmp(name, c.props[i].GetName()))
			c.props[i].SetValue(p);
	}
}

void CWaypoint::InitProps() {

	m_props[kProp_Name].SetName("name");
	m_props[kProp_Name].SetDisplayName("Targetname");
	m_props[kProp_Name].SetType(CObjProp::string);
	m_props[kProp_Name].SetSubType(FALSE);

	m_props[kProp_Id].SetName("userId");
	m_props[kProp_Id].SetDisplayName("UserID");
	m_props[kProp_Id].SetType(CObjProp::string);
	m_props[kProp_Id].SetSubType(FALSE);
	
	m_props[kProp_Floor].SetName("floor_name");
	m_props[kProp_Floor].SetDisplayName("Floor (Optional)");
	m_props[kProp_Floor].SetType(CObjProp::string);
	m_props[kProp_Floor].SetSubType(FALSE);
	
	m_props[kProp_Flags].SetName("flags");
	m_props[kProp_Flags].SetDisplayName("Flags");
	m_props[kProp_Flags].SetType(CObjProp::integer);
	m_props[kProp_Flags].SetSubType(FALSE);
	m_props[kProp_Flags].SetInt(0);
}

void CWaypoint::AddConnectionProps(Connection &c) {
	for (int i = Connection::kProp_First; i < Connection::kProp_Num; ++i)
		m_propList.AddItem(&c.props[i]);
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
	m_beingSelected = true;
	CreateGizmos(pDoc);
	CMapObject::OnAddToSelection(pDoc);
	AddSelectedConnectionProps(pDoc);
	m_beingSelected = false;
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
	return 1 + (int)(m_connections.size() * 2);
}

CRenderMesh* CWaypoint::GetRenderMesh( int num, CMapView* pView ) {
	if (num == 0)
		return &m_boxMesh;
	if (m_meshIt != m_connections.end()) {
		const Connection::Ref &c = m_meshIt->second;
		if ((num-1) & 1) {
			++m_meshIt;
			return &c->arrow;
		}
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
		if (c->tail && c->tail->IsSelected()) {
			c->CreateGizmos(doc);
		}
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

	bool undo = false;

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

			// already connected?
			if (src->m_connections.find(dst->GetUID()) != src->m_connections.end())
				continue;
			if (dst->m_connections.find(src->GetUID()) != dst->m_connections.end())
				continue;

			if (!undo) {
				undo = true;
				view->GetDocument()->GenericUndoRedoFromSelection()->SetTitle("Connect Waypoints");
				selection->SetPosition(y);
			}

			Connect(view->GetDocument(), *src, *dst);
		}

		src->AddSelectedConnectionProps(view->GetDocument());
	}
	
	view->GetDocument()->Prop_UpdateSelection();
	Sys_RedrawWindows();
}

void CWaypoint::PopupMenu_OnDisconnectWaypoints(CMapView *view) {
	CLinkedList<CMapObject> *selection = view->GetDocument()->GetSelectedObjectList();

	bool undo = false;

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

			if (!undo) {
				undo = true;
				view->GetDocument()->GenericUndoRedoFromSelection()->SetTitle("Disconnect Waypoints");
				selection->SetPosition(y);
			}

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
	it->second->DeleteGizmos(doc);
	src.m_connections.erase(it);
	dst.m_tails.erase(src.GetUID());
}

void CWaypoint::WriteToMapFile(std::fstream &fs, CTreadDoc *doc) {
	fs << "{\n\"classname\" \"waypoint\"\n";
	fs << "\"uid\" \"" << GetUID() << "\"\n";
	fs << "\"origin\" \"" << m_pos.x << " " << m_pos.y << " " << m_pos.z << "\"\n";
	fs << "\"name\" \"" << m_props[kProp_Name].GetString() << "\"\n";
	fs << "\"floor\" \"" << m_props[kProp_Floor].GetString() << "\"\n";
	fs << "\"flags\" \"" << m_props[kProp_Flags].GetInt() << "\"\n";
	
	int i = 0;
	for (Connection::Map::const_iterator it = m_connections.begin(); it != m_connections.end(); ++it, ++i) {
		const Connection::Ref &c = it->second;
		fs << "\"connection " << i << "\" \"" << it->second->tailId << 
			" " << c->props[Connection::kProp_Flags].GetInt() <<
			" ( " << c->ctrls[0].x << " " << c->ctrls[0].y << " " << c->ctrls[0].z << 
			" ) ( " << c->ctrls[1].x << " " << c->ctrls[1].y << " " << c->ctrls[1].z << " )\"\n";
		fs << "\"connection_fwd_start " << i << "\"" << c->props[Connection::kProp_FwdStart].GetString() << "\"\n";
		fs << "\"connection_fwd_end " << i << "\"" << c->props[Connection::kProp_FwdEnd].GetString() << "\"\n";
		fs << "\"connection_back_start " << i << "\"" << c->props[Connection::kProp_BackStart].GetString() << "\"\n";
		fs << "\"connection_back_end " << i << "\"" << c->props[Connection::kProp_BackEnd].GetString() << "\"\n";
	}

	fs << "}\n";
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
	head = 0;
	tail = 0;
	headId = -1;
	tailId = -1;

	for (int i = 0; i < 2; ++i) {
		gizmos[i] = 0;
		gizmos3D[i][0] = 0;
		gizmos3D[i][1] = 0;
		gizmos3D[i][2] = 0;
	}

	InitProps();
}

CWaypoint::Connection::~Connection() {
	delete [] mesh.xyz;
	delete [] mesh.tris;
	delete [] arrow.xyz;
	delete [] arrow.tris;
	props[kProp_Flags].GetChoices()->ReleaseList();
}

void CWaypoint::Connection::InitProps() {
	props[kProp_FwdStart].SetName("fwdstart");
	props[kProp_FwdStart].SetDisplayName("*A -> B");
	props[kProp_FwdStart].SetType(CObjProp::script);
	props[kProp_FwdStart].SetSubType(FALSE);

	props[kProp_FwdEnd].SetName("fwdend");
	props[kProp_FwdEnd].SetDisplayName("A -> *B");
	props[kProp_FwdEnd].SetType(CObjProp::script);
	props[kProp_FwdEnd].SetSubType(FALSE);

	props[kProp_BackStart].SetName("backstart");
	props[kProp_BackStart].SetDisplayName("*B -> A");
	props[kProp_BackStart].SetType(CObjProp::script);
	props[kProp_BackStart].SetSubType(FALSE);

	props[kProp_BackEnd].SetName("backend");
	props[kProp_BackEnd].SetDisplayName("B -> *A");
	props[kProp_BackEnd].SetType(CObjProp::script);
	props[kProp_BackEnd].SetSubType(FALSE);

	props[kProp_Flags].SetName("c_flags");
	props[kProp_Flags].SetDisplayName("Connection Flags");
	props[kProp_Flags].SetType(CObjProp::integer);
	props[kProp_Flags].SetSubType(FALSE);
	

	flags[kFlag_AtoB].SetName("AtoB");
	flags[kFlag_AtoB].SetDisplayName("Enable A -> B");
	flags[kFlag_AtoB].SetType(CObjProp::integer);
	flags[kFlag_AtoB].SetInt(1 << kFlag_AtoB);

	flags[kFlag_BtoA].SetName("BtoA");
	flags[kFlag_BtoA].SetDisplayName("Enable B -> A");
	flags[kFlag_BtoA].SetType(CObjProp::integer);
	flags[kFlag_BtoA].SetInt(1 << kFlag_BtoA);

	flags[kFlag_BtoAUseAtoBScript].SetName("BtoAUseAtoBScript");
	flags[kFlag_BtoAUseAtoBScript].SetDisplayName("(B -> A) use (A -> B) Script");
	flags[kFlag_BtoAUseAtoBScript].SetType(CObjProp::integer);
	flags[kFlag_BtoAUseAtoBScript].SetInt(1 << kFlag_BtoAUseAtoBScript);

	flags[kFlag_AutoFace].SetName("autoFace");
	flags[kFlag_AutoFace].SetDisplayName("Auto Face");
	flags[kFlag_AutoFace].SetType(CObjProp::integer);
	flags[kFlag_AutoFace].SetInt(1 << kFlag_AutoFace);

	props[kProp_Flags].AddChoice(&flags[kFlag_AtoB]);
	props[kProp_Flags].AddChoice(&flags[kFlag_BtoA]);
	props[kProp_Flags].AddChoice(&flags[kFlag_BtoAUseAtoBScript]);
	props[kProp_Flags].AddChoice(&flags[kFlag_AutoFace]);

	props[kProp_Flags].SetInt(
		(1 << kFlag_AtoB) | (1 << kFlag_BtoA) | (1 << kFlag_AutoFace)
	);
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

	InitArrow();
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

	UpdateArrow();
}

void CWaypoint::Connection::InitArrow() {
	arrow.cmds = GL_TRIANGLES;
	arrow.num_pts = 5;
	arrow.num_tris = 6;
	arrow.xyz = new vec3[arrow.num_pts];
	arrow.tris = new unsigned int[arrow.num_tris * 3];

	//           (0)
	//            +
	//           / \
	//          /   \
	//  (1)(4) +-----+ (2)(3)

	arrow.tris[0] = 0;
	arrow.tris[1] = 2;
	arrow.tris[2] = 1;

	arrow.tris[3] = 0;
	arrow.tris[4] = 3;
	arrow.tris[5] = 2;

	arrow.tris[6] = 0;
	arrow.tris[7] = 1;
	arrow.tris[8] = 4;

	arrow.tris[9] = 0;
	arrow.tris[10] = 4;
	arrow.tris[11] = 3;

	arrow.tris[12] = 1;
	arrow.tris[13] = 2;
	arrow.tris[14] = 4;

	arrow.tris[15] = 4;
	arrow.tris[16] = 2;
	arrow.tris[17] = 3;

	arrow.allow_selected = false;
	arrow.allow_wireframe = false;
	arrow.solid2d = false;
}

void CWaypoint::Connection::UpdateArrow() {

	const vec3 &head = mesh.xyz[kNumPoints / 2];
	const vec3 &tail = mesh.xyz[kNumPoints / 2 - 1];

	vec3 fwd = head - tail;
	fwd.normalize();

	vec3 left, up;

	//           (0)
	//            +
	//           / \
	//          /   \
	//  (1)(4) +-----+ (2)(3)

	if (dot(fwd, sysAxisZ) > 0.999) {
		left = cross(fwd, sysAxisX);
		up = cross(left, fwd);
	} else {
		left = cross(fwd, sysAxisZ);
		up = cross(left, fwd);
	}

	const float kSize = 48.f;

	fwd *= kSize;
	left *= kSize*0.5f;
	up *= kSize*0.5f;

	arrow.xyz[0] = head;
	arrow.xyz[1] = head - fwd + left + up;
	arrow.xyz[2] = head - fwd - left + up;
	arrow.xyz[3] = head - fwd - left - up;
	arrow.xyz[4] = head - fwd + left - up;

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

	arrow.color2d = arrow.color3d = mesh.color2d;
	arrow.fcolor3d[0] = mesh.fcolor3d[0];
	arrow.fcolor3d[1] = mesh.fcolor3d[1];
	arrow.fcolor3d[2] = mesh.fcolor3d[2];
	arrow.fcolor3d[3] = mesh.fcolor3d[3];
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
