all: png2nds-rgb obj2nds-mesh

clean:
	rm -f png2nds-rgb obj2nds-mesh

png2nds-rgb: png2nds-rgb.c
	gcc -o png2nds-rgb -Ilibs png2nds-rgb.c -lm

obj2nds-mesh: obj2nds-mesh.c
	gcc -o obj2nds-mesh -Ilibs obj2nds-mesh.c -lm
