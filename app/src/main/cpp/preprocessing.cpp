//
// Created by CUONG on 11/26/2018.
//

#include "preprocessing.h"


Rect extract_board(Mat &grayscale, Mat &binary)
{
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	vector<Rect> rects;
	int i;
	adaptiveThreshold(grayscale, binary, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 13, 0);

#if DEBUG
	imwrite("extract_board_binary.jpg", binary);
#endif

	findContours(binary, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	for (auto &c : contours)
	{
		rects.push_back(boundingRect(c));
	}

	Rect max_rect;
	for (i = 0; i < rects.size(); ++i)
	{
		if (rects[i].area() > max_rect.area())
		{
			max_rect = rects[i];
		}
	}
	return max_rect;
}

void getBinaryMat(Mat &color, Mat &binary)
{
    Mat planes[3];
    split(color, planes);
    threshold(planes[0], planes[0], 70, 255, THRESH_BINARY);
    threshold(planes[1], planes[1], 70, 255, THRESH_BINARY);
    threshold(planes[2], planes[2], 70, 255, THRESH_BINARY);
    binary = planes[0] + planes[1] + planes[2];
}

Rect extract_board_and_rect(Mat &color, Mat &binary)
{
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    vector<Rect> rects;
    int i;
    getBinaryMat(color, binary);

#if DEBUG
    imwrite("extract_board_binary.jpg", binary);
#endif

    findContours(binary, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    for (auto &c : contours)
    {
        rects.push_back(boundingRect(c));
    }

    Rect max_rect;
    for (i = 0; i < rects.size(); ++i)
    {
        if (rects[i].area() > max_rect.area())
        {
            max_rect = rects[i];
        }
    }
    binary = binary(max_rect);
    return max_rect;
}

void extract_table(Mat & board, Mat & horizontal, Mat & vertical, int horizontal_size, int vertical_size)
{
#if DEBUG
	imwrite("extract_table_binary.jpg", board);
#endif

	Mat horizontal_structure = getStructuringElement(MORPH_RECT, Size(horizontal_size, 1));
	erode(board, horizontal, horizontal_structure);
	dilate(horizontal, horizontal, horizontal_structure);

	Mat vertical_structure = getStructuringElement(MORPH_RECT, Size(1, vertical_size));
	erode(board, vertical, vertical_structure);
	dilate(vertical, vertical, vertical_structure);

#if DEBUG
	Mat table = horizontal.clone();
	vertical.copyTo(table, vertical);
	imwrite("horizontal.jpg", horizontal);
	imwrite("vertical.jpg", vertical);
	imwrite("table.jpg", table);
#endif

}

void extract_lines(Mat & horizontal, Mat & vertical, vector<Rect>& horiz_lines, vector<Rect>& vert_lines)
{
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours(horizontal, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	for (auto &c : contours)
	{
		horiz_lines.push_back(boundingRect(c));
	}
	findContours(vertical, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	for (auto &c : contours)
	{
		vert_lines.push_back(boundingRect(c));
	}
}

void extract_circles(Mat & board, vector<Rect>& circles)
{
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours(board, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	for (auto &c : contours)
	{
		auto r = boundingRect(c);
		if (r.area() > 30 * 30 && abs(r.width - r.height) < 10)
			circles.push_back(boundingRect(c));
	}
}

void calc_board_size(vector<Rect>& horiz_lines, vector<Rect>& vert_lines, Table &board)
{
	int i;
	if(vert_lines.size() < 3 || horiz_lines.size() < 3)
		return;
	board.row = board.column = 0;
	sort(horiz_lines.begin(), horiz_lines.end(), [](const Rect &lr, const Rect &rr) -> bool
	{
		return lr.tl().y + 0.5 * lr.height < rr.tl().y + 0.5 * rr.height;
	});
	sort(vert_lines.begin(), vert_lines.end(), [](const Rect &lr, const Rect &rr) -> bool
	{
		return lr.tl().x + 0.5 * lr.width < rr.tl().x + 0.5 * rr.width;
	});
	board.width = vert_lines.back().tl().x - vert_lines.front().tl().x;
	board.height = horiz_lines.back().tl().y - horiz_lines.front().tl().y;
	board.x = vert_lines.front().tl().x;
	board.y = horiz_lines.front().tl().y;

	for (i = 1; i < horiz_lines.size(); ++i)
	{
		Rect pre_line, line;
		pre_line = horiz_lines[i - 1];
		line = horiz_lines[i];
		if (line.tl().y + 0.5 * line.height > pre_line.tl().y + 0.5 * pre_line.height + 50)
			++board.row;
	}
	for (i = 1; i < vert_lines.size(); ++i)
	{
		Rect pre_line, line;
		pre_line = vert_lines[i - 1];
		line = vert_lines[i];
		if (line.tl().x + 0.5 * line.width > pre_line.tl().x + 0.5 * pre_line.width + 50)
			++board.column;
	}
	if(board.row < 1 || board.column < 1)
        return;
	board.title_x = board.width / board.column;
	board.title_y = board.height / board.row;
}

int assign_color(vector<Vec4b>& colors, Vec3b & color)
{
	if (colors.empty())
	{
		colors.push_back(Vec4b(color[0], color[1], color[2], 1));
		return 1;
	}
	for (auto c : colors)
	{
		if (abs(c[0] - color[0]) + abs(c[1] - color[1]) + abs(c[2] - color[2]) < 30)
		{
			colors.push_back(Vec4b(color[0], color[1], color[2], c[3]));
			return c[3];
		}
	}

	int id;
	id = colors.front()[3];
	for (auto &c : colors)
		if (c[3] > id)
			id = c[3];
	++id;
	colors.push_back(Vec4b(color[0], color[1], color[2], id));
	return id;
}
