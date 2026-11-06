/****************************************************************************
Copyright (c) 2015-2016 Chukong Technologies Inc.
Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
Copyright (c) 2019-present Axmol Engine contributors (see AUTHORS.md).
 
https://axmol.dev/

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/
package dev.axmol.app;

import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.ImageView;

import java.io.InputStream;
import java.lang.ref.WeakReference;

import dev.axmol.lib.AxmolActivity;
import dev.axmol.lib.SharedLoader;
import android.os.Build;

public class AppActivity extends AxmolActivity {
    private static final String TAG = "AppActivity";
    private static final int SPLASH_FALLBACK_MS = 10000;
    private static final Handler sUiHandler = new Handler(Looper.getMainLooper());
    private static WeakReference<AppActivity> sInstance = new WeakReference<>(null);

    private View splashOverlay;

    static {
        // DNT remove, some android simulator require explicit load shared libraries, otherwise will crash
        SharedLoader.load();
    }
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        sInstance = new WeakReference<>(this);
        super.setEnableVirtualButton(false);
        super.onCreate(savedInstanceState);
        // Workaround in https://stackoverflow.com/questions/16283079/re-launch-of-activity-on-home-button-but-only-the-first-time/16447508
        if (!isTaskRoot()) {
            // Android launched another instance of the root activity into an existing task
            //  so just quietly finish and go away, dropping the user back into the activity
            //  at the top of the stack (ie: the last state of this task)
            // Don't need to finish it again since it's finished in super.onCreate .
            return;
        }
        // Make sure we're running on Pie or higher to change cutout mode
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            // Enable rendering into the cutout area
            WindowManager.LayoutParams lp = getWindow().getAttributes();
            lp.layoutInDisplayCutoutMode = WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES;
            getWindow().setAttributes(lp);
        }
        // DO OTHER INITIALIZATION BELOW
        AdsBridge.init(this);
        BillingBridge.init(this);
        HapticsBridge.init(this);
        NotificationBridge.init(this);
        LinkBridge.init(this);
        
    }

    @Override
    public void init() {
        super.init();
        showSplashOverlay();
    }

    /**
     * Android shows the launcher icon (or a blank window) while the native engine is loading. Cover that window
     * with the same artwork the game uses for its loading scene, so the launch looks the same as on iOS.
     */
    private void showSplashOverlay() {
        if (splashOverlay != null || mFrameLayout == null) {
            return;
        }

        ImageView splash = new ImageView(this);
        splash.setScaleType(ImageView.ScaleType.CENTER_CROP);
        splash.setBackgroundColor(0xFF0B1117);
        splash.setClickable(false);
        splash.setFocusable(false);

        try {
            int resId = getResources().getIdentifier("splash_background", "drawable", getPackageName());
            if (resId != 0) {
                splash.setImageResource(resId);
            } else {
                try (InputStream in = getAssets().open("Images/HD/GeneralApplicationSplashScreen.png")) {
                    splash.setImageBitmap(BitmapFactory.decodeStream(in));
                }
            }
        } catch (Exception e) {
            Log.w(TAG, "Unable to load splash overlay", e);
        }

        mFrameLayout.addView(splash, new ViewGroup.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));
        splashOverlay = splash;

        sUiHandler.postDelayed(this::hideSplashOverlay, SPLASH_FALLBACK_MS);
    }

    private void hideSplashOverlay() {
        final View overlay = splashOverlay;
        if (overlay == null) {
            return;
        }
        splashOverlay = null;
        overlay.animate()
                .alpha(0f)
                .setDuration(250)
                .withEndAction(() -> {
                    if (overlay.getParent() instanceof ViewGroup) {
                        ((ViewGroup) overlay.getParent()).removeView(overlay);
                    }
                })
                .start();
    }

    /**
     * Called from C++ once the first game scene is running. The delay lets the engine finish its first frame
     * before the overlay fades away, avoiding a flash of an empty window.
     */
    public static void hideSplashAfter(final int delayMs) {
        sUiHandler.postDelayed(() -> {
            AppActivity activity = sInstance.get();
            if (activity != null) {
                activity.hideSplashOverlay();
            }
        }, Math.max(0, delayMs));
    }

    @Override
    protected void onDestroy() {
        if (sInstance.get() == this) {
            sInstance = new WeakReference<>(null);
        }
        hideSplashOverlay();
        BillingBridge.shutdown();
        super.onDestroy();
    }
}
