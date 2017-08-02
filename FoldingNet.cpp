#include "FoldingNet.h"
#include <iostream>
#include <fstream>
#include "../glut/glut.h"

#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective


//opencv
double FoldingNet::mx = 0, FoldingNet::my = 0;
int FoldingNet::dx = 0, FoldingNet::dy = 0, FoldingNet::horizBar_x = 0, FoldingNet::vertiBar_y = 0;
bool FoldingNet::clickVertiBar = false, FoldingNet::clickHorizBar = false, FoldingNet::needScroll = false;
CvRect FoldingNet::rect_bar_horiz = { 0,0,1,1 }, FoldingNet::rect_bar_verti = {0,0,1,1};
int FoldingNet::src_x = 0, FoldingNet::src_y = 0; // 源图像中 rect_src 的左上角位置



FoldingNet::FoldingNet()
{
	pThis = this;
}


FoldingNet::~FoldingNet()
{
	_LineList.clear();
	_PolygonList.clear();
}

FoldingNet *FoldingNet::pThis = NULL;

void FoldingNet::LoadParameters()
{
	ifstream para("./data/parameters.txt");
	string type;
	float t_float;
	int t_int;
	string t_str;
	while (para)
	{
		para >> type;
		if (type == "MyPi")
		{
			para >> t_float;
			g_parameters.MyPi = t_float;
		}
		else if (type == "InputFilePath")
		{
			para >> t_str;
			g_parameters.InputFilePath = t_str;
		}
		else if (type == "MyEpsilon")
		{
			para >> t_float;
		    g_parameters.MyEpsilon = t_float;
		}
		else if (type == "MyInfinity")
		{
			para >> t_int;
			g_parameters.MyInfinity = t_int;
		}
		else if (type == "ResolutionMultipe")
		{
			para >> t_int;
			g_parameters.ResolutionMultipe = t_int;
		}
		else if (type == "ThresholdForSeparateLines")
		{
			para >> t_float;
			g_parameters.ThresholdForSeparateLines = t_float;
		}
	}

	para.close();

}

