
/*
Author:         Dr Frazer K. Noble
Institution:    Massey University, School of Engineering and Advanced Technology, Auckland, New Zealand
Date:           19/03/2018
Desription:     This program demonstrates how calibrate a camera with OpenCV using a 9*6 chessboard calibration image. It's a minimum working example.
*/

#include "stdafx.h"

#include <iostream>

#include <opencv2/opencv.hpp>

int main(int argc, char* argv[]) {

	std::cout << "Calibrating a camera using OpenCV." << std::endl;

	//Open camera, start video capture
	cv::VideoCapture camera(0);

	if (!camera.isOpened()) {
		std::cout << "Camera could not be opened." << std::endl;
	}

	cv::Mat frame, working_frame;
	cv::Size frame_size;
	cv::Size working_frame_size = { 640, 480 };

	//Board dimensions
	int board_width = 9;
	int board_height = 6;
	int number_of_boards = board_width * board_height;
	cv::Size board_size = { board_width, board_height };

	int number_of_images = 20;

	std::vector<std::vector<cv::Point2f>> image_points;			//2D points in image plane
	std::vector<std::vector<cv::Point3f>> object_points;		//3D points in real world space

	double last_captured_timestamp = 0;


	std::cout << "Start camera calibration\nChoose calibration method" << std::endl;
	std::cout << "'1': Generate new calibration values" << std::endl;
	std::cout << "'2': Use bad calibration values" << std::endl;
	std::cout << "'3': Use good calibration values" << std::endl;

	cv::Mat intrinsic_matrix, distortion_coefficients;

	cv::namedWindow("Working Frame", CV_WINDOW_AUTOSIZE);


	while (true) {
		char c;
		c = cv::waitKey(1);

		if (c == '1') {
			
			while (image_points.size() < static_cast<size_t>(number_of_images)) {
				
				//Check for empty frames
				camera >> frame;
				if (frame.empty()) {
					continue;
				}
				
				//Resize frame
				frame_size = frame.size();
				cv::resize(frame, working_frame, working_frame_size);

				//Find the chess board corners
				std::vector<cv::Point2f> corners;
				bool found = cv::findChessboardCorners(working_frame, board_size, corners);

				//If found, draw and display chess board corners
				cv::drawChessboardCorners(working_frame, board_size, corners, found);

				double timestamp = static_cast<double>(clock()) / CLOCKS_PER_SEC;

				if (found && timestamp - last_captured_timestamp > 1) {

					last_captured_timestamp = timestamp;
					working_frame ^= cv::Scalar::all(255);

					image_points.push_back(corners);
					object_points.push_back(std::vector<cv::Point3f>());

					std::vector<cv::Point3f>& opts = object_points.back();
					opts.resize(number_of_boards);
					for (int j = 0; j < number_of_boards; j++) {
						opts[j] = cv::Point3f(static_cast<float>(j / board_width), static_cast<float>(j%board_width), static_cast<float>(0));
					}
				}
				cv::imshow("Working Frame", working_frame);

				char c;
				c = cv::waitKey(1);
				if (c == 'q') {
					break;
				}
			}

			
			//Camera calibration - returns camera matrix, distortion coefficients
			//rotation and translation vectors are not stored
			double reprojection_error = cv::calibrateCamera(
				object_points,
				image_points,
				frame_size,
				intrinsic_matrix,
				distortion_coefficients,
				cv::noArray(),
				cv::noArray(),
				cv::CALIB_ZERO_TANGENT_DIST | cv::CALIB_FIX_PRINCIPAL_POINT
			);

			std::string path = "calibration_data.xml";
			cv::FileStorage fs;

			std::cout << "Reprojection error is: " << reprojection_error << std::endl;

			fs.open(path, cv::FileStorage::WRITE);

			fs << "intrinsic_matrix" << intrinsic_matrix;
			fs << "distortion_coefficients" << distortion_coefficients;
			fs << "reprojection_error" << reprojection_error;

			fs.release();

			break;
		}
		else if (c == '2') {
			std::string filename = "bad_calib.xml";
			cv::FileStorage fs;
			fs.open(filename, cv::FileStorage::READ);

			fs["intrinsic_matrix"] >> intrinsic_matrix;
			fs["distortion_coefficients"] >> distortion_coefficients;

			break;
		}
		else if (c == '3') {
			std::string filename = "good_calib.xml";
			cv::FileStorage fs;
			fs.open(filename, cv::FileStorage::READ);

			fs["intrinsic_matrix"] >> intrinsic_matrix;
			fs["distortion_coefficients"] >> distortion_coefficients;

			break;
		}
	}
	cv::destroyWindow("Working Frame");
	cv::namedWindow("Calibrated Image", CV_WINDOW_AUTOSIZE);
	cv::namedWindow("Distorted Image", CV_WINDOW_AUTOSIZE);

	if (intrinsic_matrix.empty() || distortion_coefficients.empty())
	{
		std::cout << "Calibration data is unavailable" << std::endl;
	}

	while (true)
	{
		//Undistort camera
		cv::Mat calibrate;
		camera >> working_frame;

		cv::imshow("Distorted Image", working_frame);

		cv::undistort(
			working_frame,
			calibrate,
			intrinsic_matrix,
			distortion_coefficients
		);

		cv::imshow("Calibrated Image", calibrate);

		char c;
		c = cv::waitKey(1);
		if (c == 'q') {
			break;
		}
	}

	cv::destroyAllWindows();

	std::cout << "Done." << std::endl;

	return 0;
}