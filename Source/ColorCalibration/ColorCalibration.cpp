// Copyright Epic Games, Inc. All Rights Reserved.

#include "ColorCalibration.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, ColorCalibration, "ColorCalibration" );


void UColorCalibration::convertFromlxytoRGB(FColor_lxy lxy, FLinearColor& retColor)
{
	FColor_XYZ temp;
	convertFromlxyToXYZ(lxy, temp);
}

void UColorCalibration::convertFromlxyToXYZ(FColor_lxy lxy, FColor_XYZ& retColor)
{
	float Y, x, y;
	Y = lxy.l/max_lum;
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

void UColorCalibration::convertFromLuvtolxy(FColor_Luv Luv, FColor_lxy& retColor)
{
	float d;
	FColor_lxy lxy;
	lxy.l = Luv.L;
	d = 6 * Luv.u - 16 * Luv.v + 12;
	lxy.x = 9 * Luv.u / d;
	lxy.y = 4 * Luv.v / d;
	retColor = lxy;
}

void UColorCalibration::convertFromLuvtoRGB(FColor_Luv Luv, FLinearColor& retColor)
{
	FColor_lxy lxy;
	convertFromLuvtolxy(Luv, lxy);
	convertFromlxytoRGB(lxy, retColor);
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

void UColorCalibration::convertFromXYZtolxy(FColor_XYZ XYZ, FColor_lxy& retColor)
{
	float d = XYZ.X + XYZ.Y + XYZ.Z;
	if (d == 0) {
		retColor.l = 0.0f;
		retColor.x = 0.0f;
		retColor.y = 0.0f;
	}
	else {
		retColor.l = XYZ.Y * max_lum;
		retColor.x = XYZ.X / d;
		retColor.y = XYZ.Y / d;
	}
}

void UColorCalibration::convertFromXYZtoRGB(FColor_XYZ XYZ, FLinearColor& retColor)
{
	Eigen::Matrix <double, 1, 3> temp;
	//cv::Mat temp(1, 3, CV_32F, cv::Scalar(0));
	temp << XYZ.X, XYZ.Y, XYZ.Z;

	Eigen::MatrixXd dst = temp * XYZ_to_RGB;
	retColor.R = dst(0, 0);
	retColor.G = dst(0, 1);
	retColor.B = dst(0, 2);
}

void UColorCalibration::convertFromRGBtoXYZ(FLinearColor RGB, FColor_XYZ& retColor)
{
	Eigen::Matrix <double, 1, 3> temp;
	//cv::Mat temp(1, 3, CV_32F, cv::Scalar(0));
	temp << RGB.R, RGB.G, RGB.B;

	Eigen::MatrixXd dst = temp * RGB_to_XYZ;
	retColor.X = dst(0, 0);
	retColor.Y = dst(0, 1);
	retColor.Z = dst(0, 2);
}

void UColorCalibration::readPrimariesFromCSV(FString csv_filename, TArray<FColor_lxy>& lxys)
{
	TArray<FString> TextArray;
	FString file_path = UKismetSystemLibrary::GetProjectSavedDirectory()+"/Inputs/"+csv_filename;
	LoadTextFromFile(file_path, TextArray);
	for (int i = 1; i < TextArray.Num(); i++)
	{
		FString left = "";
		FColor_lxy color_now;
		int flag = 0;
		float color_lxy[3] = { 0.0f, 0.0f, 0.0f };
		FString br = TextArray[i];
		int j = 0;
		
		while (j < br.Len()) {
			if (br[j] == ',') {
				if (flag > 0) {
					color_lxy[flag - 1] = FCString::Atof(*left);
					//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Float : %f"), color_lxy[flag-1]));
				}
				left = "";
				flag++;
			}
			else {
				left += br[j];
			}
			j++;
		}
		color_lxy[flag - 1] = FCString::Atof(*left);
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Float : %f"), color_lxy[flag - 1]));

		color_now.l = color_lxy[0];
		color_now.x = color_lxy[1];
		color_now.y = color_lxy[2];
		lxys.Add(color_now);
	}
	max_lum = lxys[3].l;
	FColor_primaries_lxy primaries(lxys[0], lxys[1], lxys[2], lxys[3]);
	solve(primaries);
}

void UColorCalibration::readPlatePointsFromCSV(FString csv_filename, int start_threshold, TArray<FTransform>& all_plates_transform)
{
	TArray<FString> TextArray;
	
	threshold.Empty();
	correct_threshold.Empty();
	incorrect_threshold.Empty();
	temp_thresh.Empty();
	start_thresh.Empty();

	for (int i = 0; i < CONFUSION_ALONG; i++) {
		threshold.Add(start_threshold);
		correct_threshold.Add(start_threshold);
		incorrect_threshold.Add(0);
		temp_thresh.Add(1);
		start_thresh.Add(start_threshold);
	}
	
	FString file_path = UKismetSystemLibrary::GetProjectSavedDirectory() + "/Inputs/" + csv_filename;
	LoadTextFromFile(file_path, TextArray);
	for (int i = 1; i < TextArray.Num(); i++)
	{
		FString left = "";

		int flag = 0;
		float trsform[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		FString br = TextArray[i];
		int j = 0;


		while (j < br.Len()) {
			if (br[j] == ',') {
				if (flag > 0) {
					trsform[flag - 1] = FCString::Atof(*left);
					//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString::Printf(TEXT("Float : %f"), trsform[flag-1]));
				}
				left = "";
				flag++;
			}
			else {
				left += br[j];
			}
			j++;
		}
		trsform[flag - 1] = FCString::Atof(*left);
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString::Printf(TEXT("Float : %f"), trsform[flag - 1]));

		FTransform trsform_now;
		trsform_now.SetLocation(FVector(trsform[0], trsform[1], trsform[2]));
		trsform[3] = FMath::Sqrt(trsform[3]) / 100.0;
		trsform_now.SetScale3D(FVector(0.0, trsform[3], trsform[3]));

		all_plates_transform.Add(trsform_now);
	}
}

bool UColorCalibration::LoadTextFromFile(FString FileName, TArray<FString>& TextArray)
{

	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*FileName))
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Could not Find File"));
		return false;
	}
	else
	{
		// Convert filepath to character array and save to array
		const TCHAR* FILEPATH = *FileName;
		return FFileHelper::LoadFileToStringArray(TextArray, *FileName);
		//return FFileHelper::LoadFileToString(SaveString, *FileName);
	}
}

