/* $Id$ */

/*
 *	Note		Value
 *	=============================
 *	C (0)		 0
 *	C#(0)/Db(0)	 1
 *	D (0)		 2
 *	D#(0)/Eb(0)	 3
 *	E (0)		 4
 *	F (0)		 5
 *	F#(0)/Gb(0)	 6
 *	G (0)		 7
 *	G#(0)/Ab(1)	 8
 *	A (1)		 9
 *	A#(1)/Bb(1)	10
 *	B (1)		11
 *	C (1)		12
 *
 *	http://dguitar.sourceforge.net/GP4format.html
 *
 *	+-------------------------------------------------------+
 *	|	  | Version					|
 *	|         +---------------------------------------------+
 *	|	  | Tablature					|
 *	| Headers +---------------------------------------------+
 *	|         | Lyrics					|
 *	|         +---------------------------------------------+
 *	|	  | Other Tablature Information			|
 *	+---------+---------------------------------------------+
 *	|	  | Measures					|
 *	|         +---------------------------------------------+
 *	|	  | Tracks					|
 *	|         +-------------+---------------+---------------+
 *	|	  |		|		| Note 1 	|
 *	|   Body  | 		| Beat 1 	+---------------+
 *	|         | Measure-	|		| Note i ...	|
 *	|         | Track 	+---------------+---------------+
 *	|         | Pairs  	|		| Note 1	|
 *	|         | 		| Beat i ...	+---------------+
 *	|	  |		|		| Note i ...	|
 *	+---------+-------------+---------------+---------------+
 *	| Chord Diagrams					|
 *	+-------------------------------------------------------+
 */

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "buf.h"

#ifndef __dead
#define __dead __attribute__((__noreturn__))
#endif

__dead void usage(void);

#define GPSTRLEN 256		/* max plus NUL */

typedef int32_t		gp_int;
typedef int16_t		gp_short;
typedef unsigned char	gp_byte;
typedef char		gp_str[GPSTRLEN];

struct gp_info {
	gp_str		gi_title;
	gp_str		gi_subtitle;
	gp_str		gi_interpret;
	gp_str		gi_album;
	gp_str		gi_author;
	gp_str		gi_copyright;
	gp_str		gi_tabauthor;
	struct buf	gi_instruction;
};

struct gp_hdr {
	gp_str		gh_version;
	struct gp_info	gh_info;
	//		gh_lyrics;
	//		gh_extra;
};

struct gp_body {
};

struct gp_tab {
	struct gp_hdr	gt_hdr;
	struct gp_body	gt_body;
};

struct gp_color {
	gp_byte		gc_white;
	gp_byte		gc_blue;
	gp_byte		gc_green;
	gp_byte		gc_red;
};

int
read_strbuf(FILE *fp, struct buf *bufp, int max)
{
	gp_byte len;
	int c, j;

	c = fgetc(fp);
	if (c == EOF)
		return (1);
	if (c >= GPSTRLEN) {
		warn("string len greater than max: %d", c);
		c = GPSTRLEN - 1;
	}
	len = (gp_byte)c;

	for (j = 0; j < len; j++) {
		c = fgetc(fp);
		if (c == EOF)
			return (1);
		buf_append(bufp, c);
	}
	for (; j < max; j++) {
		c = fgetc(fp);
		if (c != '\0')
			return (1);
	}
	buf_append(bufp, '\0');
printf("read str \"%s\"\n", buf_get(bufp));
	return (0);
}

int
read_str(FILE *fp, char buf[GPSTRLEN], int max)
{
	gp_byte len;
	int c, j;

	c = fgetc(fp);
	if (c == EOF)
		return (1);
	if (c >= GPSTRLEN) {
		warn("string len greater than max: %d", c);
		c = GPSTRLEN - 1;
	}
	len = (gp_byte)c;

	for (j = 0; j < len; j++) {
		c = fgetc(fp);
		if (c == EOF)
			return (1);
		buf[j] = c;
	}
	for (; j < max; j++) {
		c = fgetc(fp);
		if (c != '\0')
			return (1);
	}
	buf[len] = '\0';
printf("read str \"%s\"\n", buf);
	return (0);
}

