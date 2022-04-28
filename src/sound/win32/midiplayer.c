/*
 * mididemo.c
 *
 *  Created on: Dec 21, 2011
 *      Author: David J. Rager
 *       Email: djrager@fourthwoods.com
 *
 * This code is hereby released into the public domain per the Creative Commons
 * Public Domain dedication.
 *
 * http://http://creativecommons.org/publicdomain/zero/1.0/
 */
#pragma comment(lib, "winmm.lib")

#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <io.h>

HMIXER sMixerHandle;
HMIDISTRM sOut;
HANDLE hWaitingThread;
int sVolume = 127;
char *sFilename = NULL;
unsigned int sDeviceId = 0;

#define MAX_BUFFER_SIZE (512 * 12)
HANDLE event;

#pragma pack(push, 1)

struct _mid_header {
	unsigned int	id;		// identifier "MThd"
	unsigned int	size;	// always 6 in big-endian format
	unsigned short	format;	// big-endian format
	unsigned short  tracks;	// number of tracks, big-endian
	unsigned short	ticks;	// number of ticks per quarter note, big-endian
};

struct _mid_track {
	unsigned int	id;		// identifier "MTrk"
	unsigned int	length;	// track length, big-endian
};

#pragma pack(pop)

struct trk {
	struct _mid_track* track;
	unsigned char* buf;
	unsigned char last_event;
	unsigned int absolute_time;
};

struct evt {
	unsigned int absolute_time;
	unsigned char* data;
	unsigned char event;
};

static unsigned char* load_file(const unsigned char* filename, unsigned int* len)
{
	unsigned char* buf;
	unsigned int ret;
	FILE* f = fopen((char*)filename, "rb");
	if(f == NULL)
		return 0;

	fseek(f, 0, SEEK_END);
	*len = ftell(f);
	fseek(f, 0, SEEK_SET);

	buf = (unsigned char*)malloc(*len);
	if(buf == 0)
	{
		fclose(f);
		return 0;
	}

	ret = fread(buf, 1, *len, f);
	fclose(f);

	if(ret != *len)
	{
		free(buf);
		return 0;
	}

	return buf;
}

static unsigned long read_var_long(unsigned char* buf, unsigned int* bytesread)
{
	unsigned long var = 0;
	unsigned char c;

	*bytesread = 0;

	do
	{
		c = buf[(*bytesread)++];
		var = (var << 7) + (c & 0x7f);
	}
	while(c & 0x80);

	return var;
}

static unsigned short swap_bytes_short(unsigned short in)
{
	return ((in << 8) | (in >> 8));
}

static unsigned long swap_bytes_long(unsigned long in)
{
	unsigned short *p;
	p = (unsigned short*)&in;

	return (  (((unsigned long)swap_bytes_short(p[0])) << 16) |
				(unsigned long)swap_bytes_short(p[1]));
}

static struct evt get_next_event(const struct trk* track)
{
	unsigned char* buf;
	struct evt e;
	unsigned int bytesread;
	unsigned int time;

	buf = track->buf;

	time = read_var_long(buf, &bytesread);
	buf += bytesread;

	e.absolute_time = track->absolute_time + time;
	e.data = buf;
	e.event = *e.data;

	return e;
}

static int is_track_end(const struct evt* e)
{
	if(e->event == 0xff) // meta-event?
		if(*(e->data + 1) == 0x2f) // track end?
			return 1;

	return 0;
}

static void CALLBACK example9_callback(HMIDIOUT out, UINT msg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	switch (msg)
    {
	case MOM_DONE:
 		SetEvent(event);
		break;
	case MOM_POSITIONCB:
	case MOM_OPEN:
	case MOM_CLOSE:
		break;
    }
}

