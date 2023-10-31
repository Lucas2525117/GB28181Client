#ifndef _STREAM_DEFINE_H_
#define _STREAM_DEFINE_H_

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

typedef struct AudioFrame
{
	unsigned char* data;
	int datalen;

	AudioFrame()
	{
		data = nullptr;
		datalen = 0;
	}
}AudioFrame;

static bool IsVideo(int avtype)
{
	switch (avtype)
	{
	case CODEC_VIDEO_MPEG4:
	case CODEC_VIDEO_H264:
	case CODEC_VIDEO_H265:
		return true;
	default:
		return false;
	}

	return false;
}

static bool IsAudio(int avtype)
{
	switch (avtype)
	{
	case CODEC_AUDIO_G711:
	case CODEC_AUDIO_G711U:
	case CODEC_AUDIO_PCM:
	case CODEC_AUDIO_AAC:
		return true;
	default:
		return false;
	}

	return false;
}

#endif