gp_int
read_int(FILE *fp, gp_int *n)
{
	size_t j;
	int c;

	*n = 0;

	/* data is stored in little endian */
	for (j = 0; j < sizeof(gp_int); j++) {
		c = fgetc(fp);
		if (c == EOF)
			return (1);
		*n |= c << (j * 8);
	}
printf("read %d\n", *n);
	return (0);
}

int
read_hdr(FILE *fp, struct gp_tab *gt)
{
	gp_int n, dummy, inslen;
	struct buf *insbuf;
	int c, j;

	read_str(fp, gt->gt_hdr.gh_version, 30);
	read_int(fp, &n);
	read_str(fp, gt->gt_hdr.gh_info.gi_title, 0);
	if (fgetc(fp) != 0x01)
		goto bad;
	read_int(fp, &dummy);
	if (dummy != 0)
		goto bad;
	read_int(fp, &dummy);
	if (dummy != 6)
		goto bad;
	read_str(fp, gt->gt_hdr.gh_info.gi_author, 0);
	read_int(fp, &dummy);
	read_str(fp, gt->gt_hdr.gh_info.gi_album, 0);
	read_int(fp, &dummy);
	read_str(fp, gt->gt_hdr.gh_info.gi_interpret, 0);
	if (fgetc(fp) != 0x01)
		goto bad;
	read_int(fp, &dummy);
	if (dummy != 0)
		goto bad;
	read_int(fp, &dummy);
	read_str(fp, gt->gt_hdr.gh_info.gi_tabauthor, 0);
	if (fgetc(fp) != 0x01)
		goto bad;
	read_int(fp, &dummy);
	if (dummy != 0)
		goto bad;
	read_int(fp, &inslen);
	if (inslen < 0)
		goto bad;
	insbuf = &gt->gt_hdr.gh_info.gi_instruction;
	buf_init(insbuf);
	for (j = 0; j < inslen; j++) {
		read_int(fp, &dummy);
		if (buf_len(insbuf) > 0)
			buf_chop(insbuf);
		read_strbuf(fp, insbuf, 0);
	}

	return (0);

bad:
	warnx("bad file format");
	return (1);
}

int
conv(char *fn, struct gp_tab *gt)
{
	FILE *fp;

	if ((fp = fopen(fn, "r")) == NULL) {
		warn("%s", fn);
		return (1);
	}
	read_hdr(fp, gt);

	fclose(fp);
	return (0);
}

void
dump(struct gp_tab *gt)
{
	struct gp_hdr *gh;

	gh = &gt->gt_hdr;

	printf("Version: %s\n",		gh->gh_version);
	printf("Title: %s\n",		gh->gh_info.gi_title);
	printf("Author: %s\n",		gh->gh_info.gi_author);
	printf("Album: %s\n",		gh->gh_info.gi_album);
	printf("Interpret: %s\n",	gh->gh_info.gi_interpret);
	printf("Tab Author: %s\n\n",	gh->gh_info.gi_tabauthor);

	if (buf_len(&gh->gh_info.gi_instruction) > 0)
		printf("%s\n\n", buf_get(&gh->gh_info.gi_instruction));
}

int
main(int argc, char *argv[])
{
	struct gp_tab gt;
	int status, c;

	while ((c = getopt(argc, argv, "")) != -1)
		switch (c) {
		default:
			usage();
		}
	argv += optind;

	status = 0;
	while (*argv != NULL)
		if (conv(*argv++, &gt))
			status = 1;
		else
			dump(&gt);
	exit(status);
}

void
usage(void)
{
	extern char *__progname;

	fprintf(stderr, "usage: %s file ...\n", __progname);
	exit(1);
}