static unsigned int get_buffer(struct trk* tracks, unsigned int ntracks, unsigned int* out, unsigned int* outlen)
{
	MIDIEVENT e, *p;
	unsigned int streamlen = 0;
	unsigned int i;
	static unsigned int current_time = 0;

	if(tracks == NULL || out == NULL || outlen == NULL)
		return 0;

	*outlen = 0;

	while(TRUE)
	{
		unsigned int time = (unsigned int)-1;
		unsigned int idx = -1;
		struct evt evt;
		unsigned char c;

		if(((streamlen + 3) * sizeof(unsigned int)) >= MAX_BUFFER_SIZE)
			break;

		// get the next event
		for(i = 0; i < ntracks; i++)
		{
			evt = get_next_event(&tracks[i]);
			if(!(is_track_end(&evt)) && (evt.absolute_time < time))
			{
				time = evt.absolute_time;
				idx = i;
			}
		}

		// if idx == -1 then all the tracks have been read up to the end of track mark
		if(idx == -1)
			break; // we're done

		e.dwStreamID = 0; // always 0

		evt = get_next_event(&tracks[idx]);

		tracks[idx].absolute_time = evt.absolute_time;
		e.dwDeltaTime = tracks[idx].absolute_time - current_time;
		current_time = tracks[idx].absolute_time;

		if(!(evt.event & 0x80)) // running mode
		{
			unsigned char last = tracks[idx].last_event;
			c = *evt.data++; // get the first data byte
			e.dwEvent = ((unsigned long)MEVT_SHORTMSG << 24) |
						((unsigned long)last) |
						((unsigned long)c << 8);
			if(!((last & 0xf0) == 0xc0 || (last & 0xf0) == 0xd0))
			{
				c = *evt.data++; // get the second data byte
				e.dwEvent |= ((unsigned long)c << 16);
			}

			p = (MIDIEVENT*)&out[streamlen];
			*p = e;

			streamlen += 3;

			tracks[idx].buf = evt.data;
		}
		else if(evt.event == 0xff) // meta-event
		{
			evt.data++; // skip the event byte
			unsigned char meta = *evt.data++; // read the meta-event byte
			unsigned int len;

			switch(meta)
			{
			case 0x51: // only care about tempo events
				{
					unsigned char a, b, c;
					len = *evt.data++; // get the length byte, should be 3
					a = *evt.data++;
					b = *evt.data++;
					c = *evt.data++;

					e.dwEvent = ((unsigned long)MEVT_TEMPO << 24) |
								((unsigned long)a << 16) |
								((unsigned long)b << 8) |
								((unsigned long)c << 0);

					p = (MIDIEVENT*)&out[streamlen];
					*p = e;

					streamlen += 3;
				}
				break;
			default: // skip all other meta events
				len = *evt.data++; // get the length byte
				evt.data += len;
				break;
			}

			tracks[idx].buf = evt.data;
		}
		else if((evt.event & 0xf0) != 0xf0) // normal command
		{
			tracks[idx].last_event = evt.event;
			evt.data++; // skip the event byte
			c = *evt.data++; // get the first data byte
			e.dwEvent = ((unsigned long)MEVT_SHORTMSG << 24) |
						((unsigned long)evt.event << 0) |
						((unsigned long)c << 8);
			if(!((evt.event & 0xf0) == 0xc0 || (evt.event & 0xf0) == 0xd0))
			{
				c = *evt.data++; // get the second data byte
				e.dwEvent |= ((unsigned long)c << 16);
			}

			p = (MIDIEVENT*)&out[streamlen];
			*p = e;

			streamlen += 3;

			tracks[idx].buf = evt.data;
		}

	}

	*outlen = streamlen * sizeof(unsigned int);

	return 1;
}

