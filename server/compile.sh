gcc -pthread -Wall -std=c99 -I ../common -o server main.c robot.c server_actions.c ../common/actions.c ../common/list.c ../common/workers.c -l ncurses
