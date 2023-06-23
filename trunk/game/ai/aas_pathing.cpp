/***************************************************************************
 *
 * PROJECT: The Dark Mod
 * $Revision: 2568 $
 * $Date: 2008-06-25 13:29:27 -0400 (Wed, 25 Jun 2008) $
 * $Author: angua $
 *
 ***************************************************************************/

// Copyright (C) 2004 Id Software, Inc.
//

#include "../../idlib/precompiled.h"
#pragma hdrstop

static bool init_version = FileVersionList("$Source$  $Revision: 2568 $   $Date: 2008-06-25 13:29:27 -0400 (Wed, 25 Jun 2008) $", init_version);

#include "aas_local.h"
#include "../../DarkMod/TimerManager.h"


#define SUBSAMPLE_WALK_PATH		1
#define SUBSAMPLE_FLY_PATH		0

const int		maxWalkPathIterations		= 10;
const float		maxWalkPathDistance			= 500.0f;
const float		walkPathSampleDistance		= 8.0f;

const int		maxFlyPathIterations		= 10;
const float		maxFlyPathDistance			= 500.0f;
const float		flyPathSampleDistance		= 8.0f;


/*
============
idAASLocal::EdgeSplitPoint

  calculates split point of the edge with the plane
  returns true if the split point is between the edge vertices
============
*/
bool idAASLocal::EdgeSplitPoint( idVec3 &split, int edgeNum, const idPlane &plane ) const {
	const aasEdge_t *edge;
	idVec3 v1, v2;
	float d1, d2;

	edge = &file->GetEdge( edgeNum );
	v1 = file->GetVertex( edge->vertexNum[0] );
	v2 = file->GetVertex( edge->vertexNum[1] );
	d1 = v1 * plane.Normal() - plane.Dist();
	d2 = v2 * plane.Normal() - plane.Dist();

	//if ( (d1 < CM_CLIP_EPSILON && d2 < CM_CLIP_EPSILON) || (d1 > -CM_CLIP_EPSILON && d2 > -CM_CLIP_EPSILON) ) {
	if ( FLOATSIGNBITSET( d1 ) == FLOATSIGNBITSET( d2 ) ) {
		return false;
	}
	split = v1 + (d1 / (d1 - d2)) * (v2 - v1);
	return true;
}

/*
============
idAASLocal::FloorEdgeSplitPoint

  calculates either the closest or furthest point on the floor of the area which also lies on the pathPlane
  the point has to be on the front side of the frontPlane to be valid
============
*/
bool idAASLocal::FloorEdgeSplitPoint( idVec3 &bestSplit, int areaNum, const idPlane &pathPlane, const idPlane &frontPlane, bool closest ) const {
	int i, j, faceNum, edgeNum;
	const aasArea_t *area;
	const aasFace_t *face;
	idVec3 split;
	float dist, bestDist;

	if ( closest ) {
		bestDist = maxWalkPathDistance;
	} else {
		bestDist = -0.1f;
	}

	area = &file->GetArea( areaNum );

	for ( i = 0; i < area->numFaces; i++ ) {
		faceNum = file->GetFaceIndex( area->firstFace + i );
		face = &file->GetFace( abs(faceNum) );

		if ( !(face->flags & FACE_FLOOR ) ) {
			continue;
		}

		for ( j = 0; j < face->numEdges; j++ ) {
			edgeNum = file->GetEdgeIndex( face->firstEdge + j );

			if ( !EdgeSplitPoint( split, abs( edgeNum ), pathPlane ) ) {
				continue;
			}
			dist = frontPlane.Distance( split );
			if ( closest ) {
				if ( dist >= -0.1f && dist < bestDist ) {
					bestDist = dist;
					bestSplit = split;
				}
			} else {
				if ( dist > bestDist ) {
					bestDist = dist;
					bestSplit = split;
				}
			}
		}
	}

	if ( closest ) {
		return ( bestDist < maxWalkPathDistance );
	} else {
		return ( bestDist > -0.1f );
	}
}

