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

    QAudioFormat fmt;                             //ͨ��fmt�趨��Ƶ���ݸ�ʽ��ֻ����ȷ֪����Ƶ���ݵ��������������ʡ�����λ�����ſ��������ز���
    fmt.setSampleRate(48000);                     //�趨���Ų���Ƶ��Ϊ48000Hz����Ƶ����
    fmt.setSampleSize(16);                        //�趨���Ų�����ʽ������λ����Ϊ16λ(bit)����Ƶ�ļ���QAudioFormat֧�ֵ���8/16bit���������������Ϊ256/64k���ȼ�
    fmt.setChannelCount(2);                       //�趨����������ĿΪ2ͨ����������������Ƶ�ļ���mono(ƽ����)��������Ŀ��1��stero(������)��������Ŀ��2
    fmt.setCodec("audio/pcm");                    //����PCM���ݣ������������ñ�����Ϊ"audio/pcm"��"audio/pcm"�����е�ƽ̨��֧�֣�Ҳ���൱����Ƶ��ʽ��WAV,�����Է�ʽ��ѹ���ļ�¼��׽�������ݡ�����ʹ�����������ʽ ������ͨ��QAudioDeviceInfo::supportedCodecs()����ȡ��ǰƽ̨֧�ֵı����ʽ
    fmt.setByteOrder(QAudioFormat::LittleEndian); //�趨�ֽ�����С��ģʽ������Ƶ�ļ�
    fmt.setSampleType(QAudioFormat::UnSignedInt); //�趨�������͡����ݲ���λ�����趨������λ��Ϊ8��16λ������ΪQAudioFormat::UnSignedInt
    m_audioOut.reset(new QAudioOutput(fmt));
    m_deviceIO = m_audioOut->start();             //����start�����󣬷���QIODevice����ĵ�ַ
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
        //��ȡ��Ƶ��Ϣ
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

        //����buff
        QByteArray byte = (char*)audioData.get();
        m_audioBuffer->open(QIODevice::ReadWrite);
        m_audioBuffer->write(byte);
        m_audioBuffer->seek(0);

        //��ȡbuff��������
        while (!m_audioBuffer->atEnd())
        {
            memset(buffer.get(), 0x00, size);
            //�����������޿���ʱ��д���ݣ�����
            if (0 == m_audioOut->bytesFree())                  
            {
                continue;
            }

            //�ӻ���ݶ�ȡ���� �������ȡ���ļ�ĩβ���߶�ȡ���ɹ���ͨ��break��������whileѭ��
            if (m_audioBuffer->read(buffer.get(), size) <= 0)  
            {
                break;
            }

            // ����write�������ڴ�buf�е�PCM����д�뵽������, ����buf�е������ύ����������
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
