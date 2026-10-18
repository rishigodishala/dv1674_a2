.PHONY: all clean blur pearson
all: blur pearson
blur:
	$(MAKE) -C blur
pearson:
	$(MAKE) -C pearson
clean:
	$(MAKE) -C blur clean
	$(MAKE) -C pearson clean
