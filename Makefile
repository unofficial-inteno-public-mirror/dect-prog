
.PHONY: dect-prog

dect-prog:
	$(MAKE) -C src
	cp src/dect-prog .