bool UColorCalibration::SaveArrayText(FString SaveDirectory, FString FileName, TArray<FString> SaveText, bool AllowOverwriting = false)
{
	// Set complete file path
	SaveDirectory += "\\";
	SaveDirectory += FileName;

	if (!AllowOverwriting)
	{
		if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*SaveDirectory))
		{
			return false;
		}
	}

	FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*SaveDirectory);
	FString FinalString = "";
	for (FString& Each : SaveText)
	{
		FinalString += Each;
		FinalString += LINE_TERMINATOR;
	}

	return FFileHelper::SaveStringToFile(FinalString, *SaveDirectory);

}

void UColorCalibration::LoadDirectionPlates(int direction, TArray<int>& direction_nums)
{
	int up0[] = { 232, 234, 258, 260, 261, 262, 264, 267, 272, 282, 295, 297, 299, 305, 306, 307, 308, 309, 310, 312, 313, 314, 315, 316, 317, 318, 330, 334, 350, 335, 352, 336, 337, 338, 339, 340, 341, 342, 343, 344, 345, 346, 347, 348, 349, 350, 351, 353, 354, 355, 356, 357, 358, 359, 360, 361, 362, 363, 364, 365, 366, 367, 368, 369, 371, 372, 375, 376, 377, 378, 379, 380, 381, 382, 383, 384, 385, 386, 387, 388, 389, 390, 391, 392, 393, 394, 395, 396, 397, 398, 399, 400, 401, 402, 403, 404, 405, 406, 411, 412, 431, 432, 433, 434, 435, 436, 437, 438, 439, 440, 441, 442, 443, 444, 458, 459, 460, 463, 464, 465, 466, 467, 468, 469, 470, 471, 472, 473, 475, 477, 478, 479, 480, 481, 482, 483, 486, 488, 516, 519, 520 };
	int up1[] = {135, 136, 164, 177, 178, 179, 180, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 198, 207, 214, 215, 216, 224, 225, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 244, 245, 246, 248, 249, 250, 251, 252, 253, 254, 255, 256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 271, 272, 273, 274, 275, 276, 277, 279, 280, 281, 282, 283, 284, 285, 286, 287, 288, 289, 290, 291, 292, 293, 294, 295, 296, 297, 298, 299, 300, 301, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315, 316, 317, 318, 319, 329, 330, 331, 332, 333, 348, 334, 350, 335, 352, 336, 337, 338, 339, 340, 341, 342, 343, 344, 345, 346, 347, 348, 349, 350, 351, 352, 353, 354, 355, 356, 357, 358, 359, 360, 361, 362, 363, 364, 365, 366, 367, 368, 369, 370, 371, 372, 373, 374, 375, 376, 377, 378, 379, 380, 381, 382, 383, 384, 385, 386, 387, 388, 389, 390, 391, 392, 393, 394, 395, 396, 397, 398, 399, 400, 401, 402, 403, 404, 405, 406, 411, 412, 413, 433, 434, 435, 436, 437, 438, 439, 440, 441, 442, 443, 444, 458, 459, 460, 463, 465, 468, 469, 470, 472, 479, 480, 481, 482, 483, 516, 519, 520};
	int left0[] = {214, 215, 232, 234, 236, 243, 260, 261, 262, 264, 275, 282, 283, 290, 291, 293, 294, 295, 296, 297, 298, 299, 300, 301, 303, 304, 305, 306, 307, 308, 309, 310, 312, 313, 314, 315, 316, 317, 318, 319, 320, 321, 324, 325, 327, 328, 329, 330, 346, 347, 348, 349, 350, 353, 354, 355, 356, 357, 358, 359, 360, 361, 362, 363, 364, 365, 366, 367, 368, 369, 370, 371, 372, 373, 374, 375, 376, 377, 378, 381, 382, 393, 395, 396, 397, 398, 399, 400, 404, 405, 406, 411, 412, 413, 414, 415, 416, 417, 418, 419, 420, 421, 429, 431, 432, 433, 434, 435, 437, 438, 439, 440, 458, 459, 460, 463, 465, 469, 470, 471, 472, 479, 480, 481, 559, 560, 579, 581, 583};
	int left1[] = {124, 125, 126, 145, 146, 147, 164, 165, 167, 168, 170, 172, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 194, 195, 198, 202, 203, 204, 205, 206, 207, 208, 209, 210, 214, 215, 216, 224, 225, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 248, 249, 250, 251, 252, 253, 254, 261, 262, 263, 264, 265, 266, 267, 268, 269, 271, 272, 273, 274, 275, 276, 277, 278, 279, 280, 281, 282, 283, 284, 285, 286, 287, 288, 289, 290, 291, 292, 293, 294, 295, 296, 297, 298, 299, 300, 301, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315, 316, 317, 318, 319, 320, 321, 322, 323, 324, 325, 326, 327, 328, 329, 330, 331, 332, 333, 348, 334, 350, 335, 352, 336, 337, 338, 347, 348, 349, 350, 351, 352, 353, 354, 355, 356, 357, 358, 359, 360, 361, 362, 363, 364, 365, 366, 367, 368, 369, 370, 371, 372, 373, 374, 375, 376, 377, 378, 379, 380, 381, 382, 383, 384, 385, 386, 387, 388, 389, 390, 391, 392, 393, 394, 395, 396, 397, 398, 399, 400, 401, 402, 403, 404, 405, 406, 411, 412, 413, 414, 415, 416, 417, 418, 419, 420, 421, 429, 432, 433, 434, 435, 436, 437, 438, 439, 440, 441, 458, 459, 460, 463, 465, 469, 470, 472, 479, 480, 481, 483, 516, 519, 520, 552, 590, 559, 560};
	int down0[] = {125, 135, 136, 164, 165, 167, 168, 170, 172, 174, 175, 176, 177, 178, 179, 180, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 198, 214, 215, 216, 224, 225, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 248, 249, 250, 251, 252, 253, 254, 255, 256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 271, 272, 273, 274, 275, 285, 286, 287, 288, 289, 290, 291, 292, 293, 294, 295, 296, 297, 298, 299, 300, 301, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315, 316, 317, 318, 319, 320, 321, 322, 323, 324, 325, 326, 327, 328, 329, 330, 331, 332, 333, 348, 334, 350, 335, 352, 336, 337, 338, 339, 340, 341, 342, 343, 344, 345, 346, 347, 348, 349, 350, 351, 352, 353, 354, 355, 356, 357, 358, 359, 360, 369, 370, 371, 372, 373, 374, 375, 376, 377, 378, 379, 380, 381, 382, 383, 384, 385, 386, 387, 388, 389, 390, 391, 392, 393, 394, 395, 396, 397, 398, 399, 400, 401, 402, 403, 404, 405, 406, 411, 412, 413, 414, 415, 416, 417, 418, 419, 420, 421, 429, 433, 434, 435, 436, 437, 438, 439, 440, 441, 442, 443, 444, 458, 459, 460, 463, 465, 479, 480, 481, 482, 483, 516, 519, 520, 559, 560};
	int down1[] = {135, 136, 164, 176, 177, 178, 179, 180, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 198, 207, 214, 215, 216, 224, 225, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 241, 242, 243, 244, 245, 246, 248, 249, 250, 251, 252, 253, 254, 255, 256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 271, 272, 273, 282, 283, 284, 285, 286, 287, 288, 289, 290, 291, 292, 293, 294, 295, 296, 297, 298, 299, 300, 301, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315, 316, 317, 318, 319, 320, 321, 322, 323, 324, 325, 326, 327, 328, 329, 330, 331, 332, 333, 348, 334, 350, 335, 352, 336, 337, 338, 339, 340, 341, 342, 343, 344, 345, 346, 347, 348, 349, 350, 351, 352, 353, 354, 355, 356, 357, 358, 359, 364, 365, 366, 367, 368, 369, 370, 371, 372, 373, 374, 375, 376, 377, 378, 379, 380, 381, 382, 383, 384, 385, 386, 387, 388, 389, 390, 391, 392, 393, 394, 395, 396, 397, 398, 399, 400, 401, 402, 403, 404, 405, 406, 411, 412, 413, 414, 415, 416, 417, 418, 419, 420, 421, 429, 433, 434, 435, 436, 437, 438, 439, 440, 441, 442, 443, 444, 458, 459, 460, 463, 479, 480, 481, 482, 483, 516, 519, 520, 559, 560};
	int right0[] = {135, 136, 164, 177, 178, 179, 180, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 198, 207, 214, 225, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 239, 241, 242, 243, 244, 245, 246, 248, 249, 250, 251, 252, 253, 254, 255, 256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 271, 272, 273, 274, 275, 276, 277, 279, 280, 281, 282, 283, 284, 285, 286, 287, 288, 289, 290, 291, 292, 293, 294, 295, 296, 297, 298, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315, 316, 317, 318, 319, 320, 321, 322, 323, 324, 325, 326, 327, 328, 329, 330, 331, 332, 333, 348, 334, 350, 335, 352, 336, 337, 338, 339, 340, 341, 342, 343, 344, 345, 346, 347, 348, 349, 350, 351, 352, 353, 354, 355, 356, 357, 358, 359, 360, 361, 362, 363, 364, 365, 366, 367, 368, 369, 370, 371, 372, 373, 374, 375, 376, 377, 378, 379, 380, 381, 382, 383, 384, 385, 386, 387, 388, 389, 400, 401, 402, 403, 404, 405, 406, 411, 412, 413, 414, 415, 416, 417, 418, 419, 420, 421, 422, 423, 424, 433, 434, 435, 436, 437, 438, 439, 440, 441, 442, 443, 444, 458, 459, 460, 463, 465, 468, 469, 470, 472, 479, 480, 481, 482, 483, 559, 560};
	int right1[] = {135, 136, 164, 177, 178, 179, 180, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 198, 199, 200, 202, 203, 204, 206, 207, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 239, 241, 242, 243, 244, 245, 246, 248, 249, 250, 251, 252, 253, 254, 255, 256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 280, 281, 282, 283, 284, 285, 286, 287, 288, 289, 290, 291, 292, 293, 294, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315, 316, 317, 318, 319, 320, 321, 322, 323, 324, 325, 326, 327, 328, 329, 330, 331, 332, 333, 348, 334, 350, 335, 352, 336, 337, 338, 339, 340, 341, 342, 343, 344, 345, 346, 347, 348, 349, 350, 351, 352, 353, 354, 355, 356, 357, 358, 359, 360, 361, 362, 363, 364, 365, 366, 367, 368, 369, 370, 371, 372, 373, 374, 375, 376, 377, 378, 379, 380, 381, 382, 383, 384, 385, 399, 400, 401, 402, 403, 404, 405, 406, 411, 412, 413, 414, 415, 416, 417, 418, 419, 420, 421, 422, 423, 424, 429, 433, 434, 435, 436, 437, 438, 439, 440, 441, 442, 443, 444, 458, 459, 460, 463, 465, 468, 469, 470, 472, 479, 480, 481, 482, 483, 518, 519, 520, 521, 559, 560};
	direction_nums.Empty();
	switch (direction)
	{
	case 0:
		if (FMath::RandBool()) {
			direction_nums.Append(up0, UE_ARRAY_COUNT(up0));
		}
		else {
			direction_nums.Append(up1, UE_ARRAY_COUNT(up1));
		}
		break;
	case 1:
		if (FMath::RandBool()) {
			direction_nums.Append(left0, UE_ARRAY_COUNT(left0));
		}
		else {
			direction_nums.Append(left1, UE_ARRAY_COUNT(left1));
		}
		break;
	case 2:
		if (FMath::RandBool()) {
			direction_nums.Append(down0, UE_ARRAY_COUNT(down0));
		}
		else {
			direction_nums.Append(down1, UE_ARRAY_COUNT(down1));
		}
		break;
	case 3:
		if (FMath::RandBool()) {
			direction_nums.Append(right0, UE_ARRAY_COUNT(right0));
		}
		else {
			direction_nums.Append(right1, UE_ARRAY_COUNT(right1));
		}
		break;
	default:
		break;
	}
}

