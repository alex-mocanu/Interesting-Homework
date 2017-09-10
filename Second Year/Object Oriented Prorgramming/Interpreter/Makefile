.PHONY: build clean run
build:
	mkdir classes;
	javac src/interpretor/*.java -d classes;
clean:
	rm -r classes
run:
	java -Xmx512m -cp classes interpretor.Main $(ARG)
