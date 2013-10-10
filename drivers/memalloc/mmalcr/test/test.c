#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "mmalcr.h"

int32_t main(int32_t argc, char **argv)
{
	mmalcr_t alcr = mmalcr_create(1192382735, 0x40000000, 12);
	if (alcr == NULL) {
		printf("mmalcr_create() error.\n");
		return -1;
	}

	if (mmalcr_release(alcr)) {
		printf("mmalcr_release() error.\n");
		return -1;
	}

	printf("All access OK.\n");

	return 0;
}
