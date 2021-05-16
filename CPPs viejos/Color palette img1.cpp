#include <opencv2\opencv.hpp>
#include <opencv2\photo.hpp>
#include <iostream>
#include <map>

using namespace cv;
using namespace std;



// https://stackoverflow.com/a/34734939/5008845 - Reduces colors by by dividing the image by a number and multiplying it again
void reduceColor_Quantization(const Mat3b& src, Mat3b& dst){
    uchar N = 32;
    dst = src / N;
    dst *= N;
}

// https://stackoverflow.com/a/34734939/5008845
void reduceColor_kmeans(const Mat3b& src, Mat3b& dst)
{
    int K = 8;
    int n = src.rows * src.cols;
    Mat data = src.reshape(1, n);
    data.convertTo(data, CV_32F);

    vector<int> labels;
    Mat1f colors;
    kmeans(data, K, labels, cv::TermCriteria(), 1, cv::KMEANS_PP_CENTERS, colors);

    for (int i = 0; i < n; ++i)
    {
        data.at<float>(i, 0) = colors(labels[i], 0);
        data.at<float>(i, 1) = colors(labels[i], 1);
        data.at<float>(i, 2) = colors(labels[i], 2);
    }

    Mat reduced = data.reshape(3, src.rows);
    reduced.convertTo(dst, CV_8U);
}

// https://stackoverflow.com/questions/35479344/how-to-get-a-color-palette-from-an-image-using-opencv
struct lessVec3b{
    bool operator()(const Vec3b& lhs, const Vec3b& rhs) const {
        return (lhs[0] != rhs[0]) ? (lhs[0] < rhs[0]) : ((lhs[1] != rhs[1]) ? (lhs[1] < rhs[1]) : (lhs[2] < rhs[2]));
    }
};
/*
//Variable Global
map<Vec3b, int, lessVec3b> palette;

map<Vec3b, int, lessVec3b> getPalette(const Mat3b& src){
    for (int r = 0; r < src.rows; ++r) {
        for (int c = 0; c < src.cols; ++c) {
            Vec3b color = src(r, c);
            //Vec3i rgb = src(r, c);

            if (palette.count(color) == 0)
                palette[color] = 1;
            else
                palette[color] = palette[color] + 1;
        }
    } return palette;
}//*/

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
//Vec3i 

int main(){
    Mat3b img1 = imread("Images/1.jpg");
 
    clock_t begin = clock();
    cutImages(img1);
    cout << float(clock() - begin) / CLOCKS_PER_SEC << endl;

    for (auto color : palette) {
        cout << "Color: " << color.first << endl;
    }
    
    return 0;
}