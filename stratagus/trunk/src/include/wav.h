/*
**	A clone of a famous game.
*/
/**@name wav.h		-	The wav file format header file. */
/*
**	(c) Copyright 1998,1999 by Lutz Sammer
**
**	$Id$
*/

#ifndef __WAV_H__
#define __WAV_H__

//@{

/*----------------------------------------------------------------------------
--	Wav
----------------------------------------------------------------------------*/

//
//	Define values for WAV format
//

/**
**	chunk names.
*/
#define RIFF		0x46464952	// "RIFF"
#define WAVE		0x45564157	// "WAVE"
#define FMT		0x20746D66	// "fmt "
#define DATA		0x61746164	// "data"

/**
**	Wav types
*/
#define WAV_UNKNOWN	0
#define WAV_PCM_CODE	1
#define WAV_ADPCM	2
#define WAV_ALAW	6
#define WAV_MULAW	7
#define WAV_OKI_ADPCM	16
#define WAV_DIGISTD	21
#define WAV_DIGIFIX	22

#define IBM_MULAW	0x0101
#define IBM_ALAW	0x0102
#define IBM_ADPCM	0x0103

#define WAV_MONO	1
#define WAV_STEREO	2

//
//	Wav format
//
typedef struct __wav_fmt__ {
    unsigned int	FMTchunk;
    unsigned int	FMTlength;
    unsigned short	Encoding;	/// 1 = PCM
    unsigned short	Channels;	/// 1 = mono, 2 = stereo
    unsigned int	Frequency;	/// One of 11025, 22050, or 44100 Hz
    unsigned int	ByteRate;	/// Average bytes per second
    unsigned short	SampleSize;	/// Bytes per sample block
    unsigned short	BitsPerSample;	/// One of 8, 12, 16
} WavFMT;

//
//	Wav data
//
typedef struct __wav_data__ {
    unsigned int	DATAchunk;
    unsigned int	DATAlength;
    unsigned char	Data[0];	/// Wave PCM data follows...
} WavDATA;

//
//	General chunk found in the WAV file
//
typedef struct __wav_chunk__ {
    unsigned int	Magic;
    unsigned int	Length;
    unsigned char	Data[0];
} WavChunk;

//@}

#endif	// !__WAV_H__
