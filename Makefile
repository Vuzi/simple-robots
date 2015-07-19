# Simple robot general Makefile

all:
	@cd common && make
	@cd client && make
	@cd server && make
	@mkdir -p bin
	@cp client/bin/client bin/client
	@cp server/bin/server bin/server
	
clean:
	@cd common && make clean
	@cd client && make clean
	@cd server && make clean
	
mrproper:
	@cd common && make mrproper
	@cd client && make mrproper
	@cd server && make mrproper
