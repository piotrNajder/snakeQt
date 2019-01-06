#ifndef C_IMAGE_H
#define C_IMAGE_H

#include <string>

enum eFileType {
	pgm = 0,
	ppm = 1,
	jpg = 3,
	jpeg = 4,
	unknown = 255
};

template <typename T = unsigned char>
class cImage
{
private:
	std::string srcFileName = "";
	eFileType srcFileType = eFileType::unknown;
	uint8_t colorChannels = 0;
	bool fromArray = false;

public:
	T **chA = nullptr;     // Alpha channel
	T **chR = nullptr;     // Red channel
	T **chG = nullptr;     // Greyscale or Green in case of RGB
	T **chB = nullptr;     // Blue channel

	uint32_t rows = 0;
	uint32_t columns = 0;
	uint32_t max_colors = 0;

public:

	cImage(const std::string fName);
	cImage(T **gsArr, uint32_t c, uint32_t r);
	cImage(T **rArr, T **gArr, T **bArr, uint32_t columns, uint32_t rows);
	cImage(uint8_t numOfColorChannels, uint32_t c, uint32_t r, uint32_t max_col = 255);
    cImage();
	~cImage();

	bool read();
	bool write(const std::string fName);

    std::string getFileName() const;

private:
	bool isGreyscale();
	bool isRgb();
	bool isRgba();

	int readPGMB_header();
	int readPGMB_data(uint32_t headerLength);
	int writePGMB_image(const std::string fname);

	int readPPMB_header();
	int readPPMB_data(uint32_t headerLength);
	int writePPMB_image(const std::string fname);

	void skipcomments(FILE *fp);
};

#endif
