OBJS	= src/bnfc/Absyn.o src/bnfc/Buffer.o src/bnfc/Lexer.o src/bnfc/Parser.o src/main.o src/shared/fail.o src/codegen/visitors/program.o src/codegen/visitors/statement.o src/codegen/visitors/expression.o src/codegen/visitors/type.o src/codegen/compiler.o src/codegen/converter.o src/codegen/symboltable.o src/codegen/environment.o src/typechecker/visitors/program.o src/typechecker/typechecker.o src/typechecker/converter.o src/typechecker/symboltable.o src/typechecker/visitors/statement.o src/typechecker/visitors/expression.o src/typechecker/visitors/type.o
SOURCE	= src/bnfc/Absyn.C src/bnfc/Buffer.C src/bnfc/Lexer.C src/bnfc/Parser.C src/main.cpp src/shared/fail.cpp src/codegen/visitors/program.cpp src/codegen/visitors/statement.cpp src/codegen/visitors/expression.cpp src/codegen/visitors/type.cpp src/codegen/compiler.cpp src/codegen/converter.cpp src/codegen/symboltable.cpp src/codegen/environment.cpp src/typechecker/visitors/program.cpp src/typechecker/typechecker.cpp src/typechecker/converter.cpp src/typechecker/symboltable.cpp src/typechecker/visitors/statement.cpp src/typechecker/visitors/expression.cpp src/typechecker/visitors/type.cpp
HEADER	= src/bnfc/Absyn.H src/bnfc/Buffer.H src/bnfc/Lexer.H src/bnfc/Parser.H src/main.h src/shared/fail.h src/codegen/visitors/program.h src/codegen/visitors/statement.h src/codegen/visitors/expression.h src/codegen/visitors/type.h src/codegen/compiler.h src/codegen/converter.h src/codegen/symboltable.h src/codegen/environment.h src/typechecker/visitors/program.h src/typechecker/typechecker.h src/typechecker/converter.h src/typechecker/symboltable.h src/typechecker/visitors/statement.h src/typechecker/visitors/expression.h src/typechecker/visitors/type.h
OUT	= jlc
CC	 = clang++
FLAGS	 = -g -c -Wall -I./src -std=c++20 `llvm-config --cxxflags --ldflags --system-libs --libs core` -frtti
LFLAGS	 = `llvm-config --cxxflags --ldflags --system-libs --libs core` -frtti

all: bnfc $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

bnfc:
	mkdir -p src/bnfc
	bnfc --cpp -m src/Javalette.cf -o src/bnfc
	$(MAKE) -C src/bnfc

src/bnfc/Absyn.o: src/bnfc/Absyn.C
	$(CC) $(FLAGS) src/bnfc/Absyn.C -o src/bnfc/Absyn.o

src/bnfc/Buffer.o: src/bnfc/Buffer.C
	$(CC) $(FLAGS) src/bnfc/Buffer.C -o src/bnfc/Buffer.o

src/bnfc/Lexer.o: src/bnfc/Lexer.C
	$(CC) $(FLAGS) src/bnfc/Lexer.C -o src/bnfc/Lexer.o

src/bnfc/Parser.o: src/bnfc/Parser.C
	$(CC) $(FLAGS) src/bnfc/Parser.C -o src/bnfc/Parser.o

src/main.o: src/main.cpp
	$(CC) $(FLAGS) src/main.cpp -o src/main.o

src/shared/fail.o: src/shared/fail.cpp
	$(CC) $(FLAGS) src/shared/fail.cpp -o src/shared/fail.o

src/codegen/visitors/program.o: src/codegen/visitors/program.cpp
	$(CC) $(FLAGS) src/codegen/visitors/program.cpp -o src/codegen/visitors/program.o

src/codegen/visitors/statement.o: src/codegen/visitors/statement.cpp
	$(CC) $(FLAGS) src/codegen/visitors/statement.cpp -o src/codegen/visitors/statement.o

src/codegen/visitors/expression.o: src/codegen/visitors/expression.cpp
	$(CC) $(FLAGS) src/codegen/visitors/expression.cpp -o src/codegen/visitors/expression.o

src/codegen/visitors/type.o: src/codegen/visitors/type.cpp
	$(CC) $(FLAGS) src/codegen/visitors/type.cpp -o src/codegen/visitors/type.o

src/codegen/compiler.o: src/codegen/compiler.cpp
	$(CC) $(FLAGS) src/codegen/compiler.cpp -o src/codegen/compiler.o

src/codegen/converter.o: src/codegen/converter.cpp
	$(CC) $(FLAGS) src/codegen/converter.cpp -o src/codegen/converter.o

src/codegen/symboltable.o: src/codegen/symboltable.cpp
	$(CC) $(FLAGS) src/codegen/symboltable.cpp -o src/codegen/symboltable.o

src/codegen/environment.o: src/codegen/environment.cpp
	$(CC) $(FLAGS) src/codegen/environment.cpp -o src/codegen/environment.o

src/typechecker/visitors/program.o: src/typechecker/visitors/program.cpp
	$(CC) $(FLAGS) src/typechecker/visitors/program.cpp -o src/typechecker/visitors/program.o

src/typechecker/typechecker.o: src/typechecker/typechecker.cpp
	$(CC) $(FLAGS) src/typechecker/typechecker.cpp -o src/typechecker/typechecker.o

src/typechecker/converter.o: src/typechecker/converter.cpp
	$(CC) $(FLAGS) src/typechecker/converter.cpp -o src/typechecker/converter.o

src/typechecker/symboltable.o: src/typechecker/symboltable.cpp
	$(CC) $(FLAGS) src/typechecker/symboltable.cpp -o src/typechecker/symboltable.o

src/typechecker/visitors/statement.o: src/typechecker/visitors/statement.cpp
	$(CC) $(FLAGS) src/typechecker/visitors/statement.cpp -o src/typechecker/visitors/statement.o

src/typechecker/visitors/expression.o: src/typechecker/visitors/expression.cpp
	$(CC) $(FLAGS) src/typechecker/visitors/expression.cpp -o src/typechecker/visitors/expression.o

src/typechecker/visitors/type.o: src/typechecker/visitors/type.cpp
	$(CC) $(FLAGS) src/typechecker/visitors/type.cpp -o src/typechecker/visitors/type.o


clean:
	rm -f $(OBJS) $(OUT)
