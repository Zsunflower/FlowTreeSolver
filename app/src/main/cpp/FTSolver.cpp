//
// Created by CUONG on 11/27/2018.
//

#include "FTSolver.h"
#include <android/log.h>

#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,"FTSolver",__VA_ARGS__)

FTSolver::FTSolver(Mat &gameMat)
{
    inputMat = gameMat;
    boundingEdge = nullptr;
    vars = nullptr;
    nColor = 0;
}

bool FTSolver::preprocessMat()
{
    Mat binary, binary_line;
    board_rect = extract_board_and_rect(inputMat, binary);
    boardMat = inputMat(board_rect);

    cvtColor(boardMat, binary_line, COLOR_BGR2GRAY);
    threshold(binary_line, binary_line, 50, 255, THRESH_BINARY);
    adaptiveThreshold(binary_line, binary_line, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 13, 0);

    adaptiveThreshold(binary, binary, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 13, 0);

    Mat horiz, vert;
    extract_table(binary_line, horiz, vert);

    extract_lines(horiz, vert, horiz_lines, vert_lines);
    if(horiz_lines.size() < 3 || vert_lines.size() < 3)
        return false;
    calc_board_size(horiz_lines, vert_lines, _table);
    if(_table.row < 1 || _table.column < 1)
        return false;

    detectWalls(horiz, vert);
    //Remove all lines
    binary.setTo(0, horiz);
    binary.setTo(0, vert);
    vector<Rect> circles;
    extract_circles(binary, circles);
    if(circles.size() < 3)
        return false;
    vector<Vec4b> list_colors;

    for(auto &c : circles)
    {
        int x = int(c.tl().x + 0.5 * c.width);
        int y = int(c.tl().y + 0.5 * c.height);
        Vec3b color = boardMat.at<Vec3b>(y, x);
        int colorid = assign_color(list_colors, color);
        x = (x - _table.x) / _table.title_x;
        y = (y - _table.y) / _table.title_y;
        _circles.push_back(Point3i(x, y, colorid));
        _colors.push_back(color);
    }
    return true;
}

void FTSolver::initBoard()
{
    int i, j;
    int x, y, z;
    boundingEdge = new int *[_table.row]{nullptr};
    for(i = 0; i < _table.row; ++i)
    {
        boundingEdge[i] = new int[_table.column]{0};
    }

    for(auto &hline : horiz_lines)
    {
        x = (hline.tl().x - _table.x + 0.2 * _table.title_x) / _table.title_x;
        z = (hline.tl().x + hline.width - _table.x + 0.2 * _table.title_x) / _table.title_x;
        y = (hline.tl().y - _table.y + 0.2 * _table.title_y) / _table.title_y;

        for(i = x; i < z; ++i)
        {
            if(y < _table.row)
                ++boundingEdge[y][i];
            if(y > 0)
                ++boundingEdge[y - 1][i];
        }
    }

    for(auto &vline : vert_lines)
    {
        x = (vline.tl().y - _table.y + 0.2 * _table.title_y) / _table.title_y;
        z = (vline.tl().y + vline.height - _table.y + 0.2 * _table.title_y) / _table.title_y;
        y = (vline.tl().x - _table.x + 0.2 * _table.title_x) / _table.title_x;

        for(i = x; i < z; ++i)
        {
            if(y < _table.column)
                ++boundingEdge[i][y];
            if(y > 0)
                ++boundingEdge[i][y - 1];
        }
    }
    _board = new Board(_table.row, _table.column);
    for(auto &c : _circles)
    {
        _board->addCircle(c);
        if(c.z > nColor)
            nColor = c.z;
    }
}

