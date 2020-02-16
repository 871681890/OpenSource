/***************
����ս�����������Ӿ�ʶ�� - ͼ���˲�������
Author��������
***************/
#include<algorithm>
#include<opencv2/opencv.hpp>
using namespace cv;
using namespace std;

//����������������ѭ�������ƶ�x���������ƶ�y��Ԫ��
void circshift(Mat &out, const Point &delta)
{
	Size sz = out.size();

	// ������
	assert(sz.height > 0 && sz.width > 0);
	// �����������Ҫ�ƶ�
	if ((sz.height == 1 && sz.width == 1) || (delta.x == 0 && delta.y == 0))
		return;

	// ��Ҫ�ƶ������ı任�����������������������
	int x = delta.x;
	int y = delta.y;
	if (x > 0) x = x % sz.width;
	if (y > 0) y = y % sz.height;
	if (x < 0) x = x % sz.width + sz.width;
	if (y < 0) y = y % sz.height + sz.height;


	// ��ά��MatҲ���ƶ�
	vector<Mat> planes;
	split(out, planes);

	for (size_t i = 0; i < planes.size(); i++)
	{
		// ��ֱ�����ƶ�
		Mat tmp0, tmp1, tmp2, tmp3;
		Mat q0(planes[i], Rect(0, 0, sz.width, sz.height - y));
		Mat q1(planes[i], Rect(0, sz.height - y, sz.width, y));
		q0.copyTo(tmp0);
		q1.copyTo(tmp1);
		tmp0.copyTo(planes[i](Rect(0, y, sz.width, sz.height - y)));
		tmp1.copyTo(planes[i](Rect(0, 0, sz.width, y)));

		// ˮƽ�����ƶ�
		Mat q2(planes[i], Rect(0, 0, sz.width - x, sz.height));
		Mat q3(planes[i], Rect(sz.width - x, 0, x, sz.height));
		q2.copyTo(tmp2);
		q3.copyTo(tmp3);
		tmp2.copyTo(planes[i](Rect(x, 0, sz.width - x, sz.height)));
		tmp3.copyTo(planes[i](Rect(0, 0, x, sz.height)));
	}

	merge(planes, out);
}

//������������Ƶ��ͼ�ĵ�Ƶ�����Ƶ��м�
void fftshift(Mat &out)
{
	Size sz = out.size();
	Point pt(0, 0);
	pt.x = (int)floor(sz.width / 2.0);
	pt.y = (int)floor(sz.height / 2.0);
	circshift(out, pt);
}

//����������
void ifftshift(Mat &out)
{
	Size sz = out.size();
	Point pt(0, 0);
	pt.x = (int)ceil(sz.width / 2.0);
	pt.y = (int)ceil(sz.height / 2.0);
	circshift(out, pt);
}

//����������ͼ����ɫ�������ƣ���0-255��������ʾѹ����[0,1]�ĸ�����
void im2double (Mat &src)
{
	src.convertTo(src, CV_64F);
	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.cols; j++)
		{
			src.at<double>(i, j) /= 255.0;
		}
	}
}

//��˹̬ͬ�˲���Ҫ�����������0-255�Ҷ�ͼ����ͨ��ͼ��TongTiLuBo.m
void homomorphicFilter(Mat &src,Mat &dst)
{
	Mat logImage, fftImage;
	Mat padded;
	//Step1.ͼ��ȡ����
	logImage = src;
	im2double(logImage);
	add(logImage, Scalar(1.0), logImage); //��ԭͼ���ÿ�����ص��1������log(0)�ķǷ�����
	log(logImage, logImage); //��������
	//Step2.����Ҷ�任
	Mat planes[] = { logImage, Mat::zeros(logImage.size(), CV_64F) }; //Ϊ������ʵ�����鲿 ����洢�ռ�
	Mat complexI;
	merge(planes, 2, complexI);
	dft(complexI, complexI);
	Mat magI;
	magI = complexI;
	fftshift(magI);
	//Step3.̬ͬ�˲�
	//�����˲�����H(u,v)
	Mat H = Mat::zeros(magI.size(),CV_64F);
	const int n1 = int(magI.rows / 2), n2 = int(magI.cols / 2);
	const double c = 2, rL = 0.5, rH = 4.7;
	int d0 = min(magI.rows, magI.cols);
	for (int i = 0; i < magI.rows; i++)
	{
		for (int j = 0; j < magI.cols; j++)
		{
			int d = (1 + i - n1) * (1 + i - n1) + (1 + j - n2) * (1 + j - n2);

			H.at<double>(i, j) = (rH - rL)*(1 - exp(-c * (d / double(2 * d0 * d0)))) + rL;
			magI.at<double>(i, j) *= H.at<double>(i, j);
		}
	}
	//Step4.����Ҷ��任
	ifftshift(magI);
	idft(magI, magI, DFT_INVERSE+DFT_SCALE+ DFT_COMPLEX_OUTPUT);
	//Step5.ͼ��ָ������
	exp(magI, magI);
	magI -= 1;
	split(magI, planes);
	dst = planes[0];
}