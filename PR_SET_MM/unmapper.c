#include<errno.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/prctl.h>
#include<sys/mman.h>


typedef struct map {
	long long int addr;
	long long int length;
	char* path;
} map;

int set_exe_file(char *path){
	int fd = open(path, O_RDONLY);
	if(prctl(PR_SET_MM, PR_SET_MM_EXE_FILE, fd, 0, 0)){
		perror("prctl error has occured");
		exit(1);
	}
}

void unmap(map maps[], int count){
	for(int i = 0; i < count; i++){
		if(munmap((int*)maps[i].addr, maps[i].length)){
			perror("munmap error has occured");
			exit(1);
		}
	}
}

int get_file_maps(map *maps, char* old_path){
	FILE *file = fopen("/proc/self/maps", "r");
	char line[1024];
	char *path;
	int length = 0;
	int mapIndex = 0;
	unsigned long start, end;

	while (!feof(file)) {
	    if (fgets(line, sizeof(line) / sizeof(char), file) == NULL) {
		    break;
	    }
	    if (sscanf(line, "%lx-%lx", &start, &end) != 2) {
		    continue; // could not parse. fail gracefully and try again on the next line.
	    }
	    
	    int pathIndex = 0;
	    int permIndex = 0;
	    for(int i = 0; line[i] != '\0'; i++){
		    if(permIndex == 0 && line[i] == ' '){
			    permIndex = i+1;
		    }
		    if(line[i] == '/'){
			    pathIndex = i;
			    break;
		    }
	    }
	    if(pathIndex == 0){
		    continue; // Not a file map
	    }

	    // Check for executable permission to preserve GOT
	    //if(line[permIndex+2] == '-'){
	    	//continue;
	    //}
	    path = malloc(1024);
	    if(sscanf(line + pathIndex, "%s", path) == 0){
		    continue; // failed to parse
	    }
	    if(strcmp(old_path, path) != 0){
		    continue; // This isn't the requested map
	    }
	    length = end - start;
	    map m = {start, length, path};
	    maps[mapIndex] = m;
	    mapIndex += 1;
	}
	fclose(file);
	return mapIndex;
}

void change_exe(char* new_path){
	char old_path[1024];
	char new_path_copy[1024];
	map maps[32];
	int mapsCount = 0;
	int size = 0;
	
	// Old data segment will be destroyed, we don't risk new_path being stored there so we copy
	strcpy(new_path_copy, new_path);

	size = readlink("/proc/self/exe", old_path, 1024);
	old_path[size] = '\0';
	if(strcmp(old_path, new_path_copy) == 0){
		return;
	}

	mapsCount = get_file_maps(maps, old_path);
	unmap(maps, mapsCount);
	printf("Successfully unmapped!");

	set_exe_file(new_path_copy);
	printf("Done!\n");

	sleep(1000);
}

