#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <limits.h>
#include <dirent.h>
#include <time.h>

char *dir = NULL;
char *path = NULL;

typedef struct {
        char magic[4];
        uint32_t format_version;
        uint64_t snapshot_id;
        uint32_t snapshot_state; /* 0 for "OPENED", 1 for "SEALED" */
        int32_t active_writers;
        uint32_t record_count;
} DatabaseHeader;


void traversal_directory(const char* curr_path);
int print_fileinfo(const char* curr_path);
unsigned long calc_hash(const char* abs_path);

int main(int argc, char *argv[])
{	struct stat st;
	int i;
	for(i=1; i<argc; i++)
	{	if(strcmp(argv[i], "--root") == 0)
		{	dir=argv[i+1];}
		else
			if(strcmp(argv[i], "--db") == 0)
			{	path=argv[i+1];}
	}

	if(dir == NULL)
	{	fprintf(stderr, "Directory not specified!\n");
		exit(1);
	}

	if(path == NULL)
	{	path="data/index.db";}
	if(0 != stat(dir, &st) )
	{	fprintf(stderr, "Stat threw an error for %s/\n", dir);
		perror("The cause is");
		exit(2);
	}

	if( ! S_ISDIR(st.st_mode) )
	{	fprintf(stderr, "Error: %s is not a directory!\n", dir);
		exit(3);
	}

	printf("Analyzing the directory: %s\n", dir);
	traversal_directory(dir);

	return 0;
}


void traversal_directory(const char* curr_path)
{	/* Recursive function for passing through the subtree */
	DIR *dir;
	struct dirent *de;
	char name[PATH_MAX];
	/* First, we print the info about our file */
	int isFolder=print_fileinfo(curr_path);
	/* If the argument is of type directory, then we will process its content */
	if(isFolder != 1)
	{	return;} /* If it isn't a directory, we don't cancel the execution with exit(), but we go back to process what is left */
	else
	{	if(NULL == (dir = opendir(curr_path)))
		{	fprintf(stderr, "Error for trying to open the folder; %s\n", curr_path);
			perror("The cause is");
			return;
		} /* If we cannot access the file, we go back and try to open the others that are left */
		/* Accessing its children */
		while(NULL != (de = readdir(dir)) )
		{	if( strcmp(de->d_name, ".") && strcmp(de->d_name, "..") )
			{	sprintf(name,"%s/%s",curr_path,de->d_name);
				traversal_directory(name);
			}
		}
	}
	closedir(dir);
}

