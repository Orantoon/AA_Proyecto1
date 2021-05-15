#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

#include <vector>
#include <algorithm>

using namespace cv;
using namespace std;

#define areaColSize 24
#define areaRowSize 13

//Class
class Area {
private:
	int id, row, col;
	int rgb; //RGB
	Vec3b avg;
	bool checked = false;
	//Area* up = nullptr, * down = nullptr, * left = nullptr, * right = nullptr;

public:
	Area* up = nullptr, * down = nullptr, * left = nullptr, * right = nullptr;

	Area(int n, int r, int c, Vec3b avg, int avg_rgb) {
		id = n; row = r; col = c; this->avg = avg; this->rgb = avg_rgb;
	}

	void print() {
		cout << "ID: " << id << " Row: " << row << " Col: " << col << endl;
		cout << "AVG: " << avg << " RGB: " << rgb << endl;
		cout << "Adjacent areas:\n" << endl;
		if (up != nullptr)
			cout << "\tUp: " << up->id;
		if (down != nullptr)
			cout << "\tDown: " << down->id;
		if (left != nullptr)
			cout << "\tLeft: " << left->id;
		if (right != nullptr)
			cout << "\tRight: " << right->id;
		cout << endl << endl;
	}

	int getID() { return id; }
	Vec2i getPos() { return Vec2i{ row, col }; }
	Vec3b getAVG() { return avg; }
	int getRGB() { return rgb; }

	void setChecked() { checked = true; }
	bool isChecked() { return checked; }

	void setUp(Area* A) { up = A; }
	void setDown(Area* A) { down = A; }
	void setLeft(Area* A) { left = A; }
	void setRight(Area* A) { right = A; }
	//Area* getUp() { return up; }
	//Area* getDown() { return down; }
	//Area* getLeft() { return left; }
	//Area* getRight() { return right; }
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

		int rgb1 = avg1[2] * 65536 + avg1[1] * 256 + avg1[0], rgb2 = avg2[2] * 65536 + avg2[1] * 256 + avg2[0];

		Area* a1 = new Area(i, row, col, avg1, rgb1), * a2 = new Area(i, row, col, avg2, rgb2);
		v1.push_back(a1); v2.push_back(a2);
	}

	Area* last = v1.back();
	int lastR = last->getPos()[0], lastC = last->getPos()[1]; //firstR = 0, firstC = 0;
	
	//add Adjacent Areas
	for (int i = 0; i < v1.size(); i++) {
		Area* a = v1[i], *b = v2[i];

		if (i > 159) {
			a->setUp(v1[i - 160]);
			b->setUp(v2[i - 160]);
		}

		if (a->getPos()[0] != lastR) {
			a->setDown(v1[i+160]);
			b->setDown(v2[i+160]);
		}

		if (a->getPos()[1] != 0) {
			a->setLeft(v1[i - 1]);
			b->setLeft(v2[i - 1]);
		}

		if (a->getPos()[1] != lastC) {
			a->setRight(v1[i + 1]);
			b->setRight(v2[i + 1]);
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

void drawNoOrder(Mat img, int i, vector<Area*> v) {
	int row = i / 160, col = i % 160;
	int minR = (areaRowSize * row), maxR = (areaRowSize * (row + 1));
	int minC = (areaColSize * col), maxC = (areaColSize * (col + 1));
	Point pt1 = Point(minC, minR), pt2 = Point(maxC, maxR);

	rectangle(img, pt1, pt2, v[i]->getAVG(), FILLED);
}

bool sortAreas(Area* A, Area* B) {
	return A->getRGB() < B->getRGB();
}

Vec3b difference(Area* A, Area* B) {
	Vec3b a = A->getAVG(), b = B->getAVG();
	return { (unsigned char)abs(a[0] - b[0]), (unsigned char)abs(a[1] - b[1]), (unsigned char)abs(a[2] - b[2]) };
}

void backtracking() {
	vector<Area*> b1 = v1, b2 = v2;

	sort(b1.begin(), b1.end(), sortAreas);
	sort(b2.begin(), b2.end(), sortAreas);

	vector<Area*>::iterator it1 = b1.begin(), it1_end = b1.end(), 
		it2 = b2.begin(), it2_end = b2.end();

	//(*it1)

	int acceptedRange = 20; //10? TEST AND CHECK

	while (it1 != it1_end && it2 != it2_end) {
		//USAR VALOR DE DIFERENCIA
		Vec3b diff = difference(*it1, *it2);

		if (diff[0] > acceptedRange || diff[1] > acceptedRange || diff[2] > acceptedRange) {
			if (sortAreas(*it1, *it2))
				it1++;
			else if (sortAreas(*it2, *it1))
				it2++;

			continue;
		}

		int counter = 0; //120 CHECK
		Area* tmp1 = *it1, * tmp2 = *it2;

		//equal
		do {
			//for each adjacent area -- Check only in v2 (Because i will check the same spaces in img1)
			if (tmp1->up != nullptr && tmp2->up != nullptr) {
				if (!tmp2->up->isChecked()) {
					diff = difference(tmp1->up, tmp2->up);

					if (diff[0] <= acceptedRange || diff[1] <= acceptedRange || diff[2] <= acceptedRange) {
						tmp1 = tmp1->up; tmp2 = tmp2->up; tmp2->setChecked(); counter++;  continue;
					}
				}
			}

			if (tmp1->down != nullptr && tmp2->down != nullptr) {
				if (!tmp2->down->isChecked()) {
					diff = difference(tmp1->down, tmp2->down);

					if (diff[0] <= acceptedRange || diff[1] <= acceptedRange || diff[2] <= acceptedRange) {
						tmp1 = tmp1->down; tmp2 = tmp2->down; tmp2->setChecked(); counter++;  continue;
					}
				}
			}

			if (tmp1->left != nullptr && tmp2->left != nullptr) {
				if (!tmp2->left->isChecked()) {
					diff = difference(tmp1->left, tmp2->left);

					if (diff[0] <= acceptedRange || diff[1] <= acceptedRange || diff[2] <= acceptedRange) {
						tmp1 = tmp1->left; tmp2 = tmp2->left; tmp2->setChecked(); counter++; continue;
					}
				}
			}

			if (tmp1->right != nullptr && tmp2->right != nullptr) {
				if (!tmp2->right->isChecked()) {
					diff = difference(tmp1->right, tmp2->right);

					if (diff[0] <= acceptedRange || diff[1] <= acceptedRange || diff[2] <= acceptedRange) {
						tmp1 = tmp1->right; tmp2 = tmp2->right; tmp2->setChecked(); counter++; continue;
					}

				}
			}

			tmp1 = tmp2 = nullptr;

		} while ((tmp1 != nullptr && tmp2 != nullptr) || counter <= 120);

		if (counter == 120) {
			cout << "LEAD!!!" << endl;
		}
	}


}


int main() {
	createAreas();
	backtracking();


	//v1[0]->print();
	//sort(v1.begin(), v1.end(), sortAreas);
	//v1[0]->print();
	//sort(v2.begin(), v2.end(), sortAreas);
	/*
	for (int i = 0; i < v1.size(); i++) {
		drawArea(img1, v1[i]);
		drawArea(img2, v2[i]);
		//drawNoOrder(img1, i, v1);
		//drawNoOrder(img2, i, v2);
		//v1[i]->print();
	}

	resize(img1, img1, Size(), 0.5, 0.5); resize(img2, img2, Size(), 0.5, 0.5);

	imshow("2", img2);
	imshow(" ", img1);
	waitKey();//*/

	//cout << v1.size();

	return 0;
}