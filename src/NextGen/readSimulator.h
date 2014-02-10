/*
 * readSimulator.h
 *
 *  Created on: 21.05.2013
 *      Author: AlexanderDilthey
 */

#ifndef READSIMULATOR_H_
#define READSIMULATOR_H_

#include <map>
#include <vector>
#include <assert.h>
#include <string>
#include <utility>
#include <ostream>
#include "../Graph/Graph.h"

/*
 * This class implements a simple-minded read simulator.
 * It reads in an empirical quality matrix such as the ones generated by readRecalibrator.cpp.
 * Conditional on expected haploid coverage and read length, the number of starting reads at each position of the reference string is ~ Poisson()
 * The difference in mate pair starting positions is assumed to be ~ Normal(parameter, parameter)
 *
 * We walk along the chromosome, compute how many reads start at each position and where the mates are. For each pair, specified in terms of its starting positions, we
 * then generate x bases (where x = read length). At each base, we generate the number of reference positions we want to jump over (deletions in read) or how many new
 * non-ref bases we want to generate (insertions in read; with error). Usually, these numbers are 0. If 0, we copy (with error) a base from the reference chromosome, and
 * increase the position pointer specifying where we are in the reference chromosome by 1. We repeat until we have x bases.
 *
 * "With error" for the copying means: for each base, we have a position in the read. We identify the corresponding columns in the recalibration matrix and
 * draw a quality value according to the distribution of quality values conditional on position in read. Conditional on a selected quality value at a particular
 * position, we have an empirical estimate of the read base being correct. We use this probability in a Bernoulli trial to decide whether any particular printed base
 * should be equal to the underlying base - if not, we randomly generate a new base.
 *
 * This read simulator has a couple of obvious flaws. Insertion rates are simply estimated from deletion rates, whereas the underlying empirical recalibration matrix
 * counts insertions as new alleles at a position and thus as an element of the total allelic error at a position. Also, the process independently selects a quality
 * value at each position, and conditional on that independently selects base correctness. In reality, both processes are not independent along a read, and a Markov chain
 * might do a better job at capturing the interdependencies.
 *
 */

extern std::string readName_field_separator;

class oneRead {
public:
	std::string name;
	std::string sequence;
	std::string quality;
	std::vector<int> coordinates_string;
	std::vector<int> coordinates_edgePath;

	oneRead(std::string read_name, std::string read_sequence, std::string read_qualities) : name(read_name), sequence(read_sequence), quality(read_qualities)
	{
		assert(read_sequence.length() == read_qualities.length());
	}
};

class oneReadPair {
public:
	std::pair<oneRead, oneRead> reads;
	unsigned int diff_starting_coordinates;

	oneReadPair(oneRead r1, oneRead r2, unsigned int difference_starting_coordinates) : reads(std::pair<oneRead, oneRead>(r1, r2)), diff_starting_coordinates(difference_starting_coordinates)
	{

	}
};



class readSimulator {
private:
	std::vector<std::map<char, double> > read_quality_frequencies;
	std::vector<std::map<char, double> > read_quality_correctness;
	std::vector<double> read_INDEL_freq;
	unsigned int read_length;
	unsigned int threads;

	bool paranoid;

public:

	readSimulator(std::string qualityMatrixFile, unsigned int readLength = 100);
	size_t simulate_paired_reads_from_string(std::string readNamePrefix, std::string& s, double expected_haploid_coverage, std::vector<std::pair<std::ofstream*, std::ofstream*>>& output_FHs_perThread, double starting_coordinates_diff_mean = 450, double starting_coordinates_diff_sd = 100);

	unsigned int getNumThreads()
	{
		return threads;
	}

	std::vector<oneReadPair> simulate_paired_reads_from_edgePath(std::vector<Edge*> edgePath, double expected_haploid_coverage, double starting_coordinates_diff_mean, double starting_coordinates_diff_sd, bool perfectly = false);
};

#endif /* READSIMULATOR_H_ */
