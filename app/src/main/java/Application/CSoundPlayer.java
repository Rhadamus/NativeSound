/* Copyright (c) 1996-2013 Clickteam
 *
 * This source code is part of the Android exporter for Clickteam Multimedia Fusion 2.
 *
 * Permission is hereby granted to any person obtaining a legal copy
 * of Clickteam Multimedia Fusion 2 to use or modify this source code for
 * debugging, optimizing, or customizing applications created with
 * Clickteam Multimedia Fusion 2.  Any other use of this source code is prohibited.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
package Application;

import android.media.AudioAttributes;
import android.media.AudioManager;
import android.os.Build;

import Banks.CSound;
import Runtime.MMFRuntime;

public class CSoundPlayer {
	public static final int SNDF_LOADONCALL = 0x0010;
	public static final int SNDF_PLAYFROMDISK = 0x0020;

	private long ptr;
	public CRunApp app;

	public AudioAttributes attributes;
	public AudioFocusFusion audioFocusFusion;
	private boolean hasFocus;

	public int maxSoundPool;
	public int maxMediaSound;

	public native void setVolume(float volume);
	public native float getVolume();
	public native void setPan(float pan);
	public native float getPan();

	public CSoundPlayer(CRunApp a) {
		app = a;
		System.loadLibrary("NativeSound");
		allocNative();

		attributes = null;
		if (Build.VERSION.SDK_INT >= 21) {
			attributes = new AudioAttributes.Builder()
					.setUsage(AudioAttributes.USAGE_GAME)
					.setContentType(AudioAttributes.CONTENT_TYPE_SONIFICATION)
					.setLegacyStreamType(AudioManager.STREAM_MUSIC)
					.build();
		}

		audioFocusFusion = new AudioFocusFusion(MMFRuntime.inst, attributes);
		AudioFocusFusion.AudioFocusListener audioflistener = result -> hasFocus = result;
		audioFocusFusion.setAudioFocusListener(audioflistener);

		hasFocus = audioFocusFusion.getAudioFocus();
		if (!hasFocus) hasFocus = audioFocusFusion.requestAudioFocus();
	}
	private native void allocNative();

	/** Plays a simple sound.
	 Note: _volume, _pan and _freq are valid if _volume != -1
	 */
	public void play(short handle, int nLoops, int channel, boolean bPrio, int _volume, int _pan, int _freq) {
		CSound sound = app.soundBank.getSoundFromHandle(handle);
		if (sound == null) return;
		if (!hasFocus) hasFocus = audioFocusFusion.requestAudioFocus();
		playNative(sound, nLoops, channel, bPrio, _volume, _pan, _freq, hasFocus);
	}
	public void playFile(String filename, int nLoops, int channel, boolean bPrio, int _volume, int _pan, int _freq) {
		CSound sound = new CSound(filename);
		sound.load();
		if (!hasFocus) hasFocus = audioFocusFusion.requestAudioFocus();
		playNative(sound, nLoops, channel, bPrio, _volume, _pan, _freq, hasFocus);
	}
	private native void playNative(CSound sound, int nLoops, int channel, boolean prio, int volume,
								   int pan, int freq, boolean focus);

	public native void setMultipleSounds(boolean bMultiple);

	public native void stopAllSounds();

	public native void stop(short handle);

	public native boolean isSoundPlaying();
	public native boolean isSamplePlaying(short handle);
	public native boolean isSamplePaused(short handle);
	public native boolean isChannelPlaying(int channel);
	public native boolean isChannelPaused(int channel);

	public native void pause(short handle);
	public void resume(short handle) {
		if (!hasFocus) hasFocus = audioFocusFusion.requestAudioFocus();
		resumeNative(handle);
	}
	private native void resumeNative(short handle);

	// Runtime Pause
	public void pause2() {
		pauseApp();
		if (hasFocus) {
			audioFocusFusion.abandonAudioFocus();
			hasFocus = false;
		}
	}
	private native void pauseApp();

	public native void pauseAllChannels();

	// Runtime Resume
	public void resume2() {
		if (!hasFocus) hasFocus = audioFocusFusion.requestAudioFocus();
		resumeApp();
	}
	private native void resumeApp();

	public void resumeAllChannels() {
		hasFocus = audioFocusFusion.requestAudioFocus();
		resumeAllChannelsNative();
	}
	private native void resumeAllChannelsNative();

	public native void pauseChannel(int channel);

	public native void stopChannel(int channel);

	public void resumeChannel(int channel) {
		if (!hasFocus) hasFocus = audioFocusFusion.requestAudioFocus();
		resumeChannelNative(channel);
	}
	private native void resumeChannelNative(int channel);

	public native int getChannel(String name);
	public native String getChannelSampleName(int channel);

	public native int getChannelDuration(int channel);
	public native int getSampleDuration(String name);

	public native void setPositionChannel(int channel, int pos);
	public native int getPositionChannel(int channel);

	public native int getSamplePosition(String name);

	public native void setFrequencyChannel(int channel, int frequency);

	public native void setVolumeChannel(int channel, float volume);
	public native float getVolumeChannel(int channel);

	public native float getSampleVolume(String name);

	public native void setPanChannel(int channel, float pan);
	public native float getPanChannel(int channel);

	public native float getSamplePan(String name);

	public native void setPosition(short handle, int pos);

	public void setVolume(short handle, float volume) {
		setVolumeSample(handle, volume);
	}
	private native void setVolumeSample(short handle, float volume);

	public native void setFrequency(short handle, int frequency);
	public native int getFrequency(int channel);
	public native int getSampleFrequency(String name);

	public void setPan(short handle, float pan) {
		setPanSample(handle, pan);
	}
	private native void setPanSample(short handle, float pan);

	public native void lockChannel(int channel);
	public native void unlockChannel(int channel);

	public void dispose() {
		if (audioFocusFusion != null) {
			audioFocusFusion.abandonAudioFocus();
			audioFocusFusion.dispose();
		}
		releaseNative();
	}
	private native void releaseNative();
}