// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
//#include "opencv2/core/mat.hpp"
//#include "opencv2/core.hpp"
#include "UEigen3/Dense"
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
};

UCLASS(BlueprintType)
class COLORCALIBRATION_API UColorCalibration : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	Eigen::Matrix <double, 3, 3> XYZ_to_RGB;
	Eigen::Matrix <double, 3, 3> RGB_to_XYZ;
	UFUNCTION(BlueprintCallable, Category = "Conversion", meta = (Keywords = "Eigen3"))
	void solve(FColor_primaries_lxy recorded);

	UFUNCTION(BlueprintCallable, Category = "Conversion", meta = (Keywords = "lxy"))
		void convertFromlxy(FColor_lxy lxy, FLinearColor& retColor);

	UFUNCTION(BlueprintCallable, Category = "Conversion", meta = (Keywords = "lxy"))
		void convertFromlxyToXYZ(FColor_lxy lxy, FColor_XYZ& retColor);

	UFUNCTION(BlueprintCallable, Category = "Conversion", meta = (Keywords = "Luv"))
		void convertFromLuv(FColor_Luv Luv, FLinearColor& retColor);

	UFUNCTION(BlueprintCallable, Category = "Conversion", meta = (Keywords = "lxy"))
		void convertFromLuvToXYZ(FColor_Luv lxy, FColor_XYZ& retColor);

	UFUNCTION(BlueprintCallable, Category = "Conversion", meta = (Keywords = "XYZ"))
		void convertFromXYZ(FColor_XYZ XYZ, FLinearColor& retColor);

	UFUNCTION(BlueprintCallable, Category = "Conversion", meta = (Keywords = "XYZ"))
		void convertFromRGB(FLinearColor RGB, FColor_XYZ& retColor);

	UFUNCTION(BlueprintCallable, Category = "Conversion", meta = (Keywords = "XYZ"))
		void readPrimariesFromCSV(FString csv_filename, TArray<FColor_lxy>& lxys);

	UFUNCTION(BlueprintCallable, Category = "Conversion", meta = (Keywords = "XYZ"))
		void readPlatePointsFromCSV(FString csv_filename, TArray<FTransform>& all_plates);

	UFUNCTION(BlueprintCallable, Category = "Custom", meta = (Keywords = "Load"))
		static bool LoadTextFromFile(FString FileName, TArray<FString>& TextArray);

	UFUNCTION(BlueprintCallable, Category = "Custom", meta = (Keywords = "Save"))
		static bool SaveArrayText(FString SaveDirectory, FString FileName, TArray<FString> SaveText, bool AllowOverwriting);
};