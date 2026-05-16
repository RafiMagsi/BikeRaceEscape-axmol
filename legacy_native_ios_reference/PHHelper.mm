//
//  PHHelper.mm
//  PZPlayer
//
//  Created by Rafi on 7/3/13.
//
//

#import "PHHelper.h"

@implementation PHHelper

// C "trampoline" function to invoke Objective-C method
void PHCall(void *self)
{
    // Call the Objective-C method using Objective-C syntax
    [self PHCall];
}

- (void)PHCall
{
    // The Objective-C function you wanted to call from C++.
    // do work here..
//    return 21 ; // half of 42
}
@end
