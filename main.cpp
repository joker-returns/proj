#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <time.h>
#include <algorithm>
#include <vector>
using namespace cv;
using namespace std;

void showImage(char * WindowName, Mat Image);
Mat removeColumn(Mat image, vector<int> index);
void showInfo(Mat image);



Mat addColumn(Mat orig, vector<int> index){
	showInfo(orig);
	Mat newImage(orig.rows, orig.cols+1,CV_8UC3);
	for(int i=0;i<orig.rows;i++){
		for(int j=0;j<orig.cols;j++){
			if(j == index[i]){
				newImage.at<Vec3b>(i,j)=orig.at<Vec3b>(i, j);
				newImage.at<Vec3b>(i,j+1) = orig.at<Vec3b>(i, j);				
			}
			else if(j > index[i]){
				newImage.at<Vec3b>(i,j+1) = orig.at<Vec3b>(i, j);
			}
			else{
					newImage.at<Vec3b>(i,j)=orig.at<Vec3b>(i, j);
			}
		}
	}
	return newImage;
}

Mat removeColumn(Mat orig, vector<int> index) {
	showInfo(orig);
	for (int i = 0; i < orig.rows; i++) {
		for (int j = 0; j < orig.cols; j++) {
			if (j == index[i])
				continue;
			else {
				if (j > index[i]) {
					orig.at<Vec3b>(i, j - 1) = orig.at<Vec3b>(i, j);
				}
				else {
					orig.at<Vec3b>(i, j) = orig.at<Vec3b>(i, j);
				}
			}
		}
	}
	//showImage("should have one more row", orig);
	return orig.colRange(0, orig.cols - 1);

}
int min(int a, int b) {
	return (a < b) ? a : b;
}
int min(int a, int b, int c) {
	int curmin = (a > b) ? b : a;
	return (curmin > c)?c : curmin;
}
int minIndex(vector<int> array, int size) {
	int minInd = 0,minvalue = array[0];
	for (int i = 1; i < size; i++) {
		if (array[i] < minvalue) {
			minvalue = array[i];
			minInd = i;
		}
	}
	return minInd;
}
void printvector(vector<int> V) {
	for (int i = 0; i < V.size(); i++) {
		cout << V[i] << " ";
	}
	cout << endl;
}
Mat computeEnergy(Mat grad, Mat image, int mode) {
	vector<vector<int> > energyMatrix;
	vector<vector<int> > Contributors;
	int rows = grad.rows, cols = grad.cols;

	for (int i = 0; i < rows; i++) {
		energyMatrix.push_back(vector<int>());
		for (int j = 0; j < cols; j++) {
			if (i == 0) {
				energyMatrix[i].push_back(grad.at<uchar>(i, j));
				Contributors.push_back(vector<int>());
				Contributors[j].push_back(j);
			}
			else {
				int addedindex;
				if (j == 0) {
					int least = min(energyMatrix[i - 1][j] + (int)grad.at<uchar>(i, j), energyMatrix[i - 1][j + 1] + (int)grad.at<uchar>(i, j));
					energyMatrix[i].push_back(least);
					if (least == energyMatrix[i - 1][j] + (int)grad.at<uchar>(i, j))
						addedindex = 0;
					else
						addedindex = 1;
				}
				else if (j == cols - 1) {
					int least = min(energyMatrix[i - 1][j] + (int)grad.at<uchar>(i, j), energyMatrix[i - 1][j - 1] + (int)grad.at<uchar>(i, j));
					energyMatrix[i].push_back(least);
					if (least == energyMatrix[i - 1][j] + (int)grad.at<uchar>(i, j))
						addedindex = 0;
					else
						addedindex = -1;
					
				}
				else {
					int least = min(energyMatrix[i - 1][j - 1] + (int)grad.at<uchar>(i, j), energyMatrix[i - 1][j] + (int)grad.at<uchar>(i, j), energyMatrix[i - 1][j + 1] + (int)grad.at<uchar>(i, j));
					energyMatrix[i].push_back(least);
					if (least == energyMatrix[i - 1][j - 1] + (int)grad.at<uchar>(i, j))
						addedindex = -1;
					else if (least == energyMatrix[i - 1][j] + (int)grad.at<uchar>(i, j))
						addedindex = 0;
					else
						addedindex = 1;
				}
				switch (addedindex)
				{
				case 1:
					Contributors[j] = Contributors[j + 1];
					Contributors[j].push_back(j);
					break;
				case 0:
					Contributors[j].push_back(j);
					break;
				case -1:
					Contributors[j] = Contributors[j - 1];
					Contributors[j].pop_back();
					Contributors[j].push_back(j);
					break;
				default:
					break;
				}
				
			}
		}
	}
	int min = minIndex(energyMatrix[rows-1], grad.cols);   // computing least seam is done

	cout << "Minium Energy seam has an energy == "<<energyMatrix[rows - 1][min] << " at " << min<< endl;
	if(mode == 0){
		return removeColumn(image,  Contributors[min]);
	}else{
		return addColumn(image, Contributors[min]);
	}
	
}
Mat computeEnergyLegacy(Mat grad, Mat image) {
	const int rows = grad.rows, cols = grad.cols;
	vector<int> energy;
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			if (i == 0)
				energy.push_back(grad.at<uchar>(i, j));
			else
				energy[j] = energy[j]+ grad.at<uchar>(i, j);
		}
		
	}
	
	int min = minIndex(energy, grad.cols);
	vector<int> indexArray;
	for (int i = 0; i < grad.rows; i++) {
		indexArray.push_back(min);
	}
	printf("Minimum energy is %d found at %d\n", energy[min], min);
	return removeColumn(image,indexArray);

}
void showImage(char * WindowName, Mat Image) {
	try {
		printf("Image has %d rows and %d cols with name %s\n", Image.rows, Image.cols, WindowName);
		/*
		if (Image.cols == 396) {
			for (int i = 0; i < Image.rows; i++) {
				for (int j = 0; j < Image.cols; j++) {
					cout << i << " " << j << " " << Image.at<Vec3b>(i, j) << endl;
				}
			}
		}
		*/
		namedWindow(WindowName, WINDOW_AUTOSIZE);
		imshow(WindowName, Image);
		waitKey(0);
	}
	catch (const char* msg) {
		cerr << msg << endl;
	}
}

