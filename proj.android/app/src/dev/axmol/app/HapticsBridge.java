package dev.axmol.app;

import android.content.Context;
import android.os.Build;
import android.os.VibrationEffect;
import android.os.Vibrator;
import android.os.VibratorManager;
import android.util.Log;

/**
 * Java side of the haptics bridge. Called from C++ (PZHaptics.cpp) through JniHelper with a kind id that
 * mirrors PZHaptics::Kind. Uses amplitude-controlled VibrationEffects when the device supports them and
 * silently does nothing when there is no vibrator.
 */
public class HapticsBridge {
    private static final String TAG = "HapticsBridge";
    private static Vibrator vibrator;

    public static void init(Context context) {
        try {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
                VibratorManager vm = (VibratorManager) context.getSystemService(Context.VIBRATOR_MANAGER_SERVICE);
                vibrator = vm != null ? vm.getDefaultVibrator() : null;
            } else {
                vibrator = (Vibrator) context.getSystemService(Context.VIBRATOR_SERVICE);
            }
        } catch (Throwable t) {
            Log.w(TAG, "no vibrator: " + t);
            vibrator = null;
        }
    }

    private static void one(long ms, int amplitude) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            vibrator.vibrate(VibrationEffect.createOneShot(ms, vibrator.hasAmplitudeControl() ? amplitude : VibrationEffect.DEFAULT_AMPLITUDE));
        } else {
            vibrator.vibrate(ms);
        }
    }

    private static void wave(long[] timings, int[] amps) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            if (vibrator.hasAmplitudeControl()) {
                vibrator.vibrate(VibrationEffect.createWaveform(timings, amps, -1));
            } else {
                vibrator.vibrate(VibrationEffect.createWaveform(timings, -1));
            }
        } else {
            vibrator.vibrate(timings, -1);
        }
    }

    public static void play(int kind) {
        if (vibrator == null || !vibrator.hasVibrator()) return;
        try {
            switch (kind) {
                case 0: one(6, 70); break;            // tick
                case 1: one(12, 110); break;          // light
                case 2: one(22, 170); break;          // medium
                case 3: one(40, 255); break;          // heavy
                case 4: wave(new long[]{0, 14, 60, 26}, new int[]{0, 150, 0, 230}); break;                       // success
                case 5: wave(new long[]{0, 22, 50, 22}, new int[]{0, 210, 0, 210}); break;                       // warning
                case 6: wave(new long[]{0, 30, 45, 30, 45, 60}, new int[]{0, 230, 0, 230, 0, 255}); break;       // error
                default: one(5, 60); break;           // selection
            }
        } catch (Throwable t) {
            Log.w(TAG, "haptic failed: " + t);
        }
    }
}
