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
import com.google.android.gms.ads.rewarded.RewardedAd;
import com.google.android.gms.ads.rewarded.RewardedAdLoadCallback;
import com.google.android.ump.ConsentInformation;
import com.google.android.ump.ConsentRequestParameters;
import com.google.android.ump.UserMessagingPlatform;

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

    private static RewardedAd rewarded;
    private static volatile boolean rewardedReady = false;
    private static boolean rewardedLoading = false;
    private static boolean rewardEarned = false;

    /** Reports to C++ (PZ::AdsController::handleRewardedFinished) once a rewarded ad has finished. */
    private static native void nativeOnRewardedFinished(boolean earned);

    /** True once the Google UMP consent flow allows ad requests (always true outside regulated regions). */
    private static volatile boolean canRequestAds = false;
    private static boolean adsStarted = false;

    /**
     * AdMob policy: collect consent (EEA / UK / US states) with Google's UMP form before any ad request, and only start
     * the SDK once ads may be requested. The form is shown by the SDK only where required.
     */
    public static void init(Activity a) {
        activity = a;
        final ConsentInformation info = UserMessagingPlatform.getConsentInformation(a);
        ConsentRequestParameters params = new ConsentRequestParameters.Builder().build();
        info.requestConsentInfoUpdate(a, params,
                () -> UserMessagingPlatform.loadAndShowConsentFormIfRequired(a, formError -> {
                    if (formError != null) Log.w(TAG, "consent form: " + formError.getMessage());
                    startAds(info);
                }),
                requestError -> {
                    Log.w(TAG, "consent info: " + requestError.getMessage());
                    startAds(info);
                });
        startAds(info);   // consent from an earlier session may already allow ads
    }

    private static synchronized void startAds(ConsentInformation info) {
        if (!info.canRequestAds()) return;
        canRequestAds = true;
        if (adsStarted || activity == null) return;
        adsStarted = true;
        MobileAds.initialize(activity, status -> Log.i(TAG, "Mobile Ads initialized"));
    }

    /** Privacy options entry point (Settings): re-opens the consent form when the user's region requires it. */
    public static void showPrivacyOptions() {
        UI.post(() -> {
            if (activity == null) return;
            UserMessagingPlatform.showPrivacyOptionsForm(activity, err -> {
                if (err != null) Log.w(TAG, "privacy options: " + err.getMessage());
            });
        });
    }

    public static boolean privacyOptionsRequired() {
        return activity != null && UserMessagingPlatform.getConsentInformation(activity)
                .getPrivacyOptionsRequirementStatus() == ConsentInformation.PrivacyOptionsRequirementStatus.REQUIRED;
    }

    public static void setBannerId(final String id) {
        bannerId = id == null ? "" : id;
    }

    public static void showBanner() {
        UI.post(() -> {
            if (activity == null || bannerId.isEmpty() || !canRequestAds) return;
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
            if (activity == null || unitId == null || unitId.isEmpty() || !canRequestAds) return;
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

    // ------------------------------------------------------------------------------------------
    // Rewarded video (used for "watch a video to revive")
    // ------------------------------------------------------------------------------------------

    public static void loadRewarded(final String unitId) {
        UI.post(() -> {
            if (activity == null || unitId == null || unitId.isEmpty() || !canRequestAds) return;
            if (rewardedLoading || rewardedReady) return;
            rewardedLoading = true;
            RewardedAd.load(activity, unitId, new AdRequest.Builder().build(), new RewardedAdLoadCallback() {
                @Override
                public void onAdLoaded(@NonNull RewardedAd ad) {
                    rewarded = ad;
                    rewardedReady = true;
                    rewardedLoading = false;
                    Log.i(TAG, "rewarded loaded");
                }

                @Override
                public void onAdFailedToLoad(@NonNull LoadAdError e) {
                    rewarded = null;
                    rewardedReady = false;
                    rewardedLoading = false;
                    Log.w(TAG, "rewarded failed: " + e.getMessage());
                }
            });
        });
    }

    public static boolean isRewardedReady() {
        return rewardedReady;
    }

    public static void showRewarded() {
        UI.post(() -> {
            if (activity == null || rewarded == null) {
                nativeOnRewardedFinished(false);
                return;
            }
            final RewardedAd ad = rewarded;
            rewarded = null;
            rewardedReady = false;
            rewardEarned = false;
            ad.setFullScreenContentCallback(new FullScreenContentCallback() {
                @Override
                public void onAdDismissedFullScreenContent() {
                    nativeOnRewardedFinished(rewardEarned);
                }

                @Override
                public void onAdFailedToShowFullScreenContent(@NonNull AdError e) {
                    Log.w(TAG, "rewarded show failed: " + e.getMessage());
                    nativeOnRewardedFinished(false);
                }
            });
            ad.show(activity, rewardItem -> rewardEarned = true);
        });
    }
}
