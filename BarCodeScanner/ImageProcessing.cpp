#include "stdafx.h"
#include "ImageProcessing.h"
#include <opencv2/opencv.hpp> 
#include <opencv2/xfeatures2d.hpp>
#include "zxing/LuminanceSource.h"
#include "zxing/Reader.h" 
#include "zxing/common/GlobalHistogramBinarizer.h"  
#include "zxing/DecodeHints.h"  
#include "zxing/datamatrix/DataMatrixReader.h"  
#include "zxing/qrcode/QRCodeReader.h"
#include "MatSource.h"
#include "zxing/result.h"
#include "zxing/BinaryBitmap.h"
#include "zxing/Binarizer.h"
#include "zxing/Exception.h"
#include "zbar.h"

using namespace cv;
using namespace cv::ml;
using namespace cv::xfeatures2d;
using namespace std;
using namespace zxing;
using namespace zbar;

ImageProcessing::ImageProcessing()
{
}


ImageProcessing::~ImageProcessing()
{
}

// 阈值化
void ImageProcessing::UseThreshold(const Mat & I, Mat & J, int threshold_value, int threshold_type)
{
	Mat src_gray;
	if (I.channels() != 1)
		cvtColor(I, src_gray, COLOR_RGB2GRAY);
	else
		src_gray = I.clone();
	threshold(src_gray, J, threshold_value, 255, threshold_type);
}

// 平滑处理
void ImageProcessing::UseBlur(const Mat& I, Mat& J, int filter, int size)
{
	switch (filter)
	{
	case 0:
		blur(I, J, Size(size, size), Point(-1, -1));
		break;
	case 1:
		GaussianBlur(I, J, Size(size, size), 0, 0);
		break;
	case 2:
		medianBlur(I, J, size);
		break;
	case 3:
		bilateralFilter(I, J, size, size * 2, size / 2);
		break;
	case 4:
	{
		Point anchor = Point(-1, -1);
		Mat kernel = Mat::ones(size, size, CV_32F) / (float)(size*size);
		filter2D(I, J, -1, kernel, anchor, 0, BORDER_DEFAULT);
	}
	break;
	default:
		break;
	}
}

// 直方图均衡化
void ImageProcessing::UseEqualizeHist(const Mat& I, Mat& J)
{
	Mat temp;
	if (I.channels() != 1) {
		cvtColor(I, temp, COLOR_BGR2GRAY);
	}
	else
	{
		I.copyTo(temp);
	}
	equalizeHist(temp, J);
}

// 形态学变换操作
void ImageProcessing::UseMorphologyEx(const Mat& I, Mat& J, int method, int kernel_type, int kernel_size)
{
	// 由于 MORPH_X的取值范围是: 2,3,4,5 和 6
	int operation = method + 2;

	Mat element = getStructuringElement(kernel_type, 
		Size(2 * kernel_size + 1, 2 * kernel_size + 1), 
		Point(kernel_size, kernel_size));

	/// 运行指定形态学操作
	morphologyEx(I, J, operation, element);
}

// 膨胀腐蚀操作
void ImageProcessing::ErosionOrDilation(const Mat & I, Mat & J, int kernel_type, int kernel_size, int method)
{
	Mat element = getStructuringElement(kernel_type,
		Size(2 * kernel_size + 1, 2 * kernel_size + 1),
		Point(kernel_size, kernel_size));
	switch (method)
	{
	case 0:
		// 腐蚀操作
		erode(I, J, element);
		break;
	case 1:
		// 膨胀操作
		dilate(I, J, element);
		break;
	default:
		break;
	}
}

