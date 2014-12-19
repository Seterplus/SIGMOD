#include <stddef.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

struct mmap_reader {
	int fd;
	off_t size;
	off_t off;
	char *line;
	mmap_reader(const char *path, off_t off = 0) : off(off) {
		fd = open(path, O_RDONLY);
		struct stat file;
		fstat(fd, &file);
		size = file.st_size;
		line = (char*)mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
	}
	~mmap_reader() {
		munmap(line, size);
		close(fd);
	}
	bool get_number(int &num) {
		while (off < size && (line[off] < '0' || line[off] > '9'))
			off++;
		if (off == size)
			return false;

		num = line[off++] - '0';
		while (off < size && line[off] >= '0' && line[off] <= '9')
			num = num * 10 + line[off++] - '0';
		return true;
	}
	bool get_number(int &numa, int &numb) {
		return get_number(numa) && get_number(numb);
	}
	void skip(off_t n) {
		off += n;
	}
};