void FoldingNet::ReadLinesFromTxt()
{
	cout << "Reading Line Segments From TXT..." << endl;
	LineSegment temp_l;
	Vertex temp_v;

	float t_value1=0;
	float t_value2=0;
	float t_value3=0;
	float t_value4=0;
	int id = 1;

	string oneLine;
	ifstream txt_in(g_parameters.InputFilePath);
	string typeofLine;

	//需要知道二维坐标点的大概范围，以便使用Flood方法
	//查找二维点坐标中XY的最大最小值
	int t_max_x = 0;
	int t_max_y = 0;
	int t_min_x = 100000;
	int t_min_y = 100000;

	int isdash;
	//逐行读取
	while (txt_in)
	{
		txt_in >> typeofLine;
		if (typeofLine == "ISDASH")
			txt_in >> isdash;
		if (typeofLine == "LINESEGMENT" || typeofLine == "BEZIER")
		{	
			txt_in >> t_value1 >> t_value2 >> t_value3 >> t_value4 ;

			//防止线段两个端点一样
			if (fabs(t_value1 - t_value3) < g_parameters.MyEpsilon && fabs(t_value2 - t_value4) < g_parameters.MyEpsilon)
				continue;

			//查找二维点坐标中XY的最大最小值
			if (t_value1 > t_value3)
			{
				if (t_max_x < t_value1)
					t_max_x = t_value1;
				if (t_min_x > t_value3)
					t_min_x = t_value3;
			}
			else
			{
				if (t_max_x < t_value3)
					t_max_x = t_value3;
				if (t_min_x > t_value1)
					t_min_x = t_value1;
			}
			
			if (t_value2 > t_value4)
			{
				if (t_max_y < t_value2)
					t_max_y = t_value2;
				if (t_min_y > t_value4)
					t_min_y = t_value4;
			}
			else
			{
				if (t_max_y < t_value4)
					t_max_y = t_value4;
				if (t_min_y > t_value2)
					t_min_y = t_value2;
			}

			temp_l.SetP1(t_value1, t_value2);
			temp_l.SetP2(t_value3, t_value4);
			temp_l.SetIsDash(isdash);
			_LineList.push_back(temp_l);
			id++;
		}

	}
	txt_in.close();

	vector<LineSegment>::iterator it, it1;

	//防止线段重复读取
	for (it = _LineList.begin(); it != _LineList.end(); it++)
	{
		for (it1 = it + 1; it1 != _LineList.end(); )
		{
			if (*it1 == *it)
			{
				if ((*it1).GetIsDash() != (*it).GetIsDash())
					(*it).SetIsDash(0);
				it1 = _LineList.erase(it1);
			}
			else
				++it1;
		}
	}

	_MinPX = int(t_min_x);
	_MinPY = int(t_min_y);
	_RangeofPX = (int(t_max_x - t_min_x) + 5)*g_parameters.ResolutionMultipe;
	_RangeofPY = (int(t_max_y - t_min_y) + 5)*g_parameters.ResolutionMultipe;

	//测试List里是否存入正确数值
	/*for (int i = 0; i < 6; i++)
	{
		cout << _LineList[i].GetP1().GetX() << "    " << _LineList[i].GetP1().GetY() << "     "
			<< _LineList[i].GetP2().GetX() << "     " << _LineList[i].GetP2().GetY() << endl;
	}
	cout << _LineList.size() << endl;*/

	//切割线段
	CutOffLines();

	/*ofstream file1("./Lines_600.txt");
	for (int i = 0; i < _LineList.size(); i++)
	{
		file1 << _LineList[i].GetP1().GetX() << " " << _LineList[i].GetP1().GetY() << " " << _LineList[i].GetP2().GetX() << " " << _LineList[i].GetP2().GetY() << endl;
	}
	file1.close();*/
}

