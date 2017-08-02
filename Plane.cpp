#include "Plane.h"
#include <algorithm>
#include<assert.h>

Plane::Plane()
{
	_LoopNo = 0;
}


Plane::~Plane()
{
}

int Plane::NumberofLines()
{
	return _Lines.size();
}

int Plane::NumberofVertices()
{
	return _Vertices.size();
}

void Plane::SetPlane(vector<LineSegment*> l)
{
	_Lines = l;
}

LineSegment Plane::IthLine(int i)
{
	return *_Lines[i];
}

Vertex Plane::IthVertex(int i)
{
	return _Vertices[i];
}


void Plane::SetPlaneNumber(int number)
{
	_PlaneNumber = number;
}

int Plane::GetPlaneNumber()
{
	return _PlaneNumber;
}

int Plane::GetLoopNo()
{
	return _LoopNo;
}

void Plane::ChangeIthLine(int i,Vertex v1, Vertex v2)
{
	_Lines[i]->SetV1(v1);
	_Lines[i]->SetV2(v2);
	_Lines[i]->SetP1(v1.GetCorrespondingPoint());
	_Lines[i]->SetP2(v2.GetCorrespondingPoint());
}

void Plane::ChangeIthVertex(int i, Vertex v1)
{
	_Vertices[i].SetX(v1.GetX());
	_Vertices[i].SetY(v1.GetY());
	_Vertices[i].SetZ(v1.GetZ());
	_Vertices[i].SetCorrespondingPoint(v1.GetCorrespondingPoint());
}


