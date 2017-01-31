/* It should be a freelist as a memory pool.
 *
 *	v1.0: a dynamic size memory pool
 *	v0.1: a fixed size memory pool
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct {
	char **buf;
	int size;
	int cnt, n, max;
} node_head;

static node_head *free_list_head;

/* Support Byte 2 4 8 16 32 64 128 256 512 1024 */
unsigned int byte_size[] = {
	2,	4,	8,	16,	32,
	64,	128,	256,	512,	1024
};

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

	if (free_list_head == NULL) {
		int i = 0;
		free_list_head = (node_head *)calloc(sizeof(byte_size) / sizeof(unsigned int),
							sizeof(node_head));
		memset(free_list_head, 0, sizeof(node_head) * sizeof(byte_size) / sizeof(unsigned int));
		for (; i < sizeof(byte_size) / sizeof(unsigned int); i ++) {
			free_list_head[i].size = (int)pow(2, (i + 1)*1.0);
		}
	}

	return &free_list_head[i];
}

void mem_pool_print(void)
{
	int i = 0;
	node_head *ptr = NULL;

	printf("====================  Memory Pool Status  ====================\n");
	printf("\tSize\t\tFree Count\tUsed number\n");
	for (; i < sizeof(byte_size) / sizeof(int); i ++) {
		ptr = &free_list_head[i];
		printf("\t%d\t\t%d\t\t%d\n", byte_size[i], ptr->n, ptr->cnt);
	}
	printf("====================  Memory Pool End  ====================\n");
}

char *t_malloc(int size)
{
	node_head *ptr = get_node_head(size);

	if (!ptr) {
		printf("Could not found size(%d) pool\n", size);
		return NULL;
	}
	ptr->cnt ++;
	if (ptr->n == 0) {
		char *p = (char *)calloc(1, (ptr->size + 4));
		*((int *)p) = ptr->size;
		return p + 4;
	} else {
		return ptr->buf[--ptr->n] + 4;
	}
}

void t_free(char *ptr)
{
	int size = *((int *)(ptr - 4));
	node_head *p = get_node_head(size);

	p->cnt --;
	if (!ptr) {
		printf("Could not found size(%d) pool\n", size);
	}
	if (p->n == p->max) {
		p->max = p->max ? p->max << 1 : 16;
		p->buf = (char **)realloc(p->buf, sizeof(char *) * p->max);
	}

	p->buf[p->n ++] = ptr - 4;
}

int main(int argc, char **argv)
{
	char *p = NULL; 
	char *p1 = t_malloc(8);
	char *p2 = t_malloc(15);
	char *p3 = t_malloc(1000);
	int i = 0;

	for (; i < 200; i ++) {
		p = t_malloc(2);
		p1 = t_malloc(8);
		p3 = t_malloc(1000);
	}
	for (; i < 200; i ++) {
		t_free(p);
		t_free(p1);
		t_free(p3);
	}
	mem_pool_print();
	return 0;
}
