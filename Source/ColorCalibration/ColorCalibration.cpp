// Copyright Epic Games, Inc. All Rights Reserved.

#include "ColorCalibration.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, ColorCalibration, "ColorCalibration" );


void UColorCalibration::convertFromlxy(FColor_lxy lxy, FLinearColor& retColor)
{
	FColor_XYZ temp;
	convertFromlxyToXYZ(lxy, temp);
}

void UColorCalibration::convertFromlxyToXYZ(FColor_lxy lxy, FColor_XYZ& retColor)
{
	float Y, x, y;
	Y = lxy.l/100.0f;
	x = lxy.x;
	y = lxy.y;
	if (y == 0.0f) {
		retColor.X = 0.0f;
		retColor.Y = 0.0f;
		retColor.Z = 0.0f;
	}
	else {
		retColor.X = x*Y/y;
		retColor.Y = Y;
		retColor.Z = (1-x-y)*Y/y;
	}
}

void UColorCalibration::convertFromLuv(FColor_Luv Luv, FLinearColor& retColor)
{
	float d;
	FColor_lxy lxy;
	lxy.l = Luv.L;
	d = 6 * Luv.u - 16 * Luv.v + 12;
	lxy.x = 9 * Luv.u / d;
	lxy.y = 4 * Luv.v / d;
	convertFromlxy(lxy, retColor);
}

void UColorCalibration::convertFromLuvToXYZ(FColor_Luv Luv, FColor_XYZ& retColor)
{
	float d;
	FColor_lxy lxy;
	lxy.l = Luv.L;
	d = 6 * Luv.u - 16 * Luv.v + 12;
	lxy.x = 9 * Luv.u / d;
	lxy.y = 4 * Luv.v / d;
	convertFromlxyToXYZ(lxy, retColor);
}

void UColorCalibration::convertFromXYZ(FColor_XYZ XYZ, FLinearColor& retColor)
{
	Eigen::Matrix <double, 1, 3> temp;
	//cv::Mat temp(1, 3, CV_32F, cv::Scalar(0));
	temp << XYZ.X, XYZ.Y, XYZ.Z;

	Eigen::MatrixXd dst = temp * XYZ_to_RGB;
	retColor.R = dst(0, 0);
	retColor.G = dst(0, 1);
	retColor.B = dst(0, 2);
}

void UColorCalibration::convertFromRGB(FLinearColor RGB, FColor_XYZ& retColor)
{
	Eigen::Matrix <double, 1, 3> temp;
	//cv::Mat temp(1, 3, CV_32F, cv::Scalar(0));
	temp << RGB.R, RGB.G, RGB.B;

	Eigen::MatrixXd dst = temp * RGB_to_XYZ;
	retColor.X = dst(0, 0);
	retColor.Y = dst(0, 1);
	retColor.Z = dst(0, 2);
}

void UColorCalibration::solve(FColor_primaries_lxy recorded) {
	//first convert from lxy
	FColor_XYZ temps[4];
	convertFromlxyToXYZ(recorded.Red, temps[0]);
	convertFromlxyToXYZ(recorded.Green, temps[1]);
	convertFromlxyToXYZ(recorded.Blue, temps[2]);
	convertFromlxyToXYZ(recorded.White, temps[3]);

	Eigen::Matrix <double, 4, 3> src1;
	src1.setZero();
	Eigen::Matrix <double, 4, 3> src2;
	src2.setZero();

	src1 << temps[0].X, temps[0].Y, temps[0].Z,
		temps[1].X, temps[1].Y, temps[1].Z,
		temps[2].X, temps[2].Y, temps[2].Z,
		temps[3].X, temps[3].Y, temps[3].Z;

	src2 << 1, 0, 0,
		0, 1, 0,
		0, 0, 1,
		1, 1, 1;

	XYZ_to_RGB = src1.template bdcSvd<Eigen::ComputeThinU | Eigen::ComputeThinV>().solve(src2);
	RGB_to_XYZ = XYZ_to_RGB.completeOrthogonalDecomposition().pseudoInverse();
	GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Blue, FString::Printf(TEXT("Post Camera Color Mode On : %f %f %f\n%f %f %f\n%f %f %f\n"), \
																								XYZ_to_RGB(0, 0), XYZ_to_RGB(0, 1), XYZ_to_RGB(0, 2), \
																								XYZ_to_RGB(1, 0), XYZ_to_RGB(1, 1), XYZ_to_RGB(1, 2), \
																								XYZ_to_RGB(2, 0), XYZ_to_RGB(2, 1), XYZ_to_RGB(2, 2)));
	//cv::solve(src1, src2, XYZ_to_RGB, cv::DECOMP_SVD);
	//RGB_to_XYZ = XYZ_to_RGB.inv(cv::DECOMP_SVD);
}

