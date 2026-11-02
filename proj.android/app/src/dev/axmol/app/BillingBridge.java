package dev.axmol.app;

import android.app.Activity;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import androidx.annotation.NonNull;

import com.android.billingclient.api.AcknowledgePurchaseParams;
import com.android.billingclient.api.BillingClient;
import com.android.billingclient.api.BillingClientStateListener;
import com.android.billingclient.api.BillingFlowParams;
import com.android.billingclient.api.BillingResult;
import com.android.billingclient.api.ConsumeParams;
import com.android.billingclient.api.PendingPurchasesParams;
import com.android.billingclient.api.ProductDetails;
import com.android.billingclient.api.Purchase;
import com.android.billingclient.api.QueryProductDetailsParams;
import com.android.billingclient.api.QueryPurchasesParams;

import java.util.Collections;

/**
 * Java side of the Google Play Billing bridge. Called from C++ (cocos2dx_StoreController.cpp)
 * through JniHelper. Purchase results are reported back to C++ via nativeOnPurchaseResult().
 */
public class BillingBridge {
    private static final String TAG = "BillingBridge";
    private static final Handler UI = new Handler(Looper.getMainLooper());

    private static BillingClient client;
    private static Activity activity;
    private static boolean connecting;
    private static Runnable pendingAction;

    private static final String[] CONSUMABLE_SUFFIXES = {".coin1", ".coin2", ".coin3"};

    public static void init(Activity a) {
        UI.post(() -> {
            activity = a;
            if (client != null) return;
            client = BillingClient.newBuilder(a.getApplicationContext())
                    .setListener((billingResult, purchases) -> {
                        if (billingResult.getResponseCode() == BillingClient.BillingResponseCode.OK
                                && purchases != null) {
                            for (Purchase purchase : purchases) {
                                handlePurchase(purchase, false);
                            }
                        } else if (billingResult.getResponseCode()
                                == BillingClient.BillingResponseCode.USER_CANCELED) {
                            Log.i(TAG, "purchase cancelled by user");
                            notifyNative("", "cancelled");
                        } else {
                            reportError("purchase update failed: "
                                    + describe(billingResult));
                        }
                    })
                    .enablePendingPurchases(PendingPurchasesParams.newBuilder()
                            .enableOneTimeProducts()
                            .build())
                    .build();
            ensureConnected(() -> {});
        });
    }

    public static void shutdown() {
        UI.post(() -> {
            if (client != null) {
                client.endConnection();
                client = null;
            }
            activity = null;
            pendingAction = null;
            connecting = false;
        });
    }

    /** Called from C++ for every StoreController::buyMarketItem(). */
    public static void buy(final String productId) {
        UI.post(() -> {
            if (productId == null || productId.isEmpty()) {
                reportError("buy called with empty product id");
                return;
            }
            ensureConnected(() -> queryAndLaunch(productId));
        });
    }

    /** Called from C++ for every StoreController::restorePurchases(). */
    public static void restore() {
        UI.post(() -> ensureConnected(BillingBridge::syncOwnedPurchases));
    }

    private static void ensureConnected(final Runnable then) {
        if (client == null) {
            reportError("billing client not initialized");
            return;
        }
        if (client.isReady()) {
            then.run();
            return;
        }

        pendingAction = then;
        if (connecting) return;

        connecting = true;
        client.startConnection(new BillingClientStateListener() {
            @Override
            public void onBillingSetupFinished(@NonNull BillingResult billingResult) {
                connecting = false;
                if (billingResult.getResponseCode() == BillingClient.BillingResponseCode.OK) {
                    Log.i(TAG, "billing connected");
                    final Runnable action = pendingAction;
                    pendingAction = null;
                    if (action != null) action.run();
                } else {
                    pendingAction = null;
                    reportError("billing setup failed: " + describe(billingResult));
                }
            }

            @Override
            public void onBillingServiceDisconnected() {
                connecting = false;
                Log.w(TAG, "billing service disconnected");
            }
        });
    }

