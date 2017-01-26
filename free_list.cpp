#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct node {
	struct node *next;
	struct node *prev;
} node;

typedef struct node_head {
	node open_list;
	char *data;
	int size;
	int account;
	int used;
} node_head;

typedef struct node_data {
	node head;
	node_head *prev;
	char *data;
} node_data;

static node_head *free_list_head;

#define DEFAULT_COUNT	2

/* Support Byte 2 4 8 16 32 64 128 256 512 1024 */
unsigned int byte_size[] = {
	2,	4,	8,	16,	32,
	64,	128,	256,	512,	1024
};

static void init_node(node *ptr)
{
	ptr->next = ptr;
	ptr->prev = ptr;	
}

static void __add_node(node *ptr, node *head)
{
	ptr->next = head;
	ptr->prev = head->prev;
	head->prev->next = ptr;
	head->prev = ptr;
}

static void __del_node(node *ptr)
{
	ptr->prev->next = ptr->next;
	ptr->next->prev = ptr->prev;
}

static void __add_node_data(node_data *ptr, node *head)
{
	__add_node(&ptr->head, head);	
}

static void __del_node_data(node_data *ptr)
{
	__del_node(&ptr->head);
}

static void init_node_head_entry(node_head *ptr, unsigned int size, unsigned int account)
{
	node_data *p = NULL;
	unsigned int i = 0;

	ptr->size = size;
	ptr->account = account;
	printf("%d\n", (sizeof(node_data) + size) * account);
	ptr->data = (char *)malloc((sizeof(node_data) + size) * account);

	for (; i < account; i ++) {
		p = (node_data *)(ptr->data + i * (sizeof(node_data) + size));
		p->data = ptr->data + i * (sizeof(node_data) + size) + sizeof(node_data);
		memset(p->data, 0, ptr->size);
		p->prev = ptr;
		__add_node_data(p, &ptr->open_list);
	}
}

static node_head *init_node_head(void)
{
	int i = 0;
	node_head *ptr = (node_head *)malloc(sizeof(node_head) * (sizeof(byte_size) / sizeof(int)));

	if (ptr) {
		memset(ptr, 0, sizeof(node_head) * (sizeof(byte_size) / sizeof(int)));
		for (; i < sizeof(byte_size) / sizeof(int); i ++) {
			init_node(&(ptr + i)->open_list);
			init_node_head_entry(ptr + i, byte_size[i], DEFAULT_COUNT);
		}
	}
	return ptr;
}

static node_data *get_free_node_data(node_head *head)
{
	node_data *p = NULL;

	/* memory node_data use up, reinit a more bigger space */
	if (head->open_list.next == &head->open_list) {
		int i = head->account;

		head->data = (char *)realloc(head->data, 
			(sizeof(node_data) + head->size) * (head->account + DEFAULT_COUNT));
		head->account += DEFAULT_COUNT;
		for (; i < head->account; i ++) {
			p = (node_data *)(head->data + i * (sizeof(node_data) + head->size));
			p->data = (head->data + i * (sizeof(node_data) + head->size)) + sizeof(node_data);
			p->prev = head;
			__add_node_data(p, &head->open_list);
		}
	}
	else {
		p = (node_data *)(head->open_list.next);	
	}
	head->used ++;
	__del_node_data(p);
	return p;
}

static int size2num(unsigned int size)
{
	int i = 0;
	int max = sizeof(byte_size) / sizeof(int);
	int min = 0;

	if (size > byte_size[max - 1]) {
		return -1;
	}

	for (i = (max + min) / 2; i < max; i = (max + min) / 2) {
		if (size == byte_size[i]) {
			break;
		} else if (size > byte_size[i]) {
			if (size < byte_size[i + 1]) {
				i += 1;
				break;
			}
			min = i;
		} else {
			if (size > byte_size[i - 1]) {
				break;
			}
			max = i;
		}
	}
	return i;
}

static node_head *get_node_head(unsigned int size)
{
	int i = size2num(size);

	if (i == -1) {
		printf("size(%d) is out of range(%d)\n", size,
				byte_size[sizeof(byte_size) / sizeof(int)]);
		return NULL;
	}
	if (NULL == free_list_head) {
		free_list_head = init_node_head();
	}
	return &free_list_head[i];
}

void mem_pool_print(void)
{
	int i = 0;
	node_head *ptr = NULL;

	printf("====================  Memory Pool Status  ====================\n");
	printf("\tSize\t\tAccount\t\tUsed number\tFree number\n");
	for (; i < sizeof(byte_size) / sizeof(int); i ++) {
		ptr = &free_list_head[i];
		printf("\t%d\t\t%d\t\t%d\t\t%d\n", byte_size[i], ptr->account,
			ptr->used, (ptr->account - ptr->used));
	}
	printf("====================  Memory Pool End  ====================\n");
}

char *t_malloc(int size)
{
	node_data *data_ptr = NULL;
	node_head *ptr = get_node_head(size);

	if (ptr == NULL) {
		printf("ptr == NULL\n");
		return NULL;
	}
	data_ptr = get_free_node_data(ptr);
	if (data_ptr) {
		return (char *)data_ptr->data;
	} else {
		return NULL;
	}
}

void t_free(char *ptr)
{
	node_data *data_p = (node_data *)(ptr - sizeof(node_data));

	memset(data_p->data, 0, data_p->prev->size);
	__add_node_data(data_p, &data_p->prev->open_list);
	data_p->prev->used --;
}

int main(int argc, char **argv)
{
	char *p = t_malloc(2);
	char *p1 = t_malloc(2);
	char *p2 = t_malloc(2);
	char *p3 = t_malloc(2);

	mem_pool_print();
	t_free(p);
	mem_pool_print();
	return 0;
}
