#include <iostream>
#include <fstream>
#include "../include/cImage.h"

template <typename T>
cImage<T>::cImage(const std::string fName) {
    std::ifstream infile(fName);
    if (infile.good()) {
        srcFileName = fName;
        std::size_t dotPos = fName.find_last_of(".");
        std::string fileExt = fName.substr(dotPos + 1);
        if (fileExt != "") {
            if (fileExt == "pgm") srcFileType = eFileType::pgm;
            else if (fileExt == "ppm") srcFileType = eFileType::ppm;
            // TODO: handle other 
            else {
                throw std::invalid_argument("File extension " + fileExt + " not supported");
            }
            read();
        }
    }
}

template <typename T>
cImage<T>::cImage(T **gsArr, uint32_t c, uint32_t r) {
    rows = r;
    columns = c;
    chG = gsArr;
    max_colors = 255; // TODO: MOVE TO method arguments
	fromArray = true;
}

template <typename T>
cImage<T>::cImage(T **rArr, T **gArr, T **bArr, uint32_t c, uint32_t r) {
    rows = r;
    columns = c;
    chR = rArr;
    chG = gArr;
    chB = bArr;
	fromArray = true;
}

template <typename T>
cImage<T>::cImage(uint8_t numOfColorChannels, uint32_t c, uint32_t r, uint32_t max_col) {

    if (numOfColorChannels == 1) {
        rows = r;
        columns = c;
        chG = new T*[rows];
        chG[0] = new T[rows * columns];
        for (uint32_t i = 1; i < rows; i++)
            chG[i] = chG[i - 1] + columns;

        max_colors = max_col;
    }
    else if (numOfColorChannels == 3) {
        rows = r; columns = c;
        chR = new T*[rows];
        chR[0] = new T[rows * columns];
        chG = new T*[rows];
        chG[0] = new T[rows * columns];
        chB = new T*[rows];
        chB[0] = new T[rows * columns];

        for (uint32_t i = 1; i < rows; i++) {
            chR[i] = chR[i - 1] + columns;
            chG[i] = chG[i - 1] + columns;
            chB[i] = chB[i - 1] + columns;
        }
        max_colors = max_col;
    }
    else {
        throw std::invalid_argument("Only picture with 1 or 3 colors channels can be created.");
    }
}

template <typename T>
cImage<T>::~cImage() {
	if (!fromArray) {
		if (chA != nullptr) {
			if (chA[0] != nullptr) {
				delete[] chA[0];
			}
			delete[] chA;
		}
		if (chR != nullptr) {
			if (chR[0] != nullptr) {
				delete[] chR[0];
			}
			delete[] chR;
		}
		if (chG != nullptr) {
			if (chG[0] != nullptr) {
				delete[] chG[0];
			}
			delete[] chG;
		}
		if (chB != nullptr) {
			if (chB[0] != nullptr) {
				delete[] chB[0];
			}
			delete[] chB;
		}
	}

}

template <typename T>
bool cImage<T>::read() {
    if (srcFileName != "" && srcFileType != eFileType::unknown) {
        int hpos;
        switch (srcFileType) {
        case eFileType::pgm:
            hpos = readPGMB_header();
            if (hpos <= 0) return false;
            chG = new T*[rows];
            chG[0] = new T[rows * columns];
            for (uint32_t i = 1; i < rows; i++) {
                chG[i] = chG[i - 1] + columns;
            }
            if (readPGMB_data(hpos) == 0) return false;
            break;
        case eFileType::ppm:
            hpos = readPPMB_header();
            if (hpos <= 0) return false;

            chR = new T*[rows];
            chR[0] = new T[rows * columns];
            chG = new T*[rows];
            chG[0] = new T[rows * columns];
            chB = new T*[rows];
            chB[0] = new T[rows * columns];

            for (uint32_t i = 1; i < rows; i++) {
                chR[i] = chR[i - 1] + columns;
                chG[i] = chG[i - 1] + columns;
                chB[i] = chB[i - 1] + columns;
            }

            if (readPPMB_data(hpos) == 0) return false;

            break;
        default:
            throw std::invalid_argument("File extension not supported");
            break;
        }
        return true;
    }
    else return false;      // no name specified
}

template <typename T>
bool cImage<T>::write(const std::string fName) {
    srcFileName = fName;
    std::size_t dotPos = fName.find_last_of(".");
    std::string fileExt = fName.substr(dotPos + 1);
    if (fileExt != "") {
        if (fileExt == "pgm") {
            srcFileType = eFileType::pgm;
            if (isGreyscale()) {
                return (bool)writePGMB_image(srcFileName.c_str());
            }
        }
        else if (fileExt == "ppm") {
            srcFileType = eFileType::ppm;
            if (isRgb()) {
                return (bool)writePPMB_image(srcFileName.c_str());
            }
        }
        // TODO: handle other 
        else {
            throw std::invalid_argument("File extension " + fileExt + " not supported");
        }
    }
    return false;
}

template <typename T>
bool cImage<T>::isGreyscale() {
    if (chA == nullptr && chR == nullptr &&
        chB == nullptr && chG != nullptr) {
        return true;
    }
    else return false;
}

