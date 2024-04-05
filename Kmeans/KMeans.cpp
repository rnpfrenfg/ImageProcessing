#include "include.h"
#include "KMeans.h"

#include <random>
#include <math.h>

/*
	int meansCount;
	Location* meansLoc;
	int width, height;
	int tried;
	int dotCount;
	int* dotLocation;
	KMeans(int width, int height, int meansCount, int dotCount);

	void SetMeansLocation(int k, int x, int y);
	void CreateRandomDots();
	void SetDotsLocation(int start, int count, int* dots);
	void Calculate(int go = 1);

	*/
int Random()
{
	static std::random_device rd;
	static std::mt19937 gen;
	static std::uniform_int_distribution<int> distribution;
	return distribution(gen);
}

KMeans::KMeans(int width, int height, int meansCount, int dotCount) {
	this->meansCount = meansCount;
	this->dotCount = dotCount;
	this->width = width;
	this->height = height;

	this->meansLoc = new Location[meansCount];
	this->dotLocation = new Location[dotCount];
	this->neareastMean = new int[dotCount];

	memset(dotLocation, 0, sizeof(Location) * dotCount);
	memset(meansLoc, 0, sizeof(Location) * meansCount);
	memset(neareastMean, 0, sizeof(int) * dotCount);

	int tried = 0;

	dotInited = false;
	meansInited = false;
}

KMeans::~KMeans() {
	delete[] meansLoc;
}

void KMeans::SetMeansLocation(int k, int x, int y) {
	if (k<0 || k>meansCount)
		return;
	meansLoc[k].x = x;
	meansLoc[k].y = y;
}

void KMeans::CreateRandomDots() {
	for (int i = 0; i < dotCount; i++) {
		int x = Random() % width;
		int y = Random() % height;
		dotLocation[i].x = x;
		dotLocation[i].y = y;
	}

	dotInited = true;
}


void KMeans::SetDotsLocation(int start, int count, int* dots)
{
	memcpy(&(meansLoc[start]), dots, count);
}

void KMeans::Init() {
	if (dotInited == false) {
		CreateRandomDots();
	}

	if (meansInited == false) {
		for (int i = 0; i < meansCount; i++) {
			auto& mean = meansLoc[i];
			if (mean.x != 0 || mean.y != 0)
				continue;
			auto& dot = dotLocation[Random() % dotCount];
			mean.x = dot.x;
			mean.y = dot.y;
		}
	}
}

void KMeans::FindNeareastMean() {
	for (int dotIndex = 0; dotIndex < dotCount; dotIndex++) {
		auto& dot = dotLocation[dotIndex];
		int nearK = 0;
		int nearDis = dot.Distance2(meansLoc[0]);

		for (int k = 1; k < meansCount; k++) {
			auto& mean = meansLoc[k];
			auto dis = dot.Distance2(mean);
			if (dis < nearDis) {
				nearDis = dis;
				nearK = k;
			}
		}

		neareastMean[dotIndex] = nearK;
	}
}

void KMeans::ReCalculateMean() {
	int* mmx = new int[meansCount];
	int* mmy = new int[meansCount];
	int* clusterCount = new int[meansCount];

	memset(mmx, 0, meansCount);
	memset(mmy, 0, meansCount);
	memset(clusterCount, 0, meansCount);

	for (int i = 0; i < dotCount; i++) {
		auto& dot = dotLocation[i];
		int k = neareastMean[i];
		mmx[k] += dot.x;
		mmy[k] += dot.y;
		clusterCount[k]++;
	}

	for (int k = 0; k < meansCount; k++) {
		auto& mean = meansLoc[k];
		mean.x = mmx[k] / (double)clusterCount[k];
		mean.y = mmy[k] / (double)clusterCount[k];

		printf("[%d] : %d , %d\n", k, mean.x, mean.y);
	}
	delete[] mmx;
	delete[] mmy;
	delete[] clusterCount;
}