void Plane::FindLoop(vector<LineSegment> &_LineList, MyMesh &sm)
{

	if (_Lines.size() == 0)
	{
		cout << "Plane doesn't exist! " << endl;
		return;
	}

	vector<vector<LineSegment>> t_ListForFindingLoop;
	vector<LineSegment> _tmpLines;
	for (int i = 0; i < _Lines.size(); ++i)
	{
		_tmpLines.push_back(*_Lines[i]);
		/*LineSegment &temp = _tmpLines.back();
		if (temp.GetV1Handle())
			std::cerr <<"ID    "<< temp.GetId() << "      V1Handle   " << *temp.GetV1Handle() << endl;
		if(temp.GetV2Handle())
		    std::cerr << "ID    "<< temp.GetId() << "      V2Handle   " << *temp.GetV2Handle()<<endl;*/
	}

	

	/*
	//将_line中每个线段首尾颠倒，push到_Line中
	int t_size = _Lines.size();
	for (int i = 0; i < t_size; i++)
	{
		LineSegment t_line = *_Lines[i];
		t_line.SwapPoint();
		_Lines.push_back(t_line);
	}
	*/
	//将_line中每个线段首尾颠倒，push到_Line中
	int t_size = _Lines.size();
	for (int i = 0; i < t_size; i++)
	{
		LineSegment t_line = _tmpLines[i];
		t_line.SwapPoint();
		_tmpLines.push_back(t_line);
	}

	//Reordering,delete redundant lines,find loop
	//but会找到两个不同方向的回路！
	vector<LineSegment> s;
	s.push_back(_tmpLines[0]);
	t_ListForFindingLoop.push_back(s);
	_tmpLines.erase(_tmpLines.begin());
	s.clear();
	vector<int> adjacent_order;

	while (_tmpLines.size() > 0)
	{
		int left = _tmpLines.size();
		int t_ListNo = t_ListForFindingLoop.size();
		for (int i = 0; i < t_ListNo; i++)
		{
			//找到与list中最后一条线段邻接的线段
			for (int j = 0; j < _tmpLines.size(); j++)
			{
				if (isTwoLinesAdjacent(t_ListForFindingLoop[i][t_ListForFindingLoop[i].size()-1], _tmpLines[j]) == true)
					adjacent_order.push_back(j);
			}

			//若邻接线段只有一条
			if (adjacent_order.size() == 1)
			{
				t_ListForFindingLoop[i].push_back(_tmpLines[adjacent_order[0]]);
				_tmpLines.erase(_tmpLines.begin() + adjacent_order[0]);			
			}

		    //若邻接线段不止一条
			else if (adjacent_order.size() > 1)
			{
				for (int k = 1; k < adjacent_order.size(); k++)
				{
					t_ListForFindingLoop.push_back(t_ListForFindingLoop[i]);
					t_ListForFindingLoop[t_ListForFindingLoop.size()-1].push_back(_tmpLines[adjacent_order[k]]);
				}
				t_ListForFindingLoop[i].push_back(_tmpLines[adjacent_order[0]]);

				//若分别添加的两条线段也相邻，则也需要添加到List中去
				/*for (int k = 0; k < adjacent_order.size()-1; k++)
					for (int k1 = k + 1; k1 < adjacent_order.size(); k1++)
					{
						if (_Lines[adjacent_order[k]].GetP1() == _Lines[adjacent_order[k1]].GetP1())
						{
							_Lines[adjacent_order[k]].SwapPoint();
							s.push_back(_Lines[adjacent_order[k]]);
							s.push_back(_Lines[adjacent_order[k1]]);
							t_ListForFindingLoop.push_back(s);
							s.clear();
						}
					}	*/		
				
				int delete_no = 0;
				for (int k = 0; k < adjacent_order.size(); k++)
				{
					delete_no = adjacent_order[k] - k;
					_tmpLines.erase(_tmpLines.begin() + delete_no);
				}
			}

			else
				continue;

			adjacent_order.clear();
		}

		if (_tmpLines.size() == left)
		{
			s.push_back(_tmpLines[0]);
			t_ListForFindingLoop.push_back(s);
			_tmpLines.erase(_tmpLines.begin());
			s.clear();
		}
	}

	//check是否有回路
	//check前面的和最后一条线段是否首尾相连
	//但应去除首尾为同一条线段的情况

	for (int i = 0; i < t_ListForFindingLoop.size(); i++)
	{
		int flag = 0;
		if (flag == 0)
		{
			for (int j = 0; j < t_ListForFindingLoop[i].size() - 1; j++)
			{
				if (isLoopFrom(t_ListForFindingLoop[i]) >= 0)
				{
					int t_loopfrom = isLoopFrom(t_ListForFindingLoop[i]);
					_tmpLines.insert(_tmpLines.end(),t_ListForFindingLoop[i].begin() + t_loopfrom, t_ListForFindingLoop[i].end());
					_LoopNo++;   //环路加1
					flag = 1;
					break;
				}
				t_ListForFindingLoop[i].pop_back();
			}
		}
		
	}

	//删除同一条线段
	//删除重复回路中的线段
	vector<LineSegment>::iterator it, it1;
	for (it = _tmpLines.begin(); it != _tmpLines.end(); it++)
	{
		for (it1 = it + 1; it1 != _tmpLines.end(); )
		{
			if (*it1 == *it)
			{
				if ((*it1).GetIsDash() != (*it).GetIsDash())
					(*it).SetIsDash(0);
				it1 = _tmpLines.erase(it1);
			}
			else
				++it1;
		}
	}
	_LoopNo = _LoopNo / 2;


	/*for (int i = 0; i < _Lines.size(); i++)
		cout << _Lines[i].GetP1().GetX() << "   " << _Lines[i].GetP1().GetY() << "   " << _Lines[i].GetP2().GetX() << "   " << _Lines[i].GetP2().GetY() << endl;*/

	//MergeLines();

	
	_Lines.clear();
	int end_1 = Where1stLoopEnds(_tmpLines);
	_1stLoopEnds = end_1;

	//将顶点值相同的已存在的handle赋值到_LineList里去
	//考虑实虚线的情况！！
	for (int i = 0; i <= end_1; i++)
	{
		for (MyMesh::VertexIter v_it = sm.vertices_begin(); v_it != sm.vertices_end(); ++v_it)
		{
			if (fabs(sm.point(*v_it)[0] - (_LineList[_tmpLines[i].GetId() - 1]).GetP1().GetX()) < g_parameters.MyEpsilon && fabs(sm.point(*v_it)[1] - (_LineList[_tmpLines[i].GetId() - 1]).GetP1().GetY()) < g_parameters.MyEpsilon)
			{
				(_LineList[_tmpLines[i].GetId() - 1]).SetV1Handle(*v_it);
				break;
			}
		}
		for (MyMesh::VertexIter v_it = sm.vertices_begin(); v_it != sm.vertices_end(); ++v_it)
			if (fabs(sm.point(*v_it)[0] - (_LineList[_tmpLines[i].GetId() - 1]).GetP2().GetX()) < g_parameters.MyEpsilon && fabs(sm.point(*v_it)[1] - (_LineList[_tmpLines[i].GetId() - 1]).GetP2().GetY()) < g_parameters.MyEpsilon)
			{
				(_LineList[_tmpLines[i].GetId() - 1]).SetV2Handle(*v_it);
				break;
			}
		
	}

	//将第一个loop里的_LineList的地址赋值到_Lines中去
	for (int i = 0; i <= end_1; i++)
	{
		int no = _tmpLines[i].GetId();
		_Lines.push_back(&_LineList[no - 1]);     //有没有问题？
	}

	
	//记录有没有第一个loop的point颠倒与否
	for (int i = 0; i <= end_1; i++)
	{
		if (_Lines[i]->GetP1() == _tmpLines[i].GetP1())
			_LinesFlipFlag.push_back(0);
		else
			_LinesFlipFlag.push_back(1);
	}

	//add handle, add face
	vector<MyMesh::VertexHandle> h;
	h.reserve(end_1 + 1);

	for (int i = 0; i <= end_1; i++)
		h.push_back(GenerateVertexHandle(i,end_1,sm));

	//cout << "==========================================" << endl;
	//for (int i = 0; i < h.size(); i++)
	//{
	//	LineSegment &temp = *_Lines[i];
	//	if (temp.GetV1Handle())
	//		std::cerr << "ID    " << temp.GetId() << "      V1Handle   " << *temp.GetV1Handle() << endl;
	//	if (temp.GetV2Handle())
	//		std::cerr << "ID    " << temp.GetId() << "      V2Handle   " << *temp.GetV2Handle() << endl;
	//}
	//	//cout <<"ID    "<<(*_Lines[i]).GetId()<<"    "<< h[i] <<endl;
	//cout << endl;

	if (_tmpLines.size() > 0)
	{

		Vertex v; //交点对应的三维点

		for (int i = 0; i < _tmpLines.size(); i++)
		{

			//v.SetCorrespondingPoint(_Lines[i].GetP2());
			v.SetVP(_tmpLines[i].GetV2());
			_Vertices.push_back(v);
		}

		if (IsCCW() == false)
		{
			std::reverse(_Vertices.begin(), _Vertices.end());
			std::reverse(_tmpLines.begin(), _tmpLines.end());
			for (int i = 0; i < _tmpLines.size(); i++)
				_tmpLines[i].SwapPoint();
			std::reverse(h.begin(), h.end());
		}

		/*cout << "==========================================" << endl;
		for (int i = 0; i < h.size(); i++)
			cout << (*_Lines[i]).GetId()<<"   ";
		cout << endl;*/

		sm.add_face(h);

	}
	else
		cout <<_PlaneNumber<< "   No Loop !!!!!!!!!!!!!" << endl;

}