int print_fileinfo(const char* curr_path)
{	/* The purpose of this function is to print info about a certain file/directory/symlink/fifo */

	struct flock lock;
        int fd, codeDeadLock;

	struct stat st;
	int result=0;

	if(0 != lstat(curr_path, &st) )
	{	fprintf(stderr, "Stat threw an error for %s\n", curr_path);
		perror("The cause is");
		return 2; /* Don't cancel the execution, but rather go back and process what is left */
	}

	printf("Analyzing the content of the file/directory:%s\n", curr_path);
	int file_type = 0;
	printf("The type of the file: ");
	switch(st.st_mode & S_IFMT)
	{	case S_IFDIR : printf("Directory\n"); file_type=1; result=1; break;
        	case S_IFREG : printf("Ordinary file\n"); file_type=2; break;
        	case S_IFLNK : printf("Link\n"); file_type=3; break;
		case S_IFIFO : printf("FIFO\n"); file_type=4; break;
		case S_IFSOCK: printf("Socket\n"); file_type=5; break;
		case S_IFBLK : printf("Block device\n"); file_type=6; break;
		case S_IFCHR : printf("Character device\n"); file_type=7; break;
		default: printf("Unknown file type");
	}

	/* Opening the file in binary mode */
	FILE *db_file = fopen(path, "rb+");
	if(db_file == NULL)
	{
		/* The file hasn't been created yet */
                db_file = fopen(path, "wb+");
                if(db_file == NULL)
                {       perror("Error while trying to create the database file");
                        exit(EXIT_FAILURE);
                }
                fd = fileno(db_file);
                lock.l_type = F_WRLCK;
                lock.l_whence = SEEK_SET;
                lock.l_start = 0;
                lock.l_len = 1; /* The actual value is ignored because the lock is put on the whole file */

                /* Trying to put the lock until I succeed */
                while( (-1 == (codeDeadLock = fcntl(fd, F_SETLK, &lock))) && ((errno == EACCES)||(errno == EAGAIN)) )
                {       fprintf(stderr, "[ProccessID:%d] Failed to put a lock...", getpid());
                        perror("\tType of error");
                        sleep(3); /* Short break before trying again */
                }
                if(codeDeadLock == -1)
                {       fprintf(stderr, "[ProcceesID:%d] Serious error while trying to put the lock...", getpid());
                        perror("\tType of error");
                        exit(3);
                }
                else
                {       printf("[ProccessID:%d] Succeed in putting the lock!\n", getpid());}

		DatabaseHeader header;
                memcpy(header.magic, "IDXI", 4);
                header.format_version = 1;
                header.snapshot_id = (uint64_t)time(NULL);
                header.snapshot_state = 0;
                header.active_writers = 1;
                header.record_count = 0;
                fwrite(&header, sizeof(DatabaseHeader), 1, db_file);

	}

	else{
        fd = fileno(db_file);
        lock.l_type = F_WRLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = 0;
        lock.l_len = 1; /* The actual value is ignored because the lock is put on the whole file */

        /* Trying to put the lock until I succeed */
        while( (-1 == (codeDeadLock = fcntl(fd, F_SETLK, &lock))) && ((errno == EACCES)||(errno == EAGAIN)) )
        {       fprintf(stderr, "[ProccessID:%d] Failed to put a lock...", getpid());
                perror("\tType of error");
                sleep(3); /* Short break before trying again */
        }
        if(codeDeadLock == -1)
        {       fprintf(stderr, "[ProcceesID:%d] Serious error while trying to put the lock...", getpid());
                perror("\tType of error");
                exit(3);
        }
        else
        {       printf("[ProccessID:%d] Succeed in putting the lock!\n", getpid());}

        }


	fseek(db_file, 0, SEEK_SET);
        DatabaseHeader header;
        size_t read_bytes = fread(&header, sizeof(DatabaseHeader), 1, db_file);
        if(read_bytes == 1){

        if(header.snapshot_state == 0){
        header.active_writers ++;
        fseek(db_file, 0, SEEK_SET); /* Going back to the top of the file so we don't overwrite */
        fwrite(&header, sizeof(DatabaseHeader), 1, db_file);

	fseek(db_file, 0, SEEK_END); /* Writing beginning with the end of the file so we can add the info discovered before */
	char abs_path[PATH_MAX];
	if (realpath(curr_path, abs_path) != NULL)
	{	size_t path_len = strlen(abs_path) + 1;
		fwrite(&path_len, sizeof(path_len), 1, db_file);
		fwrite(abs_path, sizeof(char), path_len, db_file);
	} /* Finding the real path */

	if( file_type == 2) /* Checking if it is a regular file. If so, we then find its coressponding hash number */
	{	fwrite(&st.st_size, sizeof(st.st_size), 1, db_file);
		unsigned long f_hash=calc_hash(abs_path);
		fwrite(&f_hash, sizeof(f_hash), 1, db_file);
	}
	else
	{	int n=0;
		fwrite(&n, sizeof(n), 1, db_file);
		unsigned long f_hash=0;
		fwrite(&f_hash, sizeof(f_hash), 1, db_file);
	}

	fwrite(&st.st_mtime, sizeof(st.st_mtime), 1, db_file); /* Time of last modification */

	fwrite(&st.st_dev, sizeof(st.st_dev), 1, db_file); /* ID of device conatining file */
	fwrite(&st.st_ino, sizeof(st.st_ino), 1, db_file); /* Inode number */
	fseek(db_file, 0, SEEK_SET); /* Going back to the top of the file so we don't overwrite */
        size_t read_bytes_end = fread(&header, sizeof(DatabaseHeader), 1, db_file);
        if(read_bytes_end == 1)
        {       header.record_count ++;
                header.active_writers--;
                if (header.active_writers == 0) {
                        header.snapshot_state = 1; /* Checking to see if we are the last writer */
                }
                fseek(db_file, 0, SEEK_SET);
                fwrite(&header, sizeof(DatabaseHeader), 1, db_file);
                fflush(db_file); /* We make sure the data is written on the disk */
        }
        /* Unlocking the file after we finish writing */
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);

        fclose(db_file);
        }
	else if(header.snapshot_state == 1)
        {       fprintf(stderr, "Erorr: Snapshot is already sealed\n");
                fclose(db_file);
                exit(EXIT_FAILURE);
        }
        }

        else if(read_bytes != 1){
                fprintf(stderr, "Erorr: The header of the database file cannot be read (file corrupted or empty).\n");
        }

	return result;
}

/* DJB2 Algorithm */
unsigned long calc_hash(const char* abs_path)
{	FILE *f = fopen(abs_path, "rb");
	if ( f == NULL )
	{	return 0;}
	unsigned long hash = 5381; /* Prime "magic" number */
	int c;

	while((c = fgetc(f)) != EOF) /* Reads exactly one byte from the file */
	{ hash = (( hash << 5) + hash) + (unsigned char) c;} /* Bitwise left shift by 5 positions */

	fclose(f);
	return hash;
}
