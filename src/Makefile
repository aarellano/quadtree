BUILD_CFLAGS= \
	-g \
	-Wall -Wstrict-prototypes -Werror=missing-prototypes \
	-Werror=implicit-function-declaration \
	-Werror=pointer-arith -Werror=init-self -Werror=format=2 \
	-Werror=missing-include-dirs -Werror=aggregate-return

all:
	gcc $(BUILD_CFLAGS) $(CFLAGS) -o quadtree quadtree.c drawing_c.h drawing.c

clean:
	rm -rf *.o quadtree
