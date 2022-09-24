// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Scene.h"
#include "Misc/Paths.h"
#include "Camera/CameraActor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Camera/CameraComponent.h"
#include "../ColorCalibration.h"
#include "CalibratedCamera.generated.h"

/**
 * 
 */
UCLASS()
class COLORCALIBRATION_API ACalibratedCamera : public ACameraActor
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, Category = "DEBUGGING")
		FString hexColor;

	TArray<FString> protan_hex = { "#b4989c", "#c0a3a7", "#d1b1b6", "#c2a5a9", "#bb9fa3", "#c9abaf", "#b79b9f", "#bda0a5", "#b99da1", "#dbacb3", "#d1a4aa", "#c1979d", "#c3999f", "#d6a8af", "#be949b", "#dbacb3", "#ca9ea5", "#d4a6ad", "#d99ea7", "#c59098", "#e3a5af", "#d49ba3", "#dda1aa", "#c38e96", "#e7a8b2", "#ce969f", "#c8929a", "#e69da8", "#d8939e", "#d8939e", "#f0a3af", "#e49ba6", "#f1a4b0", "#d9949e", "#d9949e", "#df98a2", "#d48693", "#e994a2", "#e08e9b", "#f49caa", "#d88995", "#d48793", "#d58794", "#ed97a5", "#f69dab", "#e88a99", "#e68998", "#fe98a9", "#fc96a7", "#e88a9a", "#fe98a9", "#fe98a9", "#e48897", "#f995a5", "#fe8ea1", "#f6899c", "#fe8ea1", "#fb8c9f", "#ec8395", "#f4889a", "#fc8d9f", "#fe8ea1", "#fe8ea1", "#fe859a", "#fe859a", "#f47f94", "#ec7b8f", "#fe859a", "#f17e92", "#fe859a", "#f27f93", "#fe859a", "#fe7d94", "#fe7d94", "#fe7d94", "#fd7c93", "#fa7a91", "#fe7d94", "#f5788e", "#fe7d94", "#f2768c", "#fe748e", "#fe748e", "#fe748e", "#fe748e", "#fe748e", "#fe748e", "#fd738d", "#fe748e", "#fe748e" };

	TArray<FString> deutan_hex = { "#bc9eae", "#ba9cad", "#bb9dad", "#be9fb0", "#c5a5b6", "#d0aec0", "#bfa0b1", "#c7a7b8", "#bc9dae", "#c79bb2", "#c99cb3", "#cfa1b9", "#daaac3", "#c89bb2", "#c599af", "#d6a7bf", "#b98fa5", "#ba90a6", "#d197b4", "#d398b6", "#e4a5c4", "#dd9fbe", "#dc9fbe", "#e5a5c5", "#d79bba", "#dda0bf", "#e3a4c3", "#ed9fc6", "#e69ac0", "#d891b5", "#e499bf", "#cf8bad", "#d790b4", "#cf8bad", "#ec9ec5", "#eea0c7", "#f99ccb", "#f89bca", "#d484ac", "#d484ad", "#d786af", "#f398c6", "#f197c4", "#f298c5", "#de8ab5", "#f08cbf", "#e987b9", "#f48ec2", "#f68fc4", "#db7fae", "#e484b5", "#fe94ca", "#df81b1", "#f890c5", "#fe8ac6", "#fe8ac6", "#fe89c5", "#fe8ac6", "#e279af", "#fe8ac6", "#e67cb2", "#fe8ac6", "#fe8ac6", "#f97cbd", "#f87cbd", "#fe7fc2", "#fe7fc2", "#fe7fc2", "#f97cbd", "#fe7fc2", "#fe7fc2", "#f87cbd", "#fe75be", "#fc74bc", "#fe75be", "#fe75be", "#fe75be", "#fe75be", "#fe75be", "#f06eb3", "#f16fb4", "#fe6bbb", "#fe6bbb", "#fe6bbb", "#fe6bbb", "#fa69b8", "#fe6bbb", "#fe6bbb", "#fe6bbb", "#fe6bbb" };

	TArray<FString> tritan_hex = { "#a59fb7", "#b2abc5", "#aca5be", "#b8b0cb", "#a19bb2", "#a19bb2", "#b7b0ca", "#b7b0ca", "#aba4bc", "#a99fc2", "#b0a6cb", "#a49bbd", "#b1a7cc", "#b9aed5", "#a49abd", "#aca2c6", "#b5abd1", "#aca2c6", "#ab9ecd", "#beb0e4", "#a497c4", "#aea1d1", "#b6a9da", "#b5a7d9", "#b6a8da", "#aea1d0", "#b2a4d4", "#beaceb", "#c1aeee", "#b3a2dd", "#bfadec", "#a494cc", "#ac9cd5", "#ae9ed8", "#a797ce", "#b5a4e0", "#b09ce1", "#bda7f1", "#b19de3", "#b6a1e8", "#b29ee4", "#ae9adf", "#a491d3", "#bea8f3", "#ab97db", "#a992df", "#bda4fa", "#b199ea", "#c1a7fe", "#c0a6fd", "#af97e6", "#aa93e0", "#c1a7fe", "#b79ef2", "#b89bf9", "#ab90e9", "#a98ee6", "#bc9efe", "#bc9efe", "#bc9efe", "#ad91eb", "#bc9efe", "#b397f4", "#af8ff4", "#ad8df1", "#ae8ff3", "#b796fe", "#b796fe", "#b796fe", "#b695fe", "#b796fe", "#b796fe", "#af8bfa", "#b28dfe", "#b28dfe", "#b28dfe", "#b28dfe", "#b28dfe", "#b28dfe", "#b28dfe", "#b28dfe", "#ad86fe", "#ad86fe", "#ad86fe", "#ad86fe", "#ad86fe", "#ad86fe", "#ad86fe", "#ad86fe", "#ad86fe" };

	TArray<FString> background_hex = { "#889191", "#8a9bfa", "#5efa98", "#85f4f4", "#fe968f", "#fe9bf0", "#e5f79b", "#e3f2f2", "#c0cccb", "#c0cce6", "#b8e6cc", "#bce5e5", "#decccc", "#dfcce6", "#d6e6cd", "#d7e4e5" };
	bool current_settings = false;
	UCameraComponent* alterCamera;
	FPostProcessSettings Normal_settings, Post_settings;
	TArray<FWeightedBlendable> postTone, preTone;
	UMaterialInstanceDynamic *premat, *postmat;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DEBUGGING")
		int serial = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DEBUGGING")
		UColorCalibration* cal_lib;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DEBUGGING")
		UMaterialInstanceDynamic* postprocess_material;

	UFUNCTION(BlueprintCallable, Category = "Camera Settings", meta = (Keywords = "Start"))
		void initialize_color_assist(FString primaries_filename, FString plates_filename, int start_threshold, TArray<FTransform>& all_plates);

	UFUNCTION(BlueprintCallable, Category = "Camera Settings", meta = (Keywords = "Start"))
		void switch_camera_settings();

	UFUNCTION(BlueprintCallable, Category = "Camera Settings", meta = (Keywords = "Start"))
		void normal_camera_settings();

	UFUNCTION(BlueprintCallable, Category = "Camera Settings", meta = (Keywords = "Start"))
		void post_camera_settings();

	UFUNCTION(BlueprintCallable, Category = "Color Conversion", meta = (Keywords = "Start"))
		void Update_post_material(FLinearColor color);

	UFUNCTION(BlueprintCallable, Category = "Color Conversion", meta = (Keywords = "Start"))
		FLinearColor convert_hex();

	UFUNCTION(BlueprintCallable, Category = "Color Mode", meta = (Keywords = "Start"))
		void Color_Mode_On(float val);

	UFUNCTION(BlueprintCallable, Category = "Color Mode", meta = (Keywords = "Start"))
		void Color_Mode_Off(float val);

	UFUNCTION(BlueprintCallable, Category = "Color Conversion", meta = (Keywords = "Start"))
		FLinearColor confusion_lines_serially();

	UFUNCTION(BlueprintCallable, Category = "Color Calibration", meta = (Keywords = "Start"))
		FLinearColor cube_colors_serially(float start, float end);
	
	UFUNCTION(BlueprintCallable, Category = "Color Calibration", meta = (Keywords = "Start"))
		FLinearColor primaries_serially();

	UFUNCTION(BlueprintCallable, Category = "Color Calibration", meta = (Keywords = "Start"))
		FLinearColor stepped_primaries_serially();
};