void FTSolver::initVar()
{
    int i, j, v;
    vars = new Minisat::Var **[_table.row]{nullptr};

    for(i = 0; i < _table.row; ++i)
    {
        vars[i] = new Minisat::Var *[_table.column]{nullptr};
        for(j = 0; j < _table.column; ++j)
        {
            if(boundingEdge[i][j] == 4)
            {
                vars[i][j] = new Minisat::Var[nColor];
                Minisat::vec<Minisat::Lit> literals;
                for(v = 0; v < nColor; ++v)
                {
                    auto var = solver.newVar();
                    vars[i][j][v] = var;
                    literals.push(Minisat::mkLit(var));
                }
                //Every cell is assigned a single color.
                exactly_one_true(literals);
                if(_board->isColor(i, j))
                {
                    //The color of every endpoint cell is known and specified.
                    int id = _board->getColorID(i, j);
                    solver.addClause(literals[id - 1]);
                }
            }
        }
    }
    createClauses();
}

void FTSolver::createClauses()
{
    int i, j, u;
    for(i = 0; i < _table.row; ++i)
    {
        for(j = 0; j < _table.column; ++j)
        {
            if(boundingEdge[i][j] == 4)
            {
                if(_board->isColor(i, j))
                {
                    //Every endpoint cell has exactly one neighbor which matches its color.
                    int id = _board->getColorID(i, j);
                    Minisat::vec<Minisat::Lit> literals;
                    if(i > 0 && boundingEdge[i - 1][j] == 4 && !isHasWall(j, i, 7))
                        literals.push(Minisat::mkLit(vars[i - 1][j][id - 1]));
                    if(i < _table.row - 1 && boundingEdge[i + 1][j] == 4 && !isHasWall(j, i, 8))
                        literals.push(Minisat::mkLit(vars[i + 1][j][id - 1]));
                    if(j > 0 && boundingEdge[i][j - 1] == 4 && !isHasWall(j, i, 9))
                        literals.push(Minisat::mkLit(vars[i][j - 1][id - 1]));
                    if(j < _table.column - 1 && boundingEdge[i][j + 1] == 4 && !isHasWall(j, i, 10))
                        literals.push(Minisat::mkLit(vars[i][j + 1][id - 1]));
                    exactly_one_true(literals);
                }
                else
                {
                    /*
                    The flow through every non-endpoint cell matches exactly one of the six direction types.
                    1	│	top-bottom
                    2	─	left-right
                    3	┘	top-left
                    4	└	top-right
                    5	┐	bottom-left
                    6	┌	bottom-right
                    */
                    Minisat::vec<Minisat::Lit> literals;
                    if(i > 0 && i < _table.row - 1 && boundingEdge[i - 1][j] == 4 &&
                       boundingEdge[i + 1][j] == 4 && !isHasWall(j, i, 1))
                    {
                        //1	│	top - bottom
                        auto var = solver.newVar();
                        literals.push(Minisat::mkLit(var));
                        for(u = 0; u < nColor; ++u)
                        {
                            solver.addClause(~Minisat::mkLit(var), ~Minisat::mkLit(vars[i][j][u]),
                                             Minisat::mkLit(vars[i - 1][j][u]));
                            solver.addClause(~Minisat::mkLit(var), Minisat::mkLit(vars[i][j][u]),
                                             ~Minisat::mkLit(vars[i - 1][j][u]));
                            solver.addClause(~Minisat::mkLit(var), ~Minisat::mkLit(vars[i][j][u]),
                                             Minisat::mkLit(vars[i + 1][j][u]));
                            solver.addClause(~Minisat::mkLit(var), Minisat::mkLit(vars[i][j][u]),
                                             ~Minisat::mkLit(vars[i + 1][j][u]));
                            //The neighbors of a cell not specified by its direction type must not match its color.
                            if(j > 0 && boundingEdge[i][j - 1] == 4 && !isHasWall(j, i, 9))
                            {
                                solver.addClause(~Minisat::mkLit(var),
                                                 ~Minisat::mkLit(vars[i][j][u]),
                                                 ~Minisat::mkLit(vars[i][j - 1][u]));
                            }
                            if(j < _table.column - 1 && boundingEdge[i][j + 1] == 4 &&
                               !isHasWall(j, i, 10))
                            {
                                solver.addClause(~Minisat::mkLit(var),
                                                 ~Minisat::mkLit(vars[i][j][u]),
                                                 ~Minisat::mkLit(vars[i][j + 1][u]));
                            }
                        }
                    }
                    if(j > 0 && j < _table.column - 1 && boundingEdge[i][j - 1] == 4 &&
                       boundingEdge[i][j + 1] == 4 && !isHasWall(j, i, 2))
                    {
                        //2	─	left-right
                        auto var = solver.newVar();
                        literals.push(Minisat::mkLit(var));
                        for(u = 0; u < nColor; ++u)
                        {
                            solver.addClause(~Minisat::mkLit(var), ~Minisat::mkLit(vars[i][j][u]),
                                             Minisat::mkLit(vars[i][j - 1][u]));
                            solver.addClause(~Minisat::mkLit(var), Minisat::mkLit(vars[i][j][u]),
                                             ~Minisat::mkLit(vars[i][j - 1][u]));
                            solver.addClause(~Minisat::mkLit(var), ~Minisat::mkLit(vars[i][j][u]),
                                             Minisat::mkLit(vars[i][j + 1][u]));
                            solver.addClause(~Minisat::mkLit(var), Minisat::mkLit(vars[i][j][u]),
                                             ~Minisat::mkLit(vars[i][j + 1][u]));
                            //The neighbors of a cell not specified by its direction type must not match its color.
                            if(i > 0 && boundingEdge[i - 1][j] == 4 && !isHasWall(j, i, 7))
                            {
                                solver.addClause(~Minisat::mkLit(var),
                                                 ~Minisat::mkLit(vars[i][j][u]),
                                                 ~Minisat::mkLit(vars[i - 1][j][u]));
                            }
                            if(i < _table.row - 1 && boundingEdge[i + 1][j] == 4 &&
                               !isHasWall(j, i, 8))
                            {
                                solver.addClause(~Minisat::mkLit(var),
                                                 ~Minisat::mkLit(vars[i][j][u]),
                                                 ~Minisat::mkLit(vars[i + 1][j][u]));
                            }
                        }
                    }
                    if(i > 0 && j > 0 && boundingEdge[i - 1][j] == 4 &&
                       boundingEdge[i][j - 1] == 4 && !isHasWall(j, i, 3))
                    {
                        //3	┘	top-left
                        auto var = solver.newVar();
                        literals.push(Minisat::mkLit(var));
                        for(u = 0; u < nColor; ++u)
                        {
                            solver.addClause(~Minisat::mkLit(var), ~Minisat::mkLit(vars[i][j][u]),
                                             Minisat::mkLit(vars[i - 1][j][u]));
                            solver.addClause(~Minisat::mkLit(var), Minisat::mkLit(vars[i][j][u]),
                                             ~Minisat::mkLit(vars[i - 1][j][u]));
                            solver.addClause(~Minisat::mkLit(var), ~Minisat::mkLit(vars[i][j][u]),
                                             Minisat::mkLit(vars[i][j - 1][u]));
                            solver.addClause(~Minisat::mkLit(var), Minisat::mkLit(vars[i][j][u]),
                                             ~Minisat::mkLit(vars[i][j - 1][u]));
                            //The neighbors of a cell not specified by its direction type must not match its color.
                            if(i < _table.row - 1 && boundingEdge[i + 1][j] == 4 &&
                               !isHasWall(j, i, 8))
                            {
                                solver.addClause(~Minisat::mkLit(var),
                                                 ~Minisat::mkLit(vars[i][j][u]),
                                                 ~Minisat::mkLit(vars[i + 1][j][u]));
                            }
                            if(j < _table.column - 1 && boundingEdge[i][j + 1] == 4 &&
                               !isHasWall(j, i, 10))
                            {
                                solver.addClause(~Minisat::mkLit(var),
                                                 ~Minisat::mkLit(vars[i][j][u]),
                                                 ~Minisat::mkLit(vars[i][j + 1][u]));
                            }
                        }
                    }
                    if(i > 0 && j < _table.column - 1 && boundingEdge[i - 1][j] == 4 &&
                       boundingEdge[i][j + 1] == 4 && !isHasWall(j, i, 4))
                    {
                        //4	└	top-right
                        auto var = solver.newVar();
                        literals.push(Minisat::mkLit(var));
                        for(u = 0; u < nColor; ++u)
                        {
                            solver.addClause(~Minisat::mkLit(var), ~Minisat::mkLit(vars[i][j][u]),
                                             Minisat::mkLit(vars[i - 1][j][u]));
                            solver.addClause(~Minisat::mkLit(var), Minisat::mkLit(vars[i][j][u]),
                                             ~Minisat::mkLit(vars[i - 1][j][u]));
                            solver.addClause(~Minisat::mkLit(var), ~Minisat::mkLit(vars[i][j][u]),
                                             Minisat::mkLit(vars[i][j + 1][u]));
                            solver.addClause(~Minisat::mkLit(var), Minisat::mkLit(vars[i][j][u]),
                                             ~Minisat::mkLit(vars[i][j + 1][u]));
                            //The neighbors of a cell not specified by its direction type must not match its color.
                            if(j > 0 && boundingEdge[i][j - 1] == 4 && !isHasWall(j, i, 9))
                            {
                                solver.addClause(~Minisat::mkLit(var),
                                                 ~Minisat::mkLit(vars[i][j][u]),
                                                 ~Minisat::mkLit(vars[i][j - 1][u]));
                            }
                            if(i < _table.row - 1 && boundingEdge[i + 1][j] == 4 &&
                               !isHasWall(j, i, 8))
                            {
                                solver.addClause(~Minisat::mkLit(var),
                                                 ~Minisat::mkLit(vars[i][j][u]),
                                                 ~Minisat::mkLit(vars[i + 1][j][u]));
                            }
                        }
                    }
                    if(i < _table.row - 1 && j > 0 && boundingEdge[i + 1][j] == 4 &&
                       boundingEdge[i][j - 1] == 4 && !isHasWall(j, i, 5))
                    {
                        //5	┐	bottom-left
                        auto var = solver.newVar();
                        literals.push(Minisat::mkLit(var));
                        for(u = 0; u < nColor; ++u)
                        {
                            solver.addClause(~Minisat::mkLit(var), ~Minisat::mkLit(vars[i][j][u]),
                                             Minisat::mkLit(vars[i + 1][j][u]));
                            solver.addClause(~Minisat::mkLit(var), Minisat::mkLit(vars[i][j][u]),
                                             ~Minisat::mkLit(vars[i + 1][j][u]));
                            solver.addClause(~Minisat::mkLit(var), ~Minisat::mkLit(vars[i][j][u]),
                                             Minisat::mkLit(vars[i][j - 1][u]));
                            solver.addClause(~Minisat::mkLit(var), Minisat::mkLit(vars[i][j][u]),
                                             ~Minisat::mkLit(vars[i][j - 1][u]));
                            //The neighbors of a cell not specified by its direction type must not match its color.
                            if(j < _table.column - 1 && boundingEdge[i][j + 1] == 4 &&
                               !isHasWall(j, i, 10))
                            {
                                solver.addClause(~Minisat::mkLit(var),
                                                 ~Minisat::mkLit(vars[i][j][u]),
                                                 ~Minisat::mkLit(vars[i][j + 1][u]));
                            }
                            if(i > 0 && boundingEdge[i - 1][j] == 4 && !isHasWall(j, i, 7))
                            {
                                solver.addClause(~Minisat::mkLit(var),
                                                 ~Minisat::mkLit(vars[i][j][u]),
                                                 ~Minisat::mkLit(vars[i - 1][j][u]));
                            }
                        }
                    }
                    if(i < _table.row - 1 && j < _table.column - 1 && boundingEdge[i + 1][j] == 4 &&
                       boundingEdge[i][j + 1] == 4 && !isHasWall(j, i, 6))
                    {
                        //6	┌	bottom-right
                        auto var = solver.newVar();
                        literals.push(Minisat::mkLit(var));
                        for(u = 0; u < nColor; ++u)
                        {
                            solver.addClause(~Minisat::mkLit(var), ~Minisat::mkLit(vars[i][j][u]),
                                             Minisat::mkLit(vars[i + 1][j][u]));
                            solver.addClause(~Minisat::mkLit(var), Minisat::mkLit(vars[i][j][u]),
                                             ~Minisat::mkLit(vars[i + 1][j][u]));
                            solver.addClause(~Minisat::mkLit(var), ~Minisat::mkLit(vars[i][j][u]),
                                             Minisat::mkLit(vars[i][j + 1][u]));
                            solver.addClause(~Minisat::mkLit(var), Minisat::mkLit(vars[i][j][u]),
                                             ~Minisat::mkLit(vars[i][j + 1][u]));
                            //The neighbors of a cell not specified by its direction type must not match its color.
                            if(i > 0 && boundingEdge[i - 1][j] == 4 && !isHasWall(j, i, 7))
                            {
                                solver.addClause(~Minisat::mkLit(var),
                                                 ~Minisat::mkLit(vars[i][j][u]),
                                                 ~Minisat::mkLit(vars[i - 1][j][u]));
                            }
                            if(j > 0 && boundingEdge[i][j - 1] == 4 && !isHasWall(j, i, 9))
                            {
                                solver.addClause(~Minisat::mkLit(var),
                                                 ~Minisat::mkLit(vars[i][j][u]),
                                                 ~Minisat::mkLit(vars[i][j - 1][u]));
                            }
                        }
                    }
                    exactly_one_true(literals);
                }
            }
        }
    }
}