void FoldingNet::CutOffLines()
{
	Point p;
	int flag = 1;
	
	//若线段长度小于2，则不进行细分
	int Min_Length = 0.5;
	
	while (flag == 1)
	{
		flag = 0;
		for (int i = 0; i < _LineList.size() - 1; i++)
		{
			if (_LineList[i].P2DLength() > Min_Length)
			{
				for (int j = i + 1; j < _LineList.size(); j++)
				{
					if (_LineList[j].P2DLength() > Min_Length)
					{
						bool judge1 = (_LineList[i].GetP1() == _LineList[j].GetP2()) || (_LineList[i].GetP2() == _LineList[j].GetP1()) || (_LineList[i].GetP1() == _LineList[j].GetP1()) || (_LineList[i].GetP2() == _LineList[j].GetP2());   //两个顶点重叠
						bool judge2 = ((_LineList[i].GetP1() == _LineList[j].GetP1()) && (_LineList[i].GetP2() == _LineList[j].GetP2())) || ((_LineList[i].GetP1() == _LineList[j].GetP2()) && (_LineList[i].GetP2() == _LineList[j].GetP1()));   //同一条线段
						if (_LineList[i].IntersectionByTwoLines(_LineList[j], p) > 0 && !judge1)
						{
							flag = 1;
							switch (_LineList[i].IntersectionByTwoLines(_LineList[j], p))
							{
							case 1:
							{
								_LineList.push_back(_LineList[i]);
								_LineList[i].SetP2(p);
								_LineList[_LineList.size() - 1].SetP1(p);
								_LineList[_LineList.size() - 1].SetId(_LineList.size());
								_LineList.push_back(_LineList[j]);
								_LineList[j].SetP2(p);
								_LineList[_LineList.size() - 1].SetP1(p);
								_LineList[_LineList.size() - 1].SetId(_LineList.size());
								break;
							}
							case 2:
							{
								if (!judge1 && !judge2)
								{
									_LineList.push_back(_LineList[i]);
									_LineList[i].SetP2(p);
									_LineList[_LineList.size() - 1].SetP1(p);
									_LineList[_LineList.size() - 1].SetId(_LineList.size());
								}
								break;
							}
							case 3:
							{
								if (!judge1 && !judge2)
								{
									_LineList.push_back(_LineList[i]);
									_LineList[i].SetP2(p);
									_LineList[_LineList.size() - 1].SetP1(p);
									_LineList[_LineList.size() - 1].SetId(_LineList.size());
								}
								break;
							}
							case 4:
							{
								if (!judge1 && !judge2)
								{
									_LineList.push_back(_LineList[j]);
									_LineList[j].SetP2(p);
									_LineList[_LineList.size() - 1].SetP1(p);
									_LineList[_LineList.size() - 1].SetId(_LineList.size());
								}
								break;
							}
							case 5:
							{
								if (!judge1 && !judge2)
								{
									_LineList.push_back(_LineList[j]);
									_LineList[j].SetP2(p);
									_LineList[_LineList.size() - 1].SetP1(p);
									_LineList[_LineList.size() - 1].SetId(_LineList.size());
								}
								break;
							}
							default:
								break;
							}
						}
					}
					
				}
			}
			
		}
			
	}
	
	//防止线段重复读取
	vector<LineSegment>::iterator it, it1;
	for (it = _LineList.begin(); it != _LineList.end(); it++)
		for (it1 = it + 1; it1 != _LineList.end() ; )
		{
			if (*it1 == *it)
			{
				//cout << (*it1).GetP1().GetX() << "    " << (*it1).GetP1().GetY() << endl;
				if ((*it1).GetIsDash() != (*it).GetIsDash())
					(*it).SetIsDash(0);
				it1 = _LineList.erase(it1);
			}
			else
				++it1;
		}

	//将二维点的x,y赋值到三维坐标中去
	for (int i = 0; i < _LineList.size(); i++)
	{
		float x1 = _LineList[i].GetP1().GetX();
		float y1 = _LineList[i].GetP1().GetY();
		float x2 = _LineList[i].GetP2().GetX();
		float y2 = _LineList[i].GetP2().GetY();

		_LineList[i].SetV1(x1, y1, 0);
		_LineList[i].SetV2(x2, y2, 0);
		_LineList[i].SetPinV1(x1, y1);
		_LineList[i].SetPinV2(x2, y2);
	}

	//set id
	for (int i = 0; i < _LineList.size(); i++)
		_LineList[i].SetId(i + 1);

}

cv::Mat FoldingNet::GrayToRGB(cv::Mat img)
{
	cv::Mat img_pseudocolor(img.rows, img.cols, CV_8UC3);//构造RGB图像，参数CV_8UC3教程文档里面有讲解  
	int tmp = 0;
	for (int y = 0; y < img.rows; y++)//转为伪彩色图像的具体算法  
	{
		for (int x = 0; x < img.cols; x++)
		{
			tmp = img.at<unsigned char>(y, x);
			//没有平面的部分值为0，render时颜色设置为白色
			if (tmp == 0)
			{
				img_pseudocolor.at<cv::Vec3b>(y, x)[0] = 255; //blue  
				img_pseudocolor.at<cv::Vec3b>(y, x)[1] = 255; //green  
				img_pseudocolor.at<cv::Vec3b>(y, x)[2] = 255; //red 
			}
			else
			{
				img_pseudocolor.at<cv::Vec3b>(y, x)[0] = abs(255 - tmp); //blue  
				img_pseudocolor.at<cv::Vec3b>(y, x)[1] = abs(127 - tmp); //green  
				img_pseudocolor.at<cv::Vec3b>(y, x)[2] = abs(0 - tmp); //red 
			}
			 
		}
	}
	return img_pseudocolor;
}