// 找直线
bool ImageProcessing::PretreatmentForFindLine(const Mat & I, configForLine config, Vec4i & l, bool isRight)
{
	Mat temp;
	if (I.channels() == 1)
		I.copyTo(temp);
	else
		cvtColor(I, temp, COLOR_BGR2GRAY);
	vector<Vec4i> lines;

	switch (config.method)
	{
	case 0:
		for (int i = 0; i < 20; i++)
		{
			//阈值
			UseThreshold(temp, temp, config.threshold + i * 2, THRESH_BINARY);
			//边缘检测
			Canny(temp, temp, 50, 200);

			HoughLinesP(temp, lines, 1, CV_PI / 180, 100, config.minLinLength, config.maxLineGap);
			if (!lines.empty())
				break;
		}
		break;
	case 1:
		for (int i = 0; i < 10; i++)
		{
			Canny(temp, temp, config.threshold1, config.threshold2, config.canny_size);

			HoughLinesP(temp, lines, 1, CV_PI / 1800, 100, config.minLinLength*(double)(100 - i) / 100, config.maxLineGap);
			if (!lines.empty())
				break;
		}
		break;
	case 2:
		for (int i = 0; i < 5; i++)
		{
			adaptiveThreshold(temp, temp, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, config.adaptiveKernel * 2 + 3, config.adaptiveOffset - i);
			Canny(temp, temp, 50, 200);
			HoughLinesP(temp, lines, 1, CV_PI / 1800, 100, config.minLinLength, config.maxLineGap);
			if (!lines.empty())
				break;
		}
		break;
	default:
		break;
	}
	//正、负方向寻找直线
	if (lines.empty())
		return false;
	if (config.line_direction)
		l = lines[0];
	else
		l = lines.back();
	return true;
}

// 扫码前预处理
void ImageProcessing::PretreatmentForScanCode(const Mat & I, Mat & J, configForCode config, int offset)
{
	//config.threshold += offset * config.cycleSize;
	if (I.channels() == 1)
		I.copyTo(J);
	else
		cvtColor(I, J, COLOR_BGR2GRAY);

	//adaptiveThreshold(J, J, 255, ADAPTIVE_THRESH_MEAN_C, config.blackOrWhite, (30 + offset) * 2 + 3, 1);
	UseThreshold(J, J, config.threshold + offset * config.cycleSize, config.blackOrWhite);
	if (config.MorphologyExTimes)
		for (int i = 0; i < config.MorphologyExTimes; i++)
			UseMorphologyEx(J, J, 0, 0, config.MorphologyExSize);
	if (config.ErosionTimes)
		for (int i = 0; i < config.ErosionTimes; i++)
			ErosionOrDilation(J, J, 0, config.ErosionSize, 0);
	//imshow("J", J);
}

bool ImageProcessing::ScanBarCodeForZxing(const Mat & I, Mat& J, int codeType, string & data, vector<Point2f>& codePoint, configForCode config, int offset)
{
	int cycle;
	if (config.MorphologyExTimes)
		cycle = 2;
	else
		cycle = 1;
	for (int i = 0; i < cycle; i++)
	{
		config.MorphologyExTimes = i;
		PretreatmentForScanCode(I, J, config, offset);
		if (!data.empty())
			data.clear();
		/*if (I.channels() == 1)
		I.copyTo(temp);
		else
		cvtColor(I, temp, CV_BGR2GRAY);*/
		try
		{
			Ref<LuminanceSource> source = MatSource::create(J);
			Ref<Reader> reader;
			DecodeHints hints;
			switch (codeType)
			{
			case 0:
				reader.reset(new qrcode::QRCodeReader);
				hints = DecodeHints(DecodeHints::QR_CODE_HINT);
				break;
			case 1:
				reader.reset(new datamatrix::DataMatrixReader);
				hints = DecodeHints(DecodeHints::DATA_MATRIX_HINT);
				break;
			default:
				break;
			}
			hints.setTryHarder(true);
			Ref<Binarizer> binarizer(new GlobalHistogramBinarizer(source));
			Ref<BinaryBitmap> bitmap(new BinaryBitmap(binarizer));
			//开始解码
			Ref<Result> result(reader->decode(bitmap, hints));
			data = result->getText()->getText();

			// Get result point count
			int resultPointCount = result->getResultPoints()->size();
			codePoint = vector<Point2f>(resultPointCount);

			for (int j = 0; j < resultPointCount; j++)
				codePoint[j] = toCvPoint(result->getResultPoints()[j]);
			return true;
			//if (resultPointCount > 0)// Draw text
			//putText(temp, result->getText()->getText(), toCvPoint(result->getResultPoints()[0]), FONT_HERSHEY_PLAIN, 1, Scalar(110, 220, 0));

		}
		catch (zxing::Exception e)
		{
			data = "error decode";
			//return false;
		}
	}
	
	return false;
}

