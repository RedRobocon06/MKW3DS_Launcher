#include "sound.h"
#include "main.h"

int parseInfoBlock(cwav_t* cwav) {
	u32 infoSize = cwav->cwavHeader->info_blck.size;
	cwav->cwavInfo = (cwavInfoBlock_t *)((u32)(cwav->fileBuf) + cwav->cwavHeader->info_blck.ref.offset);
	if (cwav->cwavInfo->header.magic != 0x4F464E49 || cwav->cwavInfo->header.size != infoSize) return 1; //magic INFO or size mismatch
	u32 channelcount = cwav->cwavInfo->channelInfoRefs.count;
	u32 encoding = cwav->cwavInfo->encoding;
	if (encoding != DSP_ADPCM) return 3; //Only DSP ADPCM supported for now
	cwav->channelInfos = malloc(4 * channelcount);
	if (!cwav->channelInfos) return 2;
	for (int i = 0; i < channelcount; i++) {
		cwavReference_t *currRef = &(cwav->cwavInfo->channelInfoRefs.references[i]);
		if (currRef->refType != CHANNEL_INFO) {
			return 3;
		}
		cwav->channelInfos[i] = (cwavchannelInfo_t*)((u8*)(&(cwav->cwavInfo->channelInfoRefs.count)) + currRef->offset);
		if (cwav->channelInfos[i]->samples.refType != SAMPLE_DATA) return 5;
	}
	cwav->DSPADPCMInfos = malloc(4 * channelcount);
	if (!cwav->DSPADPCMInfos) return 2;
	for (int i = 0; i < channelcount; i++) {
		if (cwav->channelInfos[i]->ADPCMInfo.refType != DSP_ADPCM_INFO) return 4;
		cwav->DSPADPCMInfos[i] = (cwavDSPADPCMInfo_t*)(cwav->channelInfos[i]->ADPCMInfo.offset + (u8*)(&(cwav->channelInfos[i]->samples)));
	}
	return 0;
}

void freeCwav(cwav_t* cwav) {
	if (!cwav) return;
	stopcwav(cwav);
	if (cwav->fileBuf) linearFree(cwav->fileBuf);
	if (cwav->channelInfos) free(cwav->channelInfos);
	if (cwav->DSPADPCMInfos) free(cwav->DSPADPCMInfos);
	linearFree(cwav);
}

/*void dumpCwavInfo(cwav_t* cwav, char* filename) {
	FILE* file = fopen("/cwavdump.txt", "w");
	char buf[100] = { 0 };
	sprintf(buf, "Filename: %s\n\n", filename);
	fwrite(buf, 1, strlen(buf), file);
	fseek(file, 0, SEEK_END);
	sprintf(buf, "Info block: Offset: 0x%08lX, Size: 0x%08lX\n", (u32)cwav->cwavInfo - (u32)cwav->fileBuf, cwav->cwavInfo->header.size);
	fwrite(buf, 1, strlen(buf), file);
	fseek(file, 0, SEEK_END);
	sprintf(buf, "Data block: Offset: 0x%08lX, Size: 0x%08lX\n\n", (u32)cwav->cwavData - (u32)cwav->fileBuf, cwav->cwavData->header.size);
	fwrite(buf, 1, strlen(buf), file);
	fseek(file, 0, SEEK_END);
	sprintf(buf, "Encoding: %d\n", cwav->cwavInfo->encoding);
	fwrite(buf, 1, strlen(buf), file);
	fseek(file, 0, SEEK_END);
	sprintf(buf, "Num samples: %ld\n", cwav->cwavInfo->LoopEnd);
	fwrite(buf, 1, strlen(buf), file);
	fseek(file, 0, SEEK_END);
	sprintf(buf, "Is looped: %d\n", cwav->cwavInfo->isLooped);
	fwrite(buf, 1, strlen(buf), file);
	fseek(file, 0, SEEK_END);
	sprintf(buf, "Sample rate: %ld\n", cwav->cwavInfo->sampleRate);
	fwrite(buf, 1, strlen(buf), file);
	fseek(file, 0, SEEK_END);
	sprintf(buf, "Channel count: %ld\n\n", cwav->cwavInfo->channelInfoRefs.count);
	fwrite(buf, 1, strlen(buf), file);
	fseek(file, 0, SEEK_END);
	for (int i = 0; i < cwav->cwavInfo->channelInfoRefs.count; i++) {
		sprintf(buf, "Channel id: %d\n", i);
		fwrite(buf, 1, strlen(buf), file);
		fseek(file, 0, SEEK_END);
		sprintf(buf, "Sample offset: 0x%08lX\n", (u32)((u32)cwav->channelInfos[i]->samples.offset + (u8*)(&(cwav->cwavData->data))) - (u32)(cwav->fileBuf));
		fwrite(buf, 1, strlen(buf), file);
		fseek(file, 0, SEEK_END);
		sprintf(buf, "ADPCM index: 0x%02X\n", cwav->DSPADPCMInfos[i]->context.predScale);
		fwrite(buf, 1, strlen(buf), file);
		fseek(file, 0, SEEK_END);
		sprintf(buf, "ADPCM history: 0x%08lX 0x%08lX\n", (u32)cwav->DSPADPCMInfos[i]->context.prevSample, (u32)cwav->DSPADPCMInfos[i]->context.secondPrevSample);
		fwrite(buf, 1, strlen(buf), file);
		fseek(file, 0, SEEK_END);
		sprintf(buf, "ADPCM params addr: 0x%08lX\n\n", (u32)&(cwav->DSPADPCMInfos[i]->param) - (u32)(cwav->fileBuf));
		fwrite(buf, 1, strlen(buf), file);
		fseek(file, 0, SEEK_END);
	}
}*/