void UColorCalibration::LoadAllPlatesMeshActor(TArray<AStaticMeshActor*> all_plates_actors)
{
	all_plates = all_plates_actors;
}

void UColorCalibration::ConfusionPoints(int confusion_line, FColor_lxy& start, FColor_lxy& end) {
	FColor_Luv Luv_starts[] = {FColor_Luv(0.217169819, 0.468878767), FColor_Luv(0.217030009, 0.458258869), FColor_Luv(0.200289048, 0.447181003)};
	FColor_Luv Luv_ends[] = { FColor_Luv(0.308435721, 0.474292894), FColor_Luv(0.299623829, 0.431180807), FColor_Luv(0.209764962, 0.356921098)};
	convertFromLuvtolxy(Luv_starts[confusion_line], start);
	convertFromLuvtolxy(Luv_ends[confusion_line], end);
}

void UColorCalibration::NeutralPoints(FColor_lxy& lxy) {
	FColor_Luv Luv_neutrals = FColor_Luv(0.1977, 0.4689);
	convertFromLuvtolxy(Luv_neutrals, lxy);
}

void UColorCalibration::TrivectorTestStimuli(int& confusion_line, int& new_direction)
{
	confusion_line = 0;//CONFUSION_ALONG * FMath::FRand();
	new_direction = 0;// 3 * FMath::FRand();
	AlterPlateColors(new_direction, confusion_line, threshold[confusion_line]);
}

