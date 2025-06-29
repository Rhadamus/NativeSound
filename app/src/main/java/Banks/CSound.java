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

import android.content.ContentResolver;
import android.content.res.AssetFileDescriptor;
import android.content.res.Resources;
import android.net.Uri;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.Locale;

import Runtime.Log;
import Runtime.MMFRuntime;

public class CSound {
    private long ptr;
    private final String filename;
    public boolean markedForDeletion;

    public CSound(String name, short handle, int frequency, int flags) {
        allocNative1(name, handle, frequency, flags);
        filename = null;
        markedForDeletion = false;
    }
    private native void allocNative1(String name, short handle, int frequency, int flags);

    public CSound(String filename) {
        allocNative2();
        this.filename = filename;
        markedForDeletion = false;
    }
    private native void allocNative2();
    
    public long[] openFd(short h) {
        AssetFileDescriptor fd = null;
        if (MMFRuntime.inst.assetsAvailable) {
            fd = MMFRuntime.inst.getSoundFromAssets(String.format(Locale.US, "s%04d", h));
        } else if (MMFRuntime.inst.obbAvailable) {
            fd = MMFRuntime.inst.getSoundFromOBB(String.format(Locale.US, "res/raw/s%04d", h));
        }
        if (fd == null) {
            try {
                fd = MMFRuntime.inst.getResources().openRawResourceFd(MMFRuntime.inst.getResourceID(String.format(Locale.US, "raw/s%04d", h)));
            } catch (Resources.NotFoundException e) {
                Log.Log("Could not find sound with handle " + h);
                return null;
            }
        }
        
        return new long[] { (long)fd.getParcelFileDescriptor().detachFd(), fd.getStartOffset(), fd.getLength() };
    }
    public long[] openFd() {
        AssetFileDescriptor fd;
        if (filename.contains("/")) {
            Uri uri;
            if (filename.startsWith(ContentResolver.SCHEME_FILE) || filename.startsWith(ContentResolver.SCHEME_CONTENT) || filename.startsWith(ContentResolver.SCHEME_ANDROID_RESOURCE)) {
                uri = Uri.parse(filename);
            } else {
                uri = Uri.fromFile(new File(filename));
            }

            try {
                fd = MMFRuntime.inst.getContentResolver().openAssetFileDescriptor(uri, "r");
            } catch (FileNotFoundException e) {
                Log.Log("Could not find sound file " + filename);
                return null;
            }
        } else {
            try {
                fd = MMFRuntime.inst.getResources().getAssets().openFd(filename);
            } catch (IOException e) {
                Log.Log("Could not open sound file " + filename);
                return null;
            }
        }
        
        return new long[] { (long)fd.getParcelFileDescriptor().detachFd(), fd.getStartOffset(), fd.getLength() };
    }
    
    public native void load();
    public native void release();
}