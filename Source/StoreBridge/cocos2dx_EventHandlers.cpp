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

#include "cocos2dx_EventHandlers.h"

cocos2dx_EventHandlers* cocos2dx_EventHandlers::instance = NULL;

cocos2dx_EventHandlers* cocos2dx_EventHandlers::getInstance() {
	if ( instance == NULL ) {
		instance = new cocos2dx_EventHandlers();
	}

	return instance;
}
	
void cocos2dx_EventHandlers::addHandler(IEventHandler* handler) {
	handlers.insert(handler);
}
	
void cocos2dx_EventHandlers::removeHandler(IEventHandler* handler) {
	handlers.erase(handler);
}

void cocos2dx_EventHandlers::marketPurchase(string& productId) {
	set<IEventHandler*>::iterator itr;
	for ( itr = handlers.begin(); itr != handlers.end(); itr++ )
	    (*itr)->marketPurchase(productId);
}

void cocos2dx_EventHandlers::transactionRestored(string& productId) {
	set<IEventHandler*>::iterator itr;
	for ( itr = handlers.begin(); itr != handlers.end(); itr++ )
	    (*itr)->transactionRestored(productId);
}

void cocos2dx_EventHandlers::marketRefund(string& productId) {
	set<IEventHandler*>::iterator itr;
	for ( itr = handlers.begin(); itr != handlers.end(); itr++ )
	    (*itr)->marketRefund(productId);
}

void cocos2dx_EventHandlers::virtualGoodPurchased(string& itemId) {
	set<IEventHandler*>::iterator itr;
	for ( itr = handlers.begin(); itr != handlers.end(); itr++ )
	    (*itr)->virtualGoodPurchased(itemId);
}

void cocos2dx_EventHandlers::virtualGoodEquipped(string& itemId) {
	set<IEventHandler*>::iterator itr;
	for ( itr = handlers.begin(); itr != handlers.end(); itr++ )
	    (*itr)->virtualGoodEquipped(itemId);
}

void cocos2dx_EventHandlers::virtualGoodUnequipped(string& itemId) {
	set<IEventHandler*>::iterator itr;
	for ( itr = handlers.begin(); itr != handlers.end(); itr++ )
	    (*itr)->virtualGoodUnequipped(itemId);
}

void cocos2dx_EventHandlers::billingSupported() {
	set<IEventHandler*>::iterator itr;
	for ( itr = handlers.begin(); itr != handlers.end(); itr++ )
	    (*itr)->billingSupported();
}

void cocos2dx_EventHandlers::billingNotSupported() {
	set<IEventHandler*>::iterator itr;
	for ( itr = handlers.begin(); itr != handlers.end(); itr++ )
	    (*itr)->billingNotSupported();
}

void cocos2dx_EventHandlers::marketPurchaseProcessStarted(string& productId) {
	set<IEventHandler*>::iterator itr;
	for ( itr = handlers.begin(); itr != handlers.end(); itr++ )
	    (*itr)->marketPurchaseProcessStarted(productId);
}

void cocos2dx_EventHandlers::goodsPurchaseProcessStarted() {
	set<IEventHandler*>::iterator itr;
	for ( itr = handlers.begin(); itr != handlers.end(); itr++ )
	    (*itr)->goodsPurchaseProcessStarted();
}

void cocos2dx_EventHandlers::closingStore() {
	set<IEventHandler*>::iterator itr;
	for ( itr = handlers.begin(); itr != handlers.end(); itr++ )
	    (*itr)->closingStore();
}

void cocos2dx_EventHandlers::unexpectedErrorInStore( string& message ) {
	set<IEventHandler*>::iterator itr;
	for ( itr = handlers.begin(); itr != handlers.end(); itr++ )
	    (*itr)->unexpectedErrorInStore( message );
}

void cocos2dx_EventHandlers::paymentCancelled(){
	set<IEventHandler*>::iterator itr;
	for ( itr = handlers.begin(); itr != handlers.end(); itr++ )
	    (*itr)->paymentCancelled();
}

void cocos2dx_EventHandlers::openingStore() {
	set<IEventHandler*>::iterator itr;
	for ( itr = handlers.begin(); itr != handlers.end(); itr++ )
	    (*itr)->openingStore();
}

void cocos2dx_EventHandlers::currencyBalanceChanged(string& itemId, int balance) {
	set<IEventHandler*>::iterator itr;
	for ( itr = handlers.begin(); itr != handlers.end(); itr++ )
	    (*itr)->currencyBalanceChanged(itemId, balance);
}

void cocos2dx_EventHandlers::goodBalanceChanged(string& itemId, int balance) {
	set<IEventHandler*>::iterator itr;
	for ( itr = handlers.begin(); itr != handlers.end(); itr++ )
	    (*itr)->goodBalanceChanged(itemId, balance);
}

