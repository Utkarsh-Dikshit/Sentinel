# 🛡️ Sentinel: High-Performance Multithreaded Security System

> A latency-optimized security application built with C++ and OpenCV, featuring a Producer-Consumer architecture for asynchronous video processing.

## 📖 Overview
**Sentinel** is a smart surveillance system designed to solve the "dropped frame" problem common in single-threaded security apps. By decoupling frame capture from image processing, Sentinel ensures **zero-latency monitoring** even during high-load I/O operations (like writing video evidence to disk).

It features a **Smart Hysteresis Recording System** that prevents file fragmentation by maintaining a recording buffer for 5 seconds after motion stops, mimicking professional DVR behavior.

## 🚀 Key Technical Features
* **Multithreaded Architecture:** Uses a **Producer-Consumer** pattern. Thread 1 captures hardware frames; Thread 2 handles Computer Vision (Gaussian Blur, Thresholding) and Disk I/O.
* **Memory Safety (RAII):** Implements `std::lock_guard` for deadlock prevention and a **Leaky Bucket** ring buffer to cap memory usage.
* **Smart Recording:** Uses `std::chrono` timers to manage state transitions (Active vs. Buffering) and generates timestamped `evidence_HH-MM-SS.avi` files.

## 🛠️ Tech Stack
- **Language:** C++20 (Modern C++)
- **Library:** OpenCV 4.12.0
- **Concurrency:** `std::thread`, `std::mutex`, `std::atomic`
- **IDE:** Visual Studio 2026

## 📂 Project Structure
```text
Sentinel/
├── SentinelSystem.h    # Header: Interface and Member declarations
├── SentinelSystem.cpp  # Source: Implementation of Threading & Vision Logic
└── main.cpp            # Entry Point: Instantiates the System
```
## 🔧 Build & Run

### 1. Prerequisites
* **Visual Studio 2026** (Desktop C++ Workload).
* **OpenCV 4.12.0** (Add `opencv\build\x64\vc16\bin` to System Path).

### 2. Setup
1.  **Clone:**
    ```bash
    git clone [https://github.com/Utkarsh-Dikshit/Sentinel.git](https://github.com/Utkarsh-Dikshit/Sentinel.git)
    ```
2.  **Link OpenCV:** Open `Sentinel.sln`. Right-click Project → **Properties**:
    * **Include Dir:** Point to `opencv\build\include`
    * **Library Dir:** Point to `opencv\build\x64\vc16\lib`
    * **Input:** Add `opencv_world4120.lib`

### 3. Execution
1.  **Build:** Set to **x64** (Debug or Release) and press **Ctrl+Shift+B**.
2.  **Run:** Press **F5**.
    * 🔴 **REC:** Motion Detected.
    * 🟡 **BUFFER:** 5s Cooldown (Hysteresis).
    * **Quit:** Press `q`.
