#include<iostream>
#include<opencv2/opencv.hpp>
#include<cmath>
using namespace std;
using namespace cv;
//���ڽ������ȿ�Ƚ�������
void swap(double *x,double *y)
{
	double temp;
	temp = *x;
	*x = *y;
	*y = temp;
}
//�������
double getDistance(Point A, Point B)
{
	double distance;
	distance = sqrt(pow(A.x - B.x, 2) + pow(A.y - B.y, 2));
	return distance;
}
//���Բ
bool circalute(vector<Point2f>& ptr,Point2f&center,float&radius )
{
	center = cv::Point2d(0, 0);
	radius = 0.0;
	if (ptr.size() < 3) return false;;

	double sumX = 0.0;
	double sumY = 0.0;
	double sumX2 = 0.0;
	double sumY2 = 0.0;
	double sumX3 = 0.0;
	double sumY3 = 0.0;
	double sumXY = 0.0;
	double sumX1Y2 = 0.0;
	double sumX2Y1 = 0.0;
	const double N = (double)ptr.size();
	for (int i = 0; i < ptr.size(); ++i)
	{
		double x = ptr.at(i).x;
		double y = ptr.at(i).y;
		double x2 = x * x;
		double y2 = y * y;
		double x3 = x2 * x;
		double y3 = y2 * y;
		double xy = x * y;
		double x1y2 = x * y2;
		double x2y1 = x2 * y;

		sumX += x;
		sumY += y;
		sumX2 += x2;
		sumY2 += y2;
		sumX3 += x3;
		sumY3 += y3;
		sumXY += xy;
		sumX1Y2 += x1y2;
		sumX2Y1 += x2y1;
	}
	double C = N * sumX2 - sumX * sumX;
	double D = N * sumXY - sumX * sumY;
	double E = N * sumX3 + N * sumX1Y2 - (sumX2 + sumY2) * sumX;
	double G = N * sumY2 - sumY * sumY;
	double H = N * sumX2Y1 + N * sumY3 - (sumX2 + sumY2) * sumY;
	double denominator = C * G - D * D;
	if (abs(denominator) < DBL_EPSILON) return false;
	double a = (H * D - E * G) / (denominator);
	denominator = D * D - G * C;
	if (abs(denominator) < DBL_EPSILON) return false;
	double b = (H * C - E * D) / (denominator);
	double c = -(a * sumX + b * sumY + sumX2 + sumY2) / N;
	center.x = a / (-2);
	center.y = b / (-2);
	radius = std::sqrt(a * a + b * b - 4 * c) / 2;
	return true;
}
//������
int main()
{
	/*VideoCapture cap;
	cap.open("E:\\picture\\red.avi");
	Mat srcimg;
	while (1)
	{
		cap >> srcimg;
		if (srcimg.empty())
		{
			break;
		}
		imshow("video",srcimg);
		if (waitKey(30)>= 0)
		{
			break;
		}
	}*/
	Mat srcimg;
	Mat drawcircle;
	Mat target=imread("E://picture//target.jpg");
	double radius;
	Point2f cc2;
	vector<Point2f>cirV;
	srcimg = imread("E:\\picture\\test2.png");//��ȡ����ͼƬ
	//������ɫ����ֵ��
	vector<Mat>imgChannels;
	split(srcimg,imgChannels);
	Mat midimg = imgChannels.at(2) - imgChannels.at(0);
	threshold(midimg, midimg,100,255,THRESH_BINARY);
	//���Ͳ�����ղ���ȥ������
	int structelement = 2;
	Mat element = getStructuringElement(MORPH_RECT,Size(structelement+1,structelement+1),Point(-1,-1));
	dilate(midimg, midimg, element);
	structelement = 3;
	element = getStructuringElement(MORPH_RECT, Size(2 * structelement + 1, 2 * structelement + 1), Point(-1, -1));
	morphologyEx(midimg,midimg,MORPH_CLOSE,element);
	//����ʶ��
	vector<vector<Point>>contours;
	vector<Vec4i>hierarchy;
	findContours(midimg,contours,hierarchy,RETR_CCOMP,CHAIN_APPROX_SIMPLE);
	RotatedRect rect_temp;
	//��������
	if (hierarchy.size())
	{
		for (int i = 0; i >= 0;i=hierarchy[i][0])
		{
			Point2f cc;//���ڴ��"R"�����ĵ�
			rect_temp = minAreaRect(contours[i]);
			Point2f P[4];
			rect_temp.points(P);
			Point2f srcRect[4];
			Point2f dstRect[4];
			double width;
			double height;
			width = getDistance(P[0],P[1]);
			height = getDistance(P[1], P[2]);
			//�������Σ����ڻ�ȡ͸�ӱ任�ľ���
			if (height > width)
			{
				swap(height, width);
				srcRect[0] = P[1];
				srcRect[1] = P[2];
				srcRect[2] = P[3];
				srcRect[3] = P[0];
			}
			else
			{
				srcRect[0] = P[0];
				srcRect[1] = P[1];
				srcRect[2] = P[2];
				srcRect[3] = P[3];
			}
			for (int j = 0; j < 4; j++)
			{
				line(srcimg, srcRect[j], srcRect[(j+1)%4],Scalar(0,0,255),2);
			}
			double area = width * height;
			cout << area<<" "<<width<<" "<<height<<endl; //���������������ڵ����������Ĳ���
			if (area > 7700)//ͨ�����ɸѡ��Ҷͼ
			{
				//��ȡ͸�ӱ任�������
				dstRect[0] = Point2f(0, 0);
				dstRect[1] = Point2f(width, 0);
				dstRect[2] = Point2f(width, height);
				dstRect[3] = Point2f(0, height);
				//͸�ӱ任
				Mat tran = getPerspectiveTransform(srcRect, dstRect);
				Mat perspect;
				warpPerspective(midimg, perspect, tran, midimg.size());
				imshow("tran", perspect);
				//��ȡ��Ҷ
				Mat ROI;
				ROI = perspect(Rect(0, 0, width, height));
				if (ROI.empty())
				{
					cout << "filed open" << endl;
					return -1;
				}
				else
				{
					imshow("ROI",ROI);
				}
			}
			else
			{
				cc = rect_temp.center;
				cout << cc << endl;
			}
			circle(srcimg, cc, 0.5, Scalar(0, 255, 0), 2);
			imshow("output", srcimg);
			waitKey(0);
		}
	}
	return 0;
}
	