#include "AudioIO.h"
#include "ChatClient.h"
#include "VoiceChannelManager.h"

//����	                    IP	                ��Ʈ	            ����
//Ŭ���̾�Ʈ �� ����	        ���� IP	            50505	        ����ũ �Է� ����
//���� �� Ŭ���̾�Ʈ	        Ŭ���̾�Ʈ IP	    50506	        ����Ŀ ��¿� ����
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
        //std::cerr << "PortAudio �ʱ�ȭ ����: " << Pa_GetErrorText(err) << std::endl;
        OutputDebugStringA(("PortAudio �ʱ�ȭ ����\n"));

        return false;
    }

    // ���⼱ �ƹ� ��Ʈ���� ���� ���� (StartCapture, StartPlayback���� ������ ��)
    return true;
}

void AudioIO::StartTest()
{
    // ���⿣ �ϴ� ������ PassThrough ����� ��Ʈ�� �׽�Ʈ �ڵ� �� ����
    std::cout << "StartTest" << std::endl;
    PaError err;

    err = Pa_OpenDefaultStream(&inputStream,
        NUM_CHANNELS,         // �Է� ä�� ��
        NUM_CHANNELS,         // ��� ä�� ��
        SAMPLE_FORMAT,        // ���� ����
        SAMPLE_RATE,
        FRAMES_PER_BUFFER,
        passthroughCallback,  // �ݹ� �Լ�
        nullptr);             // userData

    if (err != paNoError) {
        std::cerr << "Stream ���� ����: " << Pa_GetErrorText(err) << std::endl;

        return;
    }

    err = Pa_StartStream(inputStream);
    if (err != paNoError) {
        std::cerr << "Stream ���� ����: " << Pa_GetErrorText(err) << std::endl;

        return;
    }

    std::cout << "[AudioIO] ����ũ �Է� �� ����Ŀ ��� �׽�Ʈ ��... (5�ʰ�)\n";

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

// UDP �۽� �Լ�
bool AudioIO::StartCapture()
{
    if (inputStream == nullptr)
    {
        PaError err = Pa_OpenDefaultStream(&inputStream,
            1, 0, SAMPLE_FORMAT, SAMPLE_RATE, FRAMES_PER_BUFFER, nullptr, nullptr);
        if (err != paNoError)
        {
            std::string msg = "�Է� ��Ʈ�� ���� ����: ";
            msg += Pa_GetErrorText(err);
            msg += "\n";  
            OutputDebugStringA(msg.c_str());

            return false;
        }

        Pa_StartStream(inputStream);
    }

    if (capturing) return false; // �̹� ���� ���̸� ����
    capturing = true;

    captureThread = std::thread([this]() 
        {
        SOCKET udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
        if (udpSocket == INVALID_SOCKET)
        {
            OutputDebugStringA("UDP ���� ���� ����!\n");

            capturing = false;
            return;
        }

        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(50507);

        std::string serverIP = ChatClient::GetInstance().DiscoverServerIP();
        inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr);

        while (capturing) 
        {
            PaError err = Pa_ReadStream(inputStream, inputBuffer, FRAMES_PER_BUFFER);
            if (err != paNoError)
            {
                std::string msg = "����� �б� ����: ";
                msg += Pa_GetErrorText(err);
                msg += "\n";
                OutputDebugStringA(msg.c_str());


                continue;
            }

            // ���� �� �����
            std::ostringstream oss;
            oss << "[Capture] Sample[0~4]: ";
            for (int i = 0; i < 5; ++i) {
                oss << inputBuffer[i] << " ";
            }
            oss << "\n";
            OutputDebugStringA(oss.str().c_str());


			std::string roomId = VoiceChannelManager::GetInstance().GetCurrentVoiceRoomId();
            std::string sender = ChatClient::GetInstance().GetNickname();
            std::string header = "AUDIO:" + roomId + ":" + sender + ":";

            size_t headerLen = header.size();
            size_t audioLen = sizeof(inputBuffer);  // FRAMES_PER_BUFFER * sizeof(sample) ����� ������ ũ��
            std::vector<char> packet(headerLen + audioLen); //��Ŷ ���� �Ҵ�
            memcpy(packet.data(), header.data(), headerLen); // ��� ����
            memcpy(packet.data() + headerLen,
                reinterpret_cast<const char*>(inputBuffer),
                audioLen); // ����� ������ ����

            if(micMuted) // ����ũ ���� ������ ����
                sendto(udpSocket,  packet.data(), packet.size(), 0, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)); // UDP ����

            //int sent = sendto(udpSocket, packet.data(), packet.size(), 0, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)); // UDP ����

            //if (sent == SOCKET_ERROR)
            //{
            //    int err = WSAGetLastError();
            //    std::string msg = "[Capture] sendto error: ";
            //    msg += std::to_string(err);
            //    msg += "\n";
            //    OutputDebugStringA(msg.c_str());
            //}
            //else
            //{
            //    std::string msg = "[Capture] sendto OK, bytes=";
            //    msg += std::to_string(sent);
            //    msg += "\n";
            //    //OutputDebugStringA(msg.c_str());
            //}
        }

        closesocket(udpSocket);
        });

    return true;
}