cv::Mat FoldingNet::GrayToRainbow(cv::Mat img)
{
	cv::Mat img_color(img.rows, img.cols, CV_8UC3);//构造RGB图像  

	uchar tmp2 = 0;
	for (int y = 0; y < img.rows; y++)//转为彩虹图的具体算法，主要思路是把灰度图对应的0～255的数值分别转换成彩虹色：红、橙、黄、绿、青、蓝。  
	{
		for (int x = 0; x < img.cols; x++)
		{
			tmp2 = img.at<uchar>(y, x);
			if (tmp2 <= 51)
			{
				img_color.at<cv::Vec3b>(y, x)[0] = 255;
				img_color.at<cv::Vec3b>(y, x)[0] = tmp2 * 5;
				img_color.at<cv::Vec3b>(y, x)[0] = 0;
			}
			else if (tmp2 <= 102)
			{
				tmp2 -= 51;
				img_color.at<cv::Vec3b>(y, x)[0] = 255 - tmp2 * 5;
				img_color.at<cv::Vec3b>(y, x)[0] = 255;
				img_color.at<cv::Vec3b>(y, x)[0] = 0;
			}
			else if (tmp2 <= 153)
			{
				tmp2 -= 102;
				img_color.at<cv::Vec3b>(y, x)[0] = 0;
				img_color.at<cv::Vec3b>(y, x)[0] = 255;
				img_color.at<cv::Vec3b>(y, x)[0] = tmp2 * 5;
			}
			else if (tmp2 <= 204)
			{
				tmp2 -= 153;
				img_color.at<cv::Vec3b>(y, x)[0] = 0;
				img_color.at<cv::Vec3b>(y, x)[0] = 255 - uchar(128.0*tmp2 / 51.0 + 0.5);
				img_color.at<cv::Vec3b>(y, x)[0] = 255;
			}
			else
			{
				tmp2 -= 204;
				img_color.at<cv::Vec3b>(y, x)[0] = 0;
				img_color.at<cv::Vec3b>(y, x)[0] = 127 - uchar(127.0*tmp2 / 51.0 + 0.5);
				img_color.at<cv::Vec3b>(y, x)[0] = 255;
			}
		}
	}
	return img_color;
}

void FoldingNet::BresenhamRasterization(LineSegment lineSeg, int id)
{
	//http://blog.csdn.net/wozhengtao/article/details/51431389
	//线段像素化
	//Bresenham算法

	int y1 = round((lineSeg.GetP1().GetY() - _MinPY)*g_parameters.ResolutionMultipe) + 1;
	int x1 = round((lineSeg.GetP1().GetX() - _MinPX)*g_parameters.ResolutionMultipe) + 1;
	int y2 = round((lineSeg.GetP2().GetY() - _MinPY)*g_parameters.ResolutionMultipe) + 1;
	int x2 = round((lineSeg.GetP2().GetX() - _MinPX)*g_parameters.ResolutionMultipe) + 1;

	const bool steep = (abs(y2 - y1) > abs(x2 - x1));
	if (steep) //保证X增长的更快，即斜率不超过1
	{
		std::swap(x1, y1);
		std::swap(x2, y2);
	}
	if (x1 > x2)   //保证起点的X坐标小于终点
	{
		std::swap(x1, x2);
		std::swap(y1, y2);
	}
	const float dx = x2 - x1;
	const float dy = abs(y2 - y1);
	float error = dx / 2.0f;
	const int ystep = (y1 < y2) ? 1 : -1;
	int y = (int)y1;
	const int maxX = (int)x2;
	
	for (int x = (int)x1; x <= maxX; x++)
	{
		if (steep)
		{
			_LineLabel.at<float>(y,x) = id;
			_PolygonLabel.at<float>(y,x) = id;
		}
		else
		{
			_LineLabel.at<float>(x, y) = id;
			_PolygonLabel.at<float>(x, y) = id;
		}
		
		error -= dy;
		if (error < 0)
		{
			y += ystep;
			error += dx;
		}
	}

}

