#include "AudioIO.h"
#include "ChatClient.h"
#include "VoiceChannelManager.h"

//방향	                    IP	                포트	            설명
//클라이언트 → 서버	        서버 IP	            50505	        마이크 입력 전송
//서버 → 클라이언트	        클라이언트 IP	    50506	        스피커 출력용 수신
static int passthroughCallback(const void* inputBuffer, void* outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData)
{
    const SAMPLE* in = (const SAMPLE*)inputBuffer;
    SAMPLE* out = (SAMPLE*)outputBuffer;

    if (inputBuffer == nullptr) {
        for (unsigned int i = 0; i < framesPerBuffer; ++i) {
            *out++ = 0;
        }
    }
    else {
        for (unsigned int i = 0; i < framesPerBuffer; ++i) {
            *out++ = *in++;
        }
    }

    return paContinue;
}

AudioIO::~AudioIO() 
{
    Terminate();
}

bool AudioIO::Init()
{
    PaError err = Pa_Initialize();
    if (err != paNoError)
    {
        //std::cerr << "PortAudio 초기화 실패: " << Pa_GetErrorText(err) << std::endl;
        OutputDebugStringA(("PortAudio 초기화 실패\n"));

        return false;
    }

    // 여기선 아무 스트림도 열지 않음 (StartCapture, StartPlayback에서 열도록 함)
    return true;
}

void AudioIO::StartTest()
{
    // 여기엔 일단 간단한 PassThrough 오디오 스트림 테스트 코드 들어갈 예정
    std::cout << "StartTest" << std::endl;
    PaError err;

    err = Pa_OpenDefaultStream(&inputStream,
        NUM_CHANNELS,         // 입력 채널 수
        NUM_CHANNELS,         // 출력 채널 수
        SAMPLE_FORMAT,        // 샘플 포맷
        SAMPLE_RATE,
        FRAMES_PER_BUFFER,
        passthroughCallback,  // 콜백 함수
        nullptr);             // userData

    if (err != paNoError) {
        std::cerr << "Stream 열기 실패: " << Pa_GetErrorText(err) << std::endl;

        return;
    }

    err = Pa_StartStream(inputStream);
    if (err != paNoError) {
        std::cerr << "Stream 시작 실패: " << Pa_GetErrorText(err) << std::endl;

        return;
    }

    std::cout << "[AudioIO] 마이크 입력 → 스피커 출력 테스트 중... (5초간)\n";

    std::this_thread::sleep_for(std::chrono::seconds(5));

    Pa_StopStream(inputStream);
    Pa_CloseStream(inputStream);
}

void AudioIO::Terminate() 
{
    if (inputStream)
    {
        Pa_StopStream(inputStream);
        Pa_CloseStream(inputStream);
        inputStream = nullptr;
    }

    if (outputStream)
    {
        Pa_StopStream(outputStream);
        Pa_CloseStream(outputStream);
        outputStream = nullptr;
    }

    Pa_Terminate();
}

// UDP 송신 함수
bool AudioIO::StartCapture()
{
    if (inputStream == nullptr)
    {
        PaError err = Pa_OpenDefaultStream(&inputStream,
            1, 0, SAMPLE_FORMAT, SAMPLE_RATE, FRAMES_PER_BUFFER, nullptr, nullptr);
        if (err != paNoError)
        {
            std::string msg = "입력 스트림 열기 실패: ";
            msg += Pa_GetErrorText(err);
            msg += "\n";  
            OutputDebugStringA(msg.c_str());

            return false;
        }

        Pa_StartStream(inputStream);
    }

    if (capturing) return false; // 이미 실행 중이면 무시
    capturing = true;

    captureThread = std::thread([this]() 
        {
        SOCKET udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
        if (udpSocket == INVALID_SOCKET)
        {
            OutputDebugStringA("UDP 소켓 생성 실패!\n");

            capturing = false;
            return;
        }

        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(50507);

        std::string serverIP = ChatClient::GetInstance().DiscoverServerIP();
        inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr);


        std::string roomId = VoiceChannelManager::GetInstance().GetCurrentVoiceRoomId();
        std::string sender = ChatClient::GetInstance().GetNickname();
        std::string header = "AUDIO:" + roomId + ":" + sender + ":";

        size_t headerLen = header.size();
        size_t audioLen = sizeof(inputBuffer);  // FRAMES_PER_BUFFER * sizeof(sample) 오디오 데이터 크기

        std::vector<char> packet(headerLen + audioLen); //패킷 버퍼 할당
        memcpy(packet.data(), header.data(), header.size()); // 헤더 복사

        while (capturing) 
        {
            PaError err = Pa_ReadStream(inputStream, inputBuffer, FRAMES_PER_BUFFER);
            if (err != paNoError)
            {
                std::string msg = "오디오 읽기 실패: ";
                msg += Pa_GetErrorText(err);
                msg += "\n";
                OutputDebugStringA(msg.c_str());
                continue;
            }

            // 현재 버퍼의 최대 진폭 계산
            float maxAmp = 0.0f;
            for (int i = 0; i < FRAMES_PER_BUFFER; ++i) 
            {
                float a = fabsf(inputBuffer[i]);
                if (a > maxAmp) maxAmp = a;
            }
            // 16‑bit 기준 정규화
            float norm = maxAmp / 32768.0f;



            //// 샘플 값 디버그
            //std::ostringstream oss;
            //oss << "[Capture] Sample[0~4]: ";
            //for (int i = 0; i < 5; ++i) {
            //    oss << inputBuffer[i] << " ";
            //}
            //oss << "\n";
            //OutputDebugStringA(oss.str().c_str());

            if (micMuted)    // 마이크 꺼져 있으면 전송 안함
                continue;

            // 설정된 임계값(threshold) 보다 작으면 무음으로보내기
            if (norm < threshold)
            {
                memset(packet.data() + header.size(), 0, audioLen);
            }
            else
            {
                memcpy(packet.data() + headerLen, reinterpret_cast<const char*>(inputBuffer), audioLen); // 오디오 데이터 복사                
            }            
            sendto(udpSocket, packet.data(), packet.size(), 0, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)); // UDP 전송

        }

        closesocket(udpSocket);
        });

    return true;
}

