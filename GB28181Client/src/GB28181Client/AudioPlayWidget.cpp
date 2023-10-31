#include "AudioPlayWidget.h"

static int AudioPlayThread(void* param)
{
    AudioPlayWidget* p = (AudioPlayWidget*)param;
    if (p)
    {
        p->OnAudioWorker();
    }
    return 0;
}

static void AudioDataCB(const void* data, int datalen, void* userData)
{
    assert(userData);
    AudioPlayWidget* wgt = (AudioPlayWidget*)userData;
    wgt->Play(data, datalen);
}

AudioPlayWidget::AudioPlayWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
}

AudioPlayWidget::~AudioPlayWidget()
{
}

void AudioPlayWidget::Init()
{
    m_audioBytes.reset(new QByteArray);
    m_audioBuffer.reset(new QBuffer(m_audioBytes.get()));

    QAudioFormat fmt;                             //通过fmt设定音频数据格式。只有明确知道音频数据的声道数、采样率、采样位数，才可以正常地播放
    fmt.setSampleRate(48000);                     //设定播放采样频率为48000Hz的音频数据
    fmt.setSampleSize(16);                        //设定播放采样格式（采样位数）为16位(bit)的音频文件。QAudioFormat支持的有8/16bit，即将声音振幅化为256/64k个等级
    fmt.setChannelCount(2);                       //设定播放声道数目为2通道（立体声）的音频文件。mono(平声道)的声道数目是1，stero(立体声)的声道数目是2
    fmt.setCodec("audio/pcm");                    //播放PCM数据（裸流）得设置编码器为"audio/pcm"。"audio/pcm"在所有的平台都支持，也就相当于音频格式的WAV,以线性方式无压缩的记录捕捉到的数据。如想使用其他编码格式 ，可以通过QAudioDeviceInfo::supportedCodecs()来获取当前平台支持的编码格式
    fmt.setByteOrder(QAudioFormat::LittleEndian); //设定字节序，以小端模式播放音频文件
    fmt.setSampleType(QAudioFormat::UnSignedInt); //设定采样类型。根据采样位数来设定。采样位数为8或16位则设置为QAudioFormat::UnSignedInt
    m_audioOut.reset(new QAudioOutput(fmt));
    m_deviceIO = m_audioOut->start();             //调用start函数后，返回QIODevice对象的地址
    //m_deviceIO->open(QIODevice::ReadWrite);
    connect(m_audioOut.get(), SIGNAL(stateChanged(QAudio::State)), this, SLOT(OnStateChanged(QAudio::State)));

    m_thread = std::thread(AudioPlayThread, this);
}

void AudioPlayWidget::Stop()
{
    m_running = false;
    if (m_thread.joinable())
        m_thread.join();

    if (m_audioOut)
    {
        m_audioOut->stop();
    }
}

void AudioPlayWidget::AddData(int avtype, void* data, int len)
{
	if (m_audioDecoder)
        m_audioDecoder->InputAudioData(avtype, data, len);
}

void AudioPlayWidget::Play(const void* data, int datalen)
{
    if(m_deviceIO)
        m_deviceIO->write((const char*)data, datalen);
}

void AudioPlayWidget::OnAudioWorker()
{
    m_audioDecoder = new(std::nothrow) CAudioDecoder(AudioDataCB, this);
    while (m_running)
    {
        QThread::sleep(1);
    }

    return;

#if 0
    int size = m_audioOut->periodSize();
    std::shared_ptr<char> buffer(new char[size], std::default_delete<char[]>());
    memset(buffer.get(), 0x00, size);

    std::shared_ptr<char> audioData(new char[AUDIO_STREAM_DATA_SZIE], std::default_delete<char[]>());
    memset(audioData.get(), 0x00, AUDIO_STREAM_DATA_SZIE);
    int datalen = 0;

    while (m_running)
    {
        //获取音频信息
        {
            RecursiveGuard mtx(m_recursive_mutex);
            if (m_audioDecoder)
            {
                //if (!m_audioDecoder->GetAudioData((unsigned char*)audioData.get(), &datalen))
                {
                    QThread::sleep(1);
                    continue;
                }
            }
        }

        if (!audioData || datalen < 0)
        {
			QThread::sleep(1);
			continue;
        }

        //放入buff
        QByteArray byte = (char*)audioData.get();
        m_audioBuffer->open(QIODevice::ReadWrite);
        m_audioBuffer->write(byte);
        m_audioBuffer->seek(0);

        //读取buff送入声卡
        while (!m_audioBuffer->atEnd())
        {
            memset(buffer.get(), 0x00, size);
            //声卡缓冲区无空闲时不写数据，跳过
            if (0 == m_audioOut->bytesFree())                  
            {
                continue;
            }

            //从缓冲据读取数据 ，如果读取到文件末尾或者读取不成功则通过break函数跳出while循环
            if (m_audioBuffer->read(buffer.get(), size) <= 0)  
            {
                break;
            }

            // 调用write函数将内存buf中的PCM数据写入到扬声器, 即把buf中的数据提交到声卡发声
            m_deviceIO->write(buffer.get(), size);
        }

        m_audioBuffer->close();
        memset(audioData.get(), 0x00, AUDIO_STREAM_DATA_SZIE);
        datalen = 0;
    }
#endif
}

void AudioPlayWidget::OnStateChanged(QAudio::State newState)
{
    switch (newState)
    {
    case QAudio::IdleState:
        break;
    case QAudio::StoppedState:
        break;
    case QAudio::SuspendedState:
        break;
    case QAudio::ActiveState:
        break;
    default:
        break;
    }
}