template <typename T>
bool cImage<T>::isRgb() {
    if (chA == nullptr && chR != nullptr &&
        chB != nullptr && chG != nullptr) {
        return true;
    }
    else return false;
}

template <typename T>
bool cImage<T>::isRgba() {
    if (chA != nullptr && chR != nullptr &&
        chB != nullptr && chG != nullptr) {
        return true;
    }
    else return false;
}

template <typename T>
void cImage<T>::skipcomments(FILE *fp) {
    int ch;
    char line[256];

    while ((ch = fgetc(fp)) != EOF && isspace(ch))
        ;
    if (ch == '#') {
        fgets(line, sizeof(line), fp);
        skipcomments(fp);
    }
    else
        fseek(fp, -1, SEEK_CUR);
}

template <typename T>
int cImage<T>::readPGMB_header() {
    FILE *fp;
    size_t flen, hlen;
    char signature[3];

    if ((fp = fopen(srcFileName.c_str(), "rb")) == NULL) {
        return 0;
    }

    fseek(fp, 0, SEEK_END);
    flen = ftell(fp);    //file lenght
    fseek(fp, 0, SEEK_SET);

    fgets(signature, sizeof(signature), fp);
    if (signature[0] != 'P' || signature[1] != '5') {
        //probably not pgm binary file...
        fclose(fp);
        return 0;
    }
    skipcomments(fp);
    fscanf(fp, "%d", &columns);
    skipcomments(fp);
    fscanf(fp, "%d", &rows);
    skipcomments(fp);
    fscanf(fp, "%d", &max_colors);
    fgetc(fp);

    hlen = ftell(fp); //header lenght
    fclose(fp);
    if (rows * columns != (flen - hlen))    //we assume only one picture in the file
        return 0;

    return hlen;
}

template <typename T>
int cImage<T>::readPGMB_data(uint32_t headerLength) {
    FILE *fp;
    if ((fp = fopen(srcFileName.c_str(), "rb")) == NULL) return 0;
    fseek(fp, headerLength, SEEK_SET);
    size_t readedrows = fread(chG[0], columns, rows, fp);
    fclose(fp);

    if (rows != readedrows)
        return 0;
    return 1;
}

template <typename T>
int cImage<T>::writePGMB_image(const std::string fname) {
    FILE *fp;

    if ((fp = fopen(fname.c_str(), "wb")) == NULL) {
        return(0);
    }

    fprintf(fp, "P5\n%d %d\n# eyetom.com\n%d\n", columns, rows, max_colors);

    if (rows != fwrite(chG[0], columns, rows, fp)) {
        fclose(fp);
        return(0);
    }

    fclose(fp);
    return(1);
}

template <typename T>
int cImage<T>::readPPMB_header() {

    FILE *fp;
    size_t flen, hlen;
    char signature[3];

    if ((fp = fopen(srcFileName.c_str(), "rb")) == NULL) return 0;

    fseek(fp, 0, SEEK_END);
    flen = ftell(fp);    //file lenght
    fseek(fp, 0, SEEK_SET);

    fgets(signature, sizeof(signature), fp);
    if (signature[0] != 'P' || signature[1] != '6')
    {
        fclose(fp); return 0;
    }    //probably not pgm binary file...

    skipcomments(fp);
    fscanf(fp, "%d", &columns);
    skipcomments(fp);
    fscanf(fp, "%d", &rows);
    skipcomments(fp);
    fscanf(fp, "%d", &max_colors);
    fgetc(fp);

    hlen = ftell(fp); //header lenght
    fclose(fp);
    if (rows * 3 * columns != (flen - hlen))    //we assume only one picture in the file
        return 0;

    return hlen;
}

template <typename T>
int cImage<T>::readPPMB_data(uint32_t headerLength) {
    long i, wxh;
    FILE *fp;

    if ((fp = fopen(srcFileName.c_str(), "rb")) == NULL) return 0;

    fseek(fp, headerLength, SEEK_SET);

    wxh = rows * columns;
    for (i = 0; i < wxh; i++) {
        *(*chR + i) = (unsigned char)fgetc(fp);
        *(*chG + i) = (unsigned char)fgetc(fp);
        *(*chB + i) = (unsigned char)fgetc(fp);
    }
    return 1;
}

template <typename T>
int cImage<T>::writePPMB_image(const std::string fname) {
    long i, wxh;
    FILE *fp;

    if ((fp = fopen(fname.c_str(), "wb")) == NULL) return(0);

    fprintf(fp, "P6\n%d %d\n# eyetom.com\n%d\n", columns, rows, max_colors);

    wxh = rows * columns;
    for (i = 0; i < wxh; i++) {
        /* Write the image in pixel interleaved format. */
        fputc(*(*chR + i), fp);
        fputc(*(*chG + i), fp);
        fputc(*(*chB + i), fp);
    }

    fclose(fp);
    return(1);
}

// Workaround for linker problems with class templates
// need to check for gcc if it's also a problem
// 

template class cImage<unsigned char>;
template class cImage<char>;
template class cImage<uint32_t>;
template class cImage<int>;
template class cImage<unsigned long>;
template class cImage<long>;
template class cImage<double>;
