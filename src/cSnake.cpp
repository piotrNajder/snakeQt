#include "../include/cSnake.h"
#include <limits>
#include <cmath>
#include <iostream>

Snake::Snake(Array2D<uint8_t> g, Array2D<uint8_t> f, std::vector<Point> pts) {
    points = pts;
    gradient = g;
    flow = f;
    width = g.cols;
    height = g.rows;
}

std::vector<Point> Snake::adaptPoints() {
    int32_t loop = 0;
    bool run = true;

    while(run && loop < MAXITERATION) {
        run = calcNewPoints();
        if (AUTOADAPT && ((loop % AUTOADAPT_LOOP) == 0)) {
            removeOverlappingPoints();
            addMissingPoints();
        }
        loop++;
    }

    if (AUTOADAPT == true) rebuildSnake();
    
    return points;
}

bool Snake::calcNewPoints() {
    bool changed = false;
    Point p = Point();
    length = getSnakeLength();

    std::vector<Point> newPoints = std::vector<Point>();

    for(int32_t i = 0; i < (int32_t)points.size(); i++) {
        Point prev = points[(i + points.size() - 1) % points.size()];
        Point cur  = points[i];
        Point next = points[(i + 1) % points.size()];

        for(int32_t dy = -1; dy <= 1; dy++) {
            for(int32_t dx = -1; dx <= 1; dx++) {
                p.setLocation(cur.x + dx, cur.y + dy);
                pointsSpacing[1+dx][1+dy] = calcPointsSpacing(prev, p);
                pointsCurvature[1+dx][1+dy] = calcPointsCurvature(prev, p, next);
                e_flow[1 + dx][1 + dy]       = f_gflow(cur, p);
                e_inertia[1 + dx][1 + dy]    = f_inertia(cur, p);
            }
        }

        normalize(pointsSpacing);
        normalize(pointsCurvature);

        normalize(e_flow);
        normalize(e_inertia);

        double emin = std::numeric_limits<double>::max();
        double e = 0;

        int32_t x = 0;
        int32_t y = 0;
        for(int32_t dy = -1; dy <= 1; dy++) {
            for(int32_t dx = -1; dx <= 1; dx++) {
                e = 0;
                e += alpha * pointsSpacing[1 + dx][1 + dy];
                e += beta  * pointsCurvature[1 + dx][1 + dy];
                e += gamma * e_flow[1 + dx][1 + dy];
                e += delta * e_inertia[1 + dx][1 + dy];

                if (e < emin) {
                    emin = e;
                    x = cur.x + dx;
                    y = cur.y + dy;
                }
            }
        }

        if (x < 1) x = 1;
        if (x >= width - 1) x = width - 2;
        if (y < 1) y = 1;
        if (y >= height - 1) y = height - 2;

        if (x != cur.x || y != cur.y) {
            changed = true;
        }

        newPoints.push_back(Point(x, y));
    }
    points = newPoints;
    return changed;
}

void Snake::normalize(Array2D<double> &arr) {
    double sum = 0.0;
    for(int32_t i = 0; i < 3; i++) {
        for(int32_t j = 0; j < 3; j++) {
            sum += fabs(arr[i][j]);
        }
    }

    if (sum != 0.0) {
        for(int32_t i = 0; i < 3; i++) {
            for(int32_t j = 0; j < 3; j++) {
                arr[i][j] /= sum;
            }
        }
    }
}

double Snake::getSnakeLength() {
    double length = 0;
    for(int32_t i = 0; i < (int32_t)points.size(); i++) {
        Point cur = points[i];
        Point next = points[(i + 1) % points.size()];
        length += cur.distance(next);
    }
    return length;
}

double Snake::calcPointsSpacing(Point &prev, Point &p) {
    double un = prev.distance(p);
    double avg = length / points.size();
    double dun = fabs( un - avg);
    return dun * dun;
}

