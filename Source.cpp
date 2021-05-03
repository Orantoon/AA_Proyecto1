#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

#include <vector>
#include <algorithm>
#include <sstream>

using namespace cv;
using namespace std;

//Struct
#define numCols 3840 // CHANGE IF RESIZE

int Vec3b2int(Vec3b pix) {
	int res = 0;
	for (int i = 0; i < 3; i++) {
		res <<= 8;
		res += pix[i];
	} return res;
}

typedef struct Pixel Pixel;
typedef struct Lista Lista;

struct Pixel {
	int absPos;			//id
	int row, col;		//se pueden sacar de la pos absoluta / %
	int color;			//Vec3b -> int - 8bits 8bits 8bits
	bool deleted;		//para algoritmos luego
	Pixel* nextPixel;	//Siguiente Pixel en la imagen original
	Pixel* next;		//Siguiente Pixel en la Lista

	Pixel(int absPos, Vec3b pix) {
		this->absPos = absPos;
		row = absPos / numCols; col = absPos % numCols;
		color = Vec3b2int(pix);
		deleted = false;
		nextPixel = next = nullptr;
	}

	void addNextPixel(Pixel* next) {
		if (next!=nullptr)
			nextPixel = next;
	}

	void print() {
		cout << "Posicion absoluta: " << absPos << "\tRow: " << row << " & Col: " << col << endl;
		cout << "Color: " << color << endl;

		if (nextPixel != nullptr)
			cout << "Next pixel -> " << nextPixel->absPos << endl << endl;
		else 
			cout << endl << endl;
	}
};

struct List {
	Pixel* first, * last;
	int count = 0;

	List() {
		first = last = nullptr;
	}

	Pixel* insert(int absPos, Vec3b pix) {
		Pixel* pixN = new Pixel(absPos, pix);
		count++;
		if (!first) {
			first = last = pixN;
		}
		else {
			last->next = pixN;
			last = pixN;
		} return pixN;
	}

	Pixel* getPix(int absPos) {
		Pixel* tmp = first;
		while (tmp != nullptr && tmp->absPos != absPos) {
			tmp = tmp->next;
		} return tmp;
	}

	void print() {
		Pixel* tmp = first;
		while (tmp != nullptr) {
			tmp->print();
			tmp = tmp->next;
		}
	}
};

//at<Vec3b>(row,col)

void MyLine(Mat img, Point start, Point end){
	int thickness = 2;
	int lineType = LINE_8;
	line(img, start, end, Scalar(0, 0, 255), thickness, lineType);
}

int main() {
	cout << "Hi" << endl;

	//Images
	Mat img1 = imread("Images/a.jpg");
	Mat img2 = imread("Images/mod.jpg");

	//Structures
	List* l1 = new List();
	List* l2 = new List();

	//Iterators for moving around the Images
	MatIterator_<Vec3b> it1 = img1.begin<Vec3b>(), it1_end = img1.end<Vec3b>();
	MatIterator_<Vec3b> it2 = img2.begin<Vec3b>(), it2_end = img2.end<Vec3b>();

	//for (int i = 0; i < 10; i++, it1++) { cout << img1.at<Vec3b>(0, i) << " "<< *it1 << " " << Vec3b2int(*it1)<< endl;	}


	//Create Struct ------- Saltos de 192x108 (Max)
	int i = -1, absPos = 0;
	int row = 0, rowAnterior = -1;
	while (it1 != it1_end && (absPos <= 8294400)) {

		Pixel* pix1 = l1->insert(absPos, *it1);
		Pixel* pix2 = l2->insert(absPos, *it2);

		i++; //it1, it2 y posAbs depende de si hay salto o no

		if (i != 0) {
			l1->getPix(absPos - 1)->addNextPixel(pix1);
			l2->getPix(absPos - 1)->addNextPixel(pix2);
		}

		//if (j == 10000) break; //43000

		//rowAnterior = row;
		//row = absPos / numCols;
		//if (row != rowAnterior) {
		//	cout << endl << endl << "Row: " << row << endl;
		//} cout << "Col: " << absPos % numCols << " ";

		if (i < 9) {// no salto
			it1++;
			it2++;
			absPos++;


		} else {//salto
			i = -1;
			absPos += 192 * 10;
			it1 += 192 * 10;
			it2 += 192 * 10;


		}
	}

	cout << "Cantidad de pixeles agregados en IMG1: " << l1->count << endl;
	cout << "Cantidad de pixeles agregados en IMG2: " << l2->count << endl;

	//l1->print(); //*/

	//Sort both images so I can use the struct more efficiently (ALGORITHMS)

};