bool AudioIO::StopCapture()
{
    if (!capturing)
        return false;

    // 1) 캡처 루프에 탈출 신호 주기
    capturing = false;

    // 2) 스레드가 안전하게 종료될 때까지 대기
    if (captureThread.joinable())
        captureThread.join();

    // 3) PortAudio 스트림 닫기
    if (inputStream)
    {
        Pa_StopStream(inputStream);
        Pa_CloseStream(inputStream);
        inputStream = nullptr;
    }

    return true;
}

bool AudioIO::StartPlayback()
{
    if (outputStream == nullptr)
    {
        PaError err = Pa_OpenDefaultStream(&outputStream,
            0, 1, SAMPLE_FORMAT, SAMPLE_RATE, FRAMES_PER_BUFFER, nullptr, nullptr);
        if (err != paNoError)
        {
            std::string msg = "출력 스트림 열기 실패: ";
            msg += Pa_GetErrorText(err);
            msg += "\n";
            OutputDebugStringA(msg.c_str());

            return false;
        }

        Pa_StartStream(outputStream);
    }

    if (playing) return false;  // 중복 방지
    playing = true;

    playbackThread = std::thread([this]()
        {
        OutputDebugStringA("[AudioIO] StartPlayback 스레드 시작됨\n");


        // UDP 수신용 소켓
        playbackSocket = socket(AF_INET, SOCK_DGRAM, 0);
        if (playbackSocket == INVALID_SOCKET)
        {
            OutputDebugStringA("UDP 소켓 생성 실패!\n");

            playing = false;
            return;
        }

        // 바인딩할 포트 (예: 클라이언트 수신용 50506)
        sockaddr_in localAddr{};
        localAddr.sin_family = AF_INET;
        localAddr.sin_port = htons(50506);  // 클라이언트 수신용 포트
        localAddr.sin_addr.s_addr = INADDR_ANY;

        if (bind(playbackSocket, (sockaddr*)&localAddr, sizeof(localAddr)) == SOCKET_ERROR)
        {
            OutputDebugStringA("UDP 바인딩 실패!\n");

            closesocket(playbackSocket);
            playing = false;
            return;
        }

        // 디버그용
        sockaddr_in locals;
        int len = sizeof(locals);
        getsockname(playbackSocket, (sockaddr*)&locals, &len);

        //
        char buf[128];
        _snprintf_s(buf, sizeof(buf), _TRUNCATE,
            "Client bound to %s:%u\n",
            inet_ntoa(locals.sin_addr),
            ntohs(locals.sin_port));
        OutputDebugStringA(buf);
        //

        // non-blocking 모드 설정
        //u_long mode = 1;
        //ioctlsocket(udpSocket, FIONBIO, &mode);
         
        // 수신 전용 큰 바이트 버퍼
        constexpr int MAX_PACKET = 65507;
        std::vector<char> packetBuf(MAX_PACKET);
        SAMPLE outputBuffer[FRAMES_PER_BUFFER];

        while (playing)
        {
            //sockaddr_in fromAddr{};
            //int fromLen = sizeof(fromAddr);
            int rec = recvfrom(playbackSocket, packetBuf.data(), MAX_PACKET, 0,
                nullptr, nullptr);

            if (rec == SOCKET_ERROR) {
                int err = WSAGetLastError();

                // 에러 로깅 후 재시도
                char buf[128];
                _snprintf_s(buf, sizeof(buf), _TRUNCATE,
                    "[AudioIO] recvfrom err=%d\n", err);
                OutputDebugStringA(buf);
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }

            //if (rec <= 0)
            //{
            //    std::this_thread::sleep_for(std::chrono::milliseconds(1));
            //    continue;
            //}
            
             // “sender:…” 에서 ':' 위치 찾기
            std::string hdr(packetBuf.data(), rec);
            auto p1 = hdr.find(':');
            if (p1 == std::string::npos) continue;

            // sender·PCM 분리
            std::string sender = hdr.substr(0, p1);
            int pcmOffset = p1 + 1;

            if (rec > 0)
            {
                // pcmLen은 FRAMES_PER_BUFFER*sizeof(SAMPLE) 이라고 가정
                std::memcpy(outputBuffer,
                    packetBuf.data() + pcmOffset,
                    FRAMES_PER_BUFFER * sizeof(SAMPLE));
            }
            else
            {
                // 네트워크에서 데이터가 안 왔거나 에러면 → 무음 채우기
                std::memcpy(outputBuffer,
                    silenceBuffer,
                    FRAMES_PER_BUFFER * sizeof(SAMPLE));
            }

            bool isSilent = true;
            for (int i = 0; i < FRAMES_PER_BUFFER; ++i) {
                if (outputBuffer[i] != 0) {
                    isSilent = false;
                    break;
                }
            }    

            std::string roomId = VoiceChannelManager::GetInstance().GetCurrentVoiceRoomId();
            float vol = VoiceChannelManager::GetInstance()
                .GetParticipantVolume(roomId, sender);
            bool mute = VoiceChannelManager::GetInstance()
                .IsParticipantMuted(roomId, sender);

            // 2) 볼륨·뮤트 적용
            if (mute) {
                // 완전 무음
                std::memset(outputBuffer, 0, FRAMES_PER_BUFFER * sizeof(SAMPLE));
            }
            else if (vol != 1.0f) {
                // 샘플마다 곱해 줍니다
                for (int i = 0; i < FRAMES_PER_BUFFER; ++i) {
                    outputBuffer[i] = static_cast<SAMPLE>(outputBuffer[i] * vol);
                }
            }

            // 재생
            if (!headsetMuted)
            {
                PaError err = Pa_WriteStream(
                    outputStream,
                    outputBuffer,
                    FRAMES_PER_BUFFER);
                if (err == paOutputUnderflowed)
                {
                    // 언더플로우 났을 땐 바로 무음으로 채움
                    Pa_WriteStream(outputStream,
                        silenceBuffer,
                        FRAMES_PER_BUFFER);
                }
                else if (err != paNoError)
                {
                    char buf[256];
                    _snprintf_s(buf, sizeof(buf), _TRUNCATE,
                        "[AudioIO] Pa_WriteStream failed: %s (%d)\n",
                        Pa_GetErrorText(err), err);
                    OutputDebugStringA(buf);
                }
            }

            // 무음이 아닐때, UI에 “sender가 말하고 있다” 바로 알리기
            if (!isSilent)
            {
                auto frame = VoiceChannelManager::GetInstance().GetCurrentVoiceFrame();
                if (frame && frame->IsReady()) {
                    frame->CallAfter([=]() {
                        frame->HighlightVoiceUser(sender);
                        });
                }
            }

            


            // 너무 CPU 안 잡아먹게 잠깐 쉼
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }

        // 스레드 종료 직전에 소켓 닫기
        if (playbackSocket != INVALID_SOCKET)
        {
            closesocket(playbackSocket);
            playbackSocket = INVALID_SOCKET;
        }
        OutputDebugStringA("[AudioIO] Playback 스레드 종료됨\n");

        });
        // 오디오 쓰레드가 충분히 CPU 시간을 얻도록, 쓰레드 우선순위 높이기.
        SetThreadPriority(playbackThread.native_handle(), THREAD_PRIORITY_HIGHEST);

    return true;
}

