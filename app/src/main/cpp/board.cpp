//
// Created by CUONG on 11/26/2018.
//

#include "board.h"

Board::Board(int rows, int columns)
{
	_nrows = rows;
	_ncolumns = columns;
	init();
}

void Board::init()
{
	int i;
	_table = new int*[_nrows] { nullptr };
	for (i = 0; i < _nrows; ++i)
	{
		_table[i] = new int[_ncolumns] { 0 };
	}
}

void Board::addCircle(Point3i c)
{
	_table[c.y][c.x] = c.z;
}

bool Board::isColor(int row, int col)
{
	return _table[row][col] > 0;
}

int Board::getColorID(int row, int col)
{
	return _table[row][col];
}

void Board::setColorID(int row, int col, int id)
{
	_table[row][col] = id;
}

Board::~Board()
{
	int i;
	for (i = 0; i < _nrows; ++i)
	{
		delete[] _table[i];
	}
	delete[] _table;
}