void FTSolver::exactly_one_true(Minisat::vec<Minisat::Lit> &literals)
{
    solver.addClause(literals); //at least one true
    //all most one true
    for(size_t i = 0; i < literals.size(); ++i)
    {
        for(size_t j = i + 1; j < literals.size(); ++j)
        {
            solver.addClause(~literals[i], ~literals[j]);
        }
    }
}

bool FTSolver::solveBoard()
{
    bool ret = solver.solve();
    using Minisat::lbool;
    if(ret)
    {
        int i, j, u;
        for(i = 0; i < _table.row; ++i)
        {
            for(j = 0; j < _table.column; ++j)
            {
                if(boundingEdge[i][j] == 4)
                {
                    for(u = 0; u < nColor; ++u)
                    {
                        if(solver.modelValue(vars[i][j][u]) == l_True)
                        {
                            _board->setColorID(i, j, u + 1);
                            break;
                        }
                    }
                }
            }
        }
    }
    return ret;
}

void FTSolver::drawSolution(Mat &solution_mat)
{
    solution_mat = boardMat;
    //ID of colors drawn
    vector<int> drawnID;
    int x, y, pre_x, pre_y;
    int i;
    for(i = 0; i < _circles.size(); ++i)
    {
        int id = _circles[i].z;
        if(std::find(drawnID.begin(), drawnID.end(), id) == drawnID.end())
        {
            drawnID.push_back(id);
            x = _circles[i].x;
            y = _circles[i].y;
            Vec3b color = _colors[i];
            pre_x = x;
            pre_y = y;
            while(true)
            {
                if(y > 0 && _board->getColorID(y - 1, x) == id && (x != pre_x || y - 1 != pre_y) &&
                   !isHasWall(x, y, 7))
                {
                    drawPath(x, y, x, y - 1, solution_mat, color);
                    pre_x = x;
                    pre_y = y;
                    y -= 1;
                }
                else if(y < _table.row - 1 && _board->getColorID(y + 1, x) == id &&
                        (x != pre_x || y + 1 != pre_y) && !isHasWall(x, y, 8))
                {
                    drawPath(x, y, x, y + 1, solution_mat, color);
                    pre_x = x;
                    pre_y = y;
                    y += 1;
                }
                else if(x > 0 && _board->getColorID(y, x - 1) == id &&
                        (x - 1 != pre_x || y != pre_y) && !isHasWall(x, y, 9))
                {
                    drawPath(x, y, x - 1, y, solution_mat, color);
                    pre_x = x;
                    pre_y = y;
                    x -= 1;
                }
                else if(x < _table.column - 1 && _board->getColorID(y, x + 1) == id &&
                        (x + 1 != pre_x || y != pre_y) && !isHasWall(x, y, 10))
                {
                    drawPath(x, y, x + 1, y, solution_mat, color);
                    pre_x = x;
                    pre_y = y;
                    x += 1;
                }
                else
                    break;
            }
        }
    }
    solution_mat = inputMat;
}

