#pragma once

#include <math.h>

class Location {
public:
	int x, y;

	int Distance2(Location& loc) {
		return sqrt((loc.x - x) * (loc.x - x) + (loc.y - y) * (loc.y - y));
	}
};

class KMeans
{
public:
	KMeans(int width, int height, int meansCount, int dotCount);
	~KMeans();

	void SetMeansLocation(int k, int x, int y);
	void CreateRandomDots();
	void SetDotsLocation(int start, int count, int* dots);
	void FindNeareastMean();
	void ReCalculateMean();
	void Init();

	int meansCount;
	Location* meansLoc;
	int width, height;
	int tried;
	int dotCount;
	Location* dotLocation;
	int* neareastMean;
	int* clusterCount;

	bool dotInited;
	bool meansInited;
};

