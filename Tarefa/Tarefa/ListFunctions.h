#pragma once

struct Node;
struct Node* insertInEmpty(struct Node* head, std::string new_data);
struct Node* insertAtEnd(struct Node* head, std::string new_data);
struct Node* insertAtBegin(struct Node* head, std::string new_data);
int getCount(Node* head);
bool deleteNode(struct Node* head, Node* ptrDel);
struct Node* searchNode(struct Node* head, std::string n);
void printList(Node* head);