unsigned int example9(char *filename)
{
	unsigned char* midibuf = NULL;
	unsigned int midilen = 0;

	struct _mid_header* hdr = NULL;

	unsigned int i;

	unsigned short ntracks = 0;
	struct trk* tracks = NULL;

	unsigned int streambufsize = MAX_BUFFER_SIZE;
	unsigned int* streambuf = NULL;
	unsigned int streamlen = 0;

	MIDIPROPTIMEDIV prop;
	MIDIHDR mhdr;

	midibuf = load_file((unsigned char*)filename, &midilen);
	if(midibuf == NULL)
	{
		fprintf(stderr, "could not open %s\n", filename);
		fflush(stderr);
		return 0;
	}

	hdr = (struct _mid_header*)midibuf;
	midibuf += sizeof(struct _mid_header);
	ntracks = swap_bytes_short(hdr->tracks);

	tracks = (struct trk*)malloc(ntracks * sizeof(struct trk));
	if(tracks == NULL)
		goto error1;

	for(i = 0; i < ntracks; i++)
	{
		tracks[i].track = (struct _mid_track*)midibuf;
		tracks[i].buf = midibuf + sizeof(struct _mid_track);
		tracks[i].absolute_time = 0;
		tracks[i].last_event = 0;

		midibuf += sizeof(struct _mid_track) + swap_bytes_long(tracks[i].track->length);
	}

	streambuf = (unsigned int*)malloc(sizeof(unsigned int) * streambufsize);
	if(streambuf == NULL)
		goto error2;

	memset(streambuf, 0, sizeof(unsigned int) * streambufsize);

    if ((event = CreateEvent(0, FALSE, FALSE, 0)) == NULL)
    	goto error3;

	if (midiStreamOpen(&sOut, &sDeviceId, 1, (DWORD)example9_callback, 0, CALLBACK_FUNCTION) != MMSYSERR_NOERROR)
		goto error4;

	prop.cbStruct = sizeof(MIDIPROPTIMEDIV);
	prop.dwTimeDiv = swap_bytes_short(hdr->ticks);
	if(midiStreamProperty(sOut, (LPBYTE)&prop, MIDIPROP_SET|MIDIPROP_TIMEDIV) != MMSYSERR_NOERROR)
		goto error5;

	mhdr.lpData = (char*)streambuf;
	mhdr.dwBufferLength = mhdr.dwBytesRecorded = streambufsize;
	mhdr.dwFlags = 0;

	if(midiOutPrepareHeader((HMIDIOUT)sOut, &mhdr, sizeof(MIDIHDR)) != MMSYSERR_NOERROR)
		goto error5;

	if(midiStreamRestart(sOut) != MMSYSERR_NOERROR)
		goto error6;

	fprintf(stderr, "buffering...\n");
	fflush(stderr);
	get_buffer(tracks, ntracks, streambuf, &streamlen);
	while(streamlen > 0)
	{
		mhdr.dwBytesRecorded = streamlen;

		if(midiStreamOut(sOut, &mhdr, sizeof(MIDIHDR)) != MMSYSERR_NOERROR)
			goto error7;

		WaitForSingleObject(event, INFINITE);

		if (sFilename != filename) {
			fprintf(stderr, "switch to new file %s.\n", sFilename);
			fflush(stderr);
			break;
		}

		fprintf(stderr, "buffering...\n");
		fflush(stderr);
		get_buffer(tracks, ntracks, streambuf, &streamlen);
	}
	fprintf(stderr, "done.\n");
	fflush(stderr);

error7:
	midiOutReset((HMIDIOUT)sOut);

error6:
	midiOutUnprepareHeader((HMIDIOUT)sOut, &mhdr, sizeof(MIDIHDR));

error5:
	midiStreamClose(sOut);

error4:
	CloseHandle(event);

error3:
	free(streambuf);

error2:
	free(tracks);

error1:
	free(hdr);

	return(0);
}

