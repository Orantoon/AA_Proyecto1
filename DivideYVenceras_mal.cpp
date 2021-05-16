#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2\opencv.hpp>
#include <opencv2\photo.hpp>
#include <iostream>

#include <map>
#include <vector>
#include <algorithm>

using namespace cv;
using namespace std;


// Struct Creation ==========

//#define numCols 1920 // CHANGE IF RESIZE

// Color Palette Image 1
// https://stackoverflow.com/questions/35479344/how-to-get-a-color-palette-from-an-image-using-opencv
// https://stackoverflow.com/a/34734939/5008845 - Reduces colors by divide your image into K colors, using some clustering algorithm (Kmeans)
void reduceColor_kmeans(const Mat3b& src, Mat3b& dst) {
	int K = 8;
	int n = src.rows * src.cols;
	Mat data = src.reshape(1, n);
	data.convertTo(data, CV_32F);

	vector<int> labels;
	Mat1f colors;
	kmeans(data, K, labels, cv::TermCriteria(), 1, cv::KMEANS_PP_CENTERS, colors);

	for (int i = 0; i < n; ++i) {
		data.at<float>(i, 0) = colors(labels[i], 0);
		data.at<float>(i, 1) = colors(labels[i], 1);
		data.at<float>(i, 2) = colors(labels[i], 2);
	}

	Mat reduced = data.reshape(3, src.rows);
	reduced.convertTo(dst, CV_8U);
}

struct lessVec3i {
	bool operator()(const Vec3i& lhs, const Vec3i& rhs) const {
		return (lhs[0] != rhs[0]) ? (lhs[0] < rhs[0]) : ((lhs[1] != rhs[1]) ? (lhs[1] < rhs[1]) : (lhs[2] < rhs[2]));
	}
};

//Variable Global
map<Vec3i, int, lessVec3i> palette;

map<Vec3i, int, lessVec3i> getPalette(const Mat3b& src) {
	for (int r = 0; r < src.rows; ++r) {
		for (int c = 0; c < src.cols; ++c) {
			Vec3i color = src(r, c);

			//Order in RGB
			int tmp = color[0];
			color[0] = color[2];
			color[2] = tmp;

			if (palette.count(color) == 0)
				palette[color] = 1;
			else
				palette[color] = palette[color] + 1;
		}
	} return palette;
}

void cutImages(Mat3b img) {
	int x = 0, y = 0, tmp1 = 0, tmp2 = 0;
	while (y <= 2160 - 67) {
		x = 0;
		while (x <= 3840 - 127) {
			if (rand() % 6 != 0) {
				x += 120;
				continue;
			}

			Rect crop_region(x, y, 127, 67);
			Mat3b cropped = img(crop_region), src = cropped.clone(), reduced;

			//reduceColor_Quantization(src, reduced);
			reduceColor_kmeans(src, reduced); //Reduces the colors in the image

			//addColors
			getPalette(reduced);

			x += 120;
		} y += 67;
	}
}

// NODO

class Nodo {
public:
	Vec3i color;	// Vector de int, 3 espacios RGB
	int position;	// Posicion en imagen
	int level;	// Nivel dentro del arbol
	map<Vec3i, int, lessVec3i> pixelesRand;	// Pixeles Random dentro de la imagen
	Nodo* father;	// Puntero a nodo padre
	Nodo* son1;	// Puntero a nodo hijo 1
	Nodo* son2;	// Puntero a nodo hijo 2
	Nodo* son3;	// Puntero a nodo hijo 3
	Nodo* son4;	// Puntero a nodo hijo 4
	
	Nodo(Mat _img) {	// Primer nodo
		color = getColorAvg(0, _img);
		position = 0;
		level = 1;
		father = NULL;
		son1 = son2 = son3 = son4 = NULL;
	}
	Nodo(int _position, int _level, Mat _img, Nodo* _father) {	// No es primer nodo
		color = getColorAvg(_position, _img);
		position = _position;
		level = _level;
		father = _father;
		son1 = son2 = son3 = son4 = NULL;
	}
	