MyMesh::VertexHandle Plane::GenerateVertexHandle(int i, int max, MyMesh &sm)
{
	MyMesh::VertexHandle vh;
	MyMesh::VertexHandle v1h, v2h;
	if (i < max)
	{
		int flag1 = _LinesFlipFlag[i];
		int flag2 = _LinesFlipFlag[i+1];
		if(flag1 == 0)
			v1h = (*_Lines[i]).GetV2Handle();
		else
			v1h = (*_Lines[i]).GetV1Handle();

		if(flag2 == 0)
			v2h = (*_Lines[i + 1]).GetV1Handle();
		else
			v2h = (*_Lines[i + 1]).GetV2Handle();

		if ((MyMesh::VertexHandle(-1) == v1h) && (MyMesh::VertexHandle(-1) == v2h))
		{
			vh = sm.add_vertex(MyMesh::Point((*_Lines[i]).GetP(2-flag1).GetX(), (*_Lines[i]).GetP(2-flag1).GetY(), 0));
			(*_Lines[i]).SetVHandle(2 - flag1, vh);
			(*_Lines[i+1]).SetVHandle(1 + flag2, vh);

		}
		else if (MyMesh::VertexHandle(-1) != v1h && (MyMesh::VertexHandle(-1) != v2h))   //两个指针都有值
		{
			if (v1h != v2h)
			{
				std::cerr << "We have two different vertex handles over one vertex coordinate! " << endl;
				assert(v1h == v2h);
			}
			else
				vh = v1h;

		}
		else if ((MyMesh::VertexHandle(-1) == v1h) || (MyMesh::VertexHandle(-1) == v2h))
		{
			if (MyMesh::VertexHandle(-1) == v1h)
			{
				vh = v2h;
				//std::cerr << *v2h;
				(*_Lines[i]).SetVHandle(2 - flag1, v2h);
				
			}
			else if (MyMesh::VertexHandle(-1) == v2h)
			{
				vh = v1h;
				(*_Lines[i + 1]).SetVHandle(1 + flag2, v1h);

			}
			else
				assert(false);
		}
		else
			assert(false);
	}
	else if (i == max)
	{
		//对Lines的第一条线段的起点即最后一条线段的终点的操作
		int flag1 = _LinesFlipFlag[i];
		int flag2 = _LinesFlipFlag[0];
		if (flag1 == 0)
			v1h = (*_Lines[i]).GetV2Handle();
		else
			v1h = (*_Lines[i]).GetV1Handle();

		if (flag2 == 0)
			v2h = (*_Lines[0]).GetV1Handle();
		else
			v2h = (*_Lines[0]).GetV2Handle();

		if ((MyMesh::VertexHandle(-1) == v1h) && (MyMesh::VertexHandle(-1) == v2h))
		{
			vh = sm.add_vertex(MyMesh::Point((*_Lines[i]).GetP(2 - flag1).GetX(), (*_Lines[i]).GetP(2 - flag1).GetY(), 0));
			(*_Lines[i]).SetVHandle(2 - flag1, vh);		
			(*_Lines[0]).SetVHandle(1 + flag2, vh);

		}
		else if (MyMesh::VertexHandle(-1) != v1h && (MyMesh::VertexHandle(-1) != v2h))   //两个指针都有值
		{
			if (v1h != v2h)
			{
				cout << "We have two different vertex handles over one vertex coordinate! ";
				assert(v1h == v2h);
			}
			else
				vh = v1h;

		}
		else if ((MyMesh::VertexHandle(-1) == v1h) || (MyMesh::VertexHandle(-1) == v2h))
		{
			if (MyMesh::VertexHandle(-1) == v1h)
			{
				vh = v2h;
				(*_Lines[i]).SetVHandle(2 - flag1, v2h);
			}
			else if (MyMesh::VertexHandle(-1) == v2h)
			{
				vh = v1h;
				(*_Lines[0]).SetVHandle(1 + flag2, v1h);

			}
			else
				assert(false);
		}
		else
			assert(false);
	}
	else
		assert(i > max, "out of range when reading _Lines in GenerateVertexHandle()");
	
		//std::cerr <<"+++++"<< vh << endl;
	return vh;
}

