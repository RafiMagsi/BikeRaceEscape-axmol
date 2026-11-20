package dev.axmol.app;

import android.app.Activity;
import android.content.Intent;
import android.util.Log;

import com.google.android.gms.games.PlayGames;
import com.google.android.gms.games.PlayGamesSdk;

/**
 * Google Play Games Services v2: signs in silently and submits / shows the daily leaderboard. Called from V2Leaderboard.cpp over JNI.
 * Needs (outside the repo): a Play Games Services project linked in Play Console, its APP_ID in a
 * {@code <meta-data android:name="com.google.android.gms.games.APP_ID" android:value="@string/game_services_project_id"/>} manifest entry,
 * and the leaderboard id in Content/CompleteGameInfo.plist (v2.leaderboards.androidDaily). Every call is a safe no-op on failure.
 */
public class PlayGamesBridge {
    private static final String TAG = "PlayGamesBridge";
    private static final int RC_LEADERBOARD = 9004;
    private static Activity activity;
    private static boolean initialised = false;

    public static void init(Activity a) { activity = a; }

    private static boolean ready() {
        if (activity == null) return false;
        try {
            if (!initialised) { PlayGamesSdk.initialize(activity); initialised = true; }
            return true;
        } catch (Throwable t) {
            Log.w(TAG, "Play Games not available: " + t);
            return false;
        }
    }

    public static void submitScore(final String leaderboardId, final int score) {
        if (leaderboardId == null || leaderboardId.isEmpty() || !ready()) return;
        try {
            PlayGames.getGamesSignInClient(activity).isAuthenticated().addOnCompleteListener(task -> {
                if (task.isSuccessful() && task.getResult().isAuthenticated()) {
                    PlayGames.getLeaderboardsClient(activity).submitScore(leaderboardId, score);
                }
            });
        } catch (Throwable t) { Log.w(TAG, "submit failed: " + t); }
    }

    public static void showLeaderboard(final String leaderboardId) {
        if (leaderboardId == null || leaderboardId.isEmpty() || !ready()) return;
        try {
            PlayGames.getGamesSignInClient(activity).signIn().addOnCompleteListener(signIn -> {
                if (!(signIn.isSuccessful() && signIn.getResult().isAuthenticated())) return;
                PlayGames.getLeaderboardsClient(activity).getLeaderboardIntent(leaderboardId).addOnSuccessListener(
                    (Intent intent) -> activity.startActivityForResult(intent, RC_LEADERBOARD));
            });
        } catch (Throwable t) { Log.w(TAG, "show failed: " + t); }
    }
}
