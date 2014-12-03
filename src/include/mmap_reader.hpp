#include <stddef.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

#define PAGE_SIZE 4096

struct mmap_reader {
	int fd;
	off_t block_off;
	off_t cur_off;
	char *line;
	mmap_reader(const char *path, off_t off) :
		block_off(0), cur_off(off) {
			fd = open(path, O_RDONLY);
			line = (char*)mmap(NULL, PAGE_SIZE, PROT_READ, MAP_PRIVATE,
					fd, 0);
			turn();
		}
	~mmap_reader() {
		munmap(line, PAGE_SIZE);
		close(fd);
	}
	bool turn() {
		if (cur_off >= PAGE_SIZE) {
			block_off += cur_off & ~(PAGE_SIZE - 1);
			munmap(line, PAGE_SIZE);
			line = (char*)mmap(NULL, PAGE_SIZE, PROT_READ, MAP_SHARED,
					fd, block_off);
			cur_off &= PAGE_SIZE - 1;
			return true;
		}
		return false;
	}
	bool get_number(int &num) {
		do {
			while (cur_off < PAGE_SIZE &&
					line[cur_off] &&
					(line[cur_off] < '0' ||
					 line[cur_off] > '9'))
				cur_off++;
		} while (turn());
		if (!line[cur_off])
			return false;

		num = line[cur_off++] - '0';
		do {
			while (cur_off < PAGE_SIZE &&
					line[cur_off] >= '0' &&
					line[cur_off] <= '9')
				num = num * 10 + line[cur_off++] - '0';
		} while (turn());
		return true;
	}
	bool get_2_number(int &numa, int &numb) {
		return get_number(numa) && get_number(numb);
	}
	void skip(off_t n) {
		cur_off += n;
		turn();
	}
};
