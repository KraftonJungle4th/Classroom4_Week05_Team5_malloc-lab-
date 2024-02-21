//////////////////////////////////////////////////////////////////////////////////

/* CE1007/CZ1007 Data Structures
Lab Test: Section A - Linked List Questions
Purpose: Implementing the required functions for Question 3 */

//////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#define DOUBLE_POINTER
// #define SINGLE_POINTER

//////////////////////////////////////////////////////////////////////////////////

typedef struct _listnode
{
	int item;
	struct _listnode *next;
} ListNode; // You should not change the definition of ListNode

typedef struct _linkedlist
{
	int size;
	ListNode *head;
} LinkedList; // You should not change the definition of LinkedList

//////////////////////// function prototypes /////////////////////////////////////

// You should not change the prototype of this function
void moveOddItemsToBack(LinkedList *ll);

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
	// Initialize the linked list 1 as an empty linked list
	ll.head = NULL;
	ll.size = 0;

	printf("1: Insert an integer to the linked list:\n");
	printf("2: Move all odd integers to the back of the linked list:\n");
	printf("0: Quit:\n");

	while (c != 0)
	{
		printf("Please input your choice(1/2/0): ");
		scanf("%d", &c);

		switch (c)
		{
		case 1:
			printf("Input an integer that you want to add to the linked list: ");
			scanf("%d", &i);
			j = insertNode(&ll, ll.size, i);
			printf("The resulting linked list is: ");
			printList(&ll);
			break;
		case 2:
			moveOddItemsToBack(&ll); // You need to code this function
			printf("The resulting linked list after moving odd integers to the back of the linked list is: ");
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

void moveOddItemsToBack(LinkedList *ll)
{

/* 상단 전처리기를 활성화하세요 */
#ifdef DOUBLE_POINTER
	if (ll == NULL || ll->head == NULL) // 리스트가 없거나 (NULL), 요소가 없으면 (head NULL)
		return;

	ListNode *oddHead = NULL, *oddTail = NULL; // 짝수 노드들을 위한 임시 리스트 (노드 앞뒤로 연결만 해주면 된다)
	ListNode **pp = &ll->head;				   // List포인터ll이 가리키고있는 포인터head의 주소를 포인터포인터pp에 담음으로써,
	// pp를 통해 head포인터에도 접근이 가능하고(*pp), list포인터에도 접근이 가능하다(**pp)

	while (*pp != NULL) // pp에 담긴 주소로 한번만 찾아간다. 그럼 처음에는 head로 찾아가게됨
	{
		ListNode *current = *pp; // head를 시작으로 탐색
		if (current->item % 2)
		{						 // 홀수 노드 발견
			*pp = current->next; // 원 리스트에서 노드 제거

			// 짝수 노드 리스트에 추가
			if (oddHead == NULL)
			{
				oddHead = oddTail = current; // 첫 번째 짝수 노드
			}
			else
			{
				oddTail->next = current; // 뒤에 계속 추가
				oddTail = current;
			}
			oddTail->next = NULL; // 짝수 리스트의 마지막 노드는 NULL을 가리킴
		}
		else
			pp = &current->next; // 다음 노드로 이동
	}

	if (oddHead)
	{ // 짝수 노드 리스트가 비어 있지 않으면 원 리스트의 끝에 붙임
		*pp = oddHead;
	}
#endif

/* 상단 전처리기를 활성화하세요 */
#ifdef SINGLE_POINTER
	if (ll == NULL || ll->head == NULL) // 리스트가 비어있는 경우
		return;

	ListNode *oddHead = NULL, *oddTail = NULL; // 홀수 노드를 저장할 임시 리스트
	ListNode *prev = NULL, *search = ll->head;

	while (search != NULL)
	{
		if (search->item % 2) // 홀수 노드를 찾음
		{
			/*  search 제외 */
			if (prev)					   // 첫 번째 노드가 아닐 경우 (prev가 True => prev에 주소값이 담겨있다 => current가 첫째노드가 아니다)
				prev->next = search->next; // 현재 노드를 리스트에서 제거 (search 빠뜨리는 과정)
			else						   // 첫 번째 노드일 경우 (prev가 NULL => search 가 첫째노드)
				ll->head = search->next;   // head를 다음 노드로 변경

			/* search 를 다른노드에 저장및연결 */
			if (oddHead == NULL)			// 첫째로 발견된 홀수노드일 경우
				oddHead = oddTail = search; // 홀수 리스트의 시작과 끝을 현재 노드로 설정
			else							// 추가로 발견된 홀수노드일경우
			{
				oddTail->next = search; // 홀수 리스트의 끝에 현재 노드를 추가
				oddTail = search;		// 홀수 리스트의 끝을 업데이트
			}

			/* search노드 이동 */
			ListNode *next = search->next; // 다음 노드로 이동하기 위해 저장
			search->next = NULL;		   // 현재 노드의 next를 NULL로 설정
			search = next;				   // 다음 노드로 이동
		}
		else
		{
			prev = search;		   // 현재 노드를 이전 노드로 설정
			search = search->next; // 다음 노드로 이동
		}
	}

	/* while이 끝나고 prev가 search가 된 상황 */
	if (prev)				// 홀수 노드 리스트가 비어 있지 않은 경우
		search = oddHead;	// search는 search->next가 됨, 원래 리스트의 끝에 홀수 노드 리스트를 추가
	else					// 홀수 노드 리스트가 비어있음
		ll->head = oddHead; // 전체 리스트가 홀수 노드로만 구성된 경우
#endif
}

///////////////////////////////////////////////////////////////////////////////////

void printList(LinkedList *ll)
{
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

	while (cur != NULL)
	{
		tmp = cur->next;
		free(cur);
		cur = tmp;
	}
	ll->head = NULL;
	ll->size = 0;
}

ListNode *findNode(LinkedList *ll, int index)
{

	ListNode *temp;

	if (ll == NULL || index < 0 || index >= ll->size)
		return NULL;

	temp = ll->head;

	if (temp == NULL || index < 0)
		return NULL;

	while (index > 0)
	{
		temp = temp->next;
		if (temp == NULL)
			return NULL;
		index--;
	}

	return temp;
}

int insertNode(LinkedList *ll, int index, int value)
{

	ListNode *pre, *cur;

	if (ll == NULL || index < 0 || index > ll->size + 1)
		return -1;

	// If empty list or inserting first node, need to update head pointer
	if (ll->head == NULL || index == 0)
	{
		cur = ll->head;
		ll->head = malloc(sizeof(ListNode));
		ll->head->item = value;
		ll->head->next = cur;
		ll->size++;
		return 0;
	}

	// Find the nodes before and at the target position
	// Create a new node and reconnect the links
	if ((pre = findNode(ll, index - 1)) != NULL)
	{
		cur = pre->next;
		pre->next = malloc(sizeof(ListNode));
		pre->next->item = value;
		pre->next->next = cur;
		ll->size++;
		return 0;
	}

	return -1;
}

int removeNode(LinkedList *ll, int index)
{

	ListNode *pre, *cur;

	// Highest index we can remove is size-1
	if (ll == NULL || index < 0 || index >= ll->size)
		return -1;

	// If removing first node, need to update head pointer
	if (index == 0)
	{
		cur = ll->head->next;
		free(ll->head);
		ll->head = cur;
		ll->size--;

		return 0;
	}

	// Find the nodes before and after the target position
	// Free the target node and reconnect the links
	if ((pre = findNode(ll, index - 1)) != NULL)
	{

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