void showInfo(Mat image) {
	int rows = image.rows, cols = image.cols;
	cout << "Rows : " << rows << " Cols : " << cols << endl;
}


/** @function main */
int main(int argc, char** argv)
{

	Mat src;
	src = imread(argv[1]);

	int scale = 1;
	int delta = 0;
	int ddepth = CV_16S;

	if(argc != 4){
		showInfo(src);
		printf("Usage: ./Seam_Carving <image_name> <required_width> <required_height>");
		return -1;
	}

	/// Load an image
	
	
	int requiredRows = atoi(argv[2]),requiredCols = atoi(argv[3]),currentRows = src.rows,currentCols = src.cols;
	printf("Image resolution is %d * %d ", currentRows, currentCols );
	// cin >> requiredRows >> requiredCols;
	GaussianBlur(src, src, Size(3, 3), 0, 0, BORDER_DEFAULT);
	
	while(requiredCols != src.cols) {
		Mat src_gray;
		Mat grad;
		/// Convert it to gray
		cvtColor(src, src_gray, COLOR_RGB2GRAY);
		/// Generate grad_x and grad_y
		Mat grad_x, grad_y;
		Mat abs_grad_x, abs_grad_y;

		/// Gradient X
		//Scharr( src_gray, grad_x, ddepth, 1, 0, scale, delta, BORDER_DEFAULT );
		Sobel(src_gray, grad_x, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT);
		convertScaleAbs(grad_x, abs_grad_x);

		/// Gradient Y
		//Scharr( src_gray, grad_y, ddepth, 0, 1, scale, delta, BORDER_DEFAULT );
		Sobel(src_gray, grad_y, ddepth, 0, 1, 3, scale, delta, BORDER_DEFAULT);
		convertScaleAbs(grad_y, abs_grad_y);

		/// Total Gradient (approximate)
		addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad);
		if(requiredCols < src.cols){
			src = computeEnergy(grad,src,0);
		}else{
			src = computeEnergy(grad,src,1);
		}
		//computeEnergy(grad);

	}
	//showImage(window_name, grad);
	showImage("main", src);
	imwrite("final.jpg",src);
	return 0;
}
//sudo apt-get -o Dpkg::Options::="--force-overwrite" install cuda
