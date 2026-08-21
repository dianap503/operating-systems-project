#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <stdint.h>

char *old_snap = NULL;
char *new_snap = NULL;
char *out_file = NULL;

typedef struct {
	char magic[4];
	uint32_t format_version;
	uint64_t snapshot_state; /* 0 for "OPENED", 1 for "SEALED" */
	int32_t active_writers;
	uint32_t record_count;
} DatabaseHeader;

typedef struct {
    char *path;
    off_t size;
    unsigned long hash;
    time_t mtime;
} FileEntry;

typedef struct {
	int pid;
	int ppid;
	char state;
	char comm[256];
	char cmdline[1024];
	long rss;
	unsigned long cpu_time;
} ProcessRecord;


void compare_bd(const char *old_db, const char *new_db, const char *out_db);

int main(int argc, char *argv[])
{	int i=1;
	for(i; i<argc; i++)
	{	if( (strcmp(argv[i], "--old") == 0 ) && (i+1<argc))
		{	old_snap = argv[i+1];
			i++;
		}
		else if( (strcmp(argv[i], "--new") == 0) && (i+1<argc))
		{	new_snap = argv[i+1];
			i++;
		}
		else if( (strcmp(argv[i], "--out") == 0) && (i+1<argc))
		{	out_file = argv[i+1];
			i++;
		}
	}
	if( (old_snap == NULL) || (new_snap == NULL) || (out_file == NULL) )
	{	fprintf(stderr, "Error: Please specify the required arguments (--old, --new or --out).\n");
		exit(EXIT_FAILURE);
	}

	printf("Old snapshot: %s\n", old_snap);
    	printf("New snapshot: %s\n", new_snap);
    	printf("Output report: %s\n", out_file);

	compare_bd(old_snap, new_snap, out_file);

	return 0;
}

