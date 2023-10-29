#pragma once

#include <QWidget>
#include <QBuffer>
#include <QAudioFormat>
#include <QAudioOutput>
#include <qthread.h>
#include <thread>
#include <mutex>
#include <deque>
#include <memory>
#include "XFFmpeg.h"
#include "ui_AudioPlayWidget.h"

class AudioPlayWidget : public QWidget
{
	Q_OBJECT

public:
	AudioPlayWidget(QWidget *parent = nullptr);
	~AudioPlayWidget();

	void Init();
	void AddData(int avtype, void* data, int len);

	void OnAudioWorker();

private slots:
	void OnStateChanged(QAudio::State);

private:
	Ui::AudioPlayWidgetClass ui;

	XFFmpeg* m_ffmpeg = nullptr;
	bool m_running = true;
	std::shared_ptr<QBuffer>       m_audioBuffer = nullptr;
	std::shared_ptr<QByteArray>    m_audioBytes = nullptr;
	std::shared_ptr<QAudioOutput>  m_audioOut = nullptr;
	QIODevice*                     m_deviceIO = nullptr;
	std::thread m_thread;

	typedef std::lock_guard<std::recursive_mutex> RecursiveGuard;
	std::recursive_mutex m_recursive_mutex;

	typedef std::deque<std::string> AudioDataDeque;
	AudioDataDeque m_dequeAudioData;
};