void FoldingNet::FloodFill4Stack(int x, int y, int planenumber)
{
	//Floodfill算法找平面
	//http://lodev.org/cgtutor/floodfill.html#4-Way_Method_With_Stack

	vector<LineSegment*> LinesInOnePlane;
	Plane t_plane;

	if (planenumber == 0) //avoid infinite loop
		return;
	_MyStack.empty();

	int dx[4] = { 0, 1, 0, -1 }; // relative neighbor x coordinates
    int dy[4] = { -1, 0, 1, 0 }; // relative neighbor y coordinates

	_MyStack.push(Point(x, y));

	while (_MyStack.size() > 0)
	{
		x = int(_MyStack.top().GetX());
		y = int(_MyStack.top().GetY());
		_MyStack.pop();
		_PolygonLabel.at<float>(x, y) = planenumber;
		for (int i = 0; i < 4; i++) 
		{
			int nx = x + dx[i];
			int ny = y + dy[i];
			if (nx >= 0 && nx < _RangeofPX && ny >= 0 && ny < _RangeofPY && _PolygonLabel.at<float>(nx, ny) == 0)
			{
				_MyStack.push(Point(nx, ny));
			}

			//需要将检测到的边缘线段存储至PolygonList[i]中
			else if (nx >= 0 && nx < _RangeofPX && ny >= 0 && ny < _RangeofPY &&_LineLabel.at<float>(nx, ny) != 0)
			{
				/*for (int kk = 1; kk < 5; kk++)
					if (nx - dx[i] * kk >= 0 && nx - dx[i] * kk < _RangeofPX && ny - dy[i] * kk >= 0 && ny - dy[i] * kk < _RangeofPY)
						_PolygonLabel.at<float>(nx - dx[i] * kk, ny - dy[i] * kk) = planenumber - 1;*/

				//_PolygonLabel.at<float>(nx - dx[i], ny - dy[i]) = planenumber -1;

				if (LinesInOnePlane.size() == 0)
					LinesInOnePlane.push_back(&_LineList[_LineLabel.at<float>(nx, ny)-1]);
				else
				{
					int t_found = 0; //遍历查找id值，若找到即为1，若没找到即为0
					for (int j = 0; j < LinesInOnePlane.size(); j++)
					{
						if (LinesInOnePlane[j]->GetId() == _LineLabel.at<float>(nx, ny))
							t_found++;
					}
					
					//若没有找到该id对应的线段，则将其加入polygon
					if (t_found == 0)
					{
						LinesInOnePlane.push_back(&_LineList[_LineLabel.at<float>(nx, ny) - 1]);
					}
										
				}
			}
				
		}
	}
	t_plane.SetPlane(LinesInOnePlane);
	t_plane.SetPlaneNumber(planenumber);
	_PolygonList.push_back(t_plane);    //将同一个多边形内的线段存储至一个Polygon，暂时线段无序存储
}

