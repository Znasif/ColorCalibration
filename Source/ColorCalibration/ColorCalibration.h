// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once


#define CONFUSION_ALONG 3

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
//#include "opencv2/core/mat.hpp"
//#include "opencv2/core.hpp"
#include "UEigen3/Dense"
#include "Engine/StaticMeshActor.h"
#include "ColorCalibration.generated.h"

USTRUCT(BlueprintType)
struct FColor_lxy
{
	GENERATED_USTRUCT_BODY()
		UPROPERTY(BlueprintReadWrite, Category = "Luminance in cd/m2")
		float l;
	UPROPERTY(BlueprintReadWrite, Category = "chromaticity x")
		float x;
	UPROPERTY(BlueprintReadWrite, Category = "chromaticity y")
		float y;
};

USTRUCT(BlueprintType)
struct FColor_Luv
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(BlueprintReadWrite, Category = "Luminance in cd/m2")
		float L;
	UPROPERTY(BlueprintReadWrite, Category = "u")
		float u;
	UPROPERTY(BlueprintReadWrite, Category = "v")
		float v;

	FColor_Luv() {
		this->L = 0;
		this->u = 0;
		this->v = 0;
	}
	
	FColor_Luv(float u, float v) {
			this->L = 1;
			this->u = u;
			this->v = v;
		}
};

USTRUCT(BlueprintType)
struct FColor_XYZ
{
	GENERATED_USTRUCT_BODY()
		UPROPERTY(BlueprintReadWrite, Category = "Tristimulus")
		float X;
	UPROPERTY(BlueprintReadWrite, Category = "Tristimulus")
		float Y;
	UPROPERTY(BlueprintReadWrite, Category = "Tristimulus")
		float Z;
};

USTRUCT(BlueprintType)
struct FColor_primaries_lxy
{
	GENERATED_USTRUCT_BODY()
		UPROPERTY(BlueprintReadWrite, Category = "Red")
		FColor_lxy Red;
	UPROPERTY(BlueprintReadWrite, Category = "Red")
		FColor_lxy Green;
	UPROPERTY(BlueprintReadWrite, Category = "Red")
		FColor_lxy Blue;
	UPROPERTY(BlueprintReadWrite, Category = "Red")
		FColor_lxy White;
	
	FColor_primaries_lxy() {

	}

	FColor_primaries_lxy(FColor_lxy r, FColor_lxy g, FColor_lxy b, FColor_lxy w) {
		this->Red = r;
		this->Green = g;
		this->Blue = b;
		this->White = w;
	}
};

UCLASS(BlueprintType)
class COLORCALIBRATION_API UColorCalibration : public UObject
{
	GENERATED_BODY()
	TArray<int> threshold;
	TArray<int> correct_threshold;
	TArray<int> incorrect_threshold;
	TArray<int> temp_thresh;
	TArray<int> start_thresh;
	float lines_of_confusion[CONFUSION_ALONG];
	TArray<FString> subject_responses;
public:
	Eigen::Matrix <double, 3, 3> XYZ_to_RGB;
	Eigen::Matrix <double, 3, 3> RGB_to_XYZ;
	float max_lum = 100.0f;

	bool test_done[CONFUSION_ALONG];
	TArray<AStaticMeshActor*> all_plates;

	UPROPERTY(BlueprintReadWrite, Category = "Custom", meta = (Keywords = "Subject Data"))
		bool all_test_done;

	UPROPERTY(BlueprintReadWrite, Category = "Custom", meta = (Keywords = "Subject Data"))
		float current_time;

	UPROPERTY(BlueprintReadWrite, Category = "Plate Color", meta = (Keywords = "Neutral"))
	FLinearColor neutral_color;

	UPROPERTY(BlueprintReadWrite, Category = "Plate Color", meta = (Keywords = "Confusion"))
		FLinearColor confusion_color;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plate Color", meta = (Keywords = "Confusion"))
		UMaterial* parent_mat;

	UFUNCTION(BlueprintCallable, Category = "Conversion", meta = (Keywords = "Eigen3"))
	void solve(FColor_primaries_lxy recorded);

	UFUNCTION(BlueprintCallable, Category = "Conversion", meta = (Keywords = "lxy"))
		void convertFromlxytoRGB(FColor_lxy lxy, FLinearColor& retColor);

