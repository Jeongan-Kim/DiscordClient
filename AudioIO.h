#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <portaudio.h>
#include <cstdint>
#include <string>
#include <iostream>
#include <Winsock2.h>
#include <thread>
#include <chrono>
#include <ws2tcpip.h>

#define SAMPLE_RATE       (48000)
#define FRAMES_PER_BUFFER (1024) 
#define NUM_CHANNELS      (1)
#define SAMPLE_FORMAT     paInt16

typedef short SAMPLE;

class AudioIO 
{
public:
    ~AudioIO();

	static AudioIO& GetInstance() // 싱글톤
	{
		static AudioIO instance;
		return instance;
	}

    // 복사 방지
    AudioIO(const AudioIO&) = delete;
    void operator=(const AudioIO&) = delete;

    bool Init();
    void StartTest(); //에코 테스트
    void Terminate();

    bool StartCapture();   // 마이크 입력 시작 (UDP 송신)
    bool StopCapture();

    bool StartPlayback();  // 수신된 음성 재생 (UDP 수신)
    bool StopPlayback();

    void SetMicMuted(bool muted) { micMuted = !muted; }
    void SetHeadsetMuted(bool muted) { headsetMuted = !muted; }

    void InitializeBuffers(); // 버퍼 초기화 함수
    void Cleanup();           // 자원 정리 함수

    //bool readMic(int16_t* buffer, int frameCount);
    //bool playSpeaker(int16_t* buffer, int frameCount);

private:
    AudioIO() = default;


    std::thread captureThread;
    std::thread playbackThread;

    PaStream* inputStream = nullptr;
    PaStream* outputStream = nullptr;

    SAMPLE inputBuffer[FRAMES_PER_BUFFER];
    SAMPLE outputBuffer[FRAMES_PER_BUFFER];
    SAMPLE silenceBuffer[FRAMES_PER_BUFFER] = { 0 };

    std::atomic<bool> capturing = false;
    std::atomic<bool> playing = false;

    SOCKET playbackSocket = INVALID_SOCKET;

    bool micMuted;
    bool headsetMuted;

    float threshold = 0.1f; // 마이크 감도
};