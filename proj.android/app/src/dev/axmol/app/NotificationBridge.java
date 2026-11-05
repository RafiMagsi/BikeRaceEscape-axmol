package dev.axmol.app;

import android.Manifest;
import android.app.Activity;
import android.app.AlarmManager;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.os.Build;
import android.util.Log;

/**
 * Local reminder notifications. C++ (PZNotifications.cpp) calls the static methods through JniHelper. Reminders are
 * plain inexact alarms that fire {@link NotificationReceiver}; nothing needs a server and no exact-alarm permission.
 * Alarms do not survive a reboot, which is fine: the schedule is rebuilt every time the app goes to the background.
 */
public class NotificationBridge {
    private static final String TAG = "NotificationBridge";
    static final String CHANNEL_ID = "dino_reminders";
    private static final String PREFS = "pz_notifications";
    private static final int MAX_IDS = 24;

    private static Activity activity;

    public static void init(Activity a) {
        activity = a;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            NotificationManager nm = (NotificationManager) a.getSystemService(Context.NOTIFICATION_SERVICE);
            if (nm != null) {
                NotificationChannel ch = new NotificationChannel(CHANNEL_ID, "Gifts and rewards", NotificationManager.IMPORTANCE_DEFAULT);
                ch.setDescription("Reminders when your free gift or daily reward is ready");
                nm.createNotificationChannel(ch);
            }
        }
    }

    public static boolean hasPermission() {
        if (activity == null) return false;
        if (Build.VERSION.SDK_INT >= 33 && activity.checkSelfPermission(Manifest.permission.POST_NOTIFICATIONS) != PackageManager.PERMISSION_GRANTED) {
            return false;
        }
        NotificationManager nm = (NotificationManager) activity.getSystemService(Context.NOTIFICATION_SERVICE);
        return nm == null || nm.areNotificationsEnabled();
    }

    /** True when the system dialog can still be shown (Android 13+, not permanently declined). */
    public static boolean canAskAgain() {
        if (activity == null || Build.VERSION.SDK_INT < 33) return false;
        SharedPreferences sp = activity.getSharedPreferences(PREFS, Context.MODE_PRIVATE);
        if (!sp.getBoolean("asked", false)) return true;
        return activity.shouldShowRequestPermissionRationale(Manifest.permission.POST_NOTIFICATIONS);
    }

    public static void requestPermission() {
        final Activity a = activity;
        if (a == null || Build.VERSION.SDK_INT < 33) return;
        a.runOnUiThread(() -> {
            a.getSharedPreferences(PREFS, Context.MODE_PRIVATE).edit().putBoolean("asked", true).apply();
            a.requestPermissions(new String[]{Manifest.permission.POST_NOTIFICATIONS}, 4711);
        });
    }

    public static void schedule(int id, String title, String body, long delayMs) {
        if (activity == null) return;
        try {
            AlarmManager am = (AlarmManager) activity.getSystemService(Context.ALARM_SERVICE);
            Intent i = new Intent(activity, NotificationReceiver.class);
            i.putExtra("title", title);
            i.putExtra("body", body);
            i.putExtra("id", id);
            PendingIntent pi = PendingIntent.getBroadcast(activity, id, i, PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE);
            long at = System.currentTimeMillis() + Math.max(60_000L, delayMs);
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                am.setAndAllowWhileIdle(AlarmManager.RTC_WAKEUP, at, pi);
            } else {
                am.set(AlarmManager.RTC_WAKEUP, at, pi);
            }
        } catch (Throwable t) {
            Log.w(TAG, "schedule failed: " + t);
        }
    }

    public static void cancelAll() {
        if (activity == null) return;
        try {
            AlarmManager am = (AlarmManager) activity.getSystemService(Context.ALARM_SERVICE);
            for (int id = 1; id <= MAX_IDS; ++id) {
                Intent i = new Intent(activity, NotificationReceiver.class);
                PendingIntent pi = PendingIntent.getBroadcast(activity, id, i, PendingIntent.FLAG_NO_CREATE | PendingIntent.FLAG_IMMUTABLE);
                if (pi != null) {
                    am.cancel(pi);
                    pi.cancel();
                }
            }
        } catch (Throwable t) {
            Log.w(TAG, "cancel failed: " + t);
        }
    }
}