void FoldingNet::FindPolygonByFloodFill()
{
	
	_LineLabel = cv::Mat::zeros(_RangeofPX, _RangeofPY, CV_32FC1);
	_PolygonLabel = cv::Mat::zeros(_RangeofPX, _RangeofPY, CV_32FC1);

	cout << "Bresenham Rasterization..." << endl;
	//线段像素化
	for (int i = 0; i < _LineList.size(); i++)
		BresenhamRasterization(_LineList[i], _LineList[i].GetId());

	//查看像素化结果正确与否

	//旋转90度后的显示结果
	//cv::Mat _LineLabel_rotate;
	//cv::transpose(_LineLabel, _LineLabel_rotate);
	//cv::flip(_LineLabel_rotate, _LineLabel_rotate, 1);
	////_LineLabel_rotate.convertTo(_LineLabel_rotate, CV_8UC1);
	////_LineLabel_rotate = GrayToRGB(_LineLabel_rotate);
	////_LineLabel_rotate = _LineLabel_rotate <1;
	//imshow("Rasterization", _LineLabel_rotate);
	//cvWaitKey();

	/*imshow("Rasterization", _LineLabel);
	cvWaitKey();*/
		

	cout << "Find Polygon By FloodFill..." << endl;
	//怎么能遍历全图，填充完全图！简单点！
	//存在外部空白区域的问题需要解决

	int planenumber = 5;
	for (int i = 0; i < _RangeofPX-1; i++)
		for (int j = 0; j < _RangeofPY-1; j++)
		{
			if (_PolygonLabel.at<float>(i, j) == 0)
			{
				FloodFill4Stack(i, j, planenumber);
				planenumber = planenumber + 15;
			}
		}

	//删除最大的矩形
	//将删除的平面处的值设为0
	for (int i = 0; i < _RangeofPX - 1; i++)
		for (int j = 0; j < _RangeofPY - 1; j++)
		{
			if (_PolygonLabel.at<float>(i, j) == _PolygonList[0].GetPlaneNumber())
				_PolygonLabel.at<float>(i, j) = 0;
		}
	_PolygonList.erase(_PolygonList.begin());


	//若多边形由实线包裹而成，则删除该平面
	vector<Plane>::iterator it;
	for (it = _PolygonList.begin(); it != _PolygonList.end();)
	{
		int number_solid = 0;
		for (int j = 0; j < (*it).NumberofLines(); j++)
			if ((*it).IthLine(j).GetIsDash() == 0)
				number_solid++;

		if (number_solid == (*it).NumberofLines())
		{
			//将删除的平面处的_PolygonLabel值设为0
			for (int i = 0; i < _RangeofPX - 1; i++)
				for (int j = 0; j < _RangeofPY - 1; j++)
				{
					if (_PolygonLabel.at<float>(i, j) == (*it).GetPlaneNumber())
						_PolygonLabel.at<float>(i, j) = 0;
				}
			it = _PolygonList.erase(it);
		}
			
		else
		{
			//且对每个多边形的边缘进行重排序
			//(*it).ReorderingOfEdges();
			(*it).FindLoop(_LineList,_SM);
			++it;
		}		
	}	

	//write face colors
	_SM.request_face_colors();
	_SM.request_vertex_colors();

	OpenMesh::IO::Options wopt;
	wopt += OpenMesh::IO::Options::FaceColor;
	//wopt += OpenMesh::IO::Options::VertexColor;
	for (MyMesh::FaceIter f_it = _SM.faces_begin(); f_it != _SM.faces_end(); ++f_it)
	{
		//set plane color
		float color_b = rand() % 255;
		float color_g = rand() % 255;
		float color_r = rand() % 255;
		
		_SM.set_color(*f_it,MyMesh::Color(color_r,color_g,color_b));
	}

	//output SM
	try
	{
		if (!OpenMesh::IO::write_mesh(_SM, "./data/output.off",wopt))
		{
			std::cerr << "Cannot write mesh to file 'output.off'" << std::endl;
			return ;
		}
	}
	catch (std::exception& x)
	{
		std::cerr << x.what() << std::endl;
		return ;
	}

	/*_PolygonLabel.convertTo(_PolygonLabel, CV_8UC1);
	_PolygonLabel = GrayToRGB(_PolygonLabel);
	imshow("Test for FloodFill", _PolygonLabel);
	cvWaitKey();*/

	/*for (int i = 0; i < _LineLabel.rows; i++)
	{
		for (int j = 0; j < _LineLabel.cols; j++)
		{
			cout << _LineLabel.at<float>(i, j) << "   ";
		}
		cout << "+++++++++++++++++++++++++++++++++++++++" << endl;
	}*/

	//输出label信息
	/*ofstream file1("./labelLine.txt");
	for (int i = 0; i < _LineLabel.rows; i++)
	{
		for (int j = 0; j < _LineLabel.cols; j++)
		{
			file1 << _LineLabel.at<float>(i, j) << " ";
		}
		file1 << endl;
	}
	file1.close();

	ofstream file("./label.txt");
	for (int i = 0; i < _LineLabel.rows; i++)
	{
		for (int j = 0; j < _LineLabel.cols; j++)
		{
			file << _PolygonLabel.at<float>(i, j) <<" ";
		}
		file << endl;
	}
	file.close();*/

	//显示flooding结果
	//cv::Mat _PolygonLabel_rotate;
	//_PolygonLabel.convertTo(_PolygonLabel_rotate, CV_8UC1);

	//_PolygonLabel_rotate = GrayToRGB(_PolygonLabel_rotate);
	//cv::transpose(_PolygonLabel_rotate, _PolygonLabel_rotate);
	//cv::flip(_PolygonLabel_rotate, _PolygonLabel_rotate, 1);
	//cv::namedWindow("FloodFill Result");
 //   cvSetMouseCallback("FloodFill Result", MouseClick);
	////imshow("FloodFill Result", _PolygonLabel_rotate);

	//while (1)
	//{
	//	myShowImageScroll("FloodFill Result", &(IplImage)_PolygonLabel_rotate, 1600, 900);

	//	int KEY = cvWaitKey(10);
	//	if ((char)KEY == 27)   //Esc
	//		break;

	//}
	//cv::destroyWindow("FloodFill Result");
}

