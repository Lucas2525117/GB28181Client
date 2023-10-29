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
	m_ffmpeg = new(std::nothrow) XFFmpeg(nullptr, nullptr, this);

    m_audioBytes.reset(new QByteArray);
    m_audioBuffer.reset(new QBuffer(m_audioBytes.get()));

    QAudioFormat fmt;                             //ͨ��fmt�趨��Ƶ���ݸ�ʽ��ֻ����ȷ֪����Ƶ���ݵ��������������ʡ�����λ�����ſ��������ز���
    fmt.setSampleRate(44100);                     //�趨���Ų���Ƶ��Ϊ44100Hz����Ƶ�ļ�
    fmt.setSampleSize(16);                        //�趨���Ų�����ʽ������λ����Ϊ16λ(bit)����Ƶ�ļ���QAudioFormat֧�ֵ���8/16bit���������������Ϊ256/64k���ȼ�
    fmt.setChannelCount(1);                       //�趨����������ĿΪ2ͨ����������������Ƶ�ļ���mono(ƽ����)��������Ŀ��1��stero(������)��������Ŀ��2
    fmt.setCodec("audio/pcm");                    //����PCM���ݣ������������ñ�����Ϊ"audio/pcm"��"audio/pcm"�����е�ƽ̨��֧�֣�Ҳ���൱����Ƶ��ʽ��WAV,�����Է�ʽ��ѹ���ļ�¼��׽�������ݡ�����ʹ�����������ʽ ������ͨ��QAudioDeviceInfo::supportedCodecs()����ȡ��ǰƽ̨֧�ֵı����ʽ
    fmt.setByteOrder(QAudioFormat::LittleEndian); //�趨�ֽ�����С��ģʽ������Ƶ�ļ�
    fmt.setSampleType(QAudioFormat::UnSignedInt); //�趨�������͡����ݲ���λ�����趨������λ��Ϊ8��16λ������ΪQAudioFormat::UnSignedInt
    m_audioOut.reset(new QAudioOutput(fmt));
    m_deviceIO = m_audioOut->start();                     //����start�����󣬷���QIODevice����ĵ�ַ
    m_deviceIO->open(QIODevice::ReadWrite);
    connect(m_audioOut.get(), SIGNAL(stateChanged(QAudio::State)), this, SLOT(OnStateChanged(QAudio::State)));

    m_thread = std::thread(AudioPlayThread, this);
}

void AudioPlayWidget::AddData(int avtype, void* data, int len)
{
	if (m_ffmpeg)
		m_ffmpeg->Input(avtype, data, len);
}

void AudioPlayWidget::OnAudioWorker()
{
    int size = m_audioOut->periodSize();
    std::shared_ptr<char> buffer(new char[size], std::default_delete<char[]>());
    memset(buffer.get(), 0x00, size);

    while (m_running)
    {
        //��ȡ��Ƶ��Ϣ
        unsigned char* audioData = nullptr;
        int datalen = 0;
        {
            RecursiveGuard mtx(m_recursive_mutex);
            if (m_ffmpeg)
            {
                if (!m_ffmpeg->GetAudioData(audioData, &datalen))
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
        QByteArray byte = (char*)audioData;
        m_audioBuffer->open(QIODevice::ReadWrite);
        m_audioBuffer->write(byte);
        m_audioBuffer->seek(0);

        //��ȡbuff��������
        while (!m_audioBuffer->atEnd())
        {
            memset(buffer.get(), 0x00, size);
            if (0 == m_audioOut->bytesFree())                  //�����������޿���ʱ��д���ݣ�����
            {
                continue;
            }

            if (m_audioBuffer->read(buffer.get(), size) <= 0)  //�ӻ���ݶ�ȡ���� �������ȡ���ļ�ĩβ���߶�ȡ���ɹ���ͨ��break��������whileѭ��
            {
                break;
            }
            m_deviceIO->write(buffer.get(), size);             //����write�������ڴ�buf�е�PCM����д�뵽������,����buf�е������ύ����������
        }

        m_audioBuffer->close();
    }
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
