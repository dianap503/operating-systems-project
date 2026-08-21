#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <dirent.h>
#include <time.h>
#include <errno.h>
#include <stdint.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#define MAX_PATH 256

char *path = NULL;

typedef struct {
	char magic[4];
	uint32_t format_version;
	uint64_t snapshot_id;
	uint32_t snapshot_state; /* 0 for "OPENED", 1 for "SEALED" */
	int32_t active_writers;
	uint32_t record_count;
} DatabaseHeader;

typedef struct {
	int pid;
	int ppid;
	char state;
	char comm[256];
	char cmdline[1024];
	long rss;
	unsigned long cpu_time;
} ProccessRecord;

void traversal_proc();
int is_number(const char* str);
void print_fileinfo(const char* curr_path);


int main(int argc, char *argv[])
{	int i;
	for(i=1; i<argc; i++)
	{	if(strcmp(argv[i], "--db") == 0)
			{	path=argv[i+1];}
	}

	if(path == NULL)
	{	path="data/proc.db";}

	traversal_proc();

	return 0;
}

void traversal_proc()
{	char *curr_dir = "/proc/";
	DIR *dir;
	struct dirent *de;
	char name[PATH_MAX];
	if(NULL == (dir = opendir(curr_dir)))
	{	fprintf(stderr, "Error while trying to open the folder %s\n", curr_dir);
		perror("the cause is");
		exit(1);
	}
	while(NULL != (de = readdir(dir)))
	{	int verify=is_number(de->d_name);
		if(verify == 1)
		{	sprintf(name,"%s%s",curr_dir,de->d_name);
			print_fileinfo(name);
		}
	}
	closedir(dir);
}


int is_number(const char* str)
{	if (str == NULL || *str == '\0') return 0; /* If the string is empty, then it isn't a number to begin with */
	int i=0;
	for(i; i<strlen(str); i++)
	{	if(!isdigit(str[i]))
		{	return 0;}
	}
	return 1;
}

void print_fileinfo(const char* curr_path)
{	/* The program captures the current processes from /proc and writes a snapshot into a binary database */
	char name1[MAX_PATH];
	char name2[MAX_PATH];
	char name3[MAX_PATH];

	char* pid = NULL;
        char* comm = NULL;
        char* state = NULL;
        char* ppid = NULL;
        char* utime = NULL;
        char* stime = NULL;
	unsigned long CPU_time = 0;
	long vm_rss = 0;

	struct flock lock;
	int fd, codeDeadLock;

	sprintf(name1,"%s%s",curr_path,"/cmdline");
	FILE *cmdline = fopen(name1, "rb");
	if(cmdline == NULL)
	{	return;}

	char cmd[1024];
	size_t bytes_read = fread(cmd, sizeof(char), 1024, cmdline);
	for(int i=0; i<bytes_read; i++)
	{	if(cmd[i] == '\0')
		{	cmd[i]=' ';}
	}
	cmd[bytes_read]='\0';
	fclose(cmdline);

	sprintf(name2,"%s%s",curr_path, "/stat");
	FILE *stat = fopen(name2, "rb");
	if(stat == NULL)
	{	return;}

	char buffer[1024];
	if(fgets(buffer, sizeof(buffer), stat) != NULL)
	{	char* myPtr = strtok(buffer, " ");
		int i=1;
		while(myPtr != NULL)
		{	if(i == 1)
				pid = myPtr;
			else if(i == 2)
				comm = myPtr;
			else if(i == 3)
				state = myPtr;
			else if(i == 4)
				ppid = myPtr;
			else if(i == 14)
				utime = myPtr;
			else if(i == 15)
				stime = myPtr;
			if( pid != NULL && comm != NULL && state != NULL && ppid != NULL && utime != NULL && stime != NULL)
				break;
			myPtr = strtok(NULL, " ");
			i++;
		}
		CPU_time = strtoul(utime, NULL, 10) + strtoul(stime, NULL, 10);
	}
	fclose(stat);

	sprintf(name3,"%s%s",curr_path, "/status");
	FILE *status = fopen(name3, "rb");
	if(status == NULL)
	{	return;}

	char vmrss[1024];
	while(fgets(vmrss, sizeof(vmrss), status) != NULL)
	{	if(strncmp(vmrss, "VmRSS:", 6) == 0)
			sscanf(vmrss, "VmRSS: %ld", &vm_rss);
	}
	fclose(status);

	FILE *db_file = fopen(path, "rb+");

        if(db_file == NULL)
        {       /* The file hasn't been created yet */
		db_file = fopen(path, "wb+");
		if(db_file == NULL)
		{	perror("Error while trying to create the databse file");
			exit(EXIT_FAILURE);
		}
		fd = fileno(db_file);
		lock.l_type = F_WRLCK;
        	lock.l_whence = SEEK_SET;
        	lock.l_start = 0;
        	lock.l_len = 1; /* The actual value is ignored because the lock is put on the whole file */

        	/* Trying to put the lock until I succeed */
        	while( (-1 == (codeDeadLock = fcntl(fd, F_SETLK, &lock))) && ((errno == EACCES)||(errno == EAGAIN)) )
               	{	fprintf(stderr, "[ProccessID:%d] Failed to put a lock...", getpid());
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
		memcpy(header.magic, "PRCI", 4);
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

	ProccessRecord proc_rec;
	proc_rec.pid = atoi(pid);
	proc_rec.ppid = atoi(ppid);
	proc_rec.state = state[0];
	snprintf(proc_rec.comm, sizeof(proc_rec.comm), "%s", comm);
	snprintf(proc_rec.cmdline, sizeof(proc_rec.cmdline), "%s", cmd);
	proc_rec.rss = vm_rss;
	proc_rec.cpu_time = CPU_time;

	fseek(db_file, 0, SEEK_END); /* Writing beginning eith the end of the file so we can add the info discovered before */
	fwrite(&proc_rec, sizeof(ProccessRecord), 1, db_file);

	fseek(db_file, 0, SEEK_SET); /* Going back to the top of the file so we don't overwrite */
	size_t read_bytes_end = fread(&header, sizeof(DatabaseHeader), 1, db_file);
	if(read_bytes_end == 1)
	{	header.record_count ++;
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
	{	fprintf(stderr, "Erorr: Snapshot is already sealed\n");
    		fclose(db_file);
    		exit(EXIT_FAILURE);
	}
	}

 	else if(bytes_read != 1){
		fprintf(stderr, "Erorr: The header of the database file cannot be read (file corrupted or empty).\n");
	}
}
