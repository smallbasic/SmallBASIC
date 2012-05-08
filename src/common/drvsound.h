// This file is part of SmallBASIC
//
// Add-on SB sound driver
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#if !defined(_DRV_SOUND_H)
#define _DRV_SOUND_H

#if defined(__cplusplus)
extern "C" {
#endif

/*
 *   driver initialization
 *   returns false on error
 */
int drvsound_init(void);

/*
 *   restores device state
 */
void drvsound_close(void);

/*
 *   plays a tone
 *       frq = frequency
 *       ms  = duration in milliseconds
 *       vol = volume (0..100)
 *       bgplay = true for play in background
 */
void drvsound_sound(int frq, int ms, int vol, int bgplay);

/*
 *   beep!
 */
void drvsound_beep(void);

/*
 * clear background queue
 */
void drvsound_clear_queue(void);

/*
 *   For OSes that does not supports threads, this enables background plays
 *   (Its called every ~50ms)
 */
void drvsound_event(void);

#if defined(__cplusplus)
}
#endif
#endif
