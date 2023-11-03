#ifndef _STREAM_DEFINE_H_
#define _STREAM_DEFINE_H_

#include <memory>

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

// 数据包负载类型
enum LenDataType
{
	LDT_streamData = 0,
	LDT_streamDataMask = (1 << LDT_streamData),

	LDT_videoInfo = 1,
	LDT_videoInfoMask = (1 << LDT_videoInfo),

	LDT_audioInfo = 2,
	LDT_audioInfoMask = (1 << LDT_audioInfo),

	LDT_osdInfo = 3,
	LDT_osdInfoMask = (1 << LDT_osdInfo),

	LDT_iFrame = 15,
	LDT_iFrameMask = (1 << LDT_iFrame),

	LDT_numLenData = 15 // 最多可以拥有的LenData数
};
#ifndef __cplusplus
typedef enum LenDataType LenDataType;
#endif

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

#pragma pack(push, 1)

typedef unsigned char BYTE;

// 数据包
struct InPack
{
	int            totalLen;        // 数据包总长度，不包含本长度字段
	unsigned short mask;            // 标记位，描述帧类型及，后面包含那些数据段，
	unsigned char  codec;           // 编码格式
	unsigned int   seqNum;			// 包序号
	int64_t        pts;             // 时间戳，绝对时间
	
	int            dataLen;         // 数据长度，
	BYTE           data[4];         // 指向流数据的第一个字节
};
#ifndef __cplusplus
typedef enum InPack InPack;
#endif

#pragma pack(pop) // #pragma pack(push, 1)

// 封装InPack，主要设置是否I帧标记，计算总长度，如果存在其他附加信息，使用者自己设置，
inline InPack* EncodeStreamInPack(InPack* pack, bool isKeyFrame)
{
	int i;
	BYTE* p;
	if (nullptr == pack)
		return nullptr;

	// 设置I帧位
	if (isKeyFrame)
		pack->mask |= LDT_iFrameMask;
	pack->mask |= LDT_streamDataMask;

	pack->totalLen = sizeof(struct InPack) - 8; // totalLen+data=8
	p = (BYTE*)(&pack->dataLen);
	for (i = 0; i < LDT_numLenData; ++i)
	{
		if (pack->mask & (1 << i))
		{
			int diff;
			int len = *((int*)p);
			if (0 == len)
				break;

			len = (len + 3) / 4 * 4 + sizeof(int); // 4字节对齐+len字段长度
			pack->totalLen += len;
			p = p + len;
			// 防止越界
			diff = (int)(p - (BYTE*)pack);
			if (diff <= 0 || diff > (pack->totalLen + (int)sizeof(int)))
				return nullptr;
		}
	}

	memset(p, 0, sizeof(int));

	return pack;
}

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

typedef void(*EncoderDataCallBack)(InPack* data, void* userData);
typedef void(*PsDataCallBack)(void* data, int len, void* userData);

#endif