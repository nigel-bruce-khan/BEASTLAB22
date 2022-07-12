CXX=c++

MAIN=main.cpp
BIN=bin
EXE=mp
OUT=ouput

DATA=input/randomlist128.txt
LEN=128
SUBLEN=10

all: prepare compile

prepare:
	bash prepare.sh

compile:
	$(CXX) $(MAIN) -o $(BIN)/$(EXE) -lm

compile_debug:
	$(CXX) $(MAIN) -o $(BIN)/$(EXE) -lm -DDEB

run:
	$(BIN)/$(EXE) $(DATA) $(LEN) $(SUBLEN)

clean:
	rm -rf $(BIN)/$(EXE)
	rm -rf $(BIN) $(OUT)
