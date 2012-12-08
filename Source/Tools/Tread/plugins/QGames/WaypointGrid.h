/*! \file WaypointGrid.h
	\copyright Copyright (c) 2012 Sunside Inc., All Rights Reserved.
	\copyright See Abducted/LICENSE for licensing terms.
	\author Joe Riedel
	\ingroup tread
*/

#pragma once

#include "System.h"
#include "vec3d_manipulator.h"
#include <map>
#include <set>

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

	void WriteToMapFile(std::fstream &fs, CTreadDoc *doc);

private:

	///////////////////////////////////////////////////////////////////////////////

	class Connection;
	class ControlPointGizmo;
	class ControlPointGizmo3D : public CVec3D_Manipulator {
	public:

		ControlPointGizmo3D();
		virtual ~ControlPointGizmo3D();

		ControlPointGizmo *src;
		bool moved;

		virtual void OnMouseDown( CMapView* pView, int nMX, int nMY, int nButtons, CPickObject* pSrc );
		virtual bool OnDrag( CMapView* pView, int nButtons, const vec3& move );
	};

	friend class ControlPointGizmo3D;

	class ControlPointGizmo : public CManipulator {
	public:

		ControlPointGizmo();
		virtual ~ControlPointGizmo();

		virtual void OnMouseDown( CMapView* pView, int nMX, int nMY, int nButtons, CPickObject* pSrc );
		virtual void OnMouseUp( CMapView* pView, int nMX, int nMY, int nButtons, CPickObject* pSrc );
		virtual void OnMouseMove( CMapView* pView, int nMX, int nMY, int nButtons, CPickObject* pSrc );
		virtual void OnMouseEnter( CMapView* pView, int nMX, int nMY, int nButtons, CPickObject* pSrc );
		virtual void OnMouseLeave( CMapView* pView, int nMX, int nMY, int nButtons, CPickObject* pSrc );
		virtual void OnDraw( CMapView* pView );

		bool hover;
		bool moved;
		bool drag;
		bool snap;
		float size;
		int lastx, lasty;
		unsigned int color;
		unsigned int hlcolor;
		vec3 *pos;
		CWaypoint *waypoint;
		Connection *connection;
	};

	friend class ControlPointGizmo;

	///////////////////////////////////////////////////////////////////////////////

	class Connection : public CPickObject {
	public:
		typedef boost::shared_ptr<Connection> Ref;
		typedef std::map<int, Ref> Map;
		
		enum {
			kNumPoints = 100
		};

		enum {
			kProp_First,
			kProp_FwdStart = kProp_First,
			kProp_FwdEnd,
			kProp_BackStart,
			kProp_BackEnd,
			kProp_Flags,
			kProp_Num
		};

		enum {
			kFlag_First,
			kFlag_AtoB = kFlag_First,
			kFlag_BtoA,
			kFlag_BtoAUseAtoBScript,
			kFlag_AutoFace,
			kFlag_Num
		};

		Connection();
		virtual ~Connection();

		void InitProps();
		void InitMesh();
		void UpdateMesh();
		void InitArrow();
		void UpdateArrow();
		void Select(bool select);
		void Bind(CTreadDoc *doc);
		void CreateGizmos(CTreadDoc *doc);
		void DeleteGizmos(CTreadDoc *doc);

		CRenderMesh mesh;
		CRenderMesh arrow;
		CObjProp props[kProp_Num];
		CObjProp flags[kFlag_Num];
		vec3 ctrls[2];
		CWaypoint *head;
		CWaypoint *tail;
		ControlPointGizmo *gizmos[2];
		ControlPointGizmo3D *gizmos3D[2][3];
		int headId;
		int tailId;
	};

	friend class Connection;
	
	///////////////////////////////////////////////////////////////////////////////

	class ContextMenu : public CObjectMenu {
	public:

		enum {
			kConnect,
			kDisconnect
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

	typedef std::set<int> IntSet;

	void CalcBounds();
	void UpdateBoxMesh();
	void NotifyAttach(CTreadDoc *doc);
	void NotifyDetach(CTreadDoc *doc);
	void NotifyAttach(CTreadDoc *doc, CWaypoint &src);
	void NotifyDetach(CTreadDoc *doc, CWaypoint &src);
	void NotifyMoved(CTreadDoc *doc);
	void NotifyMoved(CTreadDoc *doc, CWaypoint &src);
	void CreateGizmos(CTreadDoc *doc);
	void DeleteGizmos(CTreadDoc *doc);
	void CreateGizmos(CTreadDoc *doc, CWaypoint &src);
	void DeleteGizmos(CTreadDoc *doc, CWaypoint &src);
	void AddSelectedConnectionProps(CTreadDoc* pDoc);
	void InitProps();
	void AddConnectionProps(Connection &c);
	void SetSelectedConnectionProp(CObjProp *p);
	void SetConnectionProp(Connection &c, CObjProp *p);

	void PopupMenu_OnConnectWaypoints(CMapView *view);
	void PopupMenu_OnDisconnectWaypoints(CMapView *view);

	static void Connect(CTreadDoc *doc, CWaypoint &src, CWaypoint &dst);
	static void Disconnect(CTreadDoc *doc, CWaypoint &src, CWaypoint &dst, bool flip);
	
	enum Properties {
		kProp_First,
		kProp_Name = kProp_First,
		kProp_Id,
		kProp_Floor,
		kProp_Flags,
		kProp_Num
	};

	CRenderMesh m_boxMesh;
	Connection::Map m_connections;
	CLinkedList<CObjProp> m_propList;
	Connection::Map::iterator m_meshIt;
	CObjProp m_props[kProp_Num];
	IntSet m_tails;
	vec3 m_world[2];
	vec3 m_local[2];
	vec3 m_pos;
	vec3 m_boxPos;
	bool m_drag;
	bool m_inAddSelection;
	bool m_beingSelected;
};

///////////////////////////////////////////////////////////////////////////////

class CMakeWaypoint : public CObjectCreator {
public:
	virtual void Release() { delete this; }
	virtual const char *Name() { return "Waypoint Node"; }
	virtual CMapObject *CreateObject(CTreadDoc *doc) { return CWaypoint::Create(doc); }
};
