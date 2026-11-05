package dev.axmol.app;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Build;

/** Fired by the alarm scheduled in {@link NotificationBridge}: posts the reminder and opens the game when tapped. */
public class NotificationReceiver extends BroadcastReceiver {
    @Override
    public void onReceive(Context context, Intent intent) {
        String title = intent.getStringExtra("title");
        String body = intent.getStringExtra("body");
        int id = intent.getIntExtra("id", 1);
        if (title == null || body == null) return;

        NotificationManager nm = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
        if (nm == null || !nm.areNotificationsEnabled()) return;

        Intent launch = context.getPackageManager().getLaunchIntentForPackage(context.getPackageName());
        PendingIntent open = launch == null ? null
                : PendingIntent.getActivity(context, id, launch, PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE);

        Notification.Builder b = Build.VERSION.SDK_INT >= Build.VERSION_CODES.O
                ? new Notification.Builder(context, NotificationBridge.CHANNEL_ID)
                : new Notification.Builder(context);
        // Looked up by name: the app's R class lives in the application-id package, not in dev.axmol.app. The icon is kept
        // from resource shrinking by res/raw/keep.xml.
        int icon = context.getResources().getIdentifier("ic_stat_reminder", "drawable", context.getPackageName());
        if (icon == 0) icon = context.getApplicationInfo().icon;
        b.setSmallIcon(icon)
                .setContentTitle(title)
                .setContentText(body)
                .setStyle(new Notification.BigTextStyle().bigText(body))
                .setAutoCancel(true);
        if (open != null) b.setContentIntent(open);
        nm.notify(id, b.build());
    }
}