void FoldingNet::Folding()
{
	cout << "Generate 3D model ..." << endl;
	Modeling m = Modeling(_SM);
	_TM = m.folding();

	_TM.request_face_colors();
	_TM.request_vertex_colors();

	//output all the vertex coordinates of _TM
	//for (MyMesh::VertexIter v_it = _TM.vertices_begin(); v_it != _TM.vertices_end(); v_it++)
	//	cout << *v_it << "    " << _TM.point(*v_it) << endl;

	OpenMesh::IO::Options wopt;
	wopt += OpenMesh::IO::Options::FaceColor;
	//output TM
	try
	{
		if (!OpenMesh::IO::write_mesh(_TM, "./data/folding.off", wopt))
		{
			std::cerr << "Cannot write mesh to file 'folding.off'" << std::endl;
			return;
		}
	}
	catch (std::exception& x)
	{
		std::cerr << x.what() << std::endl;
		return;
	}

}

// opencv滑窗的鼠标响应
void FoldingNet::MouseClick(int event, int x, int y, int flags, void* param)
{
	if (needScroll)
	{
		switch (event)
		{
		case CV_EVENT_LBUTTONDOWN:
			mx = x, my = y;
			dx = 0, dy = 0;
			// 按下左键时光标定位在水平滚动条区域内  
			if (x >= rect_bar_horiz.x && x <= rect_bar_horiz.x + rect_bar_horiz.width
				&& y >= rect_bar_horiz.y && y <= rect_bar_horiz.y + rect_bar_horiz.height)
			{
				clickHorizBar = true;
			}
			// 按下左键时光标定位在垂直滚动条区域内  
			if (x >= rect_bar_verti.x && x <= rect_bar_verti.x + rect_bar_verti.width
				&& y >= rect_bar_verti.y && y <= rect_bar_verti.y + rect_bar_verti.height)
			{
				clickVertiBar = true;
			}
			break;
		case CV_EVENT_MOUSEMOVE:
			if (clickHorizBar)
			{
				dx = fabs(x - mx) > 1 ? (int)(x - mx) : 0;
				dy = 0;
			}
			if (clickVertiBar)
			{
				dx = 0;
				dy = fabs(y - my) > 1 ? (int)(y - my) : 0;
			}
			mx = x, my = y;
			break;
		case CV_EVENT_LBUTTONUP:
			mx = x, my = y;
			dx = 0, dy = 0;
			clickHorizBar = false;
			clickVertiBar = false;
			break;
		default:
			dx = 0, dy = 0;
			break;
		}
	}
}