DWORD WINAPI MyThreadFunction(LPVOID lpParam) {
	while (1) {
		MMRESULT r = midiOutSetVolume((HMIDIOUT)sOut, (DWORD)((sVolume & 0xFF) << 8) | (sVolume & 0xFF));
		if (r != MMSYSERR_NOERROR) {
			fprintf(stderr, "Cannot set volume via midi...");
			switch (r) {
				case MMSYSERR_INVALHANDLE:
					fprintf(stderr, "Reason: handle not valid\n");
					break;
				case MMSYSERR_NOMEM:
					fprintf(stderr, "Reason: memory error\n");
					break;
				case MMSYSERR_NOTSUPPORTED:
					fprintf(stderr, "Reason: not supported\n");
					break;
				default:
					fprintf(stderr, "Reason: unknown\n");
			}
			fflush(stderr);
			MMRESULT err = mixerOpen(&sMixerHandle, sDeviceId, 0, 0, 0);
			if (err) {
				fprintf(stderr, "ERROR: Can't open Mixer Device! -- %08X\n", err);
				switch (err) {
						case MMSYSERR_ALLOCATED:
							fprintf(stderr, "MMSYSERR_ALLOCATED\n");
							break;
						case MMSYSERR_BADDEVICEID:
							fprintf(stderr, "MMSYSERR_BADDEVICEID\n");
							break;
						case MMSYSERR_INVALFLAG:
							fprintf(stderr, "MMSYSERR_INVALFLAG\n");
							break;
						case MMSYSERR_INVALHANDLE:
							fprintf(stderr, "MMSYSERR_INVALHANDLE\n");
							break;
						case MMSYSERR_INVALPARAM:
							fprintf(stderr, "MMSYSERR_INVALPARAM\n");
							break;
						case MMSYSERR_NODRIVER:
							fprintf(stderr, "MMSYSERR_NODRIVER\n");
							break;
						case MMSYSERR_NOMEM:
							fprintf(stderr, "MMSYSERR_NOMEM\n");
							break;
						default:
							fprintf(stderr, "Reason: unknown\n");
					}
					fflush(stderr);
			} else {
				MIXERCAPS     mixcaps;
				MIXERLINE     mixerline;
				MMRESULT      err;
				unsigned long i;
				/* Get info about the first Mixer Device */
				if (!(err = mixerGetDevCaps((UINT)sMixerHandle, &mixcaps, sizeof(MIXERCAPS)))) {
					/* Print out the name of each destination line */
					for (i = 0; i < mixcaps.cDestinations; i++) {
						mixerline.cbStruct = sizeof(MIXERLINE);
						mixerline.dwDestination = i;

						if (!(err = mixerGetLineInfo((HMIXEROBJ)sMixerHandle, &mixerline, MIXER_GETLINEINFOF_DESTINATION)))
						{
							fprintf(stderr, "Destination #%lu = %s\n", i, mixerline.szName);
							fflush(stderr);
						}
					}
				}
				mixerline.cbStruct = sizeof(MIXERLINE);
				mixerline.dwDestination = 0;
				mixerline.dwSource = 0;

				if ((err = mixerGetLineInfo((HMIXEROBJ)sMixerHandle, &mixerline, MIXER_GETLINEINFOF_DESTINATION))) {
					/* An error */
					fprintf(stderr, "Error #%d calling mixerGetLineInfo()\n", err);
					switch (err) {
						case MIXERR_INVALLINE:
							fprintf(stderr, "MIXERR_INVALLINE\n");
							break;
						case MMSYSERR_BADDEVICEID:
							fprintf(stderr, "MMSYSERR_BADDEVICEID\n");
							break;
						case MMSYSERR_INVALFLAG:
							fprintf(stderr, "MMSYSERR_INVALFLAG\n");
							break;
						case MMSYSERR_INVALHANDLE:
							fprintf(stderr, "MMSYSERR_INVALHANDLE\n");
							break;
						case MMSYSERR_INVALPARAM:
							fprintf(stderr, "MMSYSERR_INVALPARAM\n");
							break;
						case MMSYSERR_NODRIVER:
							fprintf(stderr, "MMSYSERR_NODRIVER\n");
							break;
						default:
							fprintf(stderr, "Reason: unknown\n");
					}
					fflush(stderr);
				} else {
					fprintf(stderr, "Mixerline: %s\n", mixerline.szName);
					MIXERCONTROL       mixerControlArray;
					MIXERLINECONTROLS  mixerLineControls;
					mixerLineControls.cbStruct = sizeof(MIXERLINECONTROLS);
					/* Tell mixerGetLineControls() for which line we're retrieving info.
					We do this by putting the desired line's ID number in dwLineID */
					mixerLineControls.dwLineID = mixerline.dwLineID;
					/* We want to fetch info on only 1 control */
					mixerLineControls.cControls = 1;
					/* Tell mixerGetLineControls() for which type of control we're
					retrieving info. We do this by putting the desired control type
					in dwControlType */
					mixerLineControls.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
					/* Give mixerGetLineControls() the address of the MIXERCONTROL
					struct to hold info */
					mixerLineControls.pamxctrl = &mixerControlArray;
					/* Tell mixerGetLineControls() how big the MIXERCONTROL is. This
					saves having to initialize the cbStruct of the MIXERCONTROL itself */
					mixerLineControls.cbmxctrl = sizeof(MIXERCONTROL);
					/* Retrieve info on only any volume slider control for this line */
					if ((err = mixerGetLineControls((HMIXEROBJ)sMixerHandle, &mixerLineControls, MIXER_GETLINECONTROLSF_ONEBYTYPE))) {
						/* An error */
						fprintf(stderr, "Error #%d calling mixerGetLineControls()\n", err);
						fflush(stderr);
					} else {
						fprintf(stderr, "Mixercontrol: %s\n", mixerControlArray.szName);
						fflush(stderr);
						// try via mixer controls
						MIXERCONTROLDETAILS_UNSIGNED value;
						MIXERCONTROLDETAILS          mixerControlDetails;
						MMRESULT                     err;
						mixerControlDetails.cbStruct = sizeof(MIXERCONTROLDETAILS);
						/* Tell mixerSetControlDetails() which control whose value we
						want to set. We do this by putting the desired control's
						ID number in dwControlID. The "Speaker Out" line's
						volume slider has an ID of 0x00000000 */
						mixerControlDetails.dwControlID = mixerControlArray.dwControlID;
						/* This is always 1 for a MIXERCONTROL_CONTROLF_UNIFORM control */
						mixerControlDetails.cChannels = 1;
						/* This is always 0 except for a MIXERCONTROL_CONTROLF_MULTIPLE control */
						mixerControlDetails.cMultipleItems = 0;
						/* Give mixerSetControlDetails() the address of the
						MIXERCONTROLDETAILS_UNSIGNED struct into which we place the value */
						mixerControlDetails.paDetails = &value;
						/* Tell mixerSetControlDetails() how big the MIXERCONTROLDETAILS_UNSIGNED is */
						mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
						/* Store the value */
						value.dwValue = (sVolume & 0xFF) << 8;
						/* Set the value of the volume slider control for this line */
						if ((err = mixerSetControlDetails((HMIXEROBJ)sMixerHandle, &mixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE))) {
							fprintf(stderr, "Error #%d calling mixerSetControlDetails() ", err);
							switch (err) {
								case MIXERR_INVALCONTROL:
									fprintf(stderr, "Invalid control\n");
									break;
								case MMSYSERR_BADDEVICEID:
									fprintf(stderr, "bad device id\n");
									break;
								case MMSYSERR_INVALFLAG:
									fprintf(stderr, "invalid flag\n");
									break;
								case MMSYSERR_INVALHANDLE:
									fprintf(stderr, "Invalid handle\n");
									break;
								case MMSYSERR_INVALPARAM:
									fprintf(stderr, "Invalid param\n");
									break;
								case MMSYSERR_NODRIVER:
									fprintf(stderr, "No driver\n");
									break;
								default:
									fprintf(stderr, "\n");
							}
							fprintf(stderr, "Will have to use windows application volume control.\n");
							fflush(stderr);
						}
					}
				}
			}
		}

		char data1, data2;
		fprintf(stderr, "read from stdin...");
		fflush(stderr);
		if (fread(&data1, sizeof(char), 1, stdin) != 1) {
			exit(0);
		} else {
			if (fread(&data2, sizeof(char), 1, stdin) != 1) {
				exit(0);
			}
		}
		fprintf(stderr, "got %x %x\n", data1, data2);
		fflush(stderr);
		if (data1 > 0) {
			// new filename
			short length = data1 | (data2 << 8);
			fprintf(stderr, "New filename sending: %d bytes\n", length);
			fflush(stderr);
			static char buf[0xffff];
			int lastRead, totalRead = 0;
			while ((lastRead = fread(buf + totalRead, sizeof(char), length - totalRead, stdin)) > 0 && totalRead < length) {
				totalRead += lastRead;
			}
			if (totalRead < length) {
				fprintf(stderr, "not enough data, got %d, expected %d bytes\n", totalRead, length);
				fflush(stderr);
				exit(0);
			}
			buf[length] = '\0';
			sFilename = buf;
			fprintf(stderr, "New filename received: %s\n", sFilename);
			fflush(stderr);
			SetEvent(event);
		} else {
			fprintf(stderr, "New volume received: %d\n", data2);
			fflush(stderr);
			sVolume = data2;
		}
	}
	return 0;
}

int main(int argc, char* argv[])
{
	if(argc != 3) {
		return fprintf(stderr, "Usage: %s <volume 0-255> <filename.mid>\n", argv[0]);
	}

	sVolume = atoi(argv[1]);
	_setmode(_fileno(stdin), _O_BINARY);
	hWaitingThread = CreateThread(NULL, 0, MyThreadFunction, NULL, 0, NULL);

	sFilename = argv[2];

	while (1) {
		example9(sFilename);
		fprintf(stdout, "1");
		fflush(stdout);
		WaitForSingleObject(event, INFINITE);
	}

	return 0;
}
