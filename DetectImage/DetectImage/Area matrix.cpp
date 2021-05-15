#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

#include <vector>

using namespace cv;
using namespace std;

#define areaColSize 24
#define areaRowSize 13

//Class
class Area {
private:
	int id, row, col;
	Vec3b avg;
	bool checked = false;
	vector<Area*> adjacent;

public:
	Area(int n, int r, int c, Vec3b avg) {
		id = n; row = r; col = c; this->avg = avg; 
	}

	void print() {
		cout << "ID: " << id << " Row: " << row << " Col: " << col << endl;
		cout << "AVG: " << avg << endl << "Adjacent areas: ";
		for (auto area : adjacent) {
			cout << area->getID() << "\t";
		} cout << endl << endl;
	}

	int getID() { return id; }
	Vec2i getPos() { return Vec2i{ row, col }; }
	Vec3b getAVG() { return avg; }
	bool isChecked() { return checked; }
	vector<Area*> getAdjacents() { return adjacent; }
	void setAdjacentArea(Area* A) {	adjacent.push_back(A); }
};

//Global Variables
vector<Area*> v1, v2;
Mat img1, img2;

void createAreas() {
	img1 = imread("Images/1.jpg"); img2 = imread("Images/2.jpg"); //3840 x 2160 //[192 x 108] - [384 - 260] //24x13

	int ARows = 160, ACols = img1.cols / areaColSize;

	for (int i = 0; i < ARows * ACols; i++) {
		Vec3b avg1, avg2;
		int row = i / ARows, col = i % ARows;
		int avg1INT[3] = { 0,0,0 }, avg2INT[3] = { 0,0,0 };
		
		//tamanos muy pequenos para minimizar diferencia de color
		int pixAmount = 200;
		Vec3b pix1, pix2;

		//Ranges for grabbing pixels
		int minR = (areaRowSize * row), maxR = (areaRowSize * (row + 1));
		int minC = (areaColSize * col), maxC = (areaColSize * (col + 1));

		//without validating repeated randoms
		for (int p = 0; p < pixAmount; p++) { //min + (rand() % (max - min)) //Random between range - 1
			int r1 = (minR + (rand() % (maxR - minR))), c1 = (minC + (rand() % (maxC - minC)));
			int r2 = (minR + (rand() % (maxR - minR))), c2 = (minC + (rand() % (maxC - minC)));

			//Accessing a pixel
			pix1 = img1.at<Vec3b>(r1, c1); pix2 = img2.at<Vec3b>(r2, c2);

			//Getting avg
			for (int j = 0; j < 3; j++) {
				avg1INT[j] += pix1[j];
				avg2INT[j] += pix2[j];
			}
		}

		for (int j = 0; j < 3; j++) {
			avg1[j] = avg1INT[j] / pixAmount;
			avg2[j] = avg2INT[j] / pixAmount;
		}

		Area* a1 = new Area(i, row, col, avg1), * a2 = new Area(i, row, col, avg2);
		v1.push_back(a1); v2.push_back(a2);
	}

	
	Area* last = v1.back();
	int lastR = last->getPos()[0], lastC = last->getPos()[1]; //firstR = 0, firstC = 0;

	
	//add Adjacent Areas
	for (int i = 0; i < v1.size(); i++) {
		Area* a = v1[i], *b = v2[i];

		if (i > 159) {
			a->setAdjacentArea(v1[i - 160]);
			b->setAdjacentArea(v2[i - 160]);
		}

		if (a->getPos()[0] != lastR) {
			a->setAdjacentArea(v1[i+160]);
			b->setAdjacentArea(v2[i+160]);
		}

		if (a->getPos()[1] != 0) {
			a->setAdjacentArea(v1[i - 1]);
			b->setAdjacentArea(v2[i - 1]);
		}

		if (a->getPos()[1] != lastC) {
			a->setAdjacentArea(v1[i + 1]);
			b->setAdjacentArea(v2[i + 1]);
		}
	}//*/
}



void drawArea(Mat img, Area* tmp) {
	int rowA = tmp->getPos()[0], colA = tmp->getPos()[1];
	int minR = (areaRowSize * rowA), maxR = (areaRowSize * (rowA + 1));
	int minC = (areaColSize * colA), maxC = (areaColSize * (colA + 1));
	Point pt1 = Point(minC, minR), pt2 = Point(maxC, maxR);

	rectangle(img, pt1, pt2, tmp->getAVG(), FILLED);
}


int main() {
	createAreas();

	
	for (int i = 0; i < v1.size(); i++) {
		drawArea(img1, v1[i]);
		drawArea(img2, v2[i]);
		//v1[i]->print();
	}

	resize(img1, img1, Size(), 0.5, 0.5); resize(img2, img2, Size(), 0.5, 0.5);

	imshow("2", img2);
	imshow(" ", img1);
	waitKey();//*/

	//cout << v1.size();

	return 0;
}