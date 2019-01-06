#include <algorithm>
#include <cmath>
#include "../include/gradflow.h"
#include "../include/cChamferDistance.h"

void getFlow(cImage<> const &inImg, Array2D<uint8_t> &gradient, Array2D<uint8_t> &flow, int32_t THRESHOLD, bool saveIntSteps) {
    uint32_t W = inImg.columns;
    uint32_t H = inImg.rows;

    cImage<uint8_t> binGrdImg;

    Array2D<uint8_t> gScaleImg = Array2D<uint8_t>(W, H);

    for (uint32_t i = 0; i < H; ++i) {
        for (uint32_t j = 0; j < W; ++j) {
            gScaleImg[i][j] = (uint8_t)((0.299 * inImg.chR[i][j]) +
                                        (0.587 * inImg.chG[i][j]) +
                                        (0.114 * inImg.chB[i][j]));
        }
    }

    uint16_t maxGradVal = 0;
    for (uint32_t y = 0; y < H - 2; y++) {
        for (uint32_t x = 0; x < W - 2; x++) {
            uint8_t p00, p10, p20, p01, p21, p02, p12, p22;
            p00 = gScaleImg[y+0][x+0]; p10 = gScaleImg[y+1][x+0]; p20 = gScaleImg[y+2][x+0];
            p01 = gScaleImg[y+0][x+1]; /*---------------------*/  p21 = gScaleImg[y+2][x+1];
            p02 = gScaleImg[y+0][x+2]; p12 = gScaleImg[y+1][x+2]; p22 = gScaleImg[y+2][x+2];

            uint32_t sx = (p20 + 2*p21 + p22) - (p00 + 2*p01 + p02);
            uint32_t sy = (p02 + 2*p12 + p22) - (p00 + 2*p10 + p10);
            uint16_t snorm = (uint16_t)sqrt(sx*sx + sy*sy);
            gradient[y + 1][x + 1] = (snorm >= 255) ? 255 : snorm;
            maxGradVal = std::max(maxGradVal, snorm);
        }
    }

    Array2D<uint8_t> binGradient = Array2D<uint8_t>(W, H);
    int32_t t = THRESHOLD * maxGradVal / 100;
    for (uint32_t y = 0; y < H; y++) {
        for (uint32_t x = 0; x < W; x++) {
            if (gradient[y][x] > t) binGradient[y][x] = 255;
            else binGradient[y][x] = 0;
        }
    }

    if (saveIntSteps) {
        binGrdImg = cImage<uint8_t>(binGradient.arr, binGradient.cols, binGradient.rows);
        binGrdImg.write("./output/" + inImg.getFileName() + "_binGrad.pgm");
    }

    Array2D<double> cdist = Array2D<double>(W, H);
    ChamferDistance chamferDist = ChamferDistance();
    chamferDist.compute(binGradient, cdist, W, H);
    for (uint32_t y = 0; y < H; y++) {
        for (uint32_t x = 0; x < W; x++) {
            int32_t fVal = 5 * cdist[y][x];
            flow[y][x] = (fVal >= 255) ? 255 : fVal;
        }
    }
}