/*
============
idAASLocal::WalkPathValid

  returns true if one can walk in a straight line between origin and goalOrigin
============
*/
bool idAASLocal::WalkPathValid( int areaNum, const idVec3 &origin, int goalAreaNum, const idVec3 &goalOrigin, int travelFlags, idVec3 &endPos, int &endAreaNum, idActor* actor ) const {
	int curAreaNum, lastAreaNum, lastAreas[4], lastAreaIndex;
	idPlane pathPlane, frontPlane, farPlane;
	idReachability *reach;
	const aasArea_t *area;
	idVec3 p, dir;

	if ( file == NULL ) {
		endPos = goalOrigin;
		endAreaNum = 0;
		return true;
	}

	lastAreas[0] = lastAreas[1] = lastAreas[2] = lastAreas[3] = areaNum;
	lastAreaIndex = 0;

	pathPlane.SetNormal( (goalOrigin - origin).Cross( file->GetSettings().gravityDir ) );
	pathPlane.Normalize();
	pathPlane.FitThroughPoint( origin );

	frontPlane.SetNormal( goalOrigin - origin );
	frontPlane.Normalize();
	frontPlane.FitThroughPoint( origin );

	farPlane.SetNormal( frontPlane.Normal() );
	farPlane.FitThroughPoint( goalOrigin );

	curAreaNum = areaNum;
	lastAreaNum = curAreaNum;

	while ( 1 ) {

		// find the furthest floor face split point on the path
		if ( !FloorEdgeSplitPoint( endPos, curAreaNum, pathPlane, frontPlane, false ) ) {
			endPos = origin;
		}

		// if we found a point near or further than the goal we're done
		if ( farPlane.Distance( endPos ) > -0.5f ) {
			break;
		}

		// if we reached the goal area we're done
		if ( curAreaNum == goalAreaNum ) {
			break;
		}

		frontPlane.SetDist( frontPlane.Normal() * endPos );

		area = &file->GetArea( curAreaNum );

		for ( reach = area->reach; reach; reach = reach->next ) {
			if ( reach->travelType != TFL_WALK ) {
				continue;
			}

			// if the reachability goes back to a previous area
			if ( reach->toAreaNum == lastAreas[0] || reach->toAreaNum == lastAreas[1] ||
					reach->toAreaNum == lastAreas[2] || reach->toAreaNum == lastAreas[3] ) {
				continue;
			}

			// if undesired travel flags are required to travel through the area
			if ( file->GetArea( reach->toAreaNum ).travelFlags & ~travelFlags ) {
				continue;
			}

			// don't optimize through an area near a ledge
			if ( file->GetArea( reach->toAreaNum ).flags & AREA_LEDGE ) {
				continue;
			}

			// find the closest floor face split point on the path
			if ( !FloorEdgeSplitPoint( p, reach->toAreaNum, pathPlane, frontPlane, true ) ) {
				continue;
			}

			// direction parallel to gravity
			dir = ( file->GetSettings().gravityDir * endPos * file->GetSettings().gravityDir ) -
						( file->GetSettings().gravityDir * p * file->GetSettings().gravityDir );
			if ( dir.LengthSqr() > Square( file->GetSettings().maxStepHeight ) ) {
				continue;
			}

			// direction orthogonal to gravity
			dir = endPos - p - dir;
			if ( dir.LengthSqr() > Square( 0.2f ) ) {
				continue;
			}

			// angua: area is forbidden (e.g. locked door)
			if (actor != NULL && gameLocal.m_AreaManager.AreaIsForbidden(reach->toAreaNum, actor))
			{
				continue;
			}

			break;
		}

		if ( !reach ) {
			return false;
		}

		lastAreas[lastAreaIndex] = curAreaNum;
		lastAreaIndex = ( lastAreaIndex + 1 ) & 3;

		curAreaNum = reach->toAreaNum;
	}

	endAreaNum = curAreaNum;

	return true;
}

