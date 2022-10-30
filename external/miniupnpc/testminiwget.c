/* $Id: testminiwget.c,v 1.4 2012/06/23 22:35:59 nanard Exp $ */
/* Project : miniupnp
 * Author : Thomas Bernard
 * Copyright (c) 2005-2012 Thomas Bernard
 * This software is subject to the conditions detailed in the
 * LICENCE file provided in this distribution.
 * */
#include <stdio.h>
#include <stdlib.h>
#include "miniwget.h"

/**
 * This program uses the miniwget / miniwget_getaddr function
 * from miniwget.c in order to retreive a web ressource using
 * a GET http method, and store it in a file.
 */
int main(int argc, char * * argv)
{
	void * data;
	int size, writtensize;
	FILE *f;
	char addr[64];

	if(argc < 3) {
		fprintf(stderr, "Usage:\t%s url file\n", argv[0]);
		fprintf(stderr, "Example:\t%s http://www.google.com/ out.html\n", argv[0]);
		return 1;
	}
	data = miniwget_getaddr(argv[1], &size, addr, sizeof(addr), 0);
	if(!data) {
		fprintf(stderr, "Error fetching %s\n", argv[1]);
		return 1;
	}
	printf("local address : %s\n", addr);
	printf("got %d bytes\n", size);
	f = fopen(argv[2], "wb");
	if(!f) {
		fprintf(stderr, "Cannot open file %s for writing\n", argv[2]);
		free(data);
		return 1;
	}
	writtensize = fwrite(data, 1, size, f);
	if(writtensize != size) {
		fprintf(stderr, "Could only write %d bytes out of %d to %s\n",
		        writtensize, size, argv[2]);
	} else {
		printf("%d bytes written to %s\n", writtensize, argv[2]);
	}
	fclose(f);
	free(data);
	return 0;
}