void FTSolver::drawPath(int x0, int y0, int x1, int y1, Mat &board, Vec3b color)
{
    //Draw a connection from (x0, y0) to (x1, y1)
    line(board,
         Point((x0 + 0.5) * _table.title_x + _table.x, (y0 + 0.5) * _table.title_y + _table.y),
         Point((x1 + 0.5) * _table.title_x + _table.x, (y1 + 0.5) * _table.title_y + _table.y),
         Scalar(color[0], color[1], color[2], 255), 0.3 * _table.title_x, LINE_AA);
}

FTSolver::~FTSolver()
{
    int i, j;

    if(_board)
        delete _board;
    if(boundingEdge)
    {
        for(i = 0; i < _table.row; ++i)
        {
            delete[] boundingEdge[i];
        }
        delete[] boundingEdge;
    }
    if(vars)
    {
        for(i = 0; i < _table.row; ++i)
        {
            for(j = 0; j < _table.column; ++j)
            {
                if(vars[i][j])
                    delete[] vars[i][j];
            }
            delete[] vars[i];
        }
        delete[] vars;
    }
}

void FTSolver::logDebug()
{
    int i;
    LOGD("Size: %d x %d", _table.row, _table.column);
    LOGD("Total circles: %d", _circles.size());
    LOGD("%d, %d", _table.title_x, _table.title_y);
    for(i = 0; i < _circles.size(); ++i)
    {
        Point3i c = _circles[i];
        Vec3b color = _colors[i];
        LOGD("(x, y) = (%d, %d), colorid: %d", c.y, c.x, c.z);
    }
}

