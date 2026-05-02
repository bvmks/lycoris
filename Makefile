
default:
	cd src && $(MAKE) && echo "all good"

install:
	cd src && $(MAKE) install

clean:
	cd src && $(MAKE) clean
	cd lib && $(MAKE) clean
