//
// Created by CUONG on 11/27/2018.
//

#ifndef FLOWTREESOLVER_SOLVER_H
#define FLOWTREESOLVER_SOLVER_H


#include "preprocessing.h"
#include "board.h"
#include "minisat/core/Solver.h"


class FTSolver
{
	Mat inputMat, boardMat;
	Rect board_rect;
	vector<Rect> horiz_lines, vert_lines;
	vector<Point3i> hz_walls, vt_walls;
	vector<Point3i> _circles;
	vector<Vec3b> _colors;
	Board *_board = nullptr;
	Table _table;
	int **boundingEdge;
	int nColor;
	Minisat::Var ***vars;
	Minisat::Solver solver;

public:
	FTSolver(Mat &gameMat);
	bool preprocessMat();
	void detectWalls(Mat &horiz, Mat &vert);
    bool isHasWall(int i, int j, int type);
    void initBoard();
	void initVar();
	void createClauses();
	void exactly_one_true(Minisat::vec<Minisat::Lit> &literals);

	bool solveBoard();
	void drawSolution(Mat &solution);
	void drawPath(int x0, int y0, int x1, int y1, Mat &board, Vec3b color);

	void logDebug();
	~FTSolver();
};

#endif //FLOWTREESOLVER_SOLVER_H
