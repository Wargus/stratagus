#ifdef USE_LIBCDA
#ifdef linux
/* libcda; Linux component.
 *
 * Peter Wang <tjaden@users.sf.net>
 */

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/cdrom.h>
#include <errno.h>
#include "libcda.h"


/* It appears not all drivers support the CDROMPLAYTRKIND ioctl yet
 * (unfortunately).  Until then, we use CDROMPLAYMSF.
 */
#define USE_PLAYMSF


#define MIN(x,y)     (((x) < (y)) ? (x) : (y))
#define MAX(x,y)     (((x) > (y)) ? (x) : (y))
#define MID(x,y,z)   MAX((x), MIN((y), (z)))


static int fd = -1;

static char _cd_error[256];
const char* cd_error = _cd_error;


static void copy_cd_error(void)
{
	strncpy(_cd_error, strerror(errno), sizeof(_cd_error));
	_cd_error[sizeof _cd_error - 1] = 0;
}


static int get_tocentry(int track, struct cdrom_tocentry* e)
{
	memset(e, 0, sizeof(struct cdrom_tocentry));
	e->cdte_track = track;
	e->cdte_format = CDROM_MSF;

	if (ioctl(fd, CDROMREADTOCENTRY, e) < 0) {
		copy_cd_error();
		return -1;
	}

	return 0;
}


static int get_subchnl(struct cdrom_subchnl* s)
{
	memset(s, 0, sizeof(struct cdrom_subchnl));
	s->cdsc_format = CDROM_MSF;
	if (ioctl(fd, CDROMSUBCHNL, s) < 0) {
		copy_cd_error();
		return -1;
	}

	return 0;
}


/* cd_init:
 *  Initialise library.  Return zero on success.
 */
int cd_init(void)
{
	char* device;

	device = getenv("CDAUDIO");
	if (!device) {
		device = "/dev/cdrom";
	}

	if (fd != -1) {
		close(fd);
	}

	fd = open(device, O_RDONLY | O_NONBLOCK);
	if (fd < 0) {
		copy_cd_error();
		return -1;
	}

	return 0;
}


/* cd_exit:
 *  Shutdown.
 */
void cd_exit(void)
{
	if (fd != -1) {
		close(fd);
		fd = -1;
	}
}


static int play(int t1, int t2)
{
#ifdef USE_PLAYMSF
	struct cdrom_tocentry e0, e1;
	struct cdrom_msf msf;
	int last;

	if (cd_get_tracks(0, &last) != 0)
		return -1;

	/* cdrom.h: The leadout track is always 0xAA, regardless
	 * of # of tracks on disc. */
	if (t2 == last) {
		t2 = CDROM_LEADOUT;
	} else {
		++t2;
	}

	if ((get_tocentry(t1, &e0) != 0) || (get_tocentry(t2, &e1) != 0)) {
		return -1;
	}

	msf.cdmsf_min0 = e0.cdte_addr.msf.minute;
	msf.cdmsf_sec0 = e0.cdte_addr.msf.second;
	msf.cdmsf_frame0 = e0.cdte_addr.msf.frame;
	msf.cdmsf_min1 = e1.cdte_addr.msf.minute;
	msf.cdmsf_sec1 = e1.cdte_addr.msf.second;
	msf.cdmsf_frame1 = e1.cdte_addr.msf.frame;

	if (ioctl(fd, CDROMPLAYMSF, &msf) < 0) {
		copy_cd_error();
		return -1;
	}

	return 0;
#else
	struct cdrom_ti idx;

	memset(&idx, 0, sizeof(idx));
	idx.cdti_trk0 = t1;
	idx.cdti_trk1 = t2;
	if (ioctl(fd, CDROMPLAYTRKIND, &idx) < 0) {
		copy_cd_error();
		return -1;
	}

	return 0;
#endif
}


/* cd_play:
 *  Play specified track.  Return zero on success.
 */
int cd_play(int track)
{
	return play(track, track);
}


/* cd_play_range:
 *  Play from START to END tracks.  Return zero on success.
 */
int cd_play_range(int start, int end)
{
	return play(start, end);
}


/* cd_play_from:
 *  Play from track to end of disc.  Return zero on success.
 */
int cd_play_from(int track)
{
	int last;

	if (cd_get_tracks(0, &last) != 0) {
		return -1;
	}

	return play(track, last);
}


/* cd_current_track:
 *  Return track currently in playback, or zero if stopped.
 */
int cd_current_track(void)
{
	struct cdrom_subchnl s;

	get_subchnl(&s);
	if (s.cdsc_audiostatus == CDROM_AUDIO_PLAY) {
		return s.cdsc_trk;
	} else {
		return 0;
	}
}


/* cd_pause:
 *  Pause playback.
 */
void cd_pause(void)
{
	ioctl(fd, CDROMPAUSE);
}


/* cd_resume:
 *  Resume playback.
 */
void cd_resume(void)
{
	if (cd_is_paused()) {
		ioctl(fd, CDROMRESUME);
	}
}


/* cd_is_paused:
 *  Return non-zero if playback is paused.
 */
int cd_is_paused(void)
{
	struct cdrom_subchnl s;

	get_subchnl(&s);
	return (s.cdsc_audiostatus == CDROM_AUDIO_PAUSED);
}


/* cd_stop:
 *  Stop playback.
 */
void cd_stop(void)
{
	ioctl(fd, CDROMSTOP);
}


/* cd_get_tracks:
 *  Get first and last tracks of CD.  Return zero on success.
 */