Point ImageProcessing::toCvPoint(Ref<ResultPoint> resultPoint)
{
	return Point(resultPoint->getX(), resultPoint->getY());
}

// 寻找QR Code轮廓
bool ImageProcessing::FindCodeCoutours(const Mat & I, Mat & J, configForCode config, RotatedRect & rotatedRect, int offset)
{
	int cycle;
	if (config.MorphologyExTimes)
		cycle = 2;
	else
		cycle = 1;
	for (int i = 0; i < cycle; i++)
	{
		Mat temp;
		config.threshold += offset * config.cycleSize;
		config.MorphologyExTimes = i;
		PretreatmentForFindCode(I, temp, config);

		Scalar color = Scalar(255);
		vector<vector<Point> > contours, contours2;
		vector<Vec4i> hierarchy;
		Mat drawing2 = Mat::zeros(I.size(), CV_8UC1);
		findContours(temp, contours, hierarchy, RETR_TREE, CHAIN_APPROX_NONE);

		int ic = 0;
		//程序的核心筛选
		int parentIdx = -1;
		for (size_t i = 0; i < contours.size(); i++)
		{
			if (hierarchy[i][2] != -1 && ic == 0)
			{
				parentIdx = i;
				ic++;
			}
			else if (hierarchy[i][2] != -1)
			{
				ic++;
			}
			else if (hierarchy[i][2] == -1)
			{
				ic = 0;
				parentIdx = -1;
			}

			if (ic >= 2)
			{
				if (contourArea(contours[parentIdx + 1]) < contourArea(contours[parentIdx]) / 2.5)
				{
					ic = 0;
					parentIdx = -1;
					continue;
				}
				contours2.push_back(contours[parentIdx]);
				drawContours(drawing2, contours, parentIdx, color);
				ic = 0;
				parentIdx = -1;
			}
		}
		//imshow("提取前", drawing2);
		if (contours2.size() > 2)
		{
			vector<Point> point(contours2.size());
			for (size_t i = 0; i < contours2.size(); i++)
				point[i] = Center_cal(contours2, i);

			for (size_t i = 0; i < contours2.size(); i++)
				line(drawing2, point[i%contours2.size()], point[(i + 1) % contours2.size()], color);

			//imshow("提取后", drawing2);

			vector<vector<Point> > contours_all;
			vector<Vec4i> hierarchy_all;

			findContours(drawing2, contours_all, hierarchy_all
				, RETR_EXTERNAL, CHAIN_APPROX_NONE, Point(0, 0));//RETR_EXTERNAL表示只寻找最外层轮廓
			//求最小包围矩形
			rotatedRect = minAreaRect(contours_all[0]);
			return true;
		}
	}
	return false;
}

// 寻找二维码前预处理
void ImageProcessing::PretreatmentForFindCode(const Mat& I, Mat& J, configForCode config)
{
	if (I.channels() == 1)
	{
		I.copyTo(J);
	}
	else
	{
		cvtColor(I, J, COLOR_BGR2GRAY);
	}
	//adaptiveThreshold(J, J, 255, ADAPTIVE_THRESH_MEAN_C, config.blackOrWhite ? THRESH_BINARY : THRESH_BINARY_INV, 43, 1);
	UseThreshold(J, J, config.threshold, config.blackOrWhite ? THRESH_BINARY : THRESH_BINARY_INV);
	for (int i = 0; i < config.MorphologyExTimes; i++)
		UseMorphologyEx(J, J, 1, 0, config.MorphologyExSize);
	//if (config.BlurSize)
		//UseBlur(temp, temp, 1, config.BlurSize);
	//UseEqualizeHist(J, J);
	//imshow("temp", J);
}

