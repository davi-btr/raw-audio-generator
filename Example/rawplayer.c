#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <assert.h>
#include <math.h>
#include <stdint.h>

#define CHANNELS 2
#define FRAMES 1024
#define SAMPLERATE 48000

float buf[FRAMES*CHANNELS];

//#define DEBUG

#ifdef DEBUG
#define dbg_printf printf
#else
#define dbg_printf(args...)
#endif

int main (int argc, char *argv[])
{
  int err;
  snd_pcm_t *handle;
  snd_pcm_hw_params_t *hw_params;
  const char* fname = argv[1];

  if ((err = snd_pcm_open (&handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
    fprintf (stderr, "cannot open audio device %s (%s)\n", 
	     argv[1],
	     snd_strerror (err));
    exit (1);
  }

  if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
    fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
	     snd_strerror (err));
    exit (1);
  }

  if ((err = snd_pcm_hw_params_any (handle, hw_params)) < 0) {
    fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
	     snd_strerror (err));
    exit (1);
  }

  if ((err = snd_pcm_hw_params_set_access (handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
    fprintf (stderr, "cannot set access type (%s)\n",
	     snd_strerror (err));
    exit (1);
  }

  if ((err = snd_pcm_hw_params_set_format (handle, hw_params, SND_PCM_FORMAT_FLOAT_LE)) < 0) {
    fprintf (stderr, "cannot set sample format (%s)\n",
	     snd_strerror (err));
    exit (1);
  }

  unsigned int samplerate = SAMPLERATE;
  if ((err = snd_pcm_hw_params_set_rate_near (handle, hw_params, &samplerate, 0)) < 0) {
    fprintf (stderr, "cannot set sample rate (%s)\n",
	     snd_strerror (err));
    exit (1);
  }
  dbg_printf("Got actual rate: %u\n", samplerate);

  if ((err = snd_pcm_hw_params_set_channels (handle, hw_params, CHANNELS)) < 0) {
    fprintf (stderr, "cannot set channel count (%s)\n",
	     snd_strerror (err));
    exit (1);
  }

  unsigned long frames = FRAMES;
  if ((err = snd_pcm_hw_params_set_buffer_size_near(handle, hw_params, &frames)) < 0) {
    fprintf (stderr, "cannot set buffer size near %lu (%s)\n", frames,
	     snd_strerror (err));
    exit (1);
  }
  printf("Got actual buffer size: %lu\n", frames);
  if (frames > FRAMES)
    frames = FRAMES;

  if ((err = snd_pcm_hw_params (handle, hw_params)) < 0) {
    fprintf (stderr, "cannot set parameters (%s)\n",
	     snd_strerror (err));
    exit (1);
  }

  snd_pcm_hw_params_free (hw_params);

  if ((err = snd_pcm_prepare (handle)) < 0) {
    fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
	     snd_strerror (err));
    exit (1);
  }

  if ((err = snd_pcm_start (handle)) < 0) {
    fprintf (stderr, "cannot start audio stream (%s)\n",
	     snd_strerror (err));
    exit (1);
  }

  printf("Playing back from file: %s\n", fname);
  int fd = open(fname, O_RDONLY);
  assert(fd != -1);

  while (1) {
    unsigned long to_read = frames * CHANNELS * sizeof(float);
    assert (to_read <= sizeof(buf));
    unsigned char *ptr = (unsigned char *) buf;
    do {
      unsigned long just_read = read(fd, ptr, to_read);
      assert (just_read >= 0);
      if (just_read == 0) {
        // EOF
        goto end;
      }
      to_read -= just_read;
      ptr += just_read;
    } while (to_read > 0);
    snd_pcm_sframes_t num_frames = frames;
    float *buf_tmp = buf;
    while (num_frames > 0) {
      dbg_printf("writing %lu frames\n", num_frames);
      if ((err = snd_pcm_writei (handle, buf_tmp, num_frames)) < 0) {
        fprintf (stderr, "write to audio interface failed (%s)\n",
                 snd_strerror (err));
        snd_pcm_prepare (handle);
      }
      buf_tmp += err * CHANNELS;
      num_frames -= err;
    }
  }
 end:
  close(fd);
  // drain playback stream, i.e., wait for all written samples to be played, and stop
  assert(snd_pcm_drain (handle) == 0);

  snd_pcm_close (handle);
  exit (0);
}
