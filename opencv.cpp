#include "opencv2/opencv.hpp"
#include <iostream>

using namespace cv;
using namespace std;

int main()
{
    Mat img;
    Mat img_hsv;
    Mat frame;

    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;

    double count = 0;

    VideoCapture cap = VideoCapture("http://220.69.240.120:8091/?action=stream");
    if (!cap.isOpened()) {

        cout << "카메라를 열 수 없습니다." << endl;
        return -1;
    }

    int w = cvRound(cap.get(CAP_PROP_FRAME_WIDTH));
    int h = cvRound(cap.get(CAP_PROP_FRAME_HEIGHT));
    double fps = cap.get(CAP_PROP_FPS);

    int fourcc = VideoWriter::fourcc('D', 'I', 'V', 'X');
    int delay = cvRound(1000 / fps);

    VideoWriter outputVideo("output.avi", fourcc, fps, Size(w, h));

    if (!outputVideo.isOpened()) {
        cout << "File open failed!" << endl;
        return -1;
    }

    while (1)
    {
        cap >> img;

        cvtColor(img, img_hsv, COLOR_BGR2HSV);

        Mat yellow_mask, yellow_image, img_erode, img_dilate;

        Scalar lower_yellow = Scalar(0, 20, 50);
        Scalar upper_yellow = Scalar(25, 230, 230);

        inRange(img_hsv, lower_yellow, upper_yellow, yellow_mask);

        erode(yellow_mask, img_erode, Mat::ones(Size(2, 2), CV_8UC1), Point(-1, -1), 1);
        dilate(img_erode, img_dilate, Mat::ones(Size(3, 3), CV_8UC1), Point(-1, -1), 3);

        count = 0;
        for (int j = 0; j < img_dilate.rows; j++)          //반복문으로 i,j값을 제어하면서 화소 위치 이동
        {
            for (int i = 0; i < img_dilate.cols; i++)
            {
                if (img_dilate.at<uchar>(j, i)) count++;    //검출된 비트 확인
            }
        }
        count = count / 307200 * 100;

        bitwise_and(img, img, yellow_image, img_dilate);
        frame = yellow_image;

        findContours(img_dilate, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE); //그레이스케일에서만 가능, 점 찍은 벡터
        drawContours(frame, contours, -1, Scalar(0, 0, 255), 2);

        String desc = format("rate = %.2lf%%", count);
        putText(frame, desc, Point(10, 30), FONT_HERSHEY_SIMPLEX, 1, Scalar(0,255,255), 1, LINE_AA);       //텍스트 삽입

        outputVideo.write(frame);

        imshow("video0", img);
        imshow("video1", frame);
        if (waitKey(1) == 27) break;
    }

    return 0;
}