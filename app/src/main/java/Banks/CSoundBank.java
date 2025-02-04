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
package Banks;

import Application.CRunApp;
import Application.CSoundPlayer;
import Services.CFile;

public class CSoundBank implements IEnum {
	public CSound[] sounds = null;
	public int nHandlesReel;

	public short[] handleToFlags;
	public int[] handleToFrequency;
	public String[] handleToSoundName;

	private short[] useCount;
	private final CSoundPlayer player;

	public CSoundBank(CSoundPlayer p) {
		player = p;
	}

	public void preLoad(CFile f) {
		nHandlesReel = f.readAShort();

		handleToFlags = new short[nHandlesReel];
		handleToFrequency = new int[nHandlesReel];
		handleToSoundName = new String[nHandlesReel];

		int soundNum = f.readAShort();
		for (int i = 0; i < soundNum; i++) {
			int handle = f.readAShort();
			handleToFlags[handle] = f.readAShort();
			f.skipBytes(4);	// Length
			handleToFrequency[handle] = f.readAInt();

			// SNDF_HASNAME
			if ((handleToFlags[handle] & 0x100) != 0) {
				int length = f.readAShort();
				handleToSoundName[handle] = f.readAString(length);
			}
		}

		useCount = new short[nHandlesReel];
		resetToLoad();
		sounds = null;
	}

	public CSound getSoundFromHandle(short handle) {
		if (sounds == null || handle < 0 || handle >= nHandlesReel) return null;
		return sounds[handle];
	}
	public int getSoundHandleFromName(String soundName) {
		if (sounds == null) return -1;
		for (int h = 0; h < nHandlesReel; h++) {
			if (soundName.equalsIgnoreCase(handleToSoundName[h])) return h;
		}
		return -1;
	}

	// Called when loading events, before load() is called
	public void setToLoad(short handle) {
		useCount[handle]++;
	}
	public void resetToLoad() {
		for (int i = 0; i < nHandlesReel; i++) useCount[i] = 0;
	}

	@Override
	public short enumerate(short num) {
		setToLoad(num);
		return -1;
	}

	// Load at start of frame
	public void load(final CRunApp app) {
		// If samples over frames are disabled, unload all sounds from previous frame
		// Otherwise, only unload sounds that aren't used in this frame
		for (int h = 0; h < nHandlesReel; h++) {
			if ((app.gaNewFlags & CRunApp.GANF_SAMPLESOVERFRAMES) == 0 || useCount[h] == 0) {
				if (sounds != null && sounds[h] != null) {
					if (!player.isSamplePlaying((short)h)) {
						sounds[h].release();
						sounds[h] = null;
					} else {
						sounds[h].markedForDeletion = true;
					}
				}
			}
		}

		CSound[] newSounds = new CSound[nHandlesReel];
		for (int h = 0; h < nHandlesReel; h++) {
			if (useCount[h] != 0) {
				// If sound is already loaded, just copy it
				if (sounds != null && sounds[h] != null) {
					newSounds[h] = sounds[h];
				} else {
					String name = handleToSoundName[h];
					if (name == null) name = "";

					newSounds[h] = new CSound(name, (short)h, handleToFrequency[h], handleToFlags[h]);
					newSounds[h].load((short)h);
				}
			}
		}

		sounds = newSounds;
		resetToLoad();
		CRunApp.allSoundsPreLoaded = true;
	}
	public void unloadAll() {
		if (sounds == null) return;

		for (int h = 0; h < nHandlesReel; h++) {
			if (sounds[h] != null) {
				sounds[h].release();
				sounds[h] = null;
			}
		}
	}
}