void UColorCalibration::TrivectorTestResponse(int response, int direction, int confusion_line, int& new_confusion_line, int& new_direction)
{
	if (test_done == false) {
		bool correct = response == direction;
		FString res_str = correct ? "true" : "false";
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Response : %s"), *res_str));
		if (correct) {
			correct_threshold[confusion_line] = threshold[confusion_line];
		}
		else {
			incorrect_threshold[confusion_line] = threshold[confusion_line];
		}
		int threshold_ = 0;
		updateThreshold(correct_threshold[confusion_line], incorrect_threshold[confusion_line], threshold_);
		temp_thresh[confusion_line] = threshold_;
		if (threshold[confusion_line] == temp_thresh[confusion_line]) {
			test_done = true;
		}
		else {
			threshold[confusion_line] = temp_thresh[confusion_line];
		}
		if (!correct && threshold[confusion_line] == start_thresh[confusion_line]) {
			test_done = true;
		}
		if (correct && threshold[confusion_line] == 1) test_done = true;
	}
	if (test_done == false) {
		TrivectorTestStimuli(new_confusion_line, new_direction);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString::Printf(TEXT("threshold : %d"), threshold[confusion_line]));
	}
	else {
		new_direction = 4;
	}
}

void UColorCalibration::updateThreshold(int correct, int incorrect, int& threshold_)
{
	threshold_ = (correct + incorrect) / 2;
}

