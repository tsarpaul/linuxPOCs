#include<dlfcn.h>
#include<stdlib.h>
#include<stdio.h>


int main(int argc, char *argv[], char *envp[]){
	void *handle;
	char *error;
	void (*change_exe)(char*);

	handle = dlopen("./unmapper.so", RTLD_NOW);
	if (!handle) {
            fputs (dlerror(), stderr);
            exit(1);
        }

	change_exe = dlsym(handle, "change_exe");
	if ((error = dlerror()) != NULL)  {
            fputs(error, stderr);
            exit(1);
        }

	(*change_exe)("/bin/ls");
}
