#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

#include <vector>
#include <map>
#include <algorithm>

using namespace cv;
using namespace std;

//Areas

#define numCols 1920 // Change if resize

bool sortVec3b(Vec3b a, Vec3b b) { 
	return (a[0] < b[0]) || (a[0] == b[0] && a[1] < b[1]) || (a[0] == b[0] && a[1] == b[1] && a[2] < b[2]);
}

struct Area { //96 x 54 --> 400 Areas //Puede ser hasta la mitad para mas exactitud
	int num, row, col;
	Vec3b min, max, avg; //AVG used for sorting??
	//vector<Vec3b> pixels;
	vector<int> pixels;
	bool checked;
	//Area* nextArea = nullptr; //for moving around?

	Area(int num, int rowA, int colA, Vec3b min, Vec3b max, Vec3b avg, vector<int> pix) { //vector<Vec3b> pixels) {
		this->num = num; row = rowA; col = colA;
		this->checked = false;
		this->min = min; this->max = max; this->avg = avg;

		//Sorts pixels
		//sort(pix.begin(), pix.end(), sortVec3b);
		sort(pix.begin(), pix.end());
		pixels = pix;
	}

	/*
	Area(int num, int rowA, int colA,  vector<Vec3b> pixels) {
		this->num = num; row = rowA; col = colA;
		this->checked = false;

		//this->min = min; this->max = max; this->avg = avg;

		min = max = pixels[0];
		int avg0 = 0, avg1 = 0, avg2 = 0;
		for (auto pix : pixels) {
			for (int i = 0; i < 3; i++) {
				if (pix[i] < min[i])
					min[i] = pix[i];
				if (pix[i] > max[i])
					max[i] = pix[i];
				//avg[i] += pix[i];
			}
			avg0 += pix[0]; avg1 += pix[1]; avg2 += pix[2];

		} avg0 /= pixels.size(); avg1 /= pixels.size(); avg2 /= pixels.size();
		avg[0] = avg0; avg[1] = avg1; avg[2] = avg2;
	}*/

	void printArea() {
		cout << "Numero de area: " << num << "\tRow: " << row << " & Col: " << col << endl;
		cout << "Min: " << min << " Max: " << max << " AVG: " << avg << endl;

		for (auto pix : pixels) {
			cout << pix << endl;
		} cout << endl;
	}
};

typedef struct Area Area;

//Structures
vector<Area*> v1, v2;

//-------------------------------------------------------

void MyFilledCircle(Mat img, Point center){
	circle(img, center, 1, Scalar(0, 0, 255), FILLED, LINE_8);
}


//-------------------------------------------------------
/*

void getAreasINITIAL() {
	Mat img1 = imread("Images/a.jpg"), img2 = imread("Images/mod.jpg");
	resize(img1, img1, Size(), 0.5, 0.5); resize(img2, img2, Size(), 0.5, 0.5);

	for (int numArea = 0; numArea < 400; numArea++) {
		vector<Vec3b> pixels1, pixels2;

		int rowA = numArea / 20, colA = numArea % 20;
		int minR = (54 * rowA), maxR = (54 * (rowA + 1));
		int minC = (96 * colA), maxC = (96 * (colA + 1));

		for (int i = 0; i < 20; i++) { //min + (rand() % (max - min)) //Random between range - 1
			/* //Visualize points in images
			int r1 = (minR + (rand() % (maxR - minR))), c1 = (minC + (rand() % (maxC - minC)));
			int r2 = (minR + (rand() % (maxR - minR))), c2 = (minC + (rand() % (maxC - minC)));

			Point pt1 = Point(c1, r1), pt2 = Point(c2, r2);
			
			pixels1.push_back(img1.at<Vec3b>(r1, c1));
			pixels2.push_back(img2.at<Vec3b>(r2, c2));

			MyFilledCircle(img1, pt1);
			MyFilledCircle(img2, pt2);
			// * /

			//Access pixels by at<Vec3b>
			pixels1.push_back(img1.at<Vec3b>(minR + (rand() % (maxR - minR)), minC + (rand() % (maxC - minC))));
			pixels2.push_back(img2.at<Vec3b>(minR + (rand() % (maxR - minR)), minC + (rand() % (maxC - minC))));
		}
		
		v1.push_back(new Area(numArea, rowA, colA, pixels1));
		v2.push_back(new Area(numArea, rowA, colA, pixels2));
	}

	//Test, check
	for (auto Area : v1) Area->printArea();

	//imshow("Hey", img1);
	//waitKey();
}*/
//-------------------------------------------------------

Mat img1 = imread("Images/a.jpg"), img2 = imread("Images/mod.jpg");

int Vec3b2int(Vec3b pix) {
	int res = 0;
	for (int i = 0; i < 3; i++) {
		res <<= 8;
		res += pix[i];
	} return res;
}

