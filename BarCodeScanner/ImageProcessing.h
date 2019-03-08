#pragma once

#include <opencv2/opencv.hpp> 
#include "zxing/LuminanceSource.h"
#include "zxing/result.h"

struct configForLine {
	int threshold;
	double threshold1;
	double threshold2;
	int canny_size;
	double minLinLength;
	double maxLineGap;
	bool line_direction;
	cv::Rect rect;
	bool same;
	int offset;
	int adaptiveKernel;
	int adaptiveOffset;
	int method;
};

struct configForCode {
	int threshold;
	int blackOrWhite;
	int MorphologyExTimes;
	int MorphologyExSize;
	int ErosionTimes;
	int ErosionSize;
	int BlurSize;
	int cycleTimes;
	int cycleSize;
	bool isSurf;
};

struct configForProject {
	cv::Rect rect;
	int codeType;
	double pixel_scale_x;
	double pixel_scale_y;
	bool isDebug;
	int corner;
	int pencilSize;
	int fontSize;
	int recLen;
	int imageWidth;
	int imageHeight;
	int logLevel;
};

struct codeStandardConfig {
	float highAngle;
	float lowAngle;
	double highCodeWidth;
	double lowCodeWidth;
	double highCodeHeight;
	double lowCodeHeight;
	double highWidth;
	double lowWidth;
	double highHeight;
	double lowHeight;
};

struct configForCount {
	int codeNG;
	int sizeNG;
	int positionNG;
	int angleNG;
	int otherNG;
	int All;
	int VersionNG;
};

struct configForClear {
	int days;
	int hours;
	int minutes;
	LANGID lang;
};

struct configForOffset{
	double codeWidthOffset;
	double codeHeightOffset;
	double widthOffset;
	double heightOffset;
	int codeOffset;
};

struct configForVersion {
	bool enable;
	bool hasTemplate;
	cv::Rect rect;
	int rotateAngle;
	size_t strLen;
	cv::Rect charRects[20];
};

class ImageProcessing
{
public:
	ImageProcessing();
	virtual ~ImageProcessing();
	// 阈值
	void UseThreshold(const cv::Mat& I, cv::Mat& J, int threshold_value, int threshold_type);
	// 平滑处理
	void UseBlur(const cv::Mat & I, cv::Mat & J, int filter, int size);
	// 直方图均衡化
	void UseEqualizeHist(const cv::Mat & I, cv::Mat & J);
	// 形态学变换操作
	void UseMorphologyEx(const cv::Mat& I, cv::Mat& J, int method, int kernel_type, int kernel_size);
	// 腐蚀膨胀处理
	void ErosionOrDilation(const cv::Mat& I, cv::Mat& J, int kernel_type, int kernel_size, int method);
	// 寻找直线
	bool PretreatmentForFindLine(const cv::Mat& I, configForLine config, cv::Vec4i& l, bool isRight);
	// 扫码前预处理
	void PretreatmentForScanCode(const cv::Mat& I, cv::Mat& J, configForCode config, int offset);
	// zxing解析二维码
	bool ScanBarCodeForZxing(const cv::Mat& I, cv::Mat& J, int codeType, std::string& data, std::vector<cv::Point2f>& codePoint, configForCode config, int offset);
	cv::Point toCvPoint(zxing::Ref<zxing::ResultPoint> resultPoint);
	// 寻找二维码轮廓
	bool FindCodeCoutours(const cv::Mat& I, cv::Mat& J, configForCode config, cv::RotatedRect& rotatedRect, int offset);
	// 寻找二维码前预处理
	void PretreatmentForFindCode(const cv::Mat & I, cv::Mat & J, configForCode config);
	// 寻找..点
	cv::Point Center_cal(std::vector<std::vector<cv::Point> > contours, int i);
	// Zbar解析二维码
	bool ScanBarCodeForZbar(const cv::Mat& I, cv::Mat& J, std::string& data, std::vector<cv::Point2f>& codePoint, configForCode config, int offset);
	// 版本识别
	void VersionCheck(const cv::Mat& I, configForVersion config, std::string& resChar);
	bool initModel();
	// 旋转图片（放射变换法）
	void RotateImage(const cv::Mat& I, cv::Mat& J, int angle);
	// 查找字符
	void findAllWords(configForVersion& vConfig, configForCode cConfig, CString vStr);
	// SURF准备
	bool PrepareForSURF(std::string path, configForCode config);
private:
	// 模板匹配
	void UseMatchTemplate(const cv::Mat& I, cv::Mat& templ, int method, double& value, cv::Point& p);
	cv::Ptr<cv::ml::KNearest> model;
	cv::Size templateSize;
	cv::Mat templateImage;
	// 二维特征点寻找二维码
	void FindCodeForSURF(const cv::Mat& I, std::vector<cv::Point2f>& points);
	std::vector<cv::Point2f> obj_corners;
	cv::Mat descriptors1;
	std::vector<cv::KeyPoint> keyPoint1;
};