bool AudioIO::StopPlayback()
{
    // 이미 playing=false 라면 스레드가 곧 빠져나올 것입니다
    playing = false;

    // recvfrom() 블로킹을 깨기 위해 소켓 강제 종료
    if (playbackSocket != INVALID_SOCKET)
    {
        closesocket(playbackSocket);
        playbackSocket = INVALID_SOCKET;
    }

    // 스레드가 종료될 때까지 기다리기
    if (playbackThread.joinable())
        playbackThread.join();

    // PortAudio 스트림 닫기
    if (outputStream)
    {
        Pa_StopStream(outputStream);
        Pa_CloseStream(outputStream);
        outputStream = nullptr;
    }

    OutputDebugStringA("[AudioIO] StopPlayback complete\n");
    return true;
}


void AudioIO::InitializeBuffers()
{
    std::memset(inputBuffer, 0, sizeof(float) * FRAMES_PER_BUFFER);  // inputBuffer 초기화
    std::memset(outputBuffer, 0, sizeof(float) * FRAMES_PER_BUFFER); // outputBuffer 초기화
}

void AudioIO::Cleanup()
{
    // PortAudio 스트림 정리
    if (inputStream) {
        Pa_StopStream(inputStream);
        Pa_CloseStream(inputStream);
    }
    if (outputStream) {
        Pa_StopStream(outputStream);
        Pa_CloseStream(outputStream);
    }
    Pa_Terminate();  // PortAudio 종료
}


