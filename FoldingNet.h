#pragma once
#include <stack>
#include "Plane.h"
#include "opencv.hpp"
#include "../glut/glut.h"
#include"Modeling.h"



using namespace std;


class FoldingNet
{
public:
	FoldingNet();
	~FoldingNet();

	void LoadParameters();
	void ReadLinesFromTxt();
	//切割线段
	void CutOffLines();

	//灰度图转化成伪彩色图像
	cv::Mat GrayToRGB(cv::Mat img);
	cv::Mat GrayToRainbow(cv::Mat img);
	//OpenGl显示
	static void Show2DPattern();
	static FoldingNet * pThis;   //静态对象指针

	//线段像素化，找平面
	void BresenhamRasterization(LineSegment lineSeg, int number);  //Bresenham方法的线段像素化
	void FloodFill4Stack(int x, int y, int planenumber);
	void FindPolygonByFloodFill();  //floodfill方法找多边形

	//3D modeling
	void Folding();

	//opencv 的滑窗
	static void MouseClick(int event, int x, int y, int flags, void* param);
	static void myShowImageScroll(char* title, IplImage* src_img, int winWidth, int winHeight);

	static double mx , my ;
	static int dx , dy , horizBar_x , vertiBar_y ;
	static bool clickVertiBar , clickHorizBar , needScroll ;
	static CvRect rect_bar_horiz, rect_bar_verti;
	static int src_x , src_y ; // 源图像中 rect_src 的左上角位置


private:
	vector<LineSegment> _LineList;
	vector<Plane> _PolygonList;

	//两个数组，一个填线段的序号，个是flood多边形的序号
	//数组大小为线段坐标的取值范围
	int _RangeofPX;
	int _RangeofPY;
	int _MinPX;
	int _MinPY;

	cv::Mat _LineLabel;    //利用其寻找边界
	cv::Mat _PolygonLabel; //利用其填充图像


	stack<Point> _MyStack;

	//for openmesh
	MyMesh _SM;
	MyMesh _TM;
	vector<MyMesh::VertexHandle> _VHandle;

};

