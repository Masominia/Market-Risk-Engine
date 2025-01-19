.PHONY: install build test lint run all

install:
	pip install -e ".[dev]"

CXX ?= g++
CXXFLAGS ?= -O2 -std=c++17 -Wall -Wextra -Icpp/include
LIB := $(filter-out cpp/src/main.cpp,$(wildcard cpp/src/*.cpp))

build:
	mkdir -p cpp/build
	$(CXX) $(CXXFLAGS) -o cpp/build/riskcore cpp/src/main.cpp $(LIB)
	$(CXX) $(CXXFLAGS) -o cpp/build/riskcore_tests $(wildcard cpp/tests/*.cpp) $(LIB)

lint:
	ruff check src tests scripts
	ruff format --check src tests scripts

test: build
	cpp/build/riskcore_tests
	pytest

run: build
	python -m riskcore run

all: lint test run
