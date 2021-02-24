- Camdyn Zook czook@iastate.edu
- Syed Ahmad Alhussain syedal@iastate.edu
- Nithin Sebastian nithins@iastate.edu

2/18/2021 - We modified dungeon.c to be able to **load** and **save** from a "disk"(at file path HOMEDIR/.rlg327/dungeon). When the user calls the script with the 
--load flag the program will load the binary file's values into a struct, and will generate the dungeon using the values stored within out struct. When the user calls the script with 
the --save flag, the program will write the current dungeon to the "disk".
