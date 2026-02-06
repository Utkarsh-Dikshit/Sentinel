#pragma once
#include <opencv2/opencv.hpp>
#include <thread>
#include <mutex>
#include <queue>
#include <atomic>
#include <chrono>
#include <string>
#include <sstream>
#include <iomanip>

class SentinelSystem {
public:
	SentinelSystem(int id = 0);
	~SentinelSystem();

	void start();
	void stop();
	void processLoop();

private:
	int camera_id;
	std::queue<cv::Mat> frame_queue;
	std::mutex queue_mutex;
	std::atomic<bool> is_running;
	std::thread capture_thread;

	cv::VideoWriter writer;

	// Helper Method
	void captureLoop();
};