	vector<int> getSize(int position) {		// Consigue las cordenadas de un pedazo a partir del numero posicion
		vector<int> res = { 0,0,3839,2159 };	// x, y, xSize, ySize
		int posSwitch = 0;

		while (position > 0) {
			res[2] = res[2] / 2;
			res[3] = res[3] / 2;

			posSwitch = getFirstDigit(position);
			
			switch (posSwitch) {
			case 2:
				res[0] += res[2];
				break;
			case 3:
				res[1] += res[3];
				break;
			case 4:
				res[0] += res[2];
				res[1] += res[3];
			}

			position = rmvFirstDigit(position);
		}
		//cout << res[0] << " " << res[1] << " " << res[2] << " " << res[3] << endl;
		return res;
	}

	Vec3i getColorAvg(int position, Mat3b img) {	// Consigue el promedio de color, usa 100 pixeles aleatorios
		int x, y;	// Coordenadas de la esquina izquierda arriba de la imagen
		int xSize, ySize;	// Tamaño del largo y ancho de la imagen

		Vec3i average;	// Va a guardar el color promedio
		int r = 0, g = 0, b = 0;

		vector<int> cutSize;
		cutSize = getSize(position);	// Actualiza coordenadas y tamaño
		x = cutSize[0];
		y = cutSize[1];
		xSize = cutSize[2];
		ySize = cutSize[3];

		int pixAmount = (xSize * ySize) / 2;	// La cantidad de pixeles aleatorios será la mitad del total
		int counter = pixAmount / 8;
		//without validating repeated randoms
		for (int p = 0; p < pixAmount; p++) { //min + (rand() % (max - min)) //Random between range - 1
			int newX = (x + (rand() % xSize)), newY = (y + (rand() % ySize));
			
			Vec3i pix = img(newY,newX);
			
			b += pix[0];
			g += pix[1];
			r += pix[2];
			
			int tmp = pix[0];
			pix[0] = pix[2];
			pix[2] = tmp;

			
			if (counter >= 0) {
				if (pixelesRand.count(pix) == 0)
					pixelesRand[pix] = 1;
				else
					pixelesRand[pix] = pixelesRand[pix] + 1;
				counter--;
			}
			
		}
		average[0] = r / pixAmount;
		average[1] = g / pixAmount;
		average[2] = b / pixAmount;
		//cout << r / pixAmount << " , " << g / pixAmount << " , " << b / pixAmount << endl;
		return average;
	}
	
	/*
	void print() {
		cout << "Posicion absoluta: " << absPos << "\tRow: " << row << " & Col: " << col << endl;
		cout << "Color: " << color << endl;

		if (nextPixel != nullptr)
			cout << "Next pixel -> " << nextPixel->absPos << endl << endl;
		else
			cout << "/ / / Salto / / /" << endl << endl;
	}
	*/

	int getFirstDigit(int number) {
		while (number >= 10)
			number /= 10;
		return number;
	}

	int rmvFirstDigit(int number) {
		if (number < 10)
			return 0;
		int size = 1;
		int tmp = number;
		while (tmp >= 10) {
			size *= 10;
			tmp /= 10;
		}
		size *= getFirstDigit(number);
		return number - size;
	}
};

// ARBOL
void crearArbol(Mat img, Nodo* padre, int nivel) {	// Recursivo que divide el nodo padre en 4 nuevos nodos
	if (nivel >= 6)
		return;

	int pos = padre->position;
	if (nivel != 0)
		pos *= 10;
	
	Nodo* n0 = new Nodo(pos+1, nivel+1, img, padre);
	Nodo* n1 = new Nodo(pos+2, nivel+1, img, padre);
	Nodo* n2 = new Nodo(pos+3, nivel+1, img, padre);
	Nodo* n3 = new Nodo(pos+4, nivel+1, img, padre);
	padre->son1 = n0;
	padre->son2 = n1;
	padre->son3 = n2;
	padre->son4 = n3;

	crearArbol(img, n0, nivel+1);
	crearArbol(img, n1, nivel+1);
	crearArbol(img, n2, nivel+1);
	crearArbol(img, n3, nivel+1);
}

//Raiz
Nodo* n0;