Point ImageProcessing::Center_cal(vector<vector<Point>> contours, int i)
{
	int centerx = 0, centery = 0, n = contours[i].size();
	//在提取的小正方形的边界上每隔周长个像素提取一个点的坐标，求所提取四个点的平均坐标（即为小正方形的大致中心）
	centerx = (contours[i][n / 4].x + contours[i][n * 2 / 4].x + contours[i][3 * n / 4].x + contours[i][n - 1].x) / 4;
	centery = (contours[i][n / 4].y + contours[i][n * 2 / 4].y + contours[i][3 * n / 4].y + contours[i][n - 1].y) / 4;
	return Point(centerx, centery);
}


bool ImageProcessing::ScanBarCodeForZbar(const Mat& I, Mat& J, string& data, vector<Point2f>& codePoint, configForCode config, int offset)
{
	bool res = false;
	int cycle;
	if (config.MorphologyExTimes)
		cycle = 2;
	else
		cycle = 1;
	for (int i = 0; i < cycle; i++)
	{
		config.MorphologyExTimes = i;
		PretreatmentForScanCode(I, J, config, offset);
		if (!data.empty())
			data.clear();

		ImageScanner scanner;
		scanner.set_config(ZBAR_QRCODE, ZBAR_CFG_EMIT_CHECK, 1);

		int width = J.cols;
		int height = J.rows;
		uchar *raw = (uchar *)J.data;
		Image imageZbar(width, height, "Y800", raw, width * height);//zbar能识别的图像

		scanner.scan(imageZbar); //扫描条码 

		SymbolIterator symbol = imageZbar.symbol_begin();

		if (imageZbar.symbol_begin() == imageZbar.symbol_end())
		{
			//cout << "查询条码失败，请检查图片！" << endl;
			data = "error decode";
		}
		else
		{
			for (; symbol != imageZbar.symbol_end(); ++symbol)
			{
				data = symbol->get_data();
				//data = symbol->xml();
				/*Symbol::PointIterator pIterator = symbol->point_begin();
				for (int i = 0; i < symbol->get_location_size(); ++pIterator)
				{
				Point p;
				p.x = (*pIterator).x;
				p.y = (*pIterator).y;
				codePoint.push_back(p);
				i++;
				}*/
			}
			//imageZbar.set_data(NULL, 0);
			scanner.recycle_image(imageZbar);
			if (config.isSurf)
			{
				codePoint = vector<Point2f>(4);
				FindCodeForSURF(I, codePoint);
			}
			return true;
		}
	}
	return false;
	
}

void ImageProcessing::VersionCheck(const Mat& I, configForVersion config, string& resChar)
{
	Mat temp;
	RotateImage(I, temp, config.rotateAngle);

	if (temp.channels() == 3)
	{
		cvtColor(temp, temp, COLOR_BGR2GRAY);
	}
	if (!templateImage.data)
	{
		return;
	}
	double value;
	Point p;
	UseMatchTemplate(temp, templateImage, 0, value, p);

	Mat res = temp(Rect(p, templateSize));
	//imshow("res", res);
	vector<float> results, neighborResponses, dist;
	for (size_t i = 0; i < config.strLen; i++)
	{
		Mat J;
		resize(res(config.charRects[i]), J, Size(20, 20));
		Mat_<float> nums = J.reshape(0, 1);
		nums.convertTo(nums, CV_32F);
		float thisChar = model->findNearest(nums, 5, results, neighborResponses, dist);
		int resNum = 0;
		float sum = 0;
		for (size_t i = 0; i < neighborResponses.size(); i++)
		{
			if (thisChar == neighborResponses[i])
			{
				resNum++;
				sum += dist[i];
			}
		}
		if (resNum < 5)
		{

		}
		else if(sum / resNum < 10000.0)
		{

		}
		resChar += (char)thisChar;
		//resChar += (char)model->predict(nums);
	}
}


