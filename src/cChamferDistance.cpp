#include "../include/cChamferDistance.h"

ChamferDistance::ChamferDistance() {
    chamfer[0][0] = 1; chamfer[0][1] = 0; chamfer[0][2] = 5;
    chamfer[1][0] = 1; chamfer[1][1] = 1; chamfer[1][2] = 7;
    chamfer[2][0] = 2; chamfer[2][1] = 1; chamfer[2][2] = 11;

    normalizer = chamfer[0][2];
}

void ChamferDistance::compute(Array2D<uint8_t> &in, Array2D<double> &out, int32_t w, int32_t h) {
    width = w;
    height = h;
    
    // initialize distance
    for (int32_t y = 0; y < height; y++) {
        for (int32_t x = 0; x < width; x++) {
            if (in[y][x] ) out[y][x] = 0; // inside the object -> distance = 0
            else out[y][x] = -1; // outside the object -> to be computed
        }
    }
    
    // forward
    for (int32_t y = 0; y <= height - 1; y++) {
        for (int32_t x = 0; x <= width - 1; x++) {
            double v = out[y][x];
            if (v < 0) continue;
            for(int32_t k = 0; k < (int32_t)chamfer.rows; k++) {
                int32_t dx = chamfer[k][0];
				int32_t dy = chamfer[k][1];
				int32_t dt = chamfer[k][2];

                testAndSet(out, x + dx, y + dy, v + dt);
                if (dy != 0) testAndSet(out, x - dx, y + dy, v + dt);
                if (dx != dy) {
                    testAndSet(out, x+dy, y+dx, v+dt);
                    if (dy != 0) testAndSet(out, x-dy, y+dx, v+dt);
                }
            }
        }
    }
    
    // backward
    for (int32_t y = height - 1; y >= 0; y--) {
        for (int32_t x = width - 1; x >= 0; x--) {
            double v = out[y][x];
            if (v < 0) continue;
            for(int32_t k = 0; k < (int32_t)chamfer.rows; k++) {
                int32_t dx = chamfer[k][0];
				int32_t dy = chamfer[k][1];
				int32_t dt = chamfer[k][2];
                
                testAndSet(out, x - dx, y - dy, v + dt);
                if (dy != 0) testAndSet(out, x + dx, y - dy, v + dt);
                if (dx != dy) {
                    testAndSet(out, x - dy, y - dx, v + dt);
                    if (dy != 0) testAndSet(out, x + dy, y - dx, v + dt);
                }
            }
        }
    }
    
    // normalize
    for (int32_t y = 0; y < height; y++)
        for (int32_t x = 0; x < width; x++)
            out[y][x] = out[y][x] / normalizer;
}

void ChamferDistance::testAndSet(Array2D<double> &output, int x, int y, double newvalue) {
    if(x < 0 || x >= width) return;
    if(y < 0 || y >= height) return;
    double v = output[y][x];
    if (v >= 0 && v < newvalue) return;
    output[y][x] = newvalue;
}