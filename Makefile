all: client scheduler hospitalA hospitalB hospitalC

client: client.c
	gcc -o client client.c

scheduler: scheduler.c
	gcc -o scheduler scheduler.c

hospitalA: hospitalA.c
	gcc -o hospitalA hospitalA.c

hospitalB: hospitalB.c
	gcc -o hospitalB hospitalB.c

hospitalC: hospitalC.c
	gcc -o hospitalC hospitalC.c