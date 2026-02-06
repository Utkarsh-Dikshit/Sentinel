#include "SentinelSystem.h"
#include <iostream>

// Constructor
SentinelSystem::SentinelSystem(int id) : is_running(false), camera_id(id) {}

// Destructor
SentinelSystem::~SentinelSystem() {
	stop();
}

// Producer Thread (Camera)
void SentinelSystem::captureLoop() {
	cv::VideoCapture cap(camera_id);
	if (!cap.isOpened()) {
		std::cerr << "ERROR: Could not find the camera!" << std::endl;
		is_running = false;
		return;
	}

	cv::Mat temp_frame;
	
	while (is_running) {
		cap >> temp_frame;
		if (temp_frame.empty()) break;

		// Thread-safe queue access
		{
			std::lock_guard<std::mutex> lock(queue_mutex);
			if (frame_queue.size() > 5) {
				frame_queue.pop();
			}
			frame_queue.push(temp_frame.clone());
		}

		// Sleep 1ms to let the CPU breadth
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

// Start the system threads
void SentinelSystem::start() {
	if (is_running) return;
	is_running = true;

	// Launch the Producer Thread (Camera)
	capture_thread = std::thread(&SentinelSystem::captureLoop, this);

	// Run the Consumer Loop (Processing) on the main thread
	processLoop();
}


// Shutdown Mechanism
void SentinelSystem::stop() {
	is_running = false;
	if (capture_thread.joinable()) {
		capture_thread.join();
	}
}

// Consumer Logic (Processing)
void SentinelSystem::processLoop() {
	cv::Mat frame, gray, last_gray, diff, thresh;
	auto last_motion_time = std::chrono::steady_clock::now();
	const int record_buffer_ms = 5000; // 5 seconds hysteresis buffer

	while (is_running) {
		bool is_frame_found = false;

		// Fetch frame from shared queue
		{
			std::lock_guard<std::mutex> lock(queue_mutex);
			if (!frame_queue.empty()) {
				frame = frame_queue.front();
				frame_queue.pop();
				is_frame_found = true;
			}
		}

		if (!is_frame_found) {
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
			continue;
		}

		// --- VISION LOGIC ---S
		cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
		cv::GaussianBlur(gray, gray, cv::Size(13, 13), 0); // Reduce noise

		if (last_gray.empty()) {
			last_gray = gray.clone();
			continue;
		}

		// Calculate frame difference
		cv::absdiff(last_gray, gray, diff);
		cv::threshold(diff, thresh, 18, 255, cv::THRESH_BINARY);
		cv::dilate(thresh, thresh, cv::Mat(), cv::Point(-1, -1), 2);

		// ---- RECORDING LOGIC -----
		int motion_score = cv::countNonZero(thresh);
		auto current_time = std::chrono::steady_clock::now();
		
		// If motion detected, reset the timer
		if (motion_score > 500) {
			last_motion_time = current_time;
		}

		// Calculate how much time has passed since last motion
		auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - last_motion_time).count();
		
		// Check if we are within the recording window (Active or Buffering)
		if (elapsed_ms < record_buffer_ms) {

			// Lazy initialization of the video writer
			if (!writer.isOpened()) {
				auto now = std::chrono::system_clock::now();
				std::time_t time_now = std::chrono::system_clock::to_time_t(now);

				struct tm time_info;
				localtime_s(&time_info, &time_now);

				std::stringstream ss;
				ss << "evidence_" << std::put_time(&time_info, "%H-%M-%S") << ".avi";

				std::string filename = ss.str();
								
				writer.open(filename, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 30, frame.size(), true);
				std::cout << "[REC] Recording Started..." << filename << std::endl;
			}

			writer.write(frame);

			// Shows different text depending on state
			if (elapsed_ms < 1000) {
				// Active Motion (Red Dot)
				cv::circle(frame, cv::Point(30, 30), 10, cv::Scalar(0, 0, 255), -1);
				cv::putText(frame, "MOTION DETECTED", cv::Point(50, 40), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 255), 2);
			}
			else {
				// Buffering State, no motion but still recording for buffer time
				cv::circle(frame, cv::Point(30, 30), 10, cv::Scalar(0, 255, 255), -1);
				cv::putText(frame, "BUFFERING...", cv::Point(50, 40), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 255), 2);
			}
		}
		else {
			// Timer expired, save and close file
			if (writer.isOpened()) {
				writer.release();
				std::cout << "[STOP] Recording Saved." << std::endl;
			}
		}

		cv::imshow("Sentinel Live", frame);
		//cv::imshow("Sentinel Brain", thresh);

		last_gray = gray.clone();

		if (cv::waitKey(30) == 'q') {
			stop();
			break;
		}
	}

	// Cleanup on exit
	if (writer.isOpened()) {
		writer.release(); // Save and close the file properly
	}
	cv::destroyAllWindows();
}