void FTSolver::detectWalls(Mat &horiz, Mat &vert)
{
    int i, min_size, max_size;
    vector<Rect> horiz_walls, vert_walls;
    min_size = horiz_lines[0].height;
    max_size = min_size;
    for(i = 1; i < horiz_lines.size(); ++i)
    {
        if(horiz_lines[i].height < min_size)
        {
            min_size = horiz_lines[i].height;
        }
        else if(horiz_lines[i].height > max_size)
        {
            max_size = horiz_lines[i].height;
        }
    }
    min_size *= 2;
    ++min_size;
    if(min_size >= max_size)
        return;
    Mat hwall_mat, vwall_mat, horizontal_structure;
    horizontal_structure = getStructuringElement(MORPH_RECT, Size(1, min_size));
    erode(horiz, hwall_mat, horizontal_structure);
    dilate(hwall_mat, hwall_mat, horizontal_structure);

    horizontal_structure = getStructuringElement(MORPH_RECT, Size(min_size, 1));
    erode(vert, vwall_mat, horizontal_structure);
    dilate(vwall_mat, vwall_mat, horizontal_structure);

    extract_lines(hwall_mat, vwall_mat, horiz_walls, vert_walls);
    for(auto &hwall : horiz_walls)
    {
        Point3i wall;
        wall.x = (hwall.tl().y - _table.y + 0.2 * _table.title_y) / _table.title_y;
        wall.y = (hwall.tl().x - _table.x + 0.2 * _table.title_x) / _table.title_x;
        wall.z = (hwall.tl().x + hwall.width - _table.x + 0.2 * _table.title_x) / _table.title_x;
        if(wall.x > 0 && wall.x < _table.row)
        {
            hz_walls.push_back(wall);
        }
    }
    for(auto &vwall : vert_walls)
    {
        Point3i wall;
        wall.x = (vwall.tl().x - _table.x + 0.2 * _table.title_x) / _table.title_x;
        wall.y = (vwall.tl().y - _table.y + 0.2 * _table.title_y) / _table.title_y;
        wall.z = (vwall.tl().y + vwall.height - _table.y + 0.2 * _table.title_y) / _table.title_y;
        if(wall.x > 0 && wall.x < _table.column)
        {
            vt_walls.push_back(wall);
        }
    }
}

