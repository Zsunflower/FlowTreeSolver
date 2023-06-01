//
// Created by CUONG on 11/26/2018.
//

#ifndef FLOWTREESOLVER_BOARD_H
#define FLOWTREESOLVER_BOARD_H

#include <opencv2/opencv.hpp>

using cv::Point3i;


class Board
{
	int _nrows, _ncolumns;
	int **_table;
public:
	Board(int rows, int columns);
	void init();
	void addCircle(Point3i c);
	bool isColor(int row, int col);
	int getColorID(int row, int col);
	void setColorID(int row, int col, int id);
	~Board();
};



#endif //FLOWTREESOLVER_BOARD_H