double Snake::calcPointsCurvature(Point &prev, Point &p, Point &next) {
    int32_t ux = p.x - prev.x;
    int32_t uy = p.y - prev.y;
    double un = sqrt(ux * ux + uy * uy);

    int32_t vx = p.x - next.x;
    int32_t vy = p.y - next.y;
    double vn = sqrt(vx * vx + vy * vy);

    if (un == 0 || vn == 0) return 0;

    double cx = (vx + ux) / (un * vn);
    double cy = (vy + uy) / (un * vn);

    return cx * cx + cy * cy;
}

double Snake::f_gflow(Point &cur, Point &p) {
    int32_t dcur = flow[cur.y][cur.x];
    int32_t dp   = flow[p.y][p.x];
    return (double)(dp - dcur);
}

double Snake::f_inertia(Point &cur, Point &p) {
    double d = cur.distance(p);
    double g = gradient[cur.y][cur.x];
    return g * d;
}

void Snake::rebuildSnake() {
    double *clength = new double[points.size() + 1];
    clength[0] = 0;
    for(int32_t i = 0; i < (int32_t)points.size(); i++) {
        Point cur = points[i];
        Point next = points[(i + 1) % points.size()];
        clength[i+1] = clength[i] + cur.distance(next);
    }

    double total = clength[points.size()];
    int32_t nmb = (int32_t)(0.5 + total / AUTOADAPT_MAXLEN);

    std::vector<Point> newPoints;
    for(int32_t i = 0, j = 0; j < nmb; j++) {
        double dist = (j * total) / nmb;

        while(!(clength[i] <= dist && dist < clength[i+1])) i++;

        Point prev  = points[(i + points.size() - 1) % points.size()];
        Point cur   = points[i];
        Point next  = points[(i + 1) % points.size()];
        Point next2 = points[(i + 2) % points.size()];

        double t =  (dist - clength[i]) / (clength[i + 1] - clength[i]);
        double t2 = t * t;
        double t3 = t2 * t;
        double c0 =  1 * t3;
        double c1 = -3 * t3 + 3 * t2 +3 * t + 1;
        double c2 =  3 * t3 - 6 * t2 + 4;
        double c3 = -1 * t3 + 3 * t2 -3 * t + 1;
        double x = prev.x * c3 + cur.x * c2 +
                    next.x * c1 + next2.x * c0;
        double y = prev.y * c3 + cur.y * c2 +
                    next.y * c1 + next2.y * c0;
        Point newPoint = Point( (int32_t)(0.5 + x / 6), (int32_t)(0.5 + y / 6) );
        newPoints.push_back(newPoint);
    }
    points = newPoints;
}

void Snake::removeOverlappingPoints() {
    for(int32_t i = 0; i < (int32_t)points.size(); i++) {
        Point cur = points[i];
        for(int32_t di = 1 + points.size() / 2; di > 0; di--) {
            Point end  = points[(i + di) % points.size()];
            double dist = cur.distance(end);
            if (dist > AUTOADAPT_MINLEN) continue;
            for(int32_t k = 0; k < di; k++) {
                std::vector<Point>::iterator Pit = points.begin();
                Pit += (i + 1) % points.size();
                points.erase(Pit);
            }
            break;
        }
    }
}

void Snake::addMissingPoints() {
    for(int32_t i = 0; i < (int32_t)points.size(); i++) {
        Point prev  = points[(i + points.size()-1) % points.size()];
        Point cur   = points[i];
        Point next  = points[(i + 1) % points.size()];
        Point next2 = points[(i + 2) % points.size()];

        if (cur.distance(next) > AUTOADAPT_MAXLEN) {
            double c0 = 0.125 / 6.0;
            double c1 = 2.875 / 6.0;
            double c2 = 2.875 / 6.0;
            double c3 = 0.125 / 6.0;
            double x = prev.x * c3 + cur.x * c2 +
                       next.x * c1 + next2.x * c0;
            double y = prev.y * c3 + cur.y * c2 +
                       next.y * c1 + next2.y * c0;

            Point newPoint = Point((int)(0.5 + x), (int)(0.5 + y));
            points.insert(points.begin() + i + 1, newPoint);
            i--;
        }
    }
}