void compare_bd(const char *old_db, const char *new_db, const char *out_db)
{
	FILE *f_old = fopen(old_db, "rb");
	if(f_old == NULL)
	{	perror("Error trying to open the old file");
		exit(EXIT_FAILURE);
	}

	FILE *f_new = fopen(new_db, "rb");
        if(f_new == NULL)
        {       perror("Error trying to open the new file");
                exit(EXIT_FAILURE);
        }

	FILE *f_out = fopen(out_db, "w");
        if (f_out == NULL)
        {       perror("Error opening output file");
                exit(EXIT_FAILURE);
        }


	DatabaseHeader header_old;
	size_t read_bytes = fread(&header_old, sizeof(DatabaseHeader), 1, f_old);

	if(read_bytes != 1)
	{	fprintf(stderr, "Erorr: The header of the old database file cannot be read (file corrupted or empty).\n");}

	DatabaseHeader header_new;
        size_t bytes_read = fread(&header_new, sizeof(DatabaseHeader), 1, f_new);
        if(bytes_read != 1)
        {       fprintf(stderr, "Erorr: The header of the new database file cannot be read (file corrupted or empty).\n");
		exit(EXIT_FAILURE);
	}

	if ( (memcmp(header_old.magic, header_new.magic, 4) != 0) || (header_old.format_version != header_new.format_version) )
	{	fprintf(stderr, "Erorr: The files do not have the same database or they do not use the same format.\n");
		exit(EXIT_FAILURE);
	}

	/* For file-type databases */
	if(memcmp(header_old.magic, "IDXI", 4) == 0)
	{	int record_count_old = header_old.record_count;
		FileEntry *old_entries = malloc(record_count_old * sizeof(FileEntry));

		/* First we save our known info about the old file into an array of struct */
		for(int i=0; i<record_count_old; i++)
		{	size_t path_len;
			fread(&path_len, sizeof(path_len), 1, f_old);

			char *abs_path = malloc(path_len + 1);
			fread(abs_path, sizeof(char), path_len, f_old);
			abs_path[path_len] = '\0';

			off_t f_size;
			fread(&f_size, sizeof(f_size), 1, f_old);

			unsigned long f_hash;
			fread(&f_hash, sizeof(f_hash), 1, f_old);

			time_t f_mtime;
			fread(&f_mtime, sizeof(f_mtime), 1, f_old);

			dev_t f_dev;
			fread(&f_dev, sizeof(f_dev), 1, f_old);

			ino_t f_ino;
			fread(&f_ino, sizeof(f_ino), 1, f_old);

			old_entries[i].path = abs_path;
			old_entries[i].size = f_size;
			old_entries[i].hash = f_hash;
			old_entries[i].mtime = f_mtime;
		}

		/* We then do the same step for the new file */

		int record_count_new = header_new.record_count;
                FileEntry *new_entries = malloc(record_count_new * sizeof(FileEntry));


		for(int i=0; i<record_count_new; i++)
                {       size_t path_len;
                        fread(&path_len, sizeof(path_len), 1, f_new);

                        char *abs_path = malloc(path_len + 1) ;
                        fread(abs_path, sizeof(char), path_len, f_new);
			abs_path[path_len] = '\0';

                        off_t f_size;
                        fread(&f_size, sizeof(f_size), 1, f_new);

                        unsigned long f_hash;
                        fread(&f_hash, sizeof(f_hash), 1, f_new);

                        time_t f_mtime;
                        fread(&f_mtime, sizeof(f_mtime), 1, f_new);

			dev_t f_dev;
			fread(&f_dev, sizeof(f_dev), 1, f_old);

			ino_t f_ino;
			fread(&f_ino, sizeof(f_ino), 1, f_old);

                        new_entries[i].path = abs_path;
                        new_entries[i].size = f_size;
                        new_entries[i].hash = f_hash;
                        new_entries[i].mtime = f_mtime;
                }

		/* First we search the elements of new_entries in old_entries */

		for(int i=0; i<record_count_new; i++)
		{	int found = 0;
			for(int j=0; j<record_count_old; j++)
			{	if(strcmp(new_entries[i].path, old_entries[j].path) == 0)
				{	found = 1;
					if (new_entries[i].size != old_entries[j].size || new_entries[i].mtime != old_entries[j].mtime || new_entries[i].hash != old_entries[j].hash)
					{	fprintf(f_out, "MODIFIED: %s\n", new_entries[i].path);}
					break;
				}
			}
			if(found == 0)
			{	fprintf(f_out, "APPEARED: %s\n", new_entries[i].path);}
		}

		for(int i=0; i<record_count_old; i++)
                {       int found = 0;
                        for(int j=0; j<record_count_new; j++)
                        {       if(strcmp(old_entries[i].path, new_entries[j].path) == 0)
                                {       found = 1;
                                        break;
                                }
                        }
                        if(found == 0)
                        {       fprintf(f_out, "DISAPPEARED: %s\n", old_entries[i].path);}
                }

		/* Lastly, we free the memory */

		for(int i=0; i<record_count_new; i++)
		{	free(new_entries[i].path);}
		free(new_entries);

		for(int i=0; i<record_count_old; i++)
                {       free(old_entries[i].path);}
		free(old_entries);

	}
	/*  For process-type databases */
	else if(memcmp(header_old.magic, "PRCI", 4) == 0)
	{	/* I considered that a process changed significantly if the difference of RSS memory between the two snapshots surpassed 1024 KB */
		int record_count_old = header_old.record_count;
		ProcessRecord *old_processes = malloc(record_count_old * sizeof(ProcessRecord));
		for(int i=0; i<record_count_old; i++)
		{	fread(&old_processes[i], sizeof(ProcessRecord), 1, f_old);}

		int record_count_new = header_new.record_count;
                ProcessRecord *new_processes = malloc(record_count_new * sizeof(ProcessRecord));

		for(int i=0; i<record_count_new; i++)
		{	fread(&new_processes[i], sizeof(ProcessRecord), 1, f_new);}

		/* First we search the elements of new_processes in old_processes */

		for(int i=0; i<record_count_new; i++)
		{	int found = 0;
			for(int j=0; j<record_count_old; j++)
			{	if (new_processes[i].pid == old_processes[j].pid)
				{	found = 1;
				if (labs(new_processes[i].rss - old_processes[j].rss) > 1024) {
                			fprintf(f_out, "MODIFIED (Significant RSS change): PID %d (%s)\n", 
                       			new_processes[i].pid, new_processes[i].comm);
				}
				break;
				}
			}
			if(found == 0)
			{	fprintf(f_out, "APPEARED (New process): PID %d (%s)\n",new_processes[i].pid, new_processes[i].comm);
			}
		}

		/* Then, we search the elements of old_processes in new_processes */


	 	for(int i=0; i<record_count_old; i++)
                {       int found = 0;
                        for(int j=0; j<record_count_new; j++)
                        {       if (old_processes[i].pid == new_processes[j].pid)
                                {       found = 1;
                                	break;
				}
                        }
                        if(found == 0)
                        {       fprintf(f_out, "DISAPPEARED (Processes ended): PID %d (%s)\n", old_processes[i].pid, old_processes[i].comm);
                        }
                }
		/* Lastly, we free the memory */

		free(new_processes);
		free(old_processes);
	}

	fclose(f_old);
	fclose(f_new);
	fclose(f_out);
}
