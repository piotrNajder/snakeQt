#ifndef C_SNAKE_H
#define C_SNAKE_H

#include <stdint.h>
#include <vector>
#include "../include/cPoint.h"
#include "../include/cImage.h"
#include "../include/c2DArray.h"

class Snake {
private:
    std::vector<Point> points;
    double length = 0;
    
    int32_t width = 0;
    int32_t height = 0;
    Array2D<uint8_t> gradient;
    Array2D<uint8_t> flow;

    Array2D<double> pointsSpacing = Array2D<double>(3, 3);
    Array2D<double> pointsCurvature = Array2D<double>(3, 3);
    Array2D<double> e_flow = Array2D<double>(3, 3);
    Array2D<double> e_inertia = Array2D<double>(3, 3);

    bool calcNewPoints();

    double getSnakeLength();

    // Snake internal energy params
    double calcPointsSpacing(Point &prev, Point &p);
    double calcPointsCurvature(Point &prev, Point &p, Point &next);

    //External energy params
    double f_gflow(Point &cur, Point &p);
    double f_inertia(Point &cur, Point &p);

    void normalize(Array2D<double> &arr);
    void rebuildSnake();
    void removeOverlappingPoints();
    void addMissingPoints();

public:
    double alpha = 1.0;
    double beta = 1.0;
    double gamma = 1.0;
    double delta = 1.0;

    bool AUTOADAPT = true;
    int32_t AUTOADAPT_LOOP = 10;
    int32_t AUTOADAPT_MINLEN = 4;
    int32_t AUTOADAPT_MAXLEN = 8;
    int32_t MAXITERATION = 500;
 
    Snake(Array2D<uint8_t> g, Array2D<uint8_t> f, std::vector<Point> pts);
    std::vector<Point> adaptPoints();
};

#endif
