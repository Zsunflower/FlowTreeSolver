//
// Created by CUONG on 11/26/2018.
//

#ifndef FLOWTREESOLVER_PREPROCESSING_H
#define FLOWTREESOLVER_PREPROCESSING_H

#include <opencv2/opencv.hpp>
#include <vector>

using namespace cv;
using std::vector;

struct Table
{
	int row, column;
	int width, height;
	int x, y;
	int title_x, title_y;
};

Rect extract_board(Mat &grayscale, Mat &binary);

Rect extract_board_and_rect(Mat &color, Mat &binary);

void extract_table(Mat &board, Mat &horizontal, Mat &vertical,
                   int horizontal_size = 50, int vertical_size = 50);

void extract_lines(Mat &horizontal, Mat &vertical, vector<Rect> &horiz_lines,
                   vector<Rect> &vert_lines);

void extract_circles(Mat &board, vector<Rect> &circles);

void calc_board_size(vector<Rect> &horiz_lines, vector<Rect> &vert_lines, Table &board);

int assign_color(vector<Vec4b> &colors, Vec3b &color);

#endif //FLOWTREESOLVER_PREPROCESSING_H
