#pragma once
#include"MyMesh.h"
#include"Plane.h"

class Modeling
{
public:
	Modeling();
	Modeling(MyMesh sm);
	~Modeling();

	float computeRotateAngle(float angle, MyMesh::EdgeHandle edge, int f1, int f2);
	void RotateArbitraryAxis(float angle, MyMesh::EdgeHandle edge, float rtmatrix[4][4], int start_p);
	void changeVertexCoordinate(float **rtmatrix, int f_id);
	MyMesh folding();

private:
	vector<int> m_f1_idx;
	vector<int> m_f2_idx;
	vector<int> m_v1_idx;
	vector<int> m_v2_idx;
	MyMesh m_sm;
	MyMesh m_tm;
	vector<MyMesh::FaceHandle> m_fhlist;
	vector<MyMesh::VertexHandle> m_vhlist;

	float ***m_RTMatrix;
	float m_initialAngle;  //folding后所有邻接面的角度值

};