    private static void queryAndLaunch(final String productId) {
        QueryProductDetailsParams.Product product = QueryProductDetailsParams.Product.newBuilder()
                .setProductId(productId)
                .setProductType(BillingClient.ProductType.INAPP)
                .build();
        QueryProductDetailsParams params = QueryProductDetailsParams.newBuilder()
                .setProductList(Collections.singletonList(product))
                .build();

        client.queryProductDetailsAsync(params, (billingResult, queryResult) -> {
            if (billingResult.getResponseCode() != BillingClient.BillingResponseCode.OK) {
                reportError("product details failed for '" + productId + "': "
                        + describe(billingResult));
                return;
            }

            ProductDetails details = null;
            if (queryResult != null) {
                for (ProductDetails candidate : queryResult.getProductDetailsList()) {
                    if (productId.equals(candidate.getProductId())) {
                        details = candidate;
                        break;
                    }
                }
            }
            if (details == null) {
                reportError("product not found in Play Console: '" + productId + "'");
                return;
            }
            if (activity == null) {
                reportError("no activity for billing flow");
                return;
            }

            BillingFlowParams.ProductDetailsParams productParams =
                    BillingFlowParams.ProductDetailsParams.newBuilder()
                            .setProductDetails(details)
                            .build();
            BillingFlowParams flowParams = BillingFlowParams.newBuilder()
                    .setProductDetailsParamsList(Collections.singletonList(productParams))
                    .build();
            BillingResult launchResult = client.launchBillingFlow(activity, flowParams);
            if (launchResult.getResponseCode() != BillingClient.BillingResponseCode.OK) {
                reportError("launch billing flow failed: " + describe(launchResult));
            }
        });
    }

    private static void syncOwnedPurchases() {
        if (client == null) return;

        QueryPurchasesParams params = QueryPurchasesParams.newBuilder()
                .setProductType(BillingClient.ProductType.INAPP)
                .build();
        client.queryPurchasesAsync(params, (billingResult, purchases) -> {
            if (billingResult.getResponseCode() != BillingClient.BillingResponseCode.OK) {
                Log.w(TAG, "query owned purchases failed: " + describe(billingResult));
                return;
            }
            if (purchases == null) return;
            for (Purchase purchase : purchases) {
                handlePurchase(purchase, true);
            }
        });
    }

    private static void handlePurchase(final Purchase purchase, final boolean restoring) {
        if (purchase.getPurchaseState() == Purchase.PurchaseState.PENDING) {
            Log.i(TAG, "purchase pending: " + purchase.getProducts());
            notifyNative(firstProduct(purchase), "pending");
            return;
        }
        if (purchase.getPurchaseState() != Purchase.PurchaseState.PURCHASED) {
            return;
        }

        for (String productId : purchase.getProducts()) {
            if (isConsumable(productId)) {
                // Coins are granted once per consumed purchase. Do not re-grant on restore.
                if (restoring) continue;
                consumePurchase(purchase, productId);
            } else {
                acknowledgePurchase(purchase, productId, restoring);
            }
        }
    }

    private static void consumePurchase(final Purchase purchase, final String productId) {
        ConsumeParams params = ConsumeParams.newBuilder()
                .setPurchaseToken(purchase.getPurchaseToken())
                .build();
        client.consumeAsync(params, (billingResult, purchaseToken) -> {
            if (billingResult.getResponseCode() == BillingClient.BillingResponseCode.OK) {
                notifyNative(productId, "purchased");
            } else {
                reportError("consume failed for '" + productId + "': "
                        + describe(billingResult));
            }
        });
    }

    private static void acknowledgePurchase(final Purchase purchase, final String productId,
                                             final boolean restoring) {
        if (purchase.isAcknowledged()) {
            notifyNative(productId, restoring ? "restored" : "purchased");
            return;
        }

        AcknowledgePurchaseParams params = AcknowledgePurchaseParams.newBuilder()
                .setPurchaseToken(purchase.getPurchaseToken())
                .build();
        client.acknowledgePurchase(params, billingResult -> {
            if (billingResult.getResponseCode() == BillingClient.BillingResponseCode.OK) {
                notifyNative(productId, restoring ? "restored" : "purchased");
            } else {
                reportError("acknowledge failed for '" + productId + "': "
                        + describe(billingResult));
            }
        });
    }

    private static boolean isConsumable(String productId) {
        for (String suffix : CONSUMABLE_SUFFIXES) {
            if (productId.endsWith(suffix)) return true;
        }
        return false;
    }

    private static String firstProduct(Purchase purchase) {
        return purchase.getProducts().isEmpty() ? "" : purchase.getProducts().get(0);
    }

    private static String describe(BillingResult result) {
        return "code=" + result.getResponseCode() + " " + result.getDebugMessage();
    }

    private static void notifyNative(String productId, String status) {
        try {
            nativeOnPurchaseResult(productId == null ? "" : productId, status);
        } catch (Throwable t) {
            Log.e(TAG, "native purchase callback failed", t);
        }
    }

    private static void reportError(String message) {
        Log.w(TAG, message);
        try {
            nativeOnBillingError(message == null ? "billing error" : message);
        } catch (Throwable t) {
            Log.e(TAG, "native billing error callback failed", t);
        }
    }

    private static native void nativeOnPurchaseResult(String productId, String status);

    private static native void nativeOnBillingError(String message);
}
