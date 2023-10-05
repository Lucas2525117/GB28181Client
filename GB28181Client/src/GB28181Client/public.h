#pragma once

#include <qstring.h>
#include <qstringlist.h>

enum NodeType
{
	TypeOrg = 0,			// 组织
	TypeDevice,             // 设备
	TypeChannel,            // 通道
};

typedef struct GBData
{
	int len;
	void* data;

	GBData()
	{
		len = 0;
		data = nullptr;
	}
}GBData;

// 编码格式
enum CodecType
{
	CODEC_UNDEFINE = 0,
	CODEC_RAW_BINARY = 1,

	// 视频
	CODEC_VIDEO_ANY = 0,
	CODEC_VIDEO_MPEG4 = 0x10,
	CODEC_VIDEO_H264 = 0x1b,
	CODEC_VIDEO_H265 = 0x24,
	CODEC_VIDEO_SVAC = 0x80,

	// 音频
	CODEC_AUDIO_ANY = 0,
	CODEC_AUDIO_G711 = 0x90,
	CODEC_AUDIO_G711U = 0x91,
	CODEC_AUDIO_G7221 = 0x92,
	CODEC_AUDIO_G7231 = 0x93,
	CODEC_AUDIO_G729 = 0x99,
	CODEC_AUDIO_SVAC = 0x9B,

	CODEC_AUDIO_MP3 = 0xA1,
	CODEC_AUDIO_AAC = 0xA2,
	CODEC_AUDIO_PCM = 0xA3,

	// 图片
	CODEC_PIC_ANY = 0,
	CODEC_PIC_BMP = 0xB1,
	CODEC_PIC_JPEG = 0xB2,
	CODEC_PIC_PNG = 0xB3,
	CODEC_PIC_GIF = 0xB4,

	// 文本消息
	CODEC_MSG_ANY = 0,
	CODEC_MSG_TXT = 0xC1,
	CODEC_MSG_JSON = 0xC2,
	CODEC_MSG_XML = 0xC3,
};

// 视频YUV帧数据
typedef struct YuvFrame
{
	int64_t pts;
	int w;           // 宽度
	int h;           // 高度
	unsigned int stride_y;   //
	unsigned int stride_uv;  //
	unsigned char* y;
	unsigned char* u;
	unsigned char* v;

	YuvFrame()
	{
		pts = 0;
		w = 0;
		h = 0;
		stride_y = 0;
		stride_uv = 0;
		y = nullptr;
		u = nullptr;
		v = nullptr;
	}
}YuvFrame;

static bool ipAddrIsOK(const QString& ip)
{
	if (ip.isEmpty())
	{
		return false;
	}

	QStringList list = ip.split('.');
	if (list.size() != 4)
	{
		return false;
	}

	for (const auto& num : list)
	{
		bool ok = false;
		int temp = num.toInt(&ok);
		if (!ok || temp < 0 || temp > 255)
		{
			return false;
		}
	}

	return true;
}

