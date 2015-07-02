gcc -pthread -Wall -std=c99 -D SERVER -I ../common -o server main.c robot.c server_actions.c ../common/socket_utils.c ../common/actions.c ../common/list.c ../common/workers.c -l ncurses
