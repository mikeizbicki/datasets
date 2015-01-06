/**
 * This file is modified from opencv's calcHist_Demo.cpp
 * It constructs red, green, blue, and multicolor histograms for input files.
 */

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/opencv.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>

#include <vector>
#include <algorithm>

using namespace std;
using namespace cv;

void make_histogram_file(const char *filename, const char *color, const Mat &all_hist, int histSize)
{

    ofstream out((string(filename)+".hist."+string(color)).c_str());
    for( int i = 0; i < histSize; i++ )
    {
        if (i>0) out << ", ";
        out << cvRound(all_hist.at<float>(i-1));
    }
    out << endl;
    out.close();
}

void make_sig3d_file(const char *filename, const char *color, const Mat &all_hist, int* histSize)
{

    ofstream out((string(filename)+".sig."+string(color)).c_str());

    // create vector of signature entries
    vector<vector<float> > vec;
    for (int x=0; x<histSize[0]; x++) {
        for (int y=0; y<histSize[1]; y++) {
            for (int z=0; z<histSize[2]; z++) {
                vector<float> tmp(4);
                tmp[0]=all_hist.at<float>(x,y,z);
                tmp[1]=x*255.0/(histSize[0]-1);
                tmp[2]=y*255.0/(histSize[1]-1);
                tmp[3]=z*255.0/(histSize[2]-1);
                vec.push_back(tmp);

                //int val = cvRound(all_hist.at<float>(x,y,z));
                //
                //if (val>0) {
                //out << cvRound(x*255.0/(histSize[0]-1)) << ","
                //<< cvRound(y*255.0/(histSize[0]-1)) << ","
                //<< cvRound(z*255.0/(histSize[0]-1)) << ","
                //<< val
                //<< endl;
                //}
            }
        }
    }

    // sorting lets us display only the siglen largest entries
    sort(vec.begin(),vec.end());
    reverse(vec.begin(),vec.end());

    const int siglen=20;
    const float totalweight=1.0f;

    // normalize so that our entries sum to totalweight
    float tot=0;
    for (int i=0; i<siglen; i++) {
        tot+=vec[i][0];
    }

    for (int i=0; i<siglen; i++) {
        if (vec[i][0] > 0) {
            out << (vec[i][1]) << ","
                << (vec[i][2]) << ","
                << (vec[i][3]) << ","
                << (vec[i][0]*totalweight/tot) << endl;
        }
    }

    out.close();
}

/**
 * @function main
 */
int main( int, char** argv )
{
    Mat src, dst;

    /// Load image
    src = imread( argv[1], 1 );

    if( src.empty() ) {
        if (argv[1]) {
            cerr << "error: file [" << argv[1] << "] not a valid image" << endl;
        }
        else {
            cerr << "usage: " << argv[0] << " image_file" << endl;
        }
        return -1;

    }

    /// Separate the image in 3 places ( B, G and R )
    vector<Mat> bgr_planes;
    split( src, bgr_planes );

    /// Establish the number of bins
    int numbins=8;
    int histSize[] = {numbins,numbins,numbins};

    /// Set the ranges ( for B,G,R) )
    float range[] = { 0, 256 } ;
    const float *histRange[] = { range, range, range };

    bool uniform = true;
    bool accumulate = false;

    Mat b_hist, g_hist, r_hist, all_hist;


    /// 1d histograms
    calcHist( &bgr_planes[0], 1, 0, Mat(), b_hist, 1, histSize+0, histRange, uniform, accumulate );
    calcHist( &bgr_planes[1], 1, 0, Mat(), g_hist, 1, histSize+1, histRange, uniform, accumulate );
    calcHist( &bgr_planes[2], 1, 0, Mat(), r_hist, 1, histSize+2, histRange, uniform, accumulate );

    normalize(b_hist, b_hist, 0, 255, NORM_MINMAX, -1, Mat() );
    normalize(g_hist, g_hist, 0, 255, NORM_MINMAX, -1, Mat() );
    normalize(r_hist, r_hist, 0, 255, NORM_MINMAX, -1, Mat() );

    //make_histogram_file(argv[1],"blue",b_hist,histSize[0]);
    //make_histogram_file(argv[1],"green",g_hist,histSize[1]);
    //make_histogram_file(argv[1],"red",r_hist,histSize[2]);

    // 3d histograms
    int all_channels[]={0,1,2};
    calcHist( &src, 1, all_channels, Mat(), all_hist, 3, histSize, histRange, uniform, accumulate );

    make_sig3d_file(argv[1],"all",all_hist,histSize);

    return 0;

}

