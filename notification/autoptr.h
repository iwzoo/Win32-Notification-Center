// autoptr.h
//
// a smart pointer management class.
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

#include <atomic>
using namespace std;

namespace Win32NC{
	template<typename T>
	class AutoPtr
	{
	public:
		AutoPtr(){
			init();
		}

		AutoPtr(T* p){
			init();
			this->_data = p;
			retain();
		}

		AutoPtr(const AutoPtr<T>& ref){
			init();
			this->_data = ref._data;
			this->setRefCount( ref._refCount );
			retain();
		}

		~AutoPtr(){
			release();
		}

		T& operator* (){
			return *_data;
		}

		T* operator-> (){
			return _data;
		}

		T* p() const{
			return _data;
		}

		bool isNull(){
			return _data==NULL;
		}

		int		retain(){
			return ++_refCount;
		}
		AutoPtr<T>& operator= ( T* p){
			release();
			this->_data = p;
			_refCount = 0;
			retain();
			return *this;
		}

		AutoPtr<T>& operator= (const AutoPtr<T>& ref){
			if( this != &ref ){
				release();
				this->_data = ref._data;
				this->setRefCount( ref._refCount );
				retain();
			}
			return *this;
		}

		bool operator== (const AutoPtr<T>& ref){
			return (this->_data == ref._data );
		}

		int		release(){
			if( _refCount==0 ) return 0;
			if( --_refCount==0 ){
				if( _data )
					delete _data;
				_data = NULL;
			}
			return _refCount;
		}

	private:
		void	init(){
			_refCount=0; _data = 0; 
		}
		void setRefCount(const atomic<int>& ref){
			_refCount.store(ref.load());
		}
		atomic<int>		_refCount;
		T*		_data;
	
	};
}