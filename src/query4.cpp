#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <set>
#include <queue>
#include <bitset>
#include "include/mmap_reader.hpp"

#define PERSON_NUM 10000

static clock_t start;
#define START() {\
	fputs(__func__, stderr);\
	fputs(": ", stderr);\
	::start = clock();\
}

#define STOP() {\
	fprintf(stderr, "%f\n", (double)(clock() - ::start) / CLOCKS_PER_SEC);\
}

using namespace std;

int get_tag_id(char *tag_name) {
	START();

	int tag_length = strlen(tag_name) + 1;
	FILE *tag_file = fopen("tag.csv", "r");
	char line[200];
	int tag_id = -1;
	while (fgets(line, 200, tag_file) > 0) {
		char *split = strchr(line, '|');
		*split = split[tag_length] = 0;
		if (strcmp(split + 1, tag_name) == 0) {
			tag_id = atoi(line);
			break;
		}
	}
	fclose(tag_file);

	STOP();
	return tag_id;
}

#define FORUM_BASE 16384
vector<int> forums_hash[FORUM_BASE];
void get_forum(int tag_id) {
	START();

	for (int i = 0; i < FORUM_BASE; i++)
		forums_hash[i].clear();

	int forum, tag;
	mmap_reader forum_file("forum_hasTag_tag.csv", 16);
	while (forum_file.get_number(forum, tag))
		if (tag == tag_id)
			forums_hash[forum & (FORUM_BASE - 1)].push_back(forum);

	STOP();
}

bool members[PERSON_NUM];
int get_members() {
	START();

	memset(members, 0, sizeof members);
	int n = 0;

	int forum, member;
	mmap_reader member_file("forum_hasMember_person.csv", 28);
	while (member_file.get_number(forum, member)) {
		vector<int> &forums = forums_hash[forum & (FORUM_BASE - 1)];
		if (find(forums.begin(), forums.end(), forum) != forums.end()) {
			n += !members[member];
			members[member] = true;
		}
		member_file.skip(22);
	}

	STOP();
	return n;
}

vector<int> relation[PERSON_NUM];
void get_relation() {
	START();

	for (int i = 0; i < PERSON_NUM; i++)
		relation[i].clear();

	int person_a,person_b;
	mmap_reader relation_file("person_knows_person.csv", 20);
	while (relation_file.get_number(person_a, person_b))
		if (members[person_a] && members[person_b])
			relation[person_a].push_back(person_b);

	STOP();
}

#define DIAM 4
bitset<PERSON_NUM> dist[DIAM - 1][PERSON_NUM];
double cent_value[PERSON_NUM];
void get_cent(int n) {
	START();

	memset(cent_value, 0, sizeof cent_value);

	for (int i = 0; i < PERSON_NUM; i++) {
		dist[0][i].reset();
		for (vector<int>::iterator it = relation[i].begin();
				it != relation[i].end();
				it++) {
			int k = *it;
			dist[0][i].set(k, 1);
			for (vector<int>::iterator j = relation[k].begin();
					j != relation[k].end();
					j++)
				dist[0][i].set(*j, 1);
		}
	}
	for (int k = 1; k < DIAM - 1; k++)
		for (int i = 0; i < PERSON_NUM; i++)
			if (members[i] && relation[i].size()) {
				dist[k][i] = dist[k - 1][i];
				for (vector<int>::iterator it = relation[i].begin();
						it != relation[i].end();
						it++)
					dist[k][i] |= dist[k - 1][*it];
			}

	for (int i = 0; i < PERSON_NUM; i++)
		if (members[i] && relation[i].size()) {
			//		double r = n;
			int r = dist[DIAM - 2][i].count();
			int s = r * DIAM;
			for (int k = 0; k < DIAM - 2; k++)
				s -= dist[k][i].count();
			s -= relation[i].size() + 2;
			cent_value[i] = (double)(r - 1) * (r - 1) / s;// / (n - 1);
		}

	STOP();
}

void output(int k) {
	//vector<double> cent;
	for (int i = 0; i < k; i++) {
		double *max_cent = max_element(cent_value, cent_value + PERSON_NUM);
		printf("%ld ", max_cent - cent_value);
		//	cent.push_back(*max_cent);
		*max_cent = 0;
	}
	//printf("%% centrality values");
	//for (int i = 0; i < k; i++)
	//	printf(" %.16f", cent[i]);
	puts("");
}

void query4(int k, char *tag_name) {
	int tag_id = get_tag_id(tag_name);
	get_forum(tag_id);
	int n = get_members();
	get_relation();
	get_cent(n);
	output(k);
}

int main(int argc, char *argv[]) {
	if (argc == 3) {
		if (freopen(argv[1], "r", stdin) == 0 ||
				freopen(argv[2], "w", stdout) == 0) {
			fputs("Freopen error!\n", stderr);
			exit(0);
		}
	}

	int k;
	char tag_name[100];
	char line[200];
	while (fgets(line, 200, stdin) > 0)
		if (sscanf(line, "query4(%d, %[^)])", &k, tag_name) == 2) 
			query4(k, tag_name);

	fclose(stdin);
	fclose(stdout);
	return 0;
}
