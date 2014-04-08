a simple c++ implementation of win32 based notification center similar to cocoa NSNotificationCenter on Mac OS X and iOS.

### How to use?

1. include header file "notification/notification.h" 

2. explicitly use the namespace: using namespace Win32NC;

3. define memeber function or global function as selector

    * class memeber function 
          
            class TestClass{
            public:
            	void OnCall(Notification* notify){
            		cout << "OnCall" << endl;
            	}
            };
    
    * global callback function
        
        	void OnNotification(Notification* notify){
        		cout << "notification function" << endl;
        	}

4. register a notification

	* get the default notfication center object

            NotificationCenter* nc = NotificationCenter::defaultCenter();
 
	* class object as observer

        	TestClass *test = new TestClass();
        	nc->AddObserver("OnTest", new Observer<TestClass>(test, &TestClass::OnCall));

	* global function as callback

            nc->AddObserver("CallFunc", new SimpleObserver(OnNotification));

5. fire a notfication anywhere

	    NotificationCenter::defaultCenter()->PostNotification("OnTest");

6. unregister a notfication

        nc->RemoveObserver("OnTest", &Observer<TestClass>(test, &TestClass::OnCall), false);
