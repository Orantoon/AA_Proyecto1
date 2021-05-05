#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

#include <vector>
#include <algorithm>

using namespace cv;
using namespace std;

//Struct
#define numCols 1920 // CHANGE IF RESIZE

int Vec3b2int(Vec3b pix) {
	int res = 0;
	for (int i = 0; i < 3; i++) {
		res <<= 8;
		res += pix[i];
	} return res;
}

typedef struct Pixel Pixel;

struct Pixel {
	int absPos;			//id
	int row, col;		//se pueden sacar de la pos absoluta / %
	int color;			//Vec3b -> int - 8bits 8bits 8bits
	bool deleted;		//para algoritmos luego
	Pixel* nextPixel;	//Siguiente Pixel en la imagen original

	Pixel(int absPos, Vec3b pix) {
		this->absPos = absPos;
		row = absPos / numCols; col = absPos % numCols;
		color = Vec3b2int(pix);
		deleted = false;
		nextPixel = nullptr;
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
			cout << "/ / / Salto / / /" << endl << endl;
	}
};

//at<Vec3b>(row,col)

void MyLine(Mat img, Point start, Point end){
	int thickness = 1;
	int lineType = LINE_8;
	line(img, start, end, Scalar(0, 0, 255), thickness, lineType);
}

void printVec(vector<Pixel *> v, int n) {
	for (auto pix: v) {
		pix->print();

		if ((n--) <= 1)
			break;
	}
}

bool smallNum(Pixel* a, Pixel* b) {
	return (a->color < b->color);
}

vector<Pixel*> v1, v2;

void createStruct() {
	cout << "Hi" << endl;

	//Images
	Mat img1 = imread("Images/a.jpg"), re1;
	Mat img2 = imread("Images/mod.jpg"), re2;
	resize(img1, re1, Size(), 0.5, 0.5);
	resize(img2, re2, Size(), 0.5, 0.5);

	//Structures
	//vector<Pixel *> v1, v2;

	//Iterators for moving around the Images
	MatIterator_<Vec3b> it1 = re1.begin<Vec3b>(), it1_end = re1.end<Vec3b>();
	MatIterator_<Vec3b> it2 = re2.begin<Vec3b>(), it2_end = re2.end<Vec3b>();

	//for (int i = 0; i < 10; i++, it1++) { cout << img1.at<Vec3b>(0, i) << " "<< *it1 << " " << Vec3b2int(*it1)<< endl;	}

	//Create Struct -------
	int i = -1, absPos = 0, row, col, jump;
	unsigned int j = 0;

	Point start, end;
	Pixel* pix1, * pix2;

	while (it1 != it1_end && (absPos <= 2073600)) {

		i++;

		pix1 = new Pixel(absPos, *it1);
		pix2 = new Pixel(absPos, *it2);

		v1.push_back(pix1);
		v2.push_back(pix2);


		if (v1.size() > 2) {
			if (v1[j - 1]->row == pix1->row && v1[j - 1]->col + 1 == pix1->col)
				v1[j - 1]->addNextPixel(pix1);
			//else
				//cout << v1[j - 1]->absPos << " & " << absPos << endl;

			if (v2[j - 1]->row == pix2->row && v2[j - 1]->col + 1 == pix2->col)
				v2[j - 1]->addNextPixel(pix2);
		} 

		//ERROR, SI LA FILA ES IMPAR NO VA A AÑADIR LOS PUNTEROS QUE COINCIDEN DENTRO DE NEXT PIXEL
		
		//if (i != 0) { //add next pixel to each pixel
			//v1[j - 1]->addNextPixel(pix1);
			//v2[j - 1]->addNextPixel(pix2);
		//}

		row = absPos / numCols;
		col = absPos % numCols;

		if (row % 2 != 0) { // saltarme rows impares
			i = -1; 
			//jump = numCols;   //						(numCols[*5])
			jump = 11;			//7	(740341)			//11 (686520) //
			absPos += jump;  
			it1 += jump; it2 += jump;
		}
		else {
			if (i == 0) {		// draw //
				start.y = row;	// draw //
				start.x = col;	// draw //
			}					// draw //

			if (i < 7) { //5		// no salto			(5)		(7)
				it1++;
				it2++;
				absPos++;
			} else {				//salto
				end.y = row;				// draw //
				end.x = col;				// draw //
				MyLine(re1, start, end);	// draw //
				//MyLine(re2, start, end);	// draw //

				jump = 7; //10							(10)	(7)

				i = -1;
				absPos += jump;
				it1 += jump;
				it2 += jump;
			}
		}

		j++;
	}

	cout << "Cantidad de elementos en cada vector: " << v1.size() << endl << endl;

	//cv::imshow(" ", re1);
	//cv::waitKey(0);

	//printVec(v1, 10000);

	cout << "End!";
};

void backtracking() {
	//Sort both images so I can use the struct more efficiently (ALGORITHMS)
	sort(v1.begin(), v1.end(), smallNum); sort(v2.begin(), v2.end(), smallNum);

	//Recorrer ambos vectores

}





int main() {
	createStruct();
	backtracking();
	return 0;
}