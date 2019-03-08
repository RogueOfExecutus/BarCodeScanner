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
	// ��ֵ
	void UseThreshold(const cv::Mat& I, cv::Mat& J, int threshold_value, int threshold_type);
	// ƽ������
	void UseBlur(const cv::Mat & I, cv::Mat & J, int filter, int size);
	// ֱ��ͼ���⻯
	void UseEqualizeHist(const cv::Mat & I, cv::Mat & J);
	// ��̬ѧ�任����
	void UseMorphologyEx(const cv::Mat& I, cv::Mat& J, int method, int kernel_type, int kernel_size);
	// ��ʴ���ʹ���
	void ErosionOrDilation(const cv::Mat& I, cv::Mat& J, int kernel_type, int kernel_size, int method);
	// Ѱ��ֱ��
	bool PretreatmentForFindLine(const cv::Mat& I, configForLine config, cv::Vec4i& l, bool isRight);
	// ɨ��ǰԤ����
	void PretreatmentForScanCode(const cv::Mat& I, cv::Mat& J, configForCode config, int offset);
	// zxing������ά��
	bool ScanBarCodeForZxing(const cv::Mat& I, cv::Mat& J, int codeType, std::string& data, std::vector<cv::Point2f>& codePoint, configForCode config, int offset);
	cv::Point toCvPoint(zxing::Ref<zxing::ResultPoint> resultPoint);
	// Ѱ�Ҷ�ά������
	bool FindCodeCoutours(const cv::Mat& I, cv::Mat& J, configForCode config, cv::RotatedRect& rotatedRect, int offset);
	// Ѱ�Ҷ�ά��ǰԤ����
	void PretreatmentForFindCode(const cv::Mat & I, cv::Mat & J, configForCode config);
	// Ѱ��..��
	cv::Point Center_cal(std::vector<std::vector<cv::Point> > contours, int i);
	// Zbar������ά��
	bool ScanBarCodeForZbar(const cv::Mat& I, cv::Mat& J, std::string& data, std::vector<cv::Point2f>& codePoint, configForCode config, int offset);
	// �汾ʶ��
	void VersionCheck(const cv::Mat& I, configForVersion config, std::string& resChar);
	bool initModel();
	// ��תͼƬ������任����
	void RotateImage(const cv::Mat& I, cv::Mat& J, int angle);
	// �����ַ�
	void findAllWords(configForVersion& vConfig, configForCode cConfig, CString vStr);
	// SURF׼��
	bool PrepareForSURF(std::string path, configForCode config);
private:
	// ģ��ƥ��
	void UseMatchTemplate(const cv::Mat& I, cv::Mat& templ, int method, double& value, cv::Point& p);
	cv::Ptr<cv::ml::KNearest> model;
	cv::Size templateSize;
	cv::Mat templateImage;
	// ��ά������Ѱ�Ҷ�ά��
	void FindCodeForSURF(const cv::Mat& I, std::vector<cv::Point2f>& points);
	std::vector<cv::Point2f> obj_corners;
	cv::Mat descriptors1;
	std::vector<cv::KeyPoint> keyPoint1;
};

