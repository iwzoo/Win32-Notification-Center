// notification.cpp
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

#include "notification.h"

#include <tchar.h>

namespace Win32NC{
	const int WM_NOTIFICATION = RegisterWindowMessage(_T("WM_NOTIFICATION"));


	void NotificationCenter::AddObserver(NString name, AbstractObserver* ob)
	{
		_cs.lock();
		NotifyMap::iterator it = _observers.find(name);
		if( it == _observers.end() ){
			NotifySet obs;
			obs.insert(ob);
			_observers[name] = obs;
		} else {
			it->second.insert( ob );
		}
		_cs.unlock();
	}

	void NotificationCenter::RemoveObserver(const NString& name, AbstractObserver* ob, bool exact/*=true*/)
	{
		_cs.lock();
		NotifyMap::iterator it = _observers.find(name);
		if( it != _observers.end() ){
			if( exact ){
				it->second.erase( ob );
				delete ob;
			} else {
				NotifySet::iterator sit = it->second.begin();
				while( sit!=it->second.end() ){
					AbstractObserver* pRef = (AbstractObserver*)(*sit);
					if( pRef->EqualTo(ob) ){
						it->second.erase(sit);
						delete pRef;
						break;
					}
					sit++;
				}

			}
		} 
		_cs.unlock();
	}


	void NotificationCenter::PostNotification(const NString& name, bool bWaintUntilDone/*=true*/){
		PostNotification(name, NULL, Dictionary(), bWaintUntilDone); 
	}

	void NotificationCenter::PostNotification(const NString& name, const Dictionary& userInfo, bool bWaintUntilDone/*=true*/){

		PostNotification(name, NULL, userInfo, bWaintUntilDone);
	}

	void NotificationCenter::PostNotification(const NString& name, void* sender, const Dictionary& userInfo, bool bWaintUntilDone/*=true*/){
		if( IsWindow(_hwnd) ){
			Notification* nf = new Notification(name, sender, userInfo);
			if( bWaintUntilDone )
				SendMessage(_hwnd, WM_NOTIFICATION, (WPARAM)this, (LPARAM)nf);
			else 
				PostMessage(_hwnd, WM_NOTIFICATION, (WPARAM)this, (LPARAM)nf);
		} 
	}


	void NotificationCenter::DefaultRunloop(ShouldRunloopAbort pCB/*=NULL*/, void* context/*=NULL*/){
		MSG msg = {0};
		while( 1 ){
			if( pCB ){
				if( (*pCB)(context) )
					break;
			}
			if( GetMessage(&msg, _hwnd, 0, 0 ) ){
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			if( NULL==pCB )
				break;
		}
	}

	void NotificationCenter::destroyListeningWnd(){
		if( IsWindow(_hwnd) )
			DestroyWindow( _hwnd );
		_hwnd = NULL;

	}
	void NotificationCenter::displayNotification(Notification* pNF)
	{
		_cs.lock();
		NotifyMap::iterator it = _observers.find( pNF->name );
		if( it!=_observers.end() ){
			NotifySet obset = it->second;
			Notification* notify = pNF;
			using namespace std::placeholders;
			std::for_each(obset.begin(), obset.end(), std::bind(&AbstractObserver::Notify, _1,notify ));
		}
		_cs.unlock();
	}

	LRESULT CALLBACK WindowProc(HWND hwnd,  UINT uMsg, 	WPARAM wParam, LPARAM lParam ){
		if( uMsg==WM_NOTIFICATION ){
			NotificationCenter* nc = (NotificationCenter*) wParam;
			Notification* notify = (Notification*) lParam;
			if( nc )
				nc->displayNotification(notify);
			if( notify ) delete notify;
			return 0;
		}
		return ::DefWindowProcA(hwnd, uMsg, wParam, lParam);
	}
	bool NotificationCenter::createListeningWnd()
	{
		#define szAppName  _T("NotificationCenter")
		#define szAppClassName _T("NotificationCenterMonitorWindow")
		TCHAR wndName[MAX_PATH] = {0};
		srand((long)time(NULL));
		wsprintf(wndName, _T("%s_%d"), szAppName, rand() );
		HINSTANCE hInst = (HINSTANCE)::GetModuleHandle(NULL);
		WNDCLASSEX wc;
		wc.cbSize        = sizeof(WNDCLASSEX);
		wc.style         = CS_HREDRAW | CS_VREDRAW; // Class style(s).
		wc.lpfnWndProc   = (WNDPROC)WindowProc;        // Window Procedure
		wc.cbClsExtra    = 0;                       // No per-class extra data.
		wc.cbWndExtra    = 0;                       // No per-window extra data.
		wc.hInstance     = hInst;               // Owner of this class
		wc.hIcon         = NULL; // Icon name from .RC
		wc.hCursor       = NULL; // Cursor
		wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH); // Black color
		wc.lpszMenuName  = szAppName;               // Menu name from .RC
		wc.lpszClassName = szAppClassName;               // Name to register as
		wc.hIconSm       = NULL;

		if( !::RegisterClassEx(&wc) ){
			if( !RegisterClass((LPWNDCLASS)&wc.style))
				return false;
		}

		_hwnd = CreateWindow(szAppClassName, _T(""),0, CW_USEDEFAULT,0,CW_USEDEFAULT,0,NULL,NULL,hInst,NULL);
		if( _hwnd==NULL ) 
			return false;
		UpdateWindow(_hwnd);
		return true;
	}

}