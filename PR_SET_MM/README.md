Changes /proc/<pid>/exe link to a new executable of your choice

Compile:

gcc main.c -o main -ldl -g
<br>
gcc -shared unmapper.c -o unmapper.so -fPIC -g

