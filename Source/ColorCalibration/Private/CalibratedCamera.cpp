// Fill out your copyright notice in the Description page of Project Settings.


#include "CalibratedCamera.h"

template <typename ObjClass>
static FORCEINLINE ObjClass* LoadObjFromPath(const FName& Path)
{
	if (Path == NAME_None) return nullptr;

	return Cast<ObjClass>(StaticLoadObject(ObjClass::StaticClass(), nullptr, *Path.ToString()));
}

static FORCEINLINE UMaterial* LoadMaterialFromPath(const FName& Path)
{
	if (Path == NAME_None) return nullptr;

	return LoadObjFromPath<UMaterial>(Path);
}


void ACalibratedCamera::BeginPlay()
{
	Super::BeginPlay();
	FString sPath = "/Game/PostTonemap";
	UMaterial* parent_mat = LoadMaterialFromPath(FName(*sPath));
	alterCamera = GetCameraComponent();
	postmat = UMaterialInstanceDynamic::Create(parent_mat, this);
	postTone.Add(FWeightedBlendable(1.0, postmat));

	sPath = "/Game/PreTonemap";
	parent_mat = LoadMaterialFromPath(FName(*sPath));
	premat = UMaterialInstanceDynamic::Create(parent_mat, this);
	preTone.Add(FWeightedBlendable(1.0, premat));
	
	cal_lib = NewObject<UColorCalibration>();
	Normal_settings = alterCamera->PostProcessSettings;
}

void ACalibratedCamera::initialize_color_assist(FString primaries_filename, FString plates_filename, int start_threshold, TArray<FTransform>& all_plates_transform) {
	TArray<FColor_lxy> primaries;
	cal_lib->readPrimariesFromCSV(primaries_filename, primaries);
	cal_lib->readPlatePointsFromCSV(plates_filename, start_threshold, all_plates_transform);
}

// Called every frame
void ACalibratedCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACalibratedCamera::switch_camera_settings() {
	if (current_settings) {
		alterCamera->PostProcessSettings = Normal_settings;
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Normal Camera"));
	}
	else {
		postprocess_material = postmat;
		Post_settings.WeightedBlendables = postTone;
		alterCamera->PostProcessSettings = Post_settings;
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Post Camera"));
	}
	current_settings = !current_settings;
}

void ACalibratedCamera::normal_camera_settings() {
	if (current_settings) {
		alterCamera->PostProcessSettings = Normal_settings;
		current_settings = !current_settings;
	}
	else {
		return;
	}
}

void ACalibratedCamera::post_camera_settings() {
	if (current_settings) {
		return;
	}
	else {
		alterCamera->PostProcessSettings = Post_settings;
		current_settings = !current_settings;
	}
}

FLinearColor ACalibratedCamera::convert_hex() {
	return FLinearColor(FColor::FromHex(hexColor));
}

void ACalibratedCamera::Color_Mode_On(float val=0.0f) {
	if (current_settings) {
		Post_settings.bOverride_AutoExposureBias = true;
		Post_settings.bOverride_BloomIntensity = true;
		Post_settings.bOverride_MotionBlurAmount = true;
		Post_settings.bOverride_GrainJitter = true;
		Post_settings.bOverride_SceneFringeIntensity = true;
		Post_settings.bOverride_VignetteIntensity = true;
		Post_settings.bOverride_GrainIntensity = true;
		Post_settings.bOverride_AutoExposureMaxBrightness = true;
		Post_settings.bOverride_AutoExposureMinBrightness = true;
		Post_settings.AutoExposureBias = 0.0f;
		Post_settings.AutoExposureMaxBrightness = 1.0f;
		Post_settings.AutoExposureMinBrightness = 1.0f;
		Post_settings.BloomIntensity = 0.0f;
		Post_settings.MotionBlurAmount = 0.0f;
		Post_settings.GrainJitter = 0.0f;
		Post_settings.SceneFringeIntensity = 0.0f;
		Post_settings.VignetteIntensity = 0.0f;
		Post_settings.GrainIntensity = 0.0f;
		if (val == 0.0) {
			postprocess_material = premat;
			Post_settings.WeightedBlendables = preTone;
		}
		else {
			postprocess_material = postmat;
			Post_settings.WeightedBlendables = postTone;
		}
		alterCamera->PostProcessSettings = Post_settings;
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("Post Camera Color Mode On : %f"), val));
	}
	return;
}

