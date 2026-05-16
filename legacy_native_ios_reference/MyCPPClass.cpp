//
//  MyCPPClass.cpp
//  DogAgility
//
//  Created by Developer on 04/07/2013.
//
//

#include "MyCPPClass.h"
#include "MyObject-C-Interface.h"

void MyCPPClass::someMethod()
{
    // To invoke an Objective-C method from C++, use
    // the C trampoline function
    MyObjectDoSomethingWith ();
}