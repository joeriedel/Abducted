/*! \file WaypointGrid.h
	\copyright Copyright (c) 2010 Sunside Inc., All Rights Reserved.
	\copyright See Abducted/LICENSE for licensing terms.
	\author Joe Riedel
	\ingroup packages
*/

#pragma once

#include "System.h"
#include "vec3d_manipulator.h"
#include <list>
#include <map>

///////////////////////////////////////////////////////////////////////////////
class CWaypointDrag3DGizmo;
class AbductedUserData;
class CQuakeGame;

class CWaypoint : public CMapObject {
public:

	enum {
		ClassBits = MAPOBJ_CLASS_WAYPOINT
	};

	static CWaypoint *Create(CTreadDoc *doc);
	
	CWaypoint();
	CWaypoint(const CWaypoint &m);
	virtual ~CWaypoint();

	virtual void SetObjectWorldPos( const vec3& pos, CTreadDoc* pDoc );
	virtual void GetWorldMinsMaxs(vec3 *pMins, vec3 *pMaxs);
	virtual void GetObjectMinsMaxs(vec3 *pMins, vec3 *pMaxs);

	virtual vec3 GetObjectWorldPos();

	void OnMouseDown( CMapView* pView, int nMX, int nMY, int nButtons, CPickObject* pSrc );
	void OnMouseUp( CMapView* pView, int nMX, int nMY, int nButtons, CPickObject* pSrc );
	void OnMouseMove( CMapView* pView, int nMX, int nMY, int nButtons, CPickObject* pSrc );

	virtual void Nudge(const vec3 &amt, CTreadDoc *doc);

	virtual bool WriteToFile( CFile* pFile, CTreadDoc* pDoc, int nVersion );
	virtual bool ReadFromFile( CFile* pFile, CTreadDoc* pDoc, int nVersion );

	virtual CLinkedList<CObjProp>* GetPropList( CTreadDoc* pDoc );

	virtual void SetProp( CTreadDoc* pDoc, CObjProp* prop );
	virtual void OnAddToMap( CTreadDoc* pDoc );
	virtual void OnAddToSelection( CTreadDoc* pDoc );
	virtual void OnRemoveFromMap( CTreadDoc* pDoc );

	virtual int GetClass() {
		return ClassBits;
	}

	virtual CMapObject* Clone();

	virtual int GetNumRenderMeshes( CMapView* pView );
	virtual CRenderMesh* GetRenderMesh( int num, CMapView* pView );
	virtual const char* GetRootName();

	virtual bool OnPopupMenu( CMapView* pView, int nMX, int nMY, int nButtons, CPickObject* pSrc);

private:
	
	///////////////////////////////////////////////////////////////////////////////

	class ContextMenu : public CObjectMenu {
	public:

		enum {
			kConnectWaypoints,
			kDisconnectWaypoints
		};

		ContextMenu();
		virtual ~ContextMenu();

		CWaypoint *waypoint;
		CMapView *view;

		virtual void OnUpdateCmdUI(int id, CCmdUI *ui);
		virtual void OnMenuItem(int id);
	};

	friend class WaypointGizmo;
	friend class CWaypointDrag3DGizmo;
	friend class AbductedUserData;
	friend class CQuakeGame;

	void CalcBounds();
	void UpdateBoxMesh();

	void PopupMenu_OnConnectWaypoints(CMapView *view);
	void PopupMenu_OnDisconnectWaypoints(CMapView *view);
	
	CRenderMesh m_boxMesh;
	vec3 m_world[2];
	vec3 m_local[2];
	vec3 m_pos;
	vec3 m_boxPos;
	bool m_drag;
};

///////////////////////////////////////////////////////////////////////////////

class CMakeWaypoint : public CObjectCreator
{
public:
	virtual void Release() { delete this; }
	virtual const char *Name() { return "Waypoint Node"; }
	virtual CMapObject *CreateObject(CTreadDoc *doc) { return CWaypoint::Create(doc); }
};