void ACalibratedCamera::Color_Mode_Off(float val=1.0f) {
	if (current_settings) {
		Post_settings.bOverride_AutoExposureBias = false;
		Post_settings.bOverride_AutoExposureMaxBrightness = false;
		Post_settings.bOverride_AutoExposureMinBrightness = false;
		Post_settings.bOverride_BloomIntensity = false;
		Post_settings.bOverride_MotionBlurAmount = false;
		Post_settings.bOverride_GrainJitter = false;
		Post_settings.bOverride_SceneFringeIntensity = false;
		Post_settings.bOverride_VignetteIntensity = false;
		Post_settings.bOverride_GrainIntensity = false;
		if (val == 0.0) {
			postprocess_material = premat;
			Post_settings.WeightedBlendables = preTone;
		}
		else {
			postprocess_material = postmat;
			Post_settings.WeightedBlendables = postTone;
		}
		alterCamera->PostProcessSettings = Post_settings;
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("Post Camera Color Mode Off : %f"), val));
	}
	return;
}


void ACalibratedCamera::Update_post_material(FLinearColor color) {
	//postprocess_material->SetVectorParameterValue(FName("Color"), color);
}

FLinearColor ACalibratedCamera::confusion_lines_serially() {
	if (serial < protan_hex.Num()) {
		hexColor = protan_hex[serial];
	}
	else if (serial >= protan_hex.Num() && serial < protan_hex.Num() + deutan_hex.Num()) {
		hexColor = deutan_hex[serial - protan_hex.Num()];
	}
	else if (serial >= protan_hex.Num() + deutan_hex.Num() && serial < protan_hex.Num() + deutan_hex.Num() + tritan_hex.Num()) {
		hexColor = tritan_hex[serial - protan_hex.Num() - deutan_hex.Num()];
	}
	else if (serial < protan_hex.Num() + deutan_hex.Num() + tritan_hex.Num() + background_hex.Num()) {
		hexColor = background_hex[serial - protan_hex.Num() - deutan_hex.Num() - tritan_hex.Num()];
	}
	else {
		serial = 0;
	}
	serial++;
	return convert_hex();
}

FLinearColor ACalibratedCamera::cube_colors_serially(float start, float end) {
	TArray<FLinearColor> corners;
	float arr[2] = { start, end };
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			for (int k = 0; k < 2; k++) {
				corners.Add(FLinearColor(arr[i], arr[j], arr[k]));
			}
		}
	}
	if (serial >= 0 && serial < corners.Num()) {
		return corners[serial++];
	}
	else{
		serial = 0;
		return FLinearColor::Black;
	}
}

FLinearColor ACalibratedCamera::primaries_serially() {
	if (serial >= 0 && serial < 4) {
		if (serial == 0) {
			serial++;
			return FLinearColor::Red;
		}
		else if (serial == 1) {
			serial++;
			return FLinearColor::Green;
		}
		else if (serial == 2) {
			serial++;
			return FLinearColor::Blue;
		}
		else{
			serial++;
			return FLinearColor::White;
		}
	}
	else {
		serial = 0;
		return FLinearColor::Black;
	}
}

FLinearColor ACalibratedCamera::stepped_primaries_serially() {
	TArray<FLinearColor> stepped_primaries;
	for (int i = 0; i < 3; i++) {
		float temp_color[3] = {0, 0, 0};
		for (int j = 0; j < 9; j++) {
			temp_color[i] = 0.1f*(j+1);
			stepped_primaries.Add(FLinearColor(temp_color[0], temp_color[1], temp_color[2]));
		}
	}
	float step = 0.1f;
	for (int j = 0; j < 9; j++) {
		stepped_primaries.Add(FLinearColor(step*(j+1), step * (j + 1), step * (j + 1)));
	}

	if (serial >= 0 && serial < stepped_primaries.Num()) {
		return stepped_primaries[serial++];
	}
	else {
		serial = 0;
		return FLinearColor::Black;
	}
}