void getAreas() {
	resize(img1, img1, Size(), 0.5, 0.5); resize(img2, img2, Size(), 0.5, 0.5);

	for (int numArea = 0; numArea < 400; numArea++) {
		//Area values
		Vec3b min1 = { 255, 255, 255 }, max1, avg1, min2 = { 255, 255, 255 }, max2, avg2;
		int rowA = numArea / 20, colA = numArea % 20;
		int avg1INT[3] = { 0,0,0 }, avg2INT[3] = { 0, 0, 0 };
		vector<int> pixs1, pixs2;
		
		//Ranges for grabing pixels for each area & pixels
		int minR = (54 * rowA), maxR = (54 * (rowA + 1));
		int minC = (96 * colA), maxC = (96 * (colA + 1));
		Vec3b p1, p2;
		vector<Vec2i> pointsUsed1, pointsUsed2;

		int pixelsGrabbed = 100;

		for (int i = 0; i < pixelsGrabbed; i++) { //min + (rand() % (max - min)) //Random between range - 1

			int r1 = (minR + (rand() % (maxR - minR))), c1 = (minC + (rand() % (maxC - minC)));
			int r2 = (minR + (rand() % (maxR - minR))), c2 = (minC + (rand() % (maxC - minC)));

			//Only add new points
			while (count(pointsUsed1.begin(), pointsUsed1.end(), Vec2i{r1,c1}))
				r1 = (minR + (rand() % (maxR - minR))), c1 = (minC + (rand() % (maxC - minC)));

			while (count(pointsUsed2.begin(), pointsUsed2.end(), Vec2i{ r2,c2 }))
				r2 = (minR + (rand() % (maxR - minR))), c2 = (minC + (rand() % (maxC - minC)));

			pointsUsed1.push_back(Vec2i{ r1,c1 }); pointsUsed2.push_back(Vec2i{ r2,c2 });

			//Access pixels by at<Vec3b>
			p1 = img1.at<Vec3b>(r1, c1); p2 = img2.at<Vec3b>(r2, c2);
			pixs1.push_back(Vec3b2int(p1)); pixs2.push_back(Vec3b2int(p2));

			//Getting min, max and avg
			for (int i = 0; i < 3; i++) {
				if (p1[i] < min1[i])
					min1[i] = p1[i];
				if (p1[i] > max1[i])
					max1[i] = p1[i];
				avg1INT[i] += p1[i];

				if (p2[i] < min2[i])
					min2[i] = p2[i];
				if (p2[i] > max2[i])
					max2[i] = p2[i];
				avg2INT[i] += p2[i];
			}
		}

		for (int i = 0; i < 3; i++) {
			avg1[i] = avg1INT[i] / pixelsGrabbed;
			avg2[i] = avg2INT[i] / pixelsGrabbed;
		}

		v1.push_back(new Area(numArea, rowA, colA, min1, max1, avg1, pixs1));
		v2.push_back(new Area(numArea, rowA, colA, min2, max2, avg2, pixs2));
	}

	//Check 
	//for (auto Area : v1) Area->printArea();
}

bool sortAVG(Area* a, Area* b) {
	return sortVec3b(a->avg, b->avg);
}

bool inRange(Area* a1, Area* a2) { //a2 collapses in a1
	if (a1->min[0] > a2->max[0] && a1->min[1] > a2->max[1] && a1->min[2] > a2->max[2])
		return false;
	if (a2->min[0] > a1->max[0] && a2->min[1] > a1->max[1] && a2->min[2] > a1->max[2])
		return false;
	return true;
}

Vec3b difference(Vec3b a, Vec3b b) {
	return { (unsigned char) abs(a[0] - b[0]), (unsigned char) abs(a[1]-b[1]), (unsigned char) abs(a[2] - b[2])};
}

void drawArea(Mat img, int numArea) {
	int rowA = numArea / 20, colA = numArea % 20;
	int minR = (54 * rowA), maxR = (54 * (rowA + 1));
	int minC = (96 * colA), maxC = (96 * (colA + 1));
	Point pt1 = Point(minC, minR), pt2 = Point(maxC, maxR);

	rectangle(img, pt1, pt2, Scalar(255, 0, 0), FILLED);
}



void detectImages() {
	int magicNum = 7;

	for (int i = 0; i < 399; i++) {
		for (int j = 0; j < 399; j++) {

			if (!inRange(v1[i], v2[j])) //Poda, si ni en rango saltarmelo
				continue;

			//check if contain a similar ammount of pixels
			/*vector<Vec3b> inter;
			//set_intersection(v1[i]->pixels.begin(), v1[i]->pixels.end(), v2[j]->pixels.begin(), v2[j]->pixels.end(), back_inserter(inter));
			//cout << "i: " << i << " j: " << j << "Inter size: " << inter.size() << endl;
			//if (inter.size() < 1) continue; //Not that many*/

			
			Vec3b d = difference(v1[i]->avg, v2[j]->avg);
			if (d[0] > magicNum || d[1] > magicNum || d[2] > magicNum) {continue;}//No coincidence

			//foundid = true;
			cout << "Lead? check: V1: " << i << " V2: " << j << endl;

			//v1[i]->printArea();
			//v2[j]->printArea();

			//drawArea(img2, j);
		}
	}
}

int main() {
	clock_t begin = clock();
	getAreas();
	cout << float(clock() - begin) / CLOCKS_PER_SEC << endl;

	//drawArea(img1, 128);
	//drawArea(img2, 335);

	//Sorting Areas ????????????????????????????????????????????????????????????????????????????????????????????????????????????
	//sort(v1.begin(), v1.end(), sortAVG);
	//sort(v2.begin(), v2.end(), sortAVG);

	begin = clock();
	// Solution
	detectImages();
	cout << float(clock() - begin) / CLOCKS_PER_SEC << endl;

	//drawArea(img1, 382);
	/*drawArea(img2, 229);
	drawArea(img2, 322);
	drawArea(img2, 323);
	drawArea(img2, 325);
	drawArea(img2, 327);
	drawArea(img2, 331);
	drawArea(img2, 332);
	drawArea(img2, 348);
	drawArea(img2, 396);*/

	//IMG1 - 108,109,128,129

	//drawArea(img1, 109);
	imshow("1", img1);
	imshow("2", img2);

	waitKey();

	return 0;
}