/*
============
idAASLocal::SubSampleWalkPath
============
*/
idVec3 idAASLocal::SubSampleWalkPath( int areaNum, const idVec3 &origin, const idVec3 &start, const idVec3 &end, int travelFlags, int &endAreaNum, idActor* actor ) {
	int i, numSamples, curAreaNum;
	idVec3 dir, point, nextPoint, endPos;

	dir = end - start;
	numSamples = (int) (dir.Length() / walkPathSampleDistance) + 1;

	point = start;
	for ( i = 1; i < numSamples; i++ ) {
		nextPoint = start + dir * ((float) i / numSamples);
		if ( (point - nextPoint).LengthSqr() > Square( maxWalkPathDistance ) ) {
			return point;
		}
		if ( !idAASLocal::WalkPathValid( areaNum, origin, 0, nextPoint, travelFlags, endPos, curAreaNum, actor ) ) {
			return point;
		}
		point = nextPoint;
		endAreaNum = curAreaNum;
	}
	return point;
}

/*
============
idAASLocal::WalkPathToGoal

  FIXME: don't stop optimizing on first failure ?
============
*/
bool idAASLocal::WalkPathToGoal( aasPath_t &path, int areaNum, const idVec3 &origin, int goalAreaNum, const idVec3 &goalOrigin, int travelFlags, idActor* actor ) {
	// Set the default values
	path.type = PATHTYPE_WALK;
	path.moveGoal = origin;
	path.moveAreaNum = areaNum;
	path.secondaryGoal = origin;
	path.reachability = NULL;
	path.elevatorRoute = eas::RouteInfoPtr();

	if ( file == NULL || areaNum == goalAreaNum ) {
		path.moveGoal = goalOrigin;
		return true;
	}

	int lastAreas[4] = { areaNum, areaNum, areaNum, areaNum };
	int lastAreaIndex = 0;

	int curAreaNum = areaNum;
	idReachability* reach(NULL);
	idVec3 endPos;
	int travelTime, endAreaNum;

	for ( int i = 0; i < maxWalkPathIterations; i++ ) {

		if ( !idAASLocal::RouteToGoalArea( curAreaNum, path.moveGoal, goalAreaNum, travelFlags, travelTime, &reach, actor ) ) {
			break;
		}

		if ( !reach ) {
			return false;
		}

		// no need to check through the first area
		if ( areaNum != curAreaNum ) {
			// only optimize a limited distance ahead
			if ( (reach->start - origin).LengthSqr() > Square( maxWalkPathDistance ) ) {
#if SUBSAMPLE_WALK_PATH
				path.moveGoal = SubSampleWalkPath( areaNum, origin, path.moveGoal, reach->start, travelFlags, path.moveAreaNum, actor );
#endif
				return true;
			}

			if ( !idAASLocal::WalkPathValid( areaNum, origin, 0, reach->start, travelFlags, endPos, endAreaNum, actor ) ) {
#if SUBSAMPLE_WALK_PATH
				path.moveGoal = SubSampleWalkPath( areaNum, origin, path.moveGoal, reach->start, travelFlags, path.moveAreaNum, actor );
#endif
				return true;
			}
		}

		path.moveGoal = reach->start;
		path.moveAreaNum = curAreaNum;

		if ( reach->travelType != TFL_WALK ) {
			break;
		}

		if ( !idAASLocal::WalkPathValid( areaNum, origin, 0, reach->end, travelFlags, endPos, endAreaNum, actor ) ) {
			return true;
		}

		path.moveGoal = reach->end;
		path.moveAreaNum = reach->toAreaNum;

		if ( reach->toAreaNum == goalAreaNum ) {
			if ( !idAASLocal::WalkPathValid( areaNum, origin, 0, goalOrigin, travelFlags, endPos, endAreaNum, actor ) ) {
#if SUBSAMPLE_WALK_PATH
				path.moveGoal = SubSampleWalkPath( areaNum, origin, path.moveGoal, goalOrigin, travelFlags, path.moveAreaNum, actor );
#endif
				return true;
			}
			path.moveGoal = goalOrigin;
			path.moveAreaNum = goalAreaNum;
			return true;
		}

		lastAreas[lastAreaIndex] = curAreaNum;
		lastAreaIndex = ( lastAreaIndex + 1 ) & 3;

		curAreaNum = reach->toAreaNum;

		if ( curAreaNum == lastAreas[0] || curAreaNum == lastAreas[1] ||
				curAreaNum == lastAreas[2] || curAreaNum == lastAreas[3] ) {
			common->Warning( "idAASLocal::WalkPathToGoal: local routing minimum going from area %d to area %d", areaNum, goalAreaNum );
			break;
		}
	}

	if ( !reach ) {
		// No standard reachability, check if the actor can use elevators
		if (actor != NULL && actor->CanUseElevators())
		{
			return elevatorSystem->FindRouteToGoal(path, areaNum, origin, goalAreaNum, goalOrigin, travelFlags, actor);
		}

		return false;
	}

	switch( reach->travelType ) {
		case TFL_WALKOFFLEDGE:
			path.type = PATHTYPE_WALKOFFLEDGE;
			path.secondaryGoal = reach->end;
			path.reachability = reach;
			break;
		case TFL_BARRIERJUMP:
			path.type |= PATHTYPE_BARRIERJUMP;
			path.secondaryGoal = reach->end;
			path.reachability = reach;
			break;
		case TFL_JUMP:
			path.type |= PATHTYPE_JUMP;
			path.secondaryGoal = reach->end;
			path.reachability = reach;
			break;
		default:
			break;
	}

	return true;
}

