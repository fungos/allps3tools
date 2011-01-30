
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void usage(const char *progname);

int main(int argc, char **argv)
{
	int c, line_size, total_size, nline, nbyte;
	char *end;
	FILE *fp;

	line_size = 16;
	total_size = 0;

	while ((c = getopt(argc, argv, "l:t:")) != -1)
	{
		switch (c)
		{
			case 'l':
				line_size = strtol(optarg, &end, 0);
				if ((*end != '\0'))
					usage(argv[0]);

				if (line_size < 0)
					line_size = 16;
			break;

			case 't':
				total_size = strtol(optarg, &end, 0);
				if ((*end != '\0'))
					usage(argv[0]);

				if (total_size < 0)
					total_size = 0;
			break;

			case '?':
			default:
				fprintf(stderr, "unrecognized option: %c\n", c);
				usage(argv[0]);
		}
	}

	if (argc != (optind + 1))
		usage(argv[0]);

	fp = fopen(argv[optind], "r");
	if (fp == NULL)
	{
		fprintf(stderr, "couldn't open file '%s'\n", argv[optind]);
		exit(1);
	}

	nline = 0;
	nbyte = 0;

	while ((c = fgetc(fp)) != EOF)
	{
		nbyte++;

		printf(" 0x%02X,", c);

		if ((nbyte % line_size) == 0)
		{
			nline++;
			printf("\n");
		}
	}

	fclose(fp);

	while (nbyte < total_size)
	{
		nbyte++;

		printf(" 0x00,");

		if ((nbyte % line_size) == 0)
		{
			nline++;
			printf("\n");
		}
	}

	printf("\n");

	exit(0);
}

static void usage(const char *progname)
{
	fprintf(stderr, "usage: %s [-l <line size>] [-t <total size>] <filename>\n", progname);
	exit(1);
}
