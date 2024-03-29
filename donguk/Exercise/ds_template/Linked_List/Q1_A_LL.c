//////////////////////////////////////////////////////////////////////////////////

/* CE1007/CZ1007 Data Structures
Lab Test: Section A - Linked List Questions
Purpose: Implementing the required functions for Question 1 */

//////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>

//////////////////////////////////////////////////////////////////////////////////

typedef struct _listnode{
	int item;
	struct _listnode *next;
} ListNode;			// You should not change the definition of ListNode

typedef struct _linkedlist{
	int size;
	ListNode *head;
} LinkedList;			// You should not change the definition of LinkedList


///////////////////////// function prototypes ////////////////////////////////////

//You should not change the prototype of this function
int insertSortedLL(LinkedList *ll, int item);

void printList(LinkedList *ll);
void removeAllItems(LinkedList *ll);
ListNode *findNode(LinkedList *ll, int index);
int insertNode(LinkedList *ll, int index, int value);
int removeNode(LinkedList *ll, int index);


//////////////////////////// main() //////////////////////////////////////////////

int main()
{
	LinkedList ll;
	int c, i, j;
	c = 1;

	//Initialize the linked list 1 as an empty linked list
	ll.head = NULL;
	ll.size = 0;

	printf("1: Insert an integer to the sorted linked list:\n");
	printf("2: Print the index of the most recent input value:\n");
	printf("3: Print sorted linked list:\n");
	printf("0: Quit:");
	printf("test: good!");

	while (c != 0)
	{
		printf("\nPlease input your choice(1/2/3/0): ");
		scanf("%d", &c);

		switch (c)
		{
		case 1:
			printf("Input an integer that you want to add to the linked list: ");
			scanf("%d", &i);
			j = insertSortedLL(&ll, i);
			printf("The resulting linked list is: ");
			printList(&ll);
			break;
		case 2:
			printf("The value %d was added at index %d\n", i, j);
			break;
		case 3:
			printf("The resulting sorted linked list is: ");
			printList(&ll);
			removeAllItems(&ll);
			break;
		case 0:
			removeAllItems(&ll);
			break;
		default:
			printf("Choice unknown;\n");
			break;
		}


	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////

int insertSortedLL(LinkedList *ll, int item)
{
    /* 
        정렬된 연결 리스트에 새로운 항목을 삽입하는 함수입니다.
        매개변수로는 연결 리스트 구조체 포인터(ll)와 삽입하려는 정수(item)를 받습니다.
    */

    ListNode *temp;     // 현재 노드를 가리키는 임시 포인터
    int curindex;       // 현재 인덱스를 나타내는 변수

    curindex = 0;       // 현재 인덱스를 0으로 초기화

    if(ll == NULL)      // 입력된 연결 리스트가 NULL인 경우, 삽입에 실패하므로 -1을 반환합니다.
        return -1;
    else {              // 입력된 연결 리스트가 NULL이 아닌 경우,
        temp = ll->head;    // temp를 입력된 연결 리스트의 헤드로 초기화합니다.

        while(curindex <= ll->size) {   // 현재 인덱스가 연결 리스트의 크기보다 작거나 같은 동안 반복합니다. // + curindex는 반드시 리스트의 사이즈보다 같거나 작을 것이기 때문 
            if(curindex == ll->size) {  // 현재 인덱스가 연결 리스트의 크기와 같은 경우,
                // 리스트의 맨 끝에 새 항목을 추가하고 반복문을 종료합니다.
                insertNode(ll, curindex, item);
                break;
            }
            else {  // 그렇지 않은 경우,

				// + 코드 이해를 위해서는 else문부터 else if, if 순으로 설명하기!!

                if(temp->item > item) { // 현재 노드의 값이 삽입하려는 값보다 큰 경우,
                    // 현재 위치에 새 항목을 삽입하고 반복문을 종료합니다.
                    insertNode(ll, curindex, item);
                    break;
                }
                else if(temp->item == item) {   // 현재 노드의 값이 삽입하려는 값과 같은 경우,
                    curindex = -1;              // 삽입에 실패한 것으로 간주하고 -1을 반환하고 종료합니다.
                    break;
                }
                else {  // 현재 노드의 값이 삽입하려는 값보다 작은 경우,
                    curindex = curindex + 1;    // 다음 인덱스로 이동합니다.
                    temp = temp->next;          // 다음 노드로 이동합니다.
                }
            }
        }
    }
    
    return curindex;    // 삽입한 항목의 인덱스를 반환합니다.
}

///////////////////////////////////////////////////////////////////////////////////

void printList(LinkedList *ll){

	ListNode *cur;
	if (ll == NULL)
		return;
	cur = ll->head;

	if (cur == NULL)
		printf("Empty");
	while (cur != NULL)
	{
		printf("%d ", cur->item);
		cur = cur->next;
	}
	printf("\n");
}


void removeAllItems(LinkedList *ll)
{
	ListNode *cur = ll->head;
	ListNode *tmp;

	while (cur != NULL){
		tmp = cur->next;
		free(cur); // + free를 통해 아이템 삭제.
		cur = tmp;
	}
	ll->head = NULL;
	ll->size = 0;
}


ListNode *findNode(LinkedList *ll, int index){

	ListNode *temp;

	if (ll == NULL || index < 0 || index >= ll->size) // + 인덱스 범위가 리스트 크기를 벗어나거나 리스트가 null일 경우
		return NULL;

	temp = ll->head;

	if (temp == NULL || index < 0) // + index가 0인지는 중복해서 체크 하므로 없어져도 될 것 같다.
		return NULL;

	while (index > 0){
		temp = temp->next;
		if (temp == NULL)
			return NULL;
		index--; // + index를 점차 감소시킴으로써 마침내 원하는 index를 찾았을 때에는 0이 되므로 while문을 빠져나오게 된다!
	}

	return temp;
}

int insertNode(LinkedList *ll, int index, int value){

	ListNode *pre, *cur;

	if (ll == NULL || index < 0 || index > ll->size + 1)
		return -1;

	// If empty list or inserting first node, need to update head pointer
	if (ll->head == NULL || index == 0){
		cur = ll->head;
		ll->head = malloc(sizeof(ListNode));
		ll->head->item = value;
		ll->head->next = cur;
		ll->size++;
		return 0;
	}


	// Find the nodes before and at the target position
	// Create a new node and reconnect the links
	if ((pre = findNode(ll, index - 1)) != NULL){
		cur = pre->next;
		pre->next = malloc(sizeof(ListNode));
		pre->next->item = value;
		pre->next->next = cur;
		ll->size++;
		return 0;
	}

	return -1;
}


int removeNode(LinkedList *ll, int index){

	ListNode *pre, *cur;

	// Highest index we can remove is size-1
	if (ll == NULL || index < 0 || index >= ll->size)
		return -1;

	// If removing first node, need to update head pointer
	if (index == 0){
		cur = ll->head->next;
		free(ll->head);
		ll->head = cur;
		ll->size--;

		return 0;
	}

	// Find the nodes before and after the target position
	// Free the target node and reconnect the links
	if ((pre = findNode(ll, index - 1)) != NULL){

		if (pre->next == NULL)
			return -1;

		cur = pre->next;
		pre->next = cur->next;
		free(cur);
		ll->size--;
		return 0;
	}

	return -1;
}
