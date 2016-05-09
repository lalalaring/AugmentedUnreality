/*
Copyright 2016 Krzysztof Lis

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http ://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#pragma once

#include "AUROpenCV.h"

#include "AUROpenCVCalibration.generated.h"

USTRUCT(BlueprintType)
struct FOpenCVCameraProperties
{
	GENERATED_BODY()

	/** Camera matrix in form:
		f_x,	0,		center_x;
		0,		f_y,	center_y;
		0,		0		1;
	
		During the calibration we will assume
		that f_x == f_y.
	*/
	cv::Mat CameraMatrix;

	cv::Mat DistortionCoefficients;

	// Parameters of the marker image to use.
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CameraCalibration)
	//FIntPoint Resolution;

	//float CameraPixelRatio;

	// Field of view, X is horizontal, Y is vertical
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CameraCalibration)
	FVector2D FOV;

	FOpenCVCameraProperties()
		: FOV(50., 50.)
	{
		// Default camera matrix:
		// f = 900
		// res = 800x600
		CameraMatrix.create(3, 3, CV_64FC1);
		CameraMatrix.setTo(0.0);
		double f = 900.0;
		CameraMatrix.at<double>(0, 0) = f;
		CameraMatrix.at<double>(1, 1) = f;
		CameraMatrix.at<double>(0, 2) = 400.0;
		CameraMatrix.at<double>(1, 2) = 300.0;

		DistortionCoefficients.create(5, 1, CV_64FC1);
		DistortionCoefficients.setTo(0);
	}

	/**
	 * Attempts to load calibration data from file in OpenCV format.
	 * Returns true if successful, otherwise file was probably not found.
	 */
	bool LoadFromFile(FString const& file_path);

	/**
	 * Saves calibration data to file, in OpenCV format.
	 * http://docs.opencv.org/3.1.0/dd/d74/tutorial_file_input_output_with_xml_yml.html
	 */
	bool SaveToFile(FString const& file_path) const;

	/** Calucalte FOV from CameraMatrix */
	void DeriveFOV(FIntPoint const resolution);

	void PrintToLog() const;

protected:
	static const char* KEY_CAMERA_MATRIX;
	static const char* KEY_DISTORTION;
	static const char* KEY_FOV;
};

/*
	OpenCV camera calibration using the asymmetric circles 4x11 pattern.
*/
class FOpenCVCameraCalibrationProcess
{
public:
	FOpenCVCameraCalibrationProcess();
	
	// Prepare for a new calibration, clear any the process if it is in progress.
	void Reset();

	// Try using a new frame. Time is given so that there is appropriate interval 
	// between consecutive captured frames.
	bool ProcessFrame(cv::Mat& frame, float time_now);

	bool IsFinished() const
	{
		return FramesCollected == FramesNeeded;
	}

	float GetProgress() const
	{
		return float(FramesCollected) / float(FramesNeeded);
	}

	FOpenCVCameraProperties const& GetCameraProperties() const 
	{
		return CameraProperties;
	}

protected:
	// Number of frames to be captured before calibration is calculated.
	int32 FramesNeeded;

	// Time between capturing consecutive frames
	float MinInterval;

	// Number of rows / columns in the pattern.
	cv::Size PatternSize;

	int32 CalibrationFlags;

	// Distance between rows/columns
	float SquareSize; 

	std::vector<cv::Mat> DetectedPointSets;
	int32 FramesCollected;
	float LastFrameTime;

	FOpenCVCameraProperties CameraProperties;

	void CalculateCalibration(FIntPoint const resolution);
};