u32 newCwav(char* filename, cwav_t** out) {
	cwav_t* cwav = NULL;
	FILE* file = NULL;
	file = fopen(filename, "rb");
	if (!file) return 1;
	cwav = linearAlloc(sizeof(cwav_t));
	if (!cwav) return 2;
	memset(cwav, 0, sizeof(cwav_t));
	fseek(file, 0, SEEK_END);
	u32 fileSize = ftell(file);
	cwav->fileBuf = linearAlloc(fileSize);
	if (!cwav->fileBuf) {
		freeCwav(cwav);
		return 3;
	}
	fseek(file, 0, SEEK_SET);
	fread(cwav->fileBuf, 1, fileSize, file);
	cwav->cwavHeader = cwav->fileBuf;
	if (cwav->cwavHeader->magic != 0x56415743 || cwav->cwavHeader->endian != 0xFEFF || cwav->cwavHeader->version != 0x02010000 || cwav->cwavHeader->blockCount != 2) {
		freeCwav(cwav);
		return 4;
	}
	int ret = parseInfoBlock(cwav);
	if (ret) {
		freeCwav(cwav);
		return ret | 5 << 4;
	}
	cwav->cwavData = (cwavDataBlock_t*)((u32)(cwav->fileBuf) + cwav->cwavHeader->data_blck.ref.offset);
	if (cwav->cwavData->header.magic != 0x41544144) {
		freeCwav(cwav);
		return 6;
	}
	cwav->playingchanids[0] = cwav->playingchanids[1] = -1;
	*out = cwav;
	//dumpCwavInfo(cwav, filename);
	return 0;
}

void stopcwav(cwav_t* cwav) {
	if (!cwav) return;
	for (int i = 0; i < 2; i++) {
		if (cwav->playingchanids[i] != -1) {
			ndspChnWaveBufClear(cwav->playingchanids[i]);
			cwav->playingchanids[i] = -1;
		}
	}
}

bool playcwav(cwav_t* cwav, int channel1, int channel2) {
	if (!cwav) return false;
	bool stereo = true;
	if (channel2 < 0) {
		stereo = false;
	} 
	if (channel1 < 0 || channel1 >= (int)(cwav->cwavInfo->channelInfoRefs.count) || channel2 >= (int)(cwav->cwavInfo->channelInfoRefs.count)) {
		return false;
	} 
	stopcwav(cwav);
	int prevchan = -1;
	for (int i = 0; i < ((stereo) ? 2 : 1); i++) {
		for (int j = 0; j < 24; j++) {
			if (ndspChnIsPlaying(j) || j == prevchan) continue;
			cwav->playingchanids[i] = j;
		}
		if (cwav->playingchanids[i] == -1) return false;
		prevchan = cwav->playingchanids[i];
		ndspChnWaveBufClear(cwav->playingchanids[i]); 
		ndspChnSetInterp(cwav->playingchanids[i], NDSP_INTERP_NONE); 
		ndspChnSetFormat(cwav->playingchanids[i], NDSP_FORMAT_ADPCM | NDSP_3D_SURROUND_PREPROCESSED); 
		ndspChnSetRate(cwav->playingchanids[i], cwav->cwavInfo->sampleRate); 
		if (!stereo)
			cwav->mix[i][0] = cwav->mix[i][1] = 0.5f;
		else
		{
			if (i == 0)
			{
				cwav->mix[i][0] = 0.8f;
				cwav->mix[i][1] = 0.0f;
				cwav->mix[i][2] = 0.2f;
				cwav->mix[i][3] = 0.0f;
			}
			else
			{
				cwav->mix[i][0] = 0.0f;
				cwav->mix[i][1] = 0.8f;
				cwav->mix[i][2] = 0.0f;
				cwav->mix[i][3] = 0.2f;
			}
		}
		ndspChnSetMix(cwav->playingchanids[i], cwav->mix[i]); 
		ndspChnSetAdpcmCoefs(cwav->playingchanids[i], cwav->DSPADPCMInfos[i ? channel2 : channel1]->param.coefs); 
		memset(&(cwav->wavebuf[i]), 0, sizeof(ndspWaveBuf)); 
		cwav->wavebuf[i].data_adpcm = (u8*)((u32)cwav->channelInfos[i ? channel2 : channel1]->samples.offset + (u8*)(&(cwav->cwavData->data))); 
		cwav->wavebuf[i].nsamples = cwav->cwavInfo->LoopEnd; 
		cwav->wavebuf[i].looping = cwav->cwavInfo->isLooped; 
		cwav->wavebuf[i].status = NDSP_WBUF_DONE; 
		cwav->wavebuf[i].adpcm_data = &(cwav->adpcmData[i]); 
		memcpy(cwav->wavebuf[i].adpcm_data, &(cwav->DSPADPCMInfos[i ? channel2 : channel1]->context), sizeof(cwavDSPADPCMContext_t)); 
	}
	for (int i = 0; i < ((stereo) ? 2 : 1); i++) {
		ndspChnWaveBufAdd(cwav->playingchanids[i], &(cwav->wavebuf[i])); 
	}
	return true;
}