/*
============
idAASLocal::FlyPathValid

  returns true if one can fly in a straight line between origin and goalOrigin
============
*/
bool idAASLocal::FlyPathValid( int areaNum, const idVec3 &origin, int goalAreaNum, const idVec3 &goalOrigin, int travelFlags, idVec3 &endPos, int &endAreaNum ) const {
	aasTrace_t trace;

	if ( file == NULL ) {
		endPos = goalOrigin;
		endAreaNum = 0;
		return true;
	}

	file->Trace( trace, origin, goalOrigin );

	endPos = trace.endpos;
	endAreaNum = trace.lastAreaNum;

	if ( trace.fraction >= 1.0f ) {
		return true;
	}

	return false;
}

/*
============
idAASLocal::SubSampleFlyPath
============
*/
idVec3 idAASLocal::SubSampleFlyPath( int areaNum, const idVec3 &origin, const idVec3 &start, const idVec3 &end, int travelFlags, int &endAreaNum ) const {
	int i, numSamples, curAreaNum;
	idVec3 dir, point, nextPoint, endPos;

	dir = end - start;
	numSamples = (int) (dir.Length() / flyPathSampleDistance) + 1;

	point = start;
	for ( i = 1; i < numSamples; i++ ) {
		nextPoint = start + dir * ((float) i / numSamples);
		if ( (point - nextPoint).LengthSqr() > Square( maxFlyPathDistance ) ) {
			return point;
		}
		if ( !idAASLocal::FlyPathValid( areaNum, origin, 0, nextPoint, travelFlags, endPos, curAreaNum ) ) {
			return point;
		}
		point = nextPoint;
		endAreaNum = curAreaNum;
	}
	return point;
}

