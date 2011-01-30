
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>

static unsigned long long ntohll(unsigned long long val);

static void usage(const char *progname);

static struct
{
	const char *name;
	int rel;
	int size_offset;
	int extra_size;
} files[] =
{
	{ "CORE_OS_PACKAGE.pkg", 1, 0x20, 0x80 },
	{ "RL_FOR_PACKAGE.img", 0, 0x18, 0x0 },
	{ "RL_FOR_PROGRAM.img", 0, 0x18, 0x0 },
	{ "dev_flash", 1, 0x20, 0x80 },
};

int main(int argc, char **argv)
{
#define N(_a)	((sizeof((_a))) / (sizeof((_a)[0])))

	FILE *fp;
	char str[4096];
	int str_pos, str_size, c, i;
	unsigned long long hdr_size, body_size;

	if (argc != 2)
	{
		usage(argv[0]);
		exit(1);
	}

	fp = fopen(argv[1], "r");
	if (fp == NULL)
	{
		fprintf(stderr, "couldn't open file '%s'\n", argv[1]);
		exit(1);
	}

	str_pos = 0;
	str_size = 0;

	while ((c = fgetc(fp)) != EOF)
	{
		if (!isalpha(c) && !isdigit(c) && (c != '.') && (c != '_'))
		{
			if (str_size == 0)
				continue;

			str[str_size++] = '\0';

			for (i = 0; i < N(files); i++)
			{
				if (strncmp(files[i].name, str, strlen(files[i].name)) == 0)
					break;
			}

			if (i < N(files))
			{
				fseek(fp, str_pos + 0x200 + 0x10, SEEK_SET);
				(void) fread(&hdr_size, 1, 8, fp);
				hdr_size = ntohll(hdr_size);

				if (files[i].rel)
					fseek(fp, str_pos + 0x200 + hdr_size + files[i].size_offset, SEEK_SET);
				else
					fseek(fp, str_pos + 0x200 + files[i].size_offset, SEEK_SET);

				(void) fread(&body_size, 1, 8, fp);
				body_size = ntohll(body_size);

				printf("0x%08x 0x%08x %s\n", str_pos + 0x200,
					(int) (hdr_size + files[i].extra_size + body_size), str);
			}

			str_size = 0;

			continue;
		}

		if (str_size == 0)
			str_pos = ftell(fp) - 1;

		str[str_size++] = c;
	}

	fclose(fp);

	exit(0);

#undef N
}

static unsigned long long ntohll(unsigned long long val)
{
	return ntohl(val & 0xFFFFFFFFULL) | ntohl(val >> 32);
}

static void usage(const char *progname)
{
	fprintf(stderr, "usage: %s <filename>\n", progname);
}