vector<int> Plane::FindThreeVertices()
{
	//find 3 vertices that not in a line
	//notice: rand could have same values,need extra judgement

	vector<int> v_id;

	int *flag_v = new int[_Vertices.size()];
	for (int i = 0; i < _Vertices.size(); i++)
		flag_v[i] = -1;

	bool flag = true;
	while (flag == true)
	{
		v_id.clear();
		int i = 0;
		while (1)
		{
			int n = rand() % _1stLoopEnds;
			if (flag_v[n] == -1)
			{
				v_id.push_back(n);
				if ((++i) == 3)
					break;
			}
		}
		std::sort(v_id.begin(), v_id.end(), less<int>()); // sort from the former vertices to the latter in _Vertices
		flag = _Vertices[v_id[0]].IsThreePointsInALine(_Vertices[v_id[1]], _Vertices[v_id[2]]);
	}
	//for (int i = 0; i < 3; i++)
	//	cout << v_id[i] << "    ";
	//cout << endl;
	return v_id;
}

bool Plane::IsCCW()
{
	// judge whether the vertices stored counter-clockwise

	vector<int> v_id = FindThreeVertices();

	float xu = _Vertices[v_id[1]].GetX() - _Vertices[v_id[0]].GetX();
	float yu = _Vertices[v_id[1]].GetY() - _Vertices[v_id[0]].GetY();
	float zu = _Vertices[v_id[1]].GetZ() - _Vertices[v_id[0]].GetZ();

	float xv = _Vertices[v_id[2]].GetX() - _Vertices[v_id[0]].GetX();
	float yv = _Vertices[v_id[2]].GetY() - _Vertices[v_id[0]].GetY();
	float zv = _Vertices[v_id[2]].GetZ() - _Vertices[v_id[0]].GetZ();

	/*float xu = _Vertices[1].GetX() - _Vertices[0].GetX();
	float yu = _Vertices[1].GetY() - _Vertices[0].GetY();
	float zu = _Vertices[1].GetZ() - _Vertices[0].GetZ();

	float xv = _Vertices[2].GetX() - _Vertices[0].GetX();
	float yv = _Vertices[2].GetY() - _Vertices[0].GetY();
	float zv = _Vertices[2].GetZ() - _Vertices[0].GetZ();*/

	float rr = xu * yv - yu * xv;
	if (rr >= 0)
		return true;
	else
		return false;

}





