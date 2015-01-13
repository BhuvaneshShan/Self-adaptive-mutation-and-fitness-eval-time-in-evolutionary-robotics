CC           = g++
CCFLAGS      = -Wall -O3

TARGETS = binarize_experiment_data unbinarize_experiment_data \
	compress_experiment_data uncompress_experiment_data \
	extract_fitness_graph_data

all: $(TARGETS)

%: %.cpp common.cpp common.h
	$(CC) -o $@ $(CCFLAGS) $< common.cpp

clean:
	rm -f $(TARGETS)