/*
============
idAASLocal::FlyPathToGoal

  FIXME: don't stop optimizing on first failure ?
============
*/
bool idAASLocal::FlyPathToGoal( aasPath_t &path, int areaNum, const idVec3 &origin, int goalAreaNum, const idVec3 &goalOrigin, int travelFlags ) const {
	int i, travelTime, curAreaNum, lastAreas[4], lastAreaIndex, endAreaNum;
	idReachability *reach(NULL);
	idVec3 endPos;

	path.type = PATHTYPE_WALK;
	path.moveGoal = origin;
	path.moveAreaNum = areaNum;
	path.secondaryGoal = origin;
	path.reachability = NULL;
	path.elevatorRoute = eas::RouteInfoPtr();

	if ( file == NULL || areaNum == goalAreaNum ) {
		path.moveGoal = goalOrigin;
		return true;
	}

	lastAreas[0] = lastAreas[1] = lastAreas[2] = lastAreas[3] = areaNum;
	lastAreaIndex = 0;

	curAreaNum = areaNum;

	for ( i = 0; i < maxFlyPathIterations; i++ ) {

		if ( !idAASLocal::RouteToGoalArea( curAreaNum, path.moveGoal, goalAreaNum, travelFlags, travelTime, &reach, NULL ) ) {
			break;
		}

		if ( !reach ) {
			return false;
		}

		// no need to check through the first area
		if ( areaNum != curAreaNum ) {
			if ( (reach->start - origin).LengthSqr() > Square( maxFlyPathDistance ) ) {
#if SUBSAMPLE_FLY_PATH
				path.moveGoal = SubSampleFlyPath( areaNum, origin, path.moveGoal, reach->start, travelFlags, path.moveAreaNum );
#endif
				return true;
			}

			if ( !idAASLocal::FlyPathValid( areaNum, origin, 0, reach->start, travelFlags, endPos, endAreaNum ) ) {
#if SUBSAMPLE_FLY_PATH
				path.moveGoal = SubSampleFlyPath( areaNum, origin, path.moveGoal, reach->start, travelFlags, path.moveAreaNum );
#endif
				return true;
			}
		}

		path.moveGoal = reach->start;
		path.moveAreaNum = curAreaNum;

		if ( !idAASLocal::FlyPathValid( areaNum, origin, 0, reach->end, travelFlags, endPos, endAreaNum ) ) {
			return true;
		}

		path.moveGoal = reach->end;
		path.moveAreaNum = reach->toAreaNum;

		if ( reach->toAreaNum == goalAreaNum ) {
			if ( !idAASLocal::FlyPathValid( areaNum, origin, 0, goalOrigin, travelFlags, endPos, endAreaNum ) ) {
#if SUBSAMPLE_FLY_PATH
				path.moveGoal = SubSampleFlyPath( areaNum, origin, path.moveGoal, goalOrigin, travelFlags, path.moveAreaNum );
#endif
				return true;
			}
			path.moveGoal = goalOrigin;
			path.moveAreaNum = goalAreaNum;
			return true;
		}

		lastAreas[lastAreaIndex] = curAreaNum;
		lastAreaIndex = ( lastAreaIndex + 1 ) & 3;

		curAreaNum = reach->toAreaNum;

		if ( curAreaNum == lastAreas[0] || curAreaNum == lastAreas[1] ||
				curAreaNum == lastAreas[2] || curAreaNum == lastAreas[3] ) {
			common->Warning( "idAASLocal::FlyPathToGoal: local routing minimum going from area %d to area %d", areaNum, goalAreaNum );
			break;
		}
	}

	if ( !reach ) {
		return false;
	}

	return true;
}

typedef struct wallEdge_s {
	int					edgeNum;
	int					verts[2];
	struct wallEdge_s *	next;
} wallEdge_t;

/*
============
idAASLocal::SortWallEdges
============
*/
void idAASLocal::SortWallEdges( int *edges, int numEdges ) const 
{
#ifdef TIMING_BUILD
	static int sortWallEdgesTimer = -1;
	if (sortWallEdgesTimer == -1)
	{
		CREATE_TIMER(sortWallEdgesTimer, name, "SortWallEdges");
	}
#endif

	START_TIMING(sortWallEdgesTimer);

	int i, j, k, numSequences;
	wallEdge_t **sequenceFirst, **sequenceLast, *wallEdges, *wallEdge;

	wallEdges = (wallEdge_t *) _alloca16( numEdges * sizeof( wallEdge_t ) );
	sequenceFirst = (wallEdge_t **)_alloca16( numEdges * sizeof( wallEdge_t * ) );
	sequenceLast = (wallEdge_t **)_alloca16( numEdges * sizeof( wallEdge_t * ) );

	for ( i = 0; i < numEdges; i++ ) {
		wallEdges[i].edgeNum = edges[i];
		GetEdgeVertexNumbers( edges[i], wallEdges[i].verts );
		wallEdges[i].next = NULL;
		sequenceFirst[i] = &wallEdges[i];
		sequenceLast[i] = &wallEdges[i];
	}

	numSequences = numEdges;
	for ( i = 0; i < numSequences; i++ ) {
		for ( j = i+1; j < numSequences; j++ ) {
			if ( sequenceFirst[i]->verts[0] == sequenceLast[j]->verts[1] ) {
				sequenceLast[j]->next = sequenceFirst[i];
				sequenceFirst[i] = sequenceFirst[j];
				break;
			}
			if ( sequenceLast[i]->verts[1] == sequenceFirst[j]->verts[0] ) {
				sequenceLast[i]->next = sequenceFirst[j];
				break;
			}
		}
		if ( j < numSequences ) {
			numSequences--;
			for ( k = j; k < numSequences; k++ ) {
				sequenceFirst[k] = sequenceFirst[k+1];
				sequenceLast[k] = sequenceLast[k+1];
			}
			i = -1;
		}
	}

	k = 0;
	for ( i = 0; i < numSequences; i++ ) {
		for ( wallEdge = sequenceFirst[i]; wallEdge; wallEdge = wallEdge->next ) {
			edges[k++] = wallEdge->edgeNum;
		}
	}
	STOP_TIMING(sortWallEdgesTimer);

}

