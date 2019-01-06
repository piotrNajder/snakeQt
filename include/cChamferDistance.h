#ifndef C_CHAMFERDISTANCE_H
#define C_CHAMFERDISTANCE_H

#include <stdint.h>
#include "../include/cImage.h"
#include "../include/c2DArray.h"

class ChamferDistance {
public:
    Array2D<int16_t> chamfer = Array2D<int16_t>(3, 3);
    int32_t normalizer = 0; 
    int32_t width = 0;
    int32_t height = 0;
    
    ChamferDistance();
    void compute(Array2D<uint8_t> &in, Array2D<double> &out, int32_t w, int32_t h);

private:
    void testAndSet(Array2D<double> &output, int x, int y, double newvalue);
};

#endif