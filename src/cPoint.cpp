#include "../include/cPoint.h"
#include <cmath>

Point::Point() {
    x = 0;
    y = 0;
}

Point::Point(int32_t newX, int32_t newY) {
    x = newX;
    y = newY;
}

void Point::setLocation(int32_t newX, int32_t newY) {
    x = newX;
    y = newY;
}

double Point::distance(const Point &B) {
    int32_t ux = x - B.x;
    int32_t uy = y - B.y;
    double un = ux * ux + uy * uy;
    return sqrt(un);
}