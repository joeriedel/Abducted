/*! \file WaypointMesh.h
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

class CWaypointMesh : public CMapObject {
public:

	enum {
		ClassBits = MAPOBJ_CLASS_WAYPOINTMESH
	};

	static CWaypointMesh *Create(CTreadDoc *doc);
	static int EnterEditMode(CMapObject* p, void* parm, void* parm2);
	static int ExitEditMode(CMapObject* p, void* parm, void* parm2);

	CWaypointMesh();
	CWaypointMesh(const CWaypointMesh &m);
	virtual ~CWaypointMesh();

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
	
	class Waypoint;
	typedef boost::shared_ptr<Waypoint> WaypointRef;
	
	///////////////////////////////////////////////////////////////////////////////

	class WaypointPair {
	public:
		Waypoint *waypoints[2];

		int Compare(const WaypointPair &p) const;

		bool operator == (const WaypointPair &p) const {
			return Compare(p) == 0;
		}

		bool operator != (const WaypointPair &p) const {
			return Compare(p) != 0;
		}

		bool operator <= (const WaypointPair &p) const {
			return Compare(p) <= 0;
		}

		bool operator >= (const WaypointPair &p) const {
			return Compare(p) >= 0;
		}

		bool operator < (const WaypointPair &p) const {
			return Compare(p) < 0;
		}

		bool operator > (const WaypointPair &p) const {
			return Compare(p) > 0;
		}
		
	};

	///////////////////////////////////////////////////////////////////////////////

	class Connection {
	public:
		typedef boost::shared_ptr<Connection> Ref;
		typedef std::list<Ref> List;
		typedef std::map<WaypointPair, Ref> Map;
		
		~Connection();

		WaypointPair pair;
		CRenderMesh::Ref mesh;
		CString commands[2]; // a->b, b->a

		void UpdateRenderMesh();
	};

	class WaypointGizmo;
	
	///////////////////////////////////////////////////////////////////////////////

	class Waypoint {
	public:
		typedef boost::shared_ptr<Waypoint> Ref;
		typedef std::list<Ref> List;
		typedef std::map<Waypoint*, Connection*> ConnectionMap;
		typedef std::map<int, Waypoint*> IdMap;

		Waypoint(int id);

		int id;
		CString name;
		vec3 pos;
		WaypointGizmo *gizmo;
		ConnectionMap connections;
		List::iterator it;
		List::iterator itSel;
	};

	///////////////////////////////////////////////////////////////////////////////

	class WaypointGizmo : public CManipulator {
	public:

		WaypointGizmo();
		virtual ~WaypointGizmo();

		static WaypointGizmo *Create(Waypoint *waypoint, CWaypointMesh *mesh);

		Waypoint *waypoint;
		CWaypointMesh *mesh;
		int lastx, lasty;
		unsigned int color;
		unsigned int hlcolor;
		bool hover;
		bool dragging;
		bool snap;
		bool moved;
		float size;

		virtual void OnMouseDown( CMapView* pView, int nMX, int nMY, int nButtons, CPickObject* pSrc );
		virtual void OnMouseUp( CMapView* pView, int nMX, int nMY, int nButtons, CPickObject* pSrc );
		virtual void OnMouseMove( CMapView* pView, int nMX, int nMY, int nButtons, CPickObject* pSrc );
		virtual void OnMouseEnter( CMapView* pView, int nMX, int nMY, int nButtons, CPickObject* pSrc );
		virtual void OnMouseLeave( CMapView* pView, int nMX, int nMY, int nButtons, CPickObject* pSrc );
		virtual bool OnPopupMenu( CMapView* pView, int nMX, int nMY, int nButtons, CPickObject* pSrc);

		void OnDraw( CMapView* pView );

		static int Drag(CManipulator *m, void *p, void *p2);
	};

	///////////////////////////////////////////////////////////////////////////////

	class ContextMenu : public CObjectMenu {
	public:

		enum {
			kEditWaypoints,
			kAddWaypoint,
			kConnectWaypoints,
			kDisconnectWaypoints,
			kEditWaypoint,
			kDeleteSelected,
		};

		ContextMenu();
		virtual ~ContextMenu();

		CWaypointMesh *mesh;
		Waypoint *waypoint;
		CMapView *view;

		virtual void OnUpdateCmdUI(int id, CCmdUI *ui);
		virtual void OnMenuItem(int id);
	};

	friend class WaypointGizmo;
	friend class CWaypointDrag3DGizmo;
	friend class AbductedUserData;
	friend class CQuakeGame;

	bool IsConnected(Waypoint &a, Waypoint &b);
	bool Connect(Waypoint &a, Waypoint &b);
	void Disconnect(Waypoint &a, Waypoint &b);
	void CalcBounds();
	Waypoint* AddWaypoint(vec3 pos);
	void DeleteWaypoint(Waypoint *waypoint);
	void UpdateRenderMeshes();
	void MakeManipulators(CTreadDoc *doc);
	void FreeManipulators(CTreadDoc *doc);
	void OnWaypointDrag(Waypoint &waypoint);
	void EnableManipulators(CTreadDoc *doc, bool enable);
	void Move(const vec3 &delta);
	void SelectManipulators(CTreadDoc *doc, bool select);
	int NumSelectedManipulators();

	void PopupMenu_OnAddWaypoint(CMapView *view, Waypoint *context);
	void PopupMenu_OnConnectWaypoints(CMapView *view, Waypoint *context);
	void PopupMenu_OnDisconnectWaypoints(CMapView *view, Waypoint *context);
	void PopupMenu_OnDeleteSelectedWaypoints(CMapView *view, Waypoint *context);
	void PopupMenu_OnEditWaypoint(CMapView *view, Waypoint *context);

	int m_nextWaypointId;
	int m_currentRenderMeshIdx;
	Connection::Map::iterator m_currentRenderMesh;
	bool m_creating;
	bool m_drag;
	vec3 m_world[2];
	vec3 m_local[2];
	vec3 m_pos;

	Waypoint::List m_waypoints;
	Connection::Map m_connections;
};

///////////////////////////////////////////////////////////////////////////////

class CWaypointDrag3DGizmo : public CVec3D_Manipulator {
public:

	CWaypointDrag3DGizmo();
	virtual ~CWaypointDrag3DGizmo();

	void OnMouseDown( CMapView* pView, int nMX, int nMY, int nButtons, CPickObject* pSrc );
	void OnMouseUp( CMapView* pView, int nMX, int nMY, int nButtons, CPickObject* pSrc );
	bool OnDrag( CMapView* pView, int nButtons, const vec3& move );
	void OnDraw( CMapView* pView );

private:

	bool m_bMoved;
};

///////////////////////////////////////////////////////////////////////////////

class CMakeWaypointMesh : public CObjectCreator
{
public:
	virtual void Release() { delete this; }
	virtual const char *Name() { return "Waypoint Mesh"; }
	virtual CMapObject *CreateObject(CTreadDoc *doc) { return CWaypointMesh::Create(doc); }
};