int cd_get_tracks(int* first, int* last)
{
	struct cdrom_tochdr toc;

	if (ioctl(fd, CDROMREADTOCHDR, &toc) < 0) {
		copy_cd_error();
		if (first) {
			*first = 0;
		}
		if (last) {
			*last = 0;
		}
		return -1;
	}

	if (first) {
		*first = toc.cdth_trk0;
	}
	if (last) {
		*last  = toc.cdth_trk1;
	}
	return 0;
}


/* cd_is_audio:
 *  Return 1 if track specified is audio,
 *  zero if it is data, -1 if an error occurs.
 */
int cd_is_audio(int track)
{
	struct cdrom_tocentry e;

	if ((cd_get_tracks(0, 0) < 0) || (get_tocentry(track, &e) < 0)) {
		return -1;
	}
	return (e.cdte_ctrl & CDROM_DATA_TRACK) ? 0 : 1;
}


/* cd_get_volume:
 *  Return volumes of left and right channels.
 */
void cd_get_volume(int* c0, int* c1)
{
	struct cdrom_volctrl vol;

	ioctl(fd, CDROMVOLREAD, &vol);
	if (c0) {
		*c0 = vol.channel0;
	}
	if (c1) {
		*c1 = vol.channel1;
	}
}


/* cd_set_volume:
 *  Set left and right channel volumes (0 - 255).
 */
void cd_set_volume(int c0, int c1)
{
	struct cdrom_volctrl vol;

	vol.channel0 = MID(0, c0, 255);
	vol.channel1 = MID(0, c1, 255);
	vol.channel2 = 0;
	vol.channel3 = 0;
	ioctl(fd, CDROMVOLCTRL, &vol);
}


/* cd_eject:
 *  Eject CD drive (if possible).
 */
void cd_eject(void)
{
	ioctl(fd, CDROMEJECT);
}


/* cd_close:
 *  Close CD drive (if possible).
 */
void cd_close(void)
{
	ioctl(fd, CDROMCLOSETRAY);
}
#endif // linux

#ifdef WIN32
/* libcda; Windows component.
 *
 * Using the string interface is probably slightly slower, but damned
 * if I'm going to code using the message interface.
 *
 * Peter Wang <tjaden@users.sf.net>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <windows.h>
#include <mmsystem.h>
#include "stratagus.h"
#include "libcda.h"


static char _cd_error[256];
const char* cd_error = _cd_error;


/* Hack. */
static int paused = 0;
static char paused_pos[20];
static char end_pos[20];


/* internal: Even the command string interface needs shielding. */

static char ret[256];

static int command(char* fmt, ...)
{
	char buf[256];
	va_list ap;
	DWORD err;

	va_start(ap, fmt);
	vsprintf(buf, fmt, ap);
	va_end(ap);

	err = mciSendString(buf, ret, sizeof ret, 0);
	if (err) {
		mciGetErrorString(err, _cd_error, sizeof _cd_error);
	}
	return err ? -1 : 0;
}


int cd_init(void)
{
	int err;

	err = command("open cdaudio wait");
	if (!err) {
		err = command("set cdaudio time format tmsf");
	}

	paused = 0;
	return err;
}


void cd_exit(void)
{
	command("close cdaudio");
}


/* internal helpers */

#define startof(track) (MCI_MAKE_TMSF(track, 0, 0, 0))

static char* lengthof(int track)
{
	command("status cdaudio length track %u", track);
	return ret;
}


int cd_play(int track)
{
	cd_stop();
	sprintf(end_pos, "%u:%s", track, lengthof(track));
	return command("play cdaudio from %lu to %s", startof(track), end_pos);
}


int cd_play_range(int start, int end)
{
	cd_stop();
	sprintf(end_pos, "%u:%s", end, lengthof(end));
	return command("play cdaudio from %lu to %s", startof(start), end_pos);
}


int cd_play_from(int track)
{
	cd_stop();
	end_pos[0] = 0;
	return command("play cdaudio from %lu", startof(track));
}


int cd_current_track(void)
{
	if ((command("status cdaudio mode") != 0) ||
			(strcmp(ret, "playing") != 0)) {
		return 0;
	}

	if (command("status cdaudio current track") != 0) {
		return 0;
	}
	return atoi(ret);
}


void cd_pause(void)
{
	/* `pause cdaudio' works like `stop' with the MCICDA driver.
	 * Therefore we hack around it.
	 */
	mciSendString("status cdaudio position", paused_pos, sizeof paused_pos, 0);
	command("pause cdaudio");
	paused = 1;
}


void cd_resume(void)
{
	if (!paused)
		return;

	if (end_pos[0]) {
		command("play cdaudio from %s to %s", paused_pos, end_pos);
	} else {
		command("play cdaudio from %s", paused_pos);
	}
	paused = 0;
}


int cd_is_paused(void)
{
	return paused;
}


void cd_stop(void)
{
	command("stop cdaudio wait");
	paused = 0;
}


int cd_get_tracks(int *first, int *last)
{
	int i;

	if (command("status cdaudio number of tracks") != 0)
		return -1;

	i = atoi(ret);

	if (first) {
		*first = 1;
	}
	if (last) {
		*last = i;
	}

	return (i) ? 0 : -1;
}


int cd_is_audio(int track)
{
	if (command("status cdaudio type track %u", track) != 0) {
		return -1;
	}
	return (strcmp(ret, "audio") == 0) ? 1 : 0;
}


void cd_get_volume(int *c0, int *c1)
{
	if (c0) {
		*c0 = 128; /* (shrug) */
	}
	if (c1) {
		*c1 = 128;
	}
}


void cd_set_volume(int c0 __attribute__((unused)),
		int c1 __attribute__((unused)))
{
}


void cd_eject(void)
{
	command("set cdaudio door open");
	paused = 0;
}


void cd_close(void)
{
	command("set cdaudio door closed");
	paused = 0;
}
#endif // WIN32
#endif // USE_LIBCDA
