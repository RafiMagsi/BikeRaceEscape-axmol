package dev.axmol.app;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

/** Opens a web page (privacy policy) in the system browser. Needs no permission. Called from PZLinks.cpp over JNI. */
public class LinkBridge {
    private static final String TAG = "LinkBridge";
    private static final Handler UI = new Handler(Looper.getMainLooper());
    private static Activity activity;

    public static void init(Activity a) { activity = a; }

    public static void openUrl(final String url) {
        UI.post(() -> {
            if (activity == null || url == null) return;
            try {
                activity.startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse(url)));
            } catch (Throwable t) {
                Log.w(TAG, "cannot open " + url + ": " + t);
            }
        });
    }
}