//opencv滑窗
void FoldingNet::myShowImageScroll(char * title, IplImage * src_img, int winWidth, int winHeight)
{
	IplImage* dst_img;
	CvRect rect_dst, // 窗口中有效的图像显示区域
		rect_src; // 窗口图像对应于源图像中的区域
	int imgWidth = src_img->width,
		imgHeight = src_img->height,
		barWidth = 25; // 滚动条的宽度（像素）
	double scale_w = (double)imgWidth / (double)winWidth, // 源图像与窗口的宽度比值
		scale_h = (double)imgHeight / (double)winHeight; // 源图像与窗口的高度比值

	if (scale_w < 1)
		winWidth = imgWidth + barWidth;
	if (scale_h < 1)
		winHeight = imgHeight + barWidth;

	int showWidth = winWidth, showHeight = winHeight; // rect_dst 的宽和高

	int horizBar_width = 0, horizBar_height = 0,
		vertiBar_width = 0, vertiBar_height = 0;

	needScroll = scale_w > 1.0 || scale_h > 1.0 ? 1 : 0;
	// 若图像大于设定的窗口大小，则显示滚动条
	if (needScroll)
	{
		dst_img = cvCreateImage(cvSize(winWidth, winHeight), src_img->depth, src_img->nChannels);
		cvZero(dst_img);
		// 源图像宽度大于窗口宽度，则显示水平滚动条
		if (scale_w > 1.0)
		{
			showHeight = winHeight - barWidth;
			horizBar_width = (int)((double)winWidth / scale_w);
			horizBar_height = winHeight - showHeight;
			horizBar_x = min(
				max(0, horizBar_x + dx),
				winWidth - horizBar_width);
			rect_bar_horiz = cvRect(
				horizBar_x,
				showHeight + 1,
				horizBar_width,
				horizBar_height);
			// 显示水平滚动条
			cvRectangleR(dst_img, rect_bar_horiz, cvScalarAll(255), -1);
		}
		// 源图像高度大于窗口高度，则显示垂直滚动条
		if (scale_h > 1.0)
		{
			showWidth = winWidth - barWidth;
			vertiBar_width = winWidth - showWidth;
			vertiBar_height = (int)((double)winHeight / scale_h);
			vertiBar_y = min(
				max(0, vertiBar_y + dy),
				winHeight - vertiBar_height);
			rect_bar_verti = cvRect(
				showWidth + 1,
				vertiBar_y,
				vertiBar_width,
				vertiBar_height);
			// 显示垂直滚动条
			cvRectangleR(dst_img, rect_bar_verti, cvScalarAll(255), -1);
		}

		showWidth = min(showWidth, imgWidth);
		showHeight = min(showHeight, imgHeight);
		// 设置窗口显示区的 ROI
		rect_dst = cvRect(0, 0, showWidth, showHeight);
		cvSetImageROI(dst_img, rect_dst);
		// 设置源图像的 ROI
		src_x = (int)((double)horizBar_x*scale_w);
		src_y = (int)((double)vertiBar_y*scale_h);
		src_x = min(src_x, imgWidth - showWidth);
		src_y = min(src_y, imgHeight - showHeight);
		rect_src = cvRect(src_x, src_y, showWidth, showHeight);
		cvSetImageROI(src_img, rect_src);
		// 将源图像内容复制到窗口显示区
		cvCopy(src_img, dst_img);

		cvResetImageROI(dst_img);
		cvResetImageROI(src_img);
		// 显示图像和滚动条
		cvShowImage(title, dst_img);

		cvReleaseImage(&dst_img);
	}
	// 源图像小于设定窗口，则直接显示图像，无滚动条
	else
	{
		cvShowImage(title, src_img);
	}
}


