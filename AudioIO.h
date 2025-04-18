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

	static AudioIO& GetInstance() // �̱���
	{
		static AudioIO instance;
		return instance;
	}

    // ���� ����
    AudioIO(const AudioIO&) = delete;
    void operator=(const AudioIO&) = delete;

    bool Init();
    void StartTest(); //���� �׽�Ʈ
    void Terminate();

    bool StartCapture();   // ����ũ �Է� ���� (UDP �۽�)
    bool StopCapture();

    bool StartPlayback();  // ���ŵ� ���� ��� (UDP ����)
    bool StopPlayback();

    void SetMicMuted(bool muted) { micMuted = !muted; }
    void SetHeadsetMuted(bool muted) { headsetMuted = !muted; }

    void InitializeBuffers(); // ���� �ʱ�ȭ �Լ�
    void Cleanup();           // �ڿ� ���� �Լ�

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

    float threshold = 0.1f; // ����ũ ����
};