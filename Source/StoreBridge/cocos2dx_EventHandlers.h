/*
 * Copyright (C) 2012 Soomla Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __cocos2dx_store__cocos2dx_EventHandler__
#define __cocos2dx_store__cocos2dx_EventHandler__

#include <string>
#include <set>

using namespace std;

class IEventHandler {
public:
    virtual void marketPurchase(string& productId) = 0;
    virtual void transactionRestored(string& productId) = 0;
    virtual void marketRefund(string& productId) = 0;
    virtual void virtualGoodPurchased(string& itemId) = 0;
    virtual void virtualGoodEquipped(string& itemId) = 0;
    virtual void virtualGoodUnequipped(string& itemId) = 0;
    virtual void billingSupported() = 0;
    virtual void billingNotSupported() = 0;
    virtual void marketPurchaseProcessStarted(string& productId) = 0;
    virtual void goodsPurchaseProcessStarted() = 0;
    virtual void closingStore() = 0;
    virtual void unexpectedErrorInStore( string& message ) = 0;
    virtual void paymentCancelled() = 0;
    virtual void openingStore() = 0;
	virtual void currencyBalanceChanged(string &itemId, int balance) = 0;
	virtual void goodBalanceChanged(string &itemId, int balance) = 0;
};

class cocos2dx_EventHandlers {
private:
	static cocos2dx_EventHandlers* instance;
	set <IEventHandler*> handlers;
	cocos2dx_EventHandlers() { };
public:
	static cocos2dx_EventHandlers* getInstance();
	void addHandler(IEventHandler* handler);
	void removeHandler(IEventHandler* handler);
	    
    void marketPurchase(string& productId);
    void transactionRestored(string& productId);
    void marketRefund(string& productId);
    void virtualGoodPurchased(string& itemId);
    void virtualGoodEquipped(string& itemId);
    void virtualGoodUnequipped(string& itemId);
    void billingSupported();
    void billingNotSupported();
    void marketPurchaseProcessStarted(string& productId);
    void goodsPurchaseProcessStarted();
    void closingStore();
    void unexpectedErrorInStore( string& message );
    void paymentCancelled();
    void openingStore();
	void currencyBalanceChanged(string &itemId, int balance);
	void goodBalanceChanged(string &itemId, int balance);
};

#endif /* defined(__cocos2dx_store__cocos2dx_EventHandler__) */
