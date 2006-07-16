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
	gp_str		gi_instruction;
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
read_str(FILE *fp, char buf[GPSTRLEN])
{
	gp_byte len;
	int c, j;

	c = fgetc(fp);
	if (c == EOF)
		return (0);
	if (c >= GPSTRLEN) {
		warn("string len greater than max: %d", c);
		c = GPSTRLEN - 1;
	}
	len = (gp_byte)c;

	for (j = 0; j < len; j++) {
		c = fgetc(fp);
		if (c == EOF)
			return (0);
		buf[j] = c;
	}
	buf[j] = '\0';
	return (1);
}

int
read_hdr(FILE *fp, struct gp_tab *gt)
{
	read_str(fp, gt->gt_hdr.gh_version);

	return (0);
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
	printf("Version: %s\n", gt->gt_hdr.gh_version);
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
