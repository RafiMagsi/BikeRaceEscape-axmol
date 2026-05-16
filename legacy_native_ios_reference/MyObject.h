//
//  MyObject.h
//  DogAgility
//
//  Created by Developer on 04/07/2013.
//
//

#import "MyObject-C-Interface.h"

// An Objective-C class that needs to be accessed from C++
@interface MyObject : NSObject
{
    int someVar;
}

// The Objective-C member function you want to call from C++
- (void) doSomethingWith;
@end
