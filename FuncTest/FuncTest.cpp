// FuncTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "..\notification\notification.h"

using namespace Win32NC;

class Test
{
public:
	void OnCall(Notification* notify){
		cout << "OnCall" << endl;
	}
};

atomic<bool> threadFinished = false;
void OnNotification(Notification* notify){
	cout << "notification function" << endl;
	threadFinished = true;
}

void TestCall(){
	NotificationCenter* nc = NotificationCenter::defaultCenter();
	nc->PostNotification("CallFunc", false);
}

int _tmain(int argc, _TCHAR* argv[])
{
	
	Test *test = new Test();
	NotificationCenter* nc = NotificationCenter::defaultCenter();
	nc->AddObserver("OnTest", new Observer<Test>(test, &Test::OnCall));
	nc->AddObserver("CallFunc", new SimpleObserver(OnNotification));

	std::thread thread(TestCall);
	
	nc->PostNotification("OnTest");

	thread.join();
	while( threadFinished==false ){
		nc->DefaultRunloop();
	}

	cout << "Press 'q' to quit" << endl;
	do{
	}while( getch()!='q' ) ;

	AbstractObserver* p = new Observer<Test>(test, &Test::OnCall);
	nc->RemoveObserver("OnTest", p, false );
	delete p;
	nc->RemoveObserver("OnTest", &Observer<Test>(test, &Test::OnCall), false);

	return 0;
}