bool FTSolver::isHasWall(int i, int j, int type)
{
    if(type == 1)
    {
        for(auto &hwall : hz_walls)
        {
            if(hwall.x == j || hwall.x == j + 1)
            {
                if(hwall.y <= i && hwall.z > i)
                    return true;
            }
        }
        return false;
    }
    if(type == 2)
    {
        for(auto &vwall : vt_walls)
        {
            if(vwall.x == i || vwall.x == i + 1)
            {
                if(vwall.y <= j && vwall.z > j)
                    return true;
            }
        }
        return false;
    }
    if(type == 3)
    {
        for(auto &hwall : hz_walls)
        {
            if(hwall.x == j)
            {
                if(hwall.y <= i && hwall.z > i)
                    return true;
            }
        }
        for(auto &vwall : vt_walls)
        {
            if(vwall.x == i)
            {
                if(vwall.y <= j && vwall.z > j)
                    return true;
            }
        }
        return false;
    }
    if(type == 4)
    {
        for(auto &hwall : hz_walls)
        {
            if(hwall.x == j)
            {
                if(hwall.y <= i && hwall.z > i)
                    return true;
            }
        }
        for(auto &vwall : vt_walls)
        {
            if(vwall.x == i + 1)
            {
                if(vwall.y <= j && vwall.z > j)
                    return true;
            }
        }
        return false;
    }
    if(type == 5)
    {
        for(auto &hwall : hz_walls)
        {
            if(hwall.x == j + 1)
            {
                if(hwall.y <= i && hwall.z > i)
                    return true;
            }
        }
        for(auto &vwall : vt_walls)
        {
            if(vwall.x == i)
            {
                if(vwall.y <= j && vwall.z > j)
                    return true;
            }
        }
        return false;
    }
    if(type == 6)
    {
        for(auto &hwall : hz_walls)
        {
            if(hwall.x == j + 1)
            {
                if(hwall.y <= i && hwall.z > i)
                    return true;
            }
        }
        for(auto &vwall : vt_walls)
        {
            if(vwall.x == i + 1)
            {
                if(vwall.y <= j && vwall.z > j)
                    return true;
            }
        }
        return false;
    }

    if(type == 7)
    {
        //Up
        for(auto &hwall : hz_walls)
        {
            if(hwall.x == j)
            {
                if(hwall.y <= i && hwall.z > i)
                    return true;
            }
        }
        return false;
    }
    if(type == 8)
    {
        //Down
        for(auto &hwall : hz_walls)
        {
            if(hwall.x == j + 1)
            {
                if(hwall.y <= i && hwall.z > i)
                    return true;
            }
        }
        return false;
    }
    if(type == 9)
    {
        //Left
        for(auto &vwall : vt_walls)
        {
            if(vwall.x == i)
            {
                if(vwall.y <= j && vwall.z > j)
                    return true;
            }
        }
        return false;
    }
    if(type == 10)
    {
        //Right
        for(auto &vwall : vt_walls)
        {
            if(vwall.x == i + 1)
            {
                if(vwall.y <= j && vwall.z > j)
                    return true;
            }
        }
        return false;
    }
    return false;
}