// 旋转图片（仿射变换法）
void ImageProcessing::RotateImage(const Mat& I, Mat& J, int angle)
{
	if (angle == 0)
	{
		I.copyTo(J);
		return;
	}
	else if (angle == 1)
	{
		int c = I.channels();
		J = Mat(I.cols, I.rows, I.type());
		for (int i = 0; i < J.rows; i++)
		{
			for (int j = 0; j < J.cols; j++)
			{
				if (c == 1)
				{
					J.at<uchar>(i, J.cols - j - 1) = I.at<uchar>(j, i);
				}
				if (c == 3)
				{
					J.at<Vec3b>(i, J.cols - j - 1)[0] = I.at<Vec3b>(j, i)[0];
					J.at<Vec3b>(i, J.cols - j - 1)[1] = I.at<Vec3b>(j, i)[1];
					J.at<Vec3b>(i, J.cols - j - 1)[2] = I.at<Vec3b>(j, i)[2];
				}
			}
		}
	}
	else if (angle == 2)
	{
		Point center = Point(I.cols / 2, I.rows / 2);
		Mat rot_mat(2, 3, CV_32FC1);
		rot_mat = getRotationMatrix2D(center, 180, 1);
		warpAffine(I, J, rot_mat, I.size());
	}
	else if (angle == 3)
	{
		int c = I.channels();
		J = Mat(I.cols, I.rows, I.type());
		for (int i = 0; i < J.rows; i++)
		{
			for (int j = 0; j < J.cols; j++)
			{
				if (c == 1)
				{
					J.at<uchar>(J.rows - i - 1, j) = I.at<uchar>(j, i);
				}
				if (c == 3)
				{
					J.at<Vec3b>(J.rows - i - 1, j)[0] = I.at<Vec3b>(j, i)[0];
					J.at<Vec3b>(J.rows - i - 1, j)[1] = I.at<Vec3b>(j, i)[1];
					J.at<Vec3b>(J.rows - i - 1, j)[2] = I.at<Vec3b>(j, i)[2];
				}
			}
		}
	}
}


// 模板匹配
void ImageProcessing::UseMatchTemplate(const Mat& I, Mat& templ, int method, double& value, Point& p)
{
	if (I.channels() != templ.channels())
		return;
	Mat result;
	int result_cols = I.cols - templ.cols + 1;
	int result_rows = I.rows - templ.rows + 1;

	result.create(result_cols, result_rows, CV_32FC1);

	matchTemplate(I, templ, result, method);

	//normalize(result, result, 0, 1, NORM_MINMAX, -1, Mat());

	Point minLoc;
	Point maxLoc;
	double minValue;
	double maxValue;

	minMaxLoc(result, &minValue, &maxValue, &minLoc, &maxLoc);
	if (method == TM_SQDIFF || method == TM_SQDIFF_NORMED)
	{
		p = minLoc;
		value = minValue;
	}
	else
	{
		p = maxLoc;
		value = maxValue;
	}
}


void ImageProcessing::findAllWords(configForVersion& vConfig, configForCode cConfig, CString vStr)
{
	Mat I = imread("matchTemplateImage\\version_template.jpg", IMREAD_ANYDEPTH | IMREAD_ANYCOLOR);
	if (I.data)
	{
		Mat temp;
		int c = I.channels();

		if (c == 1)
			I.copyTo(temp);
		else if (c == 3)
			cvtColor(I, temp, COLOR_BGR2GRAY);

		UseThreshold(temp, temp, cConfig.threshold, cConfig.blackOrWhite ? THRESH_BINARY : THRESH_BINARY_INV);
		vector<vector<Point>> contours;
		vector<Vec4i> hierarchy;
		findContours(temp, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE);

		if (vConfig.strLen != contours.size())
		{
			vConfig.hasTemplate = false;
		}
		else
		{
			vector<Rect> rects;
			for (size_t i = 0; i < contours.size(); i++)
			{
				Rect r = boundingRect(contours[i]);
				int index = i;
				for (size_t j = 0; j < i; j++)
				{
					if (r.br().x < rects[j].br().x)
					{
						index = j;
						break;
					}
				}
				rects.insert(rects.begin() + index, r);
			}
			for (size_t i = 0; i < contours.size(); i++)
			{
				vConfig.charRects[i] = rects[i];
			}
			cvtColor(temp, temp, COLOR_GRAY2BGR);
			for (size_t i = 0; i < vConfig.strLen; i++)
			{
				rectangle(temp, vConfig.charRects[i], Scalar(0, 0, 255), 1, 8, 0);
			}
			imshow("res", temp);
			vConfig.hasTemplate = true;
		}
	}
	else
	{
		vConfig.hasTemplate = false;
	}
}