/*void UColorConversion::convertFromXYZ(FColor_XYZ XYZ, FLinearColor& retColor)
{
	cv::Mat temp(1, 3, CV_32F, cv::Scalar(0));
	temp.at<float>(0, 0) = XYZ.X;
	temp.at<float>(0, 1) = XYZ.Y;
	temp.at<float>(0, 2) = XYZ.Z;

	cv::Mat dst;
	cv::multiply(temp, XYZ_to_RGB, dst);
	retColor.R = dst.at<float>(0, 0);
	retColor.G = dst.at<float>(0, 1);
	retColor.B = dst.at<float>(0, 2);
}

void UColorConversion::convertFromRGB(FLinearColor RGB, FColor_XYZ& retColor)
{
	cv::Mat temp(1, 3, CV_32F, cv::Scalar(0));
	temp.at<float>(0, 0) = RGB.R;
	temp.at<float>(0, 1) = RGB.G;
	temp.at<float>(0, 2) = RGB.B;

	cv::Mat dst;
	cv::multiply(temp, RGB_to_XYZ, dst);
	retColor.X = dst.at<float>(0, 0);
	retColor.Y = dst.at<float>(0, 1);
	retColor.Z = dst.at<float>(0, 2);
}

void UColorConversion::solve(FColor_primaries_lxy recorded) {
	//first convert from lxy
	FColor_XYZ temps[4];
	convertFromlxyToXYZ(recorded.Red, temps[0]);
	convertFromlxyToXYZ(recorded.Green, temps[1]);
	convertFromlxyToXYZ(recorded.Blue, temps[2]);
	convertFromlxyToXYZ(recorded.White, temps[3]);

	cv::Mat src1(4, 3, CV_32F, cv::Scalar(0)), src2(4, 3, CV_32F, cv::Scalar(0));
	src1.at<float>(0, 0) = temps[0].X;
	src1.at<float>(0, 1) = temps[0].Y;
	src1.at<float>(0, 2) = temps[0].Z;

	src1.at<float>(1, 0) = temps[1].X;
	src1.at<float>(1, 1) = temps[1].Y;
	src1.at<float>(1, 2) = temps[1].Z;

	src1.at<float>(2, 0) = temps[2].X;
	src1.at<float>(2, 1) = temps[2].Y;
	src1.at<float>(2, 2) = temps[2].Z;

	src1.at<float>(3, 0) = temps[3].X;
	src1.at<float>(3, 1) = temps[3].Y;
	src1.at<float>(3, 2) = temps[3].Z;

	src2.at<float>(0, 0) = 1;
	src2.at<float>(0, 1) = 0;
	src2.at<float>(0, 2) = 0;

	src2.at<float>(1, 0) = 0;
	src2.at<float>(1, 1) = 1;
	src2.at<float>(1, 2) = 0;

	src2.at<float>(2, 0) = 0;
	src2.at<float>(2, 1) = 0;
	src2.at<float>(2, 2) = 1;

	src2.at<float>(3, 0) = 1;
	src2.at<float>(3, 1) = 1;
	src2.at<float>(3, 2) = 1;
	cv::solve(src1, src2, XYZ_to_RGB, cv::DECOMP_SVD);
	RGB_to_XYZ = XYZ_to_RGB.inv(cv::DECOMP_SVD);
}

*/