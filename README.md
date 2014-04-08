a simple c++ implementation of win32 based notification center similar to cocoa NSNotificationCenter on Mac OS X and iOS.

### How to use?

1. include header file "notification/notification.h" 

2. explicitly use the namespace: using namespace Win32NC;

3. define memeber function or global function as selector
    
    * class memeber function 
```cpp
	class TestClass{
	public:
		void OnCall(Notification* notify){
			cout << "OnCall" << endl;
		}
	};
```
	* global callback function
```cpp
	void OnNotification(Notification* notify){
		cout << "notification function" << endl;
	}
```
4. 	register a notification

	get the default notfication center object
 ```cpp
	NotificationCenter* nc = NotificationCenter::defaultCenter();
 ```
	* class object as observer
```cpp
	TestClass *test = new TestClass();
	nc->AddObserver("OnTest", new Observer<TestClass>(test, &TestClass::OnCall));
```
	* global function as callback
```cpp
	nc->AddObserver("CallFunc", new SimpleObserver(OnNotification));
```
5. fire a notfication anywhere
```cpp
	NotificationCenter::defaultCenter()->PostNotification("OnTest");
```	
6. unregister a notfication
```cpp
	nc->RemoveObserver("OnTest", &Observer<TestClass>(test, &TestClass::OnCall), false);
```