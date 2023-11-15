#ifndef _RTP_UNPACK_DEFINE_
#define _RTP_UNPACK_DEFINE_

#define RTP_UNPACK_SIZE (64 * 1024)

typedef struct RtpPayloadParam
{
	int payload;
	const char* encoding;
	FILE* frtp;
	FILE* fout;
	void* decoder;
	size_t size;
	uint8_t packet[RTP_UNPACK_SIZE];

	RtpPayloadParam()
	{
		payload = 0;
		encoding = nullptr;
		frtp = nullptr;
		fout = nullptr;
		decoder = nullptr;
		size = 0;
	}
}RtpPayloadParam;



#endif
