#pragma once
#include "LineSegment.h"
#include <vector>


class Plane
{
public:
	Plane();
	~Plane();
	int NumberofLines();
	int NumberofVertices();

	void SetPlane(vector<LineSegment*> l);

	LineSegment IthLine(int i);
	Vertex IthVertex(int i);

	void SetPlaneNumber(int number);
	int GetPlaneNumber();

	int GetLoopNo();

	void ChangeIthLine(int i,Vertex v1, Vertex v2);
	void ChangeIthVertex(int i, Vertex v1);

	void FindLoop(vector<LineSegment> &_LineList,MyMesh &sm);
	MyMesh::VertexHandle GenerateVertexHandle(int i, int max, MyMesh &sm);

	vector<int> FindThreeVertices();
	bool IsCCW();

	//void MergeLines();
	bool isTwoLinesAdjacent(LineSegment &a, LineSegment &b);
	bool isTwoLinesParallel(LineSegment &a, LineSegment &b);
	int isLoopFrom(vector<LineSegment> a);
	int Where1stLoopEnds(vector<LineSegment> &a);

	int Floatcmp(float a, float b);
	float Cross(float x1, float y1, float x2, float y2);
	int PointOnLine(Point a, Point b, Point c);

private:
	vector<LineSegment*> _Lines;
	vector<Vertex> _Vertices;  //平面上的顶点
	int _PlaneNumber;
	int _LoopNo;
	vector<int> _LinesFlipFlag;
	int _1stLoopEnds;
};

