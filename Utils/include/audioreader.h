#ifndef AUDIOREADER_H
#define AUDIOREADER_H

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>

	// Definitions
	#define STREAM_LIMIT			 24
	#define EXTEND_BUFFER_IN_SEC	  1
	
	struct AudioReader_s
	{
	#ifdef _WIN32
		char fullpath[FILENAME_MAX];
	#elif __linux__
		char fullpath[PATH_MAX];
	#endif // WIN32		
		int nStream;
		int *audioStreamIndexes;
		AVFormatContext* formatContext;
		AVCodecContext** codecContexts;
		AVCodec** codecTypes;
	};
	typedef struct AudioReader_s AudioReader;	
	extern AudioReader AudioReader_Default;

	struct StreamData_s
	{
		int nChannel;
		int dataLen;
		int samplingFreq;
		double **channelData;
	};	
	typedef struct StreamData_s StreamData;

	struct AudioData_s
	{
		int nStream;
		StreamData *data;
	};
	typedef struct AudioData_s AudioData;
	extern AudioData AudioData_Default;

	// Allocations
	int initAudioReaderStruct(char* filename, AudioReader* ctx);
	int allocateAudioData(AudioData *audio, AudioReader ctx);

	// Functions	
	int* findAudioStreams(AVFormatContext* formatCtx, int* n);
	int readAudioFile(AudioReader *ctx, AudioData *data);
	double** getDataPointers(AudioData *data, int *len);
	int receivePackets(AudioReader *ctx, int idx, AudioData *data, int *currPos, AVFrame *frame);

	// Deallocations
	void deinitAudioReaderStruct(AudioReader *ctx);
	void deallocAudioData(AudioData *audio);

	// Debug functions
	void printStreamInformation(const AVFormatContext *ctx, const AVCodec* codec, const AVCodecContext* codecCtx, int audioStreamIndex);
	char* currentDateTime();

#ifdef __cplusplus
}
#endif

#endif