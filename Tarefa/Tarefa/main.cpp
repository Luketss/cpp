#include <iostream>
#include <thread>
#include <mutex>

#include "supervisorio.h"
#include "ListFunctions.h"


using namespace std;
static bool s_Finished = false;
struct Node* head = NULL;
std::mutex m;

void StartSupervisorio()
{
	using namespace std::literals::chrono_literals;

	std::cout << "Thread id= " << std::this_thread::get_id() << std::endl;

	while (!s_Finished) 
	{
		string t = GenerateSupervisorioMessage();
		m.lock();
		head = insertAtEnd(head, t);
		printList(head);
		m.unlock();
		//cout << getCount(head) << endl;
		std::this_thread::sleep_for(2s);
	}
}

int main()
{
	
	/*Node* ptrDelete = searchNode(head, 40);*/
	//bool delSuccess = deleteNode(head, ptrDelete);
	std::thread worker(StartSupervisorio);
	std::thread worker2(StartSupervisorio);
	
	std::cin.get();
	s_Finished = true;

	worker.join();
	std::cout << "Finished" << std::endl;

	std::cin.get();
}