void UColorCalibration::AlterPlateColors(int direction, int confusion_line, int threshold_)
{
	FColor_lxy neutral_points;
	
	int steps = 100;

	NeutralPoints(neutral_points);
	for (int i = 0; i < all_plates.Num(); i++) {
		UMaterialInstanceDynamic* back_plate_mat = UMaterialInstanceDynamic::Create(parent_mat, this);
		//Update background
		ColorInterp(neutral_points, neutral_points, 1, steps, neutral_color);
		back_plate_mat->SetVectorParameterValue(FName("Color"), neutral_color);
		all_plates[i]->GetStaticMeshComponent()->SetMaterial(0, back_plate_mat);
	}
	
	FColor_lxy start, end;
	ConfusionPoints(confusion_line, start, end);
	TArray<int> direction_plates_num;

	LoadDirectionPlates(direction, direction_plates_num);
	ColorInterp(start, end, threshold_, steps, confusion_color);
	int a = 0;
	for (int i = 0; i < direction_plates_num.Num(); i++) {
		int j = direction_plates_num[i];
		if (j <= 211) j = j + 1;
		UMaterialInstanceDynamic* front_plate_mat = UMaterialInstanceDynamic::Create(parent_mat, this);
		//Update foreground color
		//ColorInterp(start, end, threshold_, steps, confusion_color);
		front_plate_mat->SetVectorParameterValue(FName("Color"), confusion_color);
		all_plates[j]->GetStaticMeshComponent()->SetMaterial(0, front_plate_mat);
	}
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString::Printf(TEXT("Float : %f %f %f"), confusion_color.R, confusion_color.G, confusion_color.B));
}

void UColorCalibration::ColorInterp(FColor_lxy start, FColor_lxy end, int threshold_, int steps, FLinearColor& plate_color)
{
	FColor_lxy threshold_color;
	threshold_color.x = start.x + (threshold_ * (end.x - start.x) / steps);
	threshold_color.y = start.y + (threshold_ * (end.y - start.y) / steps);
	threshold_color.l = (FMath::FRand() * 17.0f + 7.6f);
	FColor_XYZ XYZ;
	convertFromlxyToXYZ(threshold_color, XYZ);
	convertFromXYZtoRGB(XYZ, plate_color);
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
	//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Blue, FString::Printf(TEXT("Post Camera Color Mode On : %f %f %f\n%f %f %f\n%f %f %f\n"), \
	//																							XYZ_to_RGB(0, 0), XYZ_to_RGB(0, 1), XYZ_to_RGB(0, 2), \
	//																							XYZ_to_RGB(1, 0), XYZ_to_RGB(1, 1), XYZ_to_RGB(1, 2), \
	//																							XYZ_to_RGB(2, 0), XYZ_to_RGB(2, 1), XYZ_to_RGB(2, 2)));
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