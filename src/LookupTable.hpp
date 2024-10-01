#pragma once

#include <vector>

class LookUpTable {
private:
	int width;
	int height;
	
public:
	// look-up table for each pixel
	std::vector<std::vector<std::vector<int>>> PixelLUT;

public:
	inline LookUpTable(int w, int h) : width(w), height(h) {
		for (int x = 0; x < width; x++) {
			std::vector<std::vector<int>> ltx;
			PixelLUT.push_back(ltx);
			
			for (int y = 0; y < height; y++) {
				std::vector<int> lty;
				PixelLUT[x].push_back(lty);
			}
		}
	}

	~LookUpTable(void) = default;
};
