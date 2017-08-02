#include "FoldingNet.h"
#include "../glut/glut.h"

void main(int argc, char *argv[])
{

	FoldingNet foldingPaper;
	
	foldingPaper.LoadParameters();

	//读取txt文件
	foldingPaper.ReadLinesFromTxt();

	//找平面
	foldingPaper.FindPolygonByFloodFill();

	//modeling
	foldingPaper.Folding();

	system("pause");

}


