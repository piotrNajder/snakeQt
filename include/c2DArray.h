#ifndef C_ARRAY2D_H
#define C_ARRAY2D_H

#include "../include/cOutOfBoundException.h"
#include <cstring>
#include <sstream>

template<class T>
class Array2D {
public:
    uint32_t rows = 0;
    uint32_t cols = 0;
    T **arr = nullptr;

    Array2D() {};
    Array2D(uint32_t c, uint32_t r)
    {
        cols = c;
        rows = r;

        arr = new T*[rows];
        arr[0] = new T[cols * rows];
        for (uint32_t i = 1; i < rows; i++)
            arr[i] = arr[i - 1] + cols;

        std::memset(arr[0], 0, c*r);
    }

    ~Array2D() {
        /*if (arr != nullptr) {
		    if (arr[0] != nullptr) delete[] arr[0];
		    delete[] arr;
	    }*/
    }
    
    class Array1D {
    public:
        uint32_t columns = 0;
        T* arr = nullptr;

        Array1D(T* a, uint32_t s) :arr(a) { columns = s; }

        T& operator[](uint32_t i) { 
            if (i < columns) return arr[i];
            else {
                std::stringstream ss;
                ss << "Index: " << i << " , maxIndex: " << columns << ".";
                throw OutOfBoundException("Out of bound access (columns): " + ss.str());
            }
        }
    };

    Array1D operator[] (uint32_t a) { 
        if (a < rows) return Array1D(arr[a], cols); 
        else {
            std::stringstream ss;
            ss << "Index: " << a << " , maxIndex: " << rows << ".";
            throw OutOfBoundException("Out of bound access (rows): " + ss.str());
        }
    }
};

#endif