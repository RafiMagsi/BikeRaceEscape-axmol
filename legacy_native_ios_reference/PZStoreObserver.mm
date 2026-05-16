
#import "AppController.h"
#import "PZStoreObserver.h"

@implementation PZStoreObserver
@synthesize delegate;

- (void) failedTransaction: (SKPaymentTransaction *)transaction
{
    if (transaction.error.code != SKErrorPaymentCancelled)
    {
        // Optionally, display an error here.
		NSLog(@"transaction.error: %@", [transaction.error localizedDescription]);
        
        
        UIAlertView* alertView = [[UIAlertView alloc] initWithTitle:@"Transaction Error" message:[transaction.error localizedDescription]
                                                           delegate:self 
                                                  cancelButtonTitle:@"OK" 
                                                  otherButtonTitles:nil];
        [alertView show];
        [alertView release];

    }
    [[SKPaymentQueue defaultQueue] finishTransaction: transaction];

	[delegate transactionDidError];
}

- (void)paymentQueue:(SKPaymentQueue *)queue restoreCompletedTransactionsFailedWithError:(NSError *)error{
    UIAlertView* alertView = [[UIAlertView alloc] initWithTitle:@"Restore Purchases Error" message:nil
                                                       delegate:self
                                              cancelButtonTitle:@"OK"
                                              otherButtonTitles:nil];
    [alertView show];
    [alertView release];
    
	[delegate transactionDidError];
}


-(void)recordTransaction:(SKPaymentTransaction *)transaction {
    NSString *productId = [[[NSBundle mainBundle] bundleIdentifier] stringByAppendingString:@".upgrade"];
    
    if([transaction.payment.productIdentifier isEqualToString:productId]){
		[[NSUserDefaults standardUserDefaults] setBool:YES forKey:productId];
	}
    
}

- (void)paymentQueueRestoreCompletedTransactionsFinished:(SKPaymentQueue *)queue {
    NSLog(@"%@",queue );
    NSLog(@"Restored Transactions are once again in Queue for purchasing %@",[queue transactions]);
    
    for (SKPaymentTransaction *transaction in queue.transactions){
        
        NSString *queue_pid = transaction.payment.productIdentifier;
        NSString *productId = [[[NSBundle mainBundle] bundleIdentifier] stringByAppendingString:@".upgrade"];
        
        if([queue_pid isEqualToString:productId]){
            [[NSUserDefaults standardUserDefaults] setBool:YES forKey:productId];
            [delegate transactionDidFinishRestoring];
        }
    }
}

-(void)provideContent:(NSString *)identifier{
	
}

- (void) completeTransaction: (SKPaymentTransaction *)transaction{
	NSLog(@"IAP purchased: %@", transaction.payment.productIdentifier);
		
    [self recordTransaction: transaction];
    [self provideContent: transaction.payment.productIdentifier];
    [[SKPaymentQueue defaultQueue] finishTransaction: transaction];
    
    [delegate transactionDidFinish:transaction.payment.productIdentifier];
}


- (void) restoreTransaction: (SKPaymentTransaction *)transaction{
	NSLog(@"Transaction Restored");

    [self recordTransaction: transaction];
    [self provideContent: transaction.originalTransaction.payment.productIdentifier];
    [[SKPaymentQueue defaultQueue] finishTransaction: transaction];
}

- (void)paymentQueue:(SKPaymentQueue *)queue updatedTransactions:(NSArray *)transactions{
    for (SKPaymentTransaction *transaction in transactions)
    {
        switch (transaction.transactionState)
        {
            case SKPaymentTransactionStatePurchased:
                [self completeTransaction:transaction];
                break;
            case SKPaymentTransactionStateFailed:
                [self failedTransaction:transaction];
                break;
            case SKPaymentTransactionStateRestored:
                [self restoreTransaction:transaction];
            default:
                break;
        }
    }
}
@end