bool AudioIO::StopCapture()
{
    if (!capturing)
        return false;

    // 1) ĸó ������ Ż�� ��ȣ �ֱ�
    capturing = false;

    // 2) �����尡 �����ϰ� ����� ������ ���
    if (captureThread.joinable())
        captureThread.join();

    // 3) PortAudio ��Ʈ�� �ݱ�
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
            std::string msg = "��� ��Ʈ�� ���� ����: ";
            msg += Pa_GetErrorText(err);
            msg += "\n";
            OutputDebugStringA(msg.c_str());

            return false;
        }

        Pa_StartStream(outputStream);
    }

    if (playing) return false;  // �ߺ� ����
    playing = true;

    playbackThread = std::thread([this]()
        {
        OutputDebugStringA("[AudioIO] StartPlayback ������ ���۵�\n");


        // UDP ���ſ� ����
        playbackSocket = socket(AF_INET, SOCK_DGRAM, 0);
        if (playbackSocket == INVALID_SOCKET)
        {
            OutputDebugStringA("UDP ���� ���� ����!\n");

            playing = false;
            return;
        }

        // ���ε��� ��Ʈ (��: Ŭ���̾�Ʈ ���ſ� 50506)
        sockaddr_in localAddr{};
        localAddr.sin_family = AF_INET;
        localAddr.sin_port = htons(50506);  // Ŭ���̾�Ʈ ���ſ� ��Ʈ
        localAddr.sin_addr.s_addr = INADDR_ANY;

        if (bind(playbackSocket, (sockaddr*)&localAddr, sizeof(localAddr)) == SOCKET_ERROR)
        {
            OutputDebugStringA("UDP ���ε� ����!\n");

            closesocket(playbackSocket);
            playing = false;
            return;
        }

        // ����׿�
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

        // non-blocking ��� ����
        //u_long mode = 1;
        //ioctlsocket(udpSocket, FIONBIO, &mode);

        while (playing)
        {
            sockaddr_in fromAddr{};
            int fromLen = sizeof(fromAddr);
            int bytesReceived = recvfrom(playbackSocket, (char*)outputBuffer, sizeof(outputBuffer), 0,
                (sockaddr*)&fromAddr, &fromLen);

            if (bytesReceived > 0)
            {
                int frames = bytesReceived / sizeof(SAMPLE);
                if(headsetMuted) //���� ���� ������ ���
                    Pa_WriteStream(outputStream, outputBuffer, frames);
                //PaError err = Pa_WriteStream(outputStream, outputBuffer, FRAMES_PER_BUFFER);
                //if (err != paNoError)
                //{
                //    OutputDebugStringA("[AudioIO] Pa_WriteStream ����\n");
                //}
            }

            // �ʹ� CPU �� ��Ƹ԰� ��� ��
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        // ������ ���� ������ ���� �ݱ�
        if (playbackSocket != INVALID_SOCKET)
        {
            closesocket(playbackSocket);
            playbackSocket = INVALID_SOCKET;
        }
        OutputDebugStringA("[AudioIO] Playback ������ �����\n");

        });

    return true;
}

bool AudioIO::StopPlayback()
{
    // �̹� playing=false ��� �����尡 �� �������� ���Դϴ�
    playing = false;

    // recvfrom() ���ŷ�� ���� ���� ���� ���� ����
    if (playbackSocket != INVALID_SOCKET)
    {
        closesocket(playbackSocket);
        playbackSocket = INVALID_SOCKET;
    }

    // �����尡 ����� ������ ��ٸ���
    if (playbackThread.joinable())
        playbackThread.join();

    // PortAudio ��Ʈ�� �ݱ�
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
    std::memset(inputBuffer, 0, sizeof(float) * FRAMES_PER_BUFFER);  // inputBuffer �ʱ�ȭ
    std::memset(outputBuffer, 0, sizeof(float) * FRAMES_PER_BUFFER); // outputBuffer �ʱ�ȭ
}

void AudioIO::Cleanup()
{
    // PortAudio ��Ʈ�� ����
    if (inputStream) {
        Pa_StopStream(inputStream);
        Pa_CloseStream(inputStream);
    }
    if (outputStream) {
        Pa_StopStream(outputStream);
        Pa_CloseStream(outputStream);
    }
    Pa_Terminate();  // PortAudio ����
}