bool Plane::isTwoLinesAdjacent(LineSegment &a, LineSegment &b)
{
	Point a_s = a.GetP1();
	Point a_t = a.GetP2();
	Point b_s = b.GetP1();
	Point b_t = b.GetP2();

	if (a_t == b_s && !(a_s == b_t)) //相邻，并且不是同一条线
		return true;
	else
		return false;
	
}

bool Plane::isTwoLinesParallel(LineSegment & a, LineSegment & b)
{
	//judge whether two lines are  parallel with slope
	Point a_s = a.GetP1();
	Point a_t = a.GetP2();
	Point b_s = b.GetP1();
	Point b_t = b.GetP2();

	float x1 = a_s.GetX();
	float y1 = a_s.GetY();
	float x2 = a_t.GetX();
	float y2 = a_t.GetY();

	float x3 = b_s.GetX();
	float y3 = b_s.GetY();
	float x4 = b_t.GetX();
	float y4 = b_t.GetY();

	if (abs(x1 - x2) < g_parameters.MyEpsilon || abs(x3 - x4) < g_parameters.MyEpsilon)
	{
		if (abs(x1 - x2) < g_parameters.MyEpsilon && abs(x3 - x4) < g_parameters.MyEpsilon)
			return true;
		else
			return false;
	}

	float k1 = (y2 - y1) / (x2 - x1);
	float k2 = (y4 - y3) / (x4 - x3);

	if (abs(k1 - k2) < g_parameters.MyEpsilon)
		return true;
	else
		return false;

}

int Plane::isLoopFrom(vector<LineSegment> a)
{
	//若构成Loop，返回初始节点的序号
	//若不是，则返回-1.
	for (int i = 0; i < a.size() - 1; i++)
	{
		if (a[i].GetP1() == a[a.size()-1].GetP2())
			return i;
	}
	return -1;
}

int Plane::Where1stLoopEnds(vector<LineSegment> &a)
{
	//返回第一条loop的最后一个节点的序号值
	for (int i = 0; i < a.size(); i++)
	{
		if (a[0].GetP1() == a[i].GetP2())
			return i;
	}
	return  -1;
}

int Plane::Floatcmp(float a, float b)
{
	if (fabs(a - b) <= g_parameters.MyEpsilon) return 0;
	if (a > b) return 1;
	else return -1;
}

float Plane::Cross(float x1, float y1, float x2, float y2)
{
	//叉积判断点是否在线段上
	return (x1*y2 - x2*y1)/(sqrt(x1*x1 + y1* y1)*sqrt(x2*x2 + y2*y2));
}

int Plane::PointOnLine(Point a, Point b, Point c)
{
	//求a点是不是在线段bc上，>0不在，=0与端点重合，<0在。
	float a_x = a.GetX();
	float a_y = a.GetY();
	float b_x = b.GetX();
	float b_y = b.GetY();
	float c_x = c.GetX();
	float c_y = c.GetY();
	return Floatcmp(Cross(b_x - a_x, b_y - a_y, c_x - a_x, c_y - a_y), 0);
}
