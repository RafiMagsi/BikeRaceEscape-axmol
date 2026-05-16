
#import <Foundation/Foundation.h>
#import <StoreKit/StoreKit.h>


@protocol PZStoreObserverProtocol <NSObject>
	-(void)transactionDidFinish:(NSString*)transactionIdentifier;
	-(void)transactionDidError;
	-(void)transactionDidFinishRestoring;
@end


@interface PZStoreObserver : NSObject<SKPaymentTransactionObserver> {
	id <PZStoreObserverProtocol> delegate;	
}

@property (nonatomic, assign) id <PZStoreObserverProtocol> delegate;

- (void) paymentQueue:(SKPaymentQueue *)queue updatedTransactions:(NSArray *)transactions;
- (void) restoreTransaction: (SKPaymentTransaction *)transaction;
- (void) completeTransaction: (SKPaymentTransaction *)transaction;
@end