/*
============
idAASLocal::GetWallEdges
============
*/
int idAASLocal::GetWallEdges( int areaNum, const idBounds &bounds, int travelFlags, int *edges, int maxEdges ) const 
{
#ifdef TIMING_BUILD
	static int getWallEdgesTimer = -1;
	if (getWallEdgesTimer == -1)
	{
		CREATE_TIMER(getWallEdgesTimer, name, "GetWallEdges");
	}
#endif
	START_SCOPED_TIMING(getWallEdgesTimer, scopedWallEdgesTimer);

	if ( !file ) 
	{
		return 0;
	}

	int numEdges = 0;
	int numAreas = file->GetNumAreas();

	byte *areasVisited = (byte *) _alloca16( numAreas);
	memset( areasVisited, 0, numAreas * sizeof( byte ) );
	int *areaQueue = (int *) _alloca16( numAreas * sizeof( int ) );

	int queueStart = -1;
	int queueEnd = 0;
	areaQueue[0] = areaNum;
	areasVisited[areaNum] = true;

	idReachability *reach;

	for (int curArea = areaNum; queueStart < queueEnd; curArea = areaQueue[++queueStart] ) {

		const aasArea_t *area = &file->GetArea( curArea );

		for (int i = 0; i < area->numFaces; i++) {
			int face1Num = file->GetFaceIndex(area->firstFace + i);
			const aasFace_t *face1 = &file->GetFace( abs(face1Num) );

			if ( !(face1->flags & FACE_FLOOR ) ) {
				continue;
			}

			for (int j = 0; j < face1->numEdges; j++ ) {
				int edge1Num = file->GetEdgeIndex(face1->firstEdge + j);
				int absEdge1Num = abs( edge1Num );

				// test if the edge is shared by another floor face of this area
				int k;
				for (k = 0; k < area->numFaces; k++ ) {
					if ( k == i ) {
						continue;
					}
					int face2Num = file->GetFaceIndex( area->firstFace + k );
					const aasFace_t  *face2 = &file->GetFace( abs(face2Num) );

					if ( !(face2->flags & FACE_FLOOR ) ) {
						continue;
					}
					int l;
					for (l = 0; l < face2->numEdges; l++ ) {
						int edge2Num = abs( file->GetEdgeIndex( face2->firstEdge + l ) );
						if ( edge2Num == absEdge1Num ) {
							break;
						}
					}
					if ( l < face2->numEdges ) {
						break;
					}
				}
				if ( k < area->numFaces ) {
					continue;
				}

				// test if the edge is used by a reachability
				for (reach = area->reach; reach; reach = reach->next ) {
					if ( reach->travelType & travelFlags ) {
						if ( reach->edgeNum == absEdge1Num ) {
							break;
						}
					}
				}
				if ( reach ) {
					continue;
				}

				// test if the edge is already in the list
				for ( k = 0; k < numEdges; k++ ) {
					if ( edge1Num == edges[k] ) {
						break;
					}
				}
				if ( k < numEdges ) {
					continue;
				}

				// add the edge to the list
				edges[numEdges++] = edge1Num;
				if ( numEdges >= maxEdges ) {
					return numEdges;
				}
			}
		}

		// add new areas to the queue
		for ( reach = area->reach; reach; reach = reach->next ) {
			if ( reach->travelType & travelFlags ) {
				// if the area the reachability leads to hasn't been visited yet and the area bounds touch the search bounds
				if ( !areasVisited[reach->toAreaNum] && bounds.IntersectsBounds( file->GetArea( reach->toAreaNum ).bounds ) ) {
					areaQueue[queueEnd++] = reach->toAreaNum;
					areasVisited[reach->toAreaNum] = true;
				}
			}
		}
	}
	return numEdges;
}