bool ImageProcessing::initModel()
{
	try
	{
		model = StatModel::load<KNearest>("knnData\\versionKnn.xml");
		Mat temp = imread("matchTemplateImage\\version_template.jpg", IMREAD_ANYDEPTH | IMREAD_ANYCOLOR);
		if (!temp.data)
			return false;
		templateImage = temp(Range::all(), Range(0, temp.cols / 2));
		if (templateImage.channels() != 1)
			cvtColor(templateImage, templateImage, COLOR_BGR2GRAY);
		templateSize = temp.size();
	}
	catch (...)
	{
		return false;
	}
	return true;
}


// 二维特征点寻找二维码
void ImageProcessing::FindCodeForSURF(const Mat& I, vector<Point2f>& points)
{
	Mat img2;
	if (I.channels() == 3)
		cvtColor(I, img2, COLOR_BGR2GRAY);
	else
		I.copyTo(img2);

	//Mat img1 = imread("QRCodeImage\\qrcode.jpg", IMREAD_GRAYSCALE);

	int minHessian = 1000;
	Ptr<SURF> detector = SURF::create(minHessian);
	Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create("FlannBased");
	vector<KeyPoint> keyPoint2;
	Mat descriptors2;
	vector<DMatch> matches;
	// 检测特征点
	//detector->detectAndCompute(img1, Mat(), keyPoint1, descriptors1);
	detector->detectAndCompute(img2, Mat(), keyPoint2, descriptors2);
	// 匹配图像中的描述子
	matcher->match(descriptors1, descriptors2, matches);
	vector<DMatch> good_matches;
	double max_dist = 0; double min_dist = 100;

	//-- Quick calculation of max and min distances between keypoints
	for (int i = 0; i < descriptors1.rows; i++)
	{
		double dist = matches[i].distance;
		if (dist == 0.0)
			continue;
		if (dist < min_dist) min_dist = dist;
		if (dist > max_dist) max_dist = dist;
	}
	for (int i = 0; i < descriptors1.rows; i++)
	{
		if (matches[i].distance < 2 * min_dist)
		{
			good_matches.push_back(matches[i]);
		}
	}
	vector<Point2f> obj;
	vector<Point2f> scene;

	for (size_t i = 0; i < good_matches.size(); i++)
	{
		//-- Get the keypoints from the good matches
		obj.push_back(keyPoint1[good_matches[i].queryIdx].pt);
		scene.push_back(keyPoint2[good_matches[i].trainIdx].pt);
	}

	Mat H = findHomography(obj, scene, FM_RANSAC);
	//-- Get the corners from the image_1 ( the object to be "detected" )
	/*vector<Point2f> obj_corners(4);
	obj_corners[0] = cvPoint(0, 0);
	obj_corners[1] = cvPoint(img1.cols - 1, 0);
	obj_corners[2] = cvPoint(img1.cols - 1, img1.rows - 1);
	obj_corners[3] = cvPoint(0, img1.rows - 1);*/

	perspectiveTransform(obj_corners, points, H);
}


// SURF准备
bool ImageProcessing::PrepareForSURF(string path, configForCode config)
{
	Mat img1 = imread(path, IMREAD_GRAYSCALE);
	if (!img1.data)
		return false;

	int minHessian = 1000;
	Ptr<SURF> detector = SURF::create(minHessian);
	// 检测特征点
	detector->detectAndCompute(img1, Mat(), keyPoint1, descriptors1);

	//Mat code = imread(path, IMREAD_GRAYSCALE);

	if (ScanBarCodeForZxing(img1, Mat(), 0, string(), obj_corners, config, 0))
		return true;
	else
		return false;
}
