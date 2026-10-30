package dev.axmol.app;

import android.app.Activity;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.Gravity;
import android.view.View;
import android.widget.FrameLayout;

import androidx.annotation.NonNull;

import com.google.android.gms.ads.AdError;
import com.google.android.gms.ads.AdListener;
import com.google.android.gms.ads.AdRequest;
import com.google.android.gms.ads.AdSize;
import com.google.android.gms.ads.AdView;
import com.google.android.gms.ads.FullScreenContentCallback;
import com.google.android.gms.ads.LoadAdError;
import com.google.android.gms.ads.MobileAds;
import com.google.android.gms.ads.interstitial.InterstitialAd;
import com.google.android.gms.ads.interstitial.InterstitialAdLoadCallback;

/**
 * Java side of the AdMob bridge. Called from C++ (AdsProviderAdMob.cpp) through JniHelper.
 * All SDK calls are marshalled to the UI thread.
 */
public class AdsBridge {
    private static final String TAG = "AdsBridge";
    private static final Handler UI = new Handler(Looper.getMainLooper());

    private static Activity activity;
    private static AdView banner;
    private static String bannerId = "";
    private static InterstitialAd interstitial;
    private static volatile boolean interstitialReady = false;
    private static boolean interstitialLoading = false;

    public static void init(Activity a) {
        activity = a;
        MobileAds.initialize(a, status -> Log.i(TAG, "Mobile Ads initialized"));
    }

    public static void setBannerId(final String id) {
        bannerId = id == null ? "" : id;
    }

    public static void showBanner() {
        UI.post(() -> {
            if (activity == null || bannerId.isEmpty()) return;
            if (banner == null) {
                banner = new AdView(activity);
                banner.setAdUnitId(bannerId);
                int widthDp = (int) (activity.getResources().getDisplayMetrics().widthPixels
                        / activity.getResources().getDisplayMetrics().density);
                banner.setAdSize(AdSize.getCurrentOrientationAnchoredAdaptiveBannerAdSize(activity, widthDp));
                banner.setAdListener(new AdListener() {
                    @Override
                    public void onAdFailedToLoad(@NonNull LoadAdError e) {
                        Log.w(TAG, "banner failed: " + e.getMessage());
                    }
                });
                FrameLayout.LayoutParams lp = new FrameLayout.LayoutParams(
                        FrameLayout.LayoutParams.WRAP_CONTENT,
                        FrameLayout.LayoutParams.WRAP_CONTENT,
                        Gravity.BOTTOM | Gravity.CENTER_HORIZONTAL);
                activity.addContentView(banner, lp);
                banner.loadAd(new AdRequest.Builder().build());
            }
            banner.setVisibility(View.VISIBLE);
        });
    }

    public static void hideBanner() {
        UI.post(() -> {
            if (banner != null) banner.setVisibility(View.GONE);
        });
    }

    public static void loadInterstitial(final String unitId) {
        UI.post(() -> {
            if (activity == null || unitId == null || unitId.isEmpty()) return;
            if (interstitialLoading || interstitialReady) return;
            interstitialLoading = true;
            InterstitialAd.load(activity, unitId, new AdRequest.Builder().build(),
                    new InterstitialAdLoadCallback() {
                        @Override
                        public void onAdLoaded(@NonNull InterstitialAd ad) {
                            interstitial = ad;
                            interstitialReady = true;
                            interstitialLoading = false;
                            Log.i(TAG, "interstitial loaded");
                        }

                        @Override
                        public void onAdFailedToLoad(@NonNull LoadAdError e) {
                            interstitial = null;
                            interstitialReady = false;
                            interstitialLoading = false;
                            Log.w(TAG, "interstitial failed: " + e.getMessage());
                        }
                    });
        });
    }

    public static boolean isInterstitialReady() {
        return interstitialReady;
    }

    public static void showInterstitial() {
        UI.post(() -> {
            if (activity == null || interstitial == null) return;
            final InterstitialAd ad = interstitial;
            interstitial = null;
            interstitialReady = false;
            ad.setFullScreenContentCallback(new FullScreenContentCallback() {
                @Override
                public void onAdFailedToShowFullScreenContent(@NonNull AdError e) {
                    Log.w(TAG, "interstitial show failed: " + e.getMessage());
                }
            });
            ad.show(activity);
        });
    }
}