	UFUNCTION(BlueprintCallable, Category = "Conversion", meta = (Keywords = "lxy"))
		void convertFromlxyToXYZ(FColor_lxy lxy, FColor_XYZ& retColor);

	UFUNCTION(BlueprintCallable, Category = "Conversion", meta = (Keywords = "Luv"))
		void convertFromLuvtoRGB(FColor_Luv Luv, FLinearColor& retColor);

	UFUNCTION(BlueprintCallable, Category = "Conversion", meta = (Keywords = "lxy"))
		void convertFromLuvToXYZ(FColor_Luv lxy, FColor_XYZ& retColor);

	UFUNCTION(BlueprintCallable, Category = "Conversion", meta = (Keywords = "lxy"))
		void convertFromLuvtolxy(FColor_Luv Luv, FColor_lxy& retColor);

	UFUNCTION(BlueprintCallable, Category = "Conversion", meta = (Keywords = "lxy"))
		void convertFromXYZtolxy(FColor_XYZ lxy, FColor_lxy& retColor);

	UFUNCTION(BlueprintCallable, Category = "Conversion", meta = (Keywords = "XYZ"))
		void convertFromXYZtoRGB(FColor_XYZ XYZ, FLinearColor& retColor);

	UFUNCTION(BlueprintCallable, Category = "Conversion", meta = (Keywords = "XYZ"))
		void convertFromRGBtoXYZ(FLinearColor RGB, FColor_XYZ& retColor);

	UFUNCTION(BlueprintCallable, Category = "Conversion", meta = (Keywords = "Primaries"))
		void readPrimariesFromCSV(FString csv_filename, TArray<FColor_lxy>& lxys);

	UFUNCTION(BlueprintCallable, Category = "Conversion", meta = (Keywords = "Plates"))
		void readPlatePointsFromCSV(FString csv_filename, int start_threshold, TArray<FTransform>& all_plates);

	UFUNCTION(BlueprintCallable, Category = "Custom", meta = (Keywords = "Load"))
		static bool LoadTextFromFile(FString FileName, TArray<FString>& TextArray);

	UFUNCTION(BlueprintCallable, Category = "Custom", meta = (Keywords = "Save"))
		static bool SaveArrayText(FString SaveDirectory, FString FileName, TArray<FString> SaveText, bool AllowOverwriting);

	UFUNCTION(BlueprintCallable, Category = "Custom", meta = (Keywords = "Plates"))
		void LoadDirectionPlates(int direction, TArray<int>& direction_nums);

	UFUNCTION(BlueprintCallable, Category = "Custom", meta = (Keywords = "Plates"))
		void LoadAllPlatesMeshActor(TArray<AStaticMeshActor*> all_plates_actors);

	UFUNCTION(BlueprintCallable, Category = "Custom", meta = (Keywords = "Plates"))
		void AlterPlateColors(int direction, int confusion_line, int threshold_);

	UFUNCTION(BlueprintCallable, Category = "Custom", meta = (Keywords = "Confusion Line"))
		void ColorInterp(FColor_Luv start, FColor_Luv end, int threshold_, int steps, FLinearColor& plate_color);
	
	UFUNCTION(BlueprintCallable, Category = "Custom", meta = (Keywords = "Confusion Line"))
		void NeutralPoints(FColor_Luv& Luv_neutral);

	UFUNCTION(BlueprintCallable, Category = "Custom", meta = (Keywords = "Confusion Line"))
		void TrivectorTestStimuli(int& confusion_line, int& direction);

	UFUNCTION(BlueprintCallable, Category = "Custom", meta = (Keywords = "Confusion Line"))
		void TrivectorTestResponse(int response, int direction, int confusion_line, int& new_confusion_line, int& new_direction);

	UFUNCTION(BlueprintCallable, Category = "Custom", meta = (Keywords = "Confusion Line"))
		void updateThreshold(int correct, int incorrect, int& threshold_);

	UFUNCTION(BlueprintCallable, Category = "Custom", meta = (Keywords = "Confusion Line"))
		void vectorCCT(float azimuth, FColor_Luv neutral_luv, FColor_Luv& start, FColor_Luv& end);

	UFUNCTION(BlueprintCallable, Category = "Custom", meta = (Keywords = "Subject Data"))
		void recordResponsetoCSV(FString subjectID);
};