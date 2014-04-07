// notification.h
//
// a set of classess to implement a simple mechanism of notification center similar 
// to NSNotificationCenter on Mac.
//
//Copyright (c) 2014, acsolu@gmail.com
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions are met:
//
//* Redistributions of source code must retain the above copyright notice, this
//  list of conditions and the following disclaimer.
//
//* Redistributions in binary form must reproduce the above copyright notice,
//  this list of conditions and the following disclaimer in the documentation
//  and/or other materials provided with the distribution.
//
//* Neither the name of the {organization} nor the names of its
//  contributors may be used to endorse or promote products derived from
//  this software without specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#pragma once

#include <conio.h>
#include <typeinfo>
#include <iostream>
#include <string>
#include <map>
#include <set>
#include <functional>
#include <algorithm>

#include <Windows.h>
#include <time.h>
#include <thread>

#include "autoptr.h"
#include "autocriticalsection.h"

using namespace std;

#ifdef UNICODE 
typedef string NString;
#else
typedef wstring NString;
#endif



namespace Win32NC{

	typedef map<NString, NString> Dictionary;
	// Notification class represents the notification object contains information about a notification.
	class Notification
	{
	public:
		Notification(){}
		Notification(const NString& name, void* sender, const Dictionary& info){
			this->name = name;
			this->sender = sender;
			this->userInfo = info;
		}
		NString name;
		void* sender;
		Dictionary userInfo;
	};

	// The abstract class defines structure of observers
	class AbstractObserver
	{
	public:
		virtual void Notify(Notification* notify) = 0;

		virtual AbstractObserver* Clone() = 0;

		virtual bool EqualTo(const AbstractObserver* pObserver){
			return ( this->GetCaller() == pObserver->GetCaller() && this->GetSelector() == pObserver->GetSelector() );
		}
	protected:
		virtual void* GetCaller() const = 0;
		virtual void* GetSelector() const = 0;
	};

	//The base class of observers, implemets general behaviors of a observer
	template<class C, class N>
	class ObserverBase : public AbstractObserver
	{
	public:
		typedef void (C::*SEL)(N*);
		typedef void (*FUNC)(N*);
		union SelectorType{
			SEL		sel;
			FUNC	func;
			void*	pvoid;
		};
		ObserverBase(C* caller, SEL selector){
			_caller = caller;
			_selector.sel = selector;
			//_function = NULL;

		}

		ObserverBase(FUNC selector){
			_caller = NULL;
			_selector.func = selector;
		}

		virtual void Notify(Notification* notification){
			N* pNF = dynamic_cast<N*>(notification);
			if( !_caller.isNull() && _selector.sel){
				SEL sel = (SEL)_selector.sel;
				(_caller.p()->*sel)( pNF );
			} else if( _selector.func ){
				(*_selector.func)( pNF );
			}
		}

		virtual AbstractObserver* Clone(){return NULL;}

		virtual void* GetCaller() const {

			return _caller.p();
		}
		virtual void* GetSelector() const {
			return _selector.pvoid;

		}


		~ObserverBase(){

		}
	private:

		SelectorType _selector;
		AutoPtr<C>	 _caller;

	};

	// Wrapper for class observers, specify memeber functions as callbacks
	template<class C>
	class Observer : public ObserverBase<C, Notification>
	{
	public:
		Observer(C* caller, SEL selector):ObserverBase(caller, selector){
		}

	};

	// Wrapper class for global callback functions, which have no oberver, 
	// the specified callback will be invoked when the notification received.
	class SimpleObserver : public ObserverBase<Notification , Notification>
	{
	public:

		SimpleObserver(FUNC selector): ObserverBase(selector){
		}
		ObserverBase* Clone(){
			return NULL;
		}
	};


	typedef set<AbstractObserver*> NotifySet;
	typedef map<string, NotifySet> NotifyMap;

	// window message to handle all the notifications
	extern const int WM_NOTIFICATION;

	// Class to manage and dispatch registered notifications 
	class NotificationCenter
	{
	public:
		static NotificationCenter* defaultCenter(){
			static NotificationCenter* gnc = NULL;
			if( gnc==NULL ){
				gnc = new NotificationCenter();
			}
			return gnc;
		}


		NotificationCenter():_hwnd(NULL)
		{
			createListeningWnd();
		}

		~NotificationCenter(){
			destroyListeningWnd();
		}
		// Register a new observer for notification
		void AddObserver(NString name, AbstractObserver* ob);

		// Unregister observers for a notification defined by name, 
		// when exact = true, only remove the notification's observer exactly equals to ob
		// otherwise, remove a notification if observer and selector equals to ob's
		void RemoveObserver(const NString& name, AbstractObserver* ob, bool exact=true);

		// notify the observer to fire named notifications
		// bWaitUntilDone indicates whether it blocks the thread and waits for result, or 
		// continue to execute following codes.
		void PostNotification(const NString& name, bool bWaintUntilDone=true);

		// notify observer with user defined information
		void PostNotification(const NString& name, const Dictionary& userInfo, bool bWaintUntilDone=true);


		void PostNotification(const NString& name, void* sender, const Dictionary& userInfo, bool bWaintUntilDone=true);

		typedef bool (*ShouldRunloopAbort)(void* context);

		// block current thread while ShoudlRunloopAbort returns true
		// if both parameters are NULL, retreive message once and continue the message queue
		void DefaultRunloop(ShouldRunloopAbort pCB=NULL, void* context=NULL);

	private:
		bool createListeningWnd();
		void destroyListeningWnd();

		void displayNotification(Notification* pNF);

		friend LRESULT CALLBACK WindowProc(HWND , UINT , WPARAM,LPARAM);
	private:
		NotifyMap _observers;
		HWND	  _hwnd;
		AutoCriticalSection _cs;

	};

}