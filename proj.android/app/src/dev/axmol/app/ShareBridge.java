package dev.axmol.app;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import androidx.core.content.FileProvider;

import java.io.File;

/** Opens the Android share sheet for a result-card PNG (FileProvider, no storage permission). Called from V2Share.cpp over JNI. */
public class ShareBridge {
    private static final String TAG = "ShareBridge";
    private static final Handler UI = new Handler(Looper.getMainLooper());
    private static Activity activity;

    public static void init(Activity a) { activity = a; }

    public static void shareImage(final String path, final String text) {
        UI.post(() -> {
            if (activity == null || path == null) return;
            try {
                Uri uri = FileProvider.getUriForFile(activity, activity.getPackageName() + ".fileprovider", new File(path));
                Intent send = new Intent(Intent.ACTION_SEND);
                send.setType("image/png");
                send.putExtra(Intent.EXTRA_STREAM, uri);
                send.putExtra(Intent.EXTRA_TEXT, text == null ? "" : text);
                send.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
                activity.startActivity(Intent.createChooser(send, "Share your result"));
            } catch (Throwable t) {
                Log.w(TAG, "share failed: " + t);
            }
        });
    }
}
