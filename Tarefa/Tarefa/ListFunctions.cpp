#include<iostream>
using namespace std;

struct Node
{
	string data;
	struct Node* next;
};

struct Node* insertInEmpty(struct Node* head, string new_data)
{
	// if head is not null then list is not empty, so return
	if (head != NULL)
		return head;

	// allocate memory for node
	struct Node* temp = new Node;

	// Assign the data.
	temp->data = new_data;
	head = temp;

	// Create the link.
	head->next = head;

	return head;
}

struct Node* insertAtBegin(struct Node* head, string new_data)
{
	//if list is empty then add the node by calling insertInEmpty
	if (head == NULL)
		return insertInEmpty(head, new_data);

	//else create a new node
	struct Node* temp = new Node;

	//set new data to node
	temp->data = new_data;
	temp->next = head->next;
	head->next = temp;

	return head;
}

struct Node* insertAtEnd(struct Node* head, string new_data)
{
	//if list is empty then add the node by calling insertInEmpty
	if (head == NULL)
		return insertInEmpty(head, new_data);

	//else create a new node
	struct Node* temp = new Node;

	//assign data to new node
	temp->data = new_data;
	temp->next = head->next;
	head->next = temp;
	head = temp;

	return head;
}

int getCount(Node* head)
{
	int count = 0; // Initialize count
	Node* current = head; // Initialize current
	while (current != NULL)
	{
		count++;
		current = current->next;
	}
	return count;
}

//delete the node from the list
bool deleteNode(struct Node* head, Node* ptrDel) {
	Node* cur = head;
	if (ptrDel == head) {
		head = cur->next;
		delete ptrDel;
		return true;
	}

	while (cur) {
		if (cur->next == ptrDel) {
			cur->next = ptrDel->next;
			delete ptrDel;
			return true;
		}
		cur = cur->next;
	}
	return false;
}

struct Node* searchNode(struct Node* head, string n) {
	Node* cur = head;
	while (cur) {
		if (cur->data == n) return cur;
		cur = cur->next;
	}
	cout << "No Node " << n << " in list.\n";
}

void printList(Node* head)
{
	if (head != NULL) {
		cout << head->data << " ";
		head = head->next;
	}
	cout << endl;
}