// Arbol y Lista
void createStruct() {
	cout << "Begin" << endl << endl;

	//Images
	Mat3b img1 = imread("Images/1.jpg");
	Mat img2 = imread("Images/2.jpg");
	//resize(img1, re1, Size(), 0.5, 0.5);
	//resize(img2, re2, Size(), 0.5, 0.5);


	// Imagen 1 (Color Palette)
	cutImages(img1);

	// Imagen 2 (Arbol)
	n0 = new Nodo(img2);
	crearArbol(img2, n0, 0);
	

	//cout << n0->son1->son1->son1->son1->son1->son1->position << endl;
	cout << endl << "End!" << endl << endl << endl;
};


// Divide y Venceras ==========
bool offset(Nodo* nodo, int size) {
	for (auto color : palette) {
		if (color.first[0] >= nodo->color[0] || color.first[0] <= nodo->color[0] && color.first[0] - nodo->color[0] <= size &&
			color.first[1] >= nodo->color[1] || color.first[1] <= nodo->color[1] && color.first[1] - nodo->color[1] <= size &&
			color.first[2] >= nodo->color[2] || color.first[2] <= nodo->color[2] && color.first[2] - nodo->color[2] <= size) {
			return true;
		}
	} return false;
}

int countDV(Nodo* nodo) {
	if (nodo->level != 6) {
		countDV(nodo->son1)+ countDV(nodo->son2) + countDV(nodo->son3) + countDV(nodo->son4);
	}
	if (offset(nodo,50))	// Llega hasta el ultimo nivel del arbol y prueba con todos
		return 1;
	else
		return 0;
}

void divideVenceras() {
	clock_t begin = clock();

	int res = countDV(n0);
	cout << "La cantidad de imagenes encontradas es: " << res << endl << endl;


	cout << "Tiempo Divide y Venceras: " << float(clock() - begin) / CLOCKS_PER_SEC << endl << endl;
}

// Backtracking ==========
bool pixExists(Nodo* nodo) {	// Si el pixel esta en el map del nodo, retorna true
	for (auto busca : nodo->pixelesRand) {
		for (auto color : palette) {
			if (color.first[0] == busca.first[0] && color.first[1] == busca.first[1] && color.first[2] == busca.first[2]) {
				return true;
			}
		}
	} return false;
}

int countBacktracking(Nodo* nodo) {
	if (!pixExists(nodo))	// === PODA ===
		return 0;
	if (nodo->level >= 6)	// Encontro en la ultima capa, cuenta como un trozo de imagen
		return 1;
	return countBacktracking(nodo->son1) + countBacktracking(nodo->son2) + countBacktracking(nodo->son3) + countBacktracking(nodo->son4);
}

void backtracking() {
	clock_t begin = clock();
	
	int res = countBacktracking(n0);	// 400 es el tamaño max de imagenes
	cout << "La cantidad de imagenes encontradas se encuentra entre: " << (int)(res / 40.96) << " - " << (int)(res / 10.24) << endl << endl;

	cout << "Tiempo Backtracking: " << float(clock() - begin) / CLOCKS_PER_SEC << endl << endl;
}


// Probabilista ==========
void probabilista() {
	clock_t begin = clock();

	cout << "Tiempo Probabilista: " << float(clock() - begin) / CLOCKS_PER_SEC << endl << endl;
}


// Menu ==========
void menu() {
	int opcion;
	bool salir = false;

	while (!salir) {
		cout << "=== Seleccione una opcion para recorrer la imagen. ===" << endl;
		cout << "1. Divide y Venceras" << endl;
		cout << "2. Backtracking" << endl;
		cout << "3. Probabilista" << endl;
		cout << "4. Salir" << endl;
		cout << "Opcion: ";
		cin >> opcion;
		cout << endl;
		
		switch (opcion) {
		case 1:
			divideVenceras();
			break;
		case 2:
			backtracking();
			break;
		case 3:
			probabilista();
			break;
		case 4:
			salir = true;
		}
		
	}
}


// Main ==========

int main() {
	clock_t begin = clock();
	createStruct();
	cout << "Tiempo de Estructuras: " << float(clock() - begin) / CLOCKS_PER_SEC << endl << endl;

	menu();
 
	return 0;
}