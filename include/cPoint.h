#ifndef C_POINT_H
#define C_POINT_H

#include <stdint.h>

class Point {
public:
    int32_t x;
    int32_t y;

    Point();
    Point(int32_t newX, int32_t newY);
    void setLocation(int32_t newX, int32_t newY);
    double distance(const Point &B);
};

#endif