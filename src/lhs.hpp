/* Contact:  Prof. Nicolas R. Gauger (nicolas.gauger@scicomp.uni-kl.de) or Dr. Emre Özkaya (emre.oezkaya@scicomp.uni-kl.de)
 *
 * Lead developer: Emre Özkaya (SciComp, TU Kaiserslautern)
 *
 * This file is part of RoDeO
 *
 * RoDeO is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * RoDeO is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 * You should have received a copy of the GNU
 * General Public License along with CoDiPack.
 * If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Emre Özkaya, (SciComp, TU Kaiserslautern)
 *
 *
 *
 */

#ifndef LHSSAMPLING
#define LHSSAMPLING
#include <string>
#include <armadillo>
#include "Rodeo_globals.hpp"
using namespace arma;


class LHSSamples{

private:
	unsigned int numberOfDesignVariables;
	unsigned int numberOfSamples;

	vec lowerBounds;
	vec upperBounds;
	mat samples;

	void generateSamples(void);
	uvec returnAValidInterval(mat validIntervals);
	uvec returnValidIntervalsForADimension(mat validIntervals, unsigned int dim);

public:

	LHSSamples(int d, double lb, double ub, int N);
	LHSSamples(int d, vec lb, vec ub, int N);
	LHSSamples(int d, double *lb, double* ub, int N);

	void saveSamplesToFile(std::string filename);
	void visualize(void);
	void printSamples(void);
	mat getSamples(void);

	/* test functions */

	friend void testLHS2D(void);



};


class RandomSamples{

private:
	unsigned int numberOfDesignVariables;
	unsigned int numberOfSamples;
	vec lowerBounds;
	vec upperBounds;
	mat samples;

	void generateSamples(void);



public:

	RandomSamples(int d, double lb, double ub, int N);
	RandomSamples(int d, vec lb, vec ub, int N);
	RandomSamples(int d, double *lb, double* ub, int N);

	void saveSamplesToFile(std::string filename);
	void visualize(void);
	void printSamples(void);

	/* test functions */

	friend void testRandomSamples2D(void);



};

class FullFactorialSamples{

private:
	unsigned int numberOfDesignVariables;
	unsigned int numberOfSamples;
	uvec numberOfLevels;
	vec lowerBounds;
	vec upperBounds;
	mat samples;


	void generateSamples(void);
	void incrementIndexCount(uvec &indxCount);



public:

	FullFactorialSamples(int d, double lb, double ub, int levels);
	FullFactorialSamples(int d, vec lb, vec ub, int levels);
	FullFactorialSamples(int d, double *lb, double* ub, int levels);

	FullFactorialSamples(int d, double lb, double ub, uvec levels);
	FullFactorialSamples(int d, vec lb, vec ub, uvec levels);
	FullFactorialSamples(int d, double *lb, double* ub, uvec levels);


	void saveSamplesToFile(std::string filename);
	void visualize(void);
	void printSamples(void);


	/* test functions */

	friend void testFullFactorial2D(void);



};



void testFullFactorial2D(void);
void testLHS2D(void);
void testRandomSamples2D(void);

#endif
