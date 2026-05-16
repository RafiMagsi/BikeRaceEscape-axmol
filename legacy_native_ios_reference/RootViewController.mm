/****************************************************************************
 Copyright (c) 2010-2011 cocos2d-x.org
 Copyright (c) 2010      Ricardo Quesada
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#import "RootViewController.h"

#import <iAd/ADBannerView_Deprecated.h>

#include "cocos2d.h"
#include "SimpleAudioEngine.h"
#include "PZGSharedData.h"



@implementation RootViewController


-(void)removeAd{
//    [[Neocortex getInstance] removeBannerAd];
}

-(void)initAd{

//    [[Neocortex getInstance] showBannerAd];
    
}
- (void)bannerView:(ADBannerView *)banner didFailToReceiveAdWithError:(NSError *)error{
    NSLog(@"iad banner load error: %@", [error description]);
}

- (void)bannerViewDidLoadAd:(ADBannerView *)banner{
    banner.hidden = NO;
}


- (BOOL)bannerViewActionShouldBegin:(ADBannerView *)banner willLeaveApplication:(BOOL)willLeave{
    cocos2d::CCDirector::sharedDirector()->pause();    
    CocosDenshion::SimpleAudioEngine::sharedEngine()->pauseBackgroundMusic();

    return YES;
}

- (void)bannerViewActionDidFinish:(ADBannerView *)banner{
    cocos2d::CCDirector::sharedDirector()->resume();
    CocosDenshion::SimpleAudioEngine::sharedEngine()->resumeBackgroundMusic();
}


- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    
    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    cocos2d::CCArray* array = (cocos2d::CCArray*)gsd->gameInfoResource->objectForKey("GeneralApplicationSettings");
    PZGGameInfoGeneral * gameInfoGeneral = (PZGGameInfoGeneral*)array->objectAtIndex(0);
    
    if (gameInfoGeneral->isPortraitMode) {
        return UIInterfaceOrientationIsPortrait( interfaceOrientation );
    }
    else {
        return UIInterfaceOrientationIsLandscape( interfaceOrientation );
    }
}
- (NSUInteger) supportedInterfaceOrientations{
    PZGSharedData *gsd = PZGSharedData::sharedInstanse();
    cocos2d::CCArray* array = (cocos2d::CCArray*)gsd->gameInfoResource->objectForKey("GeneralApplicationSettings");
    PZGGameInfoGeneral * gameInfoGeneral = (PZGGameInfoGeneral*)array->objectAtIndex(0);

    if (gameInfoGeneral->isPortraitMode) {
        return UIInterfaceOrientationMaskPortrait;
    }
    else {
        return UIInterfaceOrientationMaskLandscape;
    }
}

- (BOOL) shouldAutorotate {
    return YES;
}

- (void)didReceiveMemoryWarning {
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload {
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}


- (void)dealloc {
    [super dealloc];
}


@end
