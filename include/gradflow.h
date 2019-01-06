#ifndef GRADFLOW_H
#define GRADFLOW_H

#include "c2DArray.h"
#include "cImage.h"

void getFlow(cImage<> const &image, Array2D<uint8_t> &gradient, Array2D<uint8_t> &flow, int32_t THRESHOLD);

#endif // GRADFLOW_H
