/*
 * RoDeO, a Robust Design Optimization Package
 *
 * Copyright (C) 2015-2020 Chair for Scientific Computing (SciComp), TU Kaiserslautern
 * Homepage: http://www.scicomp.uni-kl.de
 * Contact:  Prof. Nicolas R. Gauger (nicolas.gauger@scicomp.uni-kl.de) or Dr. Emre Özkaya (emre.oezkaya@scicomp.uni-kl.de)
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
 * Authors: Emre Özkaya, (SciComp, TU Kaiserslautern), Lisa Kusch (SciComp, TU Kaiserslautern)
 *
 *
 *
 */

#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <cassert>
#include <vector>
#include <algorithm>
#include <fstream>

class SU2_Parameters {
public:

	std::string *keywords;
	unsigned int numberOfKeywords;

	std::string configFileName;

	unsigned int dimension = 0;
	unsigned int numberOfConstraints = 0;
	unsigned int numberOfPartitions = 1;

	bool ifObjectiveFunctionNameIsSet;
	bool ifProblemDimensionSet;
	bool ifNumberOfConstraintsSet;
	
	std::string objectiveFunctionName;
	std::vector<std::string> constraintNames;

	SU2_Parameters(){

		dimension = 0;
		numberOfKeywords = 5;
		numberOfConstraints = 0;
		numberOfPartitions = 1;
		objectiveFunctionName = "ObjectiveFunction";

		ifObjectiveFunctionNameIsSet = false;
		ifProblemDimensionSet = false;
		ifNumberOfConstraintsSet = false;

		keywords = new std::string[numberOfKeywords];
		keywords[0]="DIMENSION=";
		keywords[1]="NUMBER_OF_CONSTRAINTS=";
		keywords[2]="CONSTRAINT_FUNCTION_NAMES=";
		keywords[3]="OBJECTIVE_FUNCTION_NAME=";
                keywords[4]="NUMBER_OF_PARTITIONS=";

		configFileName = "su2_parameters.cfg";

	}

	void read(void){

		bool ifParameterAlreadySet[numberOfKeywords];

		for(unsigned int i=0; i<numberOfKeywords; i++){
			ifParameterAlreadySet[i] = false;
		}


		if(!file_exist(configFileName)){

			std::cout<<"ERROR: Parameter file for SU2 does not exist!\n";
			abort();
		}
	
		size_t len = 0;
		ssize_t readlen;
		char * line = NULL;
		FILE *inp = fopen(configFileName.c_str(),"r");

		if (inp == NULL){

			std::cout<<"ERROR: Parameter file for SU2 cannot be opened!\n";
			abort();
		}

		while ((readlen = getline(&line, &len, inp)) != -1) {


			if(line[0]!= '#'){
				std::string str(line);

				for(unsigned int key=0; key<numberOfKeywords; key++){

					std::size_t found = str.find(keywords[key]);

					if (found!=std::string::npos){

						str.erase(std::remove_if(str.begin(), str.end(), isspace), str.end());
						std::string sub_str = str.substr(found+keywords[key].length());

						switch(key){

						case 0: {
							if(ifParameterAlreadySet[key]){

								std::cout<<"ERROR: DIMENSION is already defined in the config file!\n";
								abort();
							}

							dimension = std::stoi(sub_str);

							ifProblemDimensionSet = true;
							ifParameterAlreadySet[key] = true;
							break;
						}

						case 1: {
							if(ifParameterAlreadySet[key]){

								std::cout<<"ERROR: NUMBER_OF_CONSTRAINTS is already defined in the config file!\n";
								abort();
							}

							numberOfConstraints = std::stoi(sub_str);

							ifNumberOfConstraintsSet = true;
							ifParameterAlreadySet[key] = true;
							break;
						}

						case 2: {
							if(ifParameterAlreadySet[key]){

								std::cout<<"ERROR: CONSTRAINT_FUNCTION_NAMES are already defined in the config file!\n";
								abort();
							}
							std::vector<std::string> valuesReadFromString;
							getValuesFromString(sub_str,valuesReadFromString,',');
							int numberOfConstraintsRead = valuesReadFromString.size();

							for (auto it = valuesReadFromString.begin(); it != valuesReadFromString.end(); it++){

								constraintNames.push_back(*it);

							}

							ifParameterAlreadySet[key] = true;
							break;
						}

						case 3: {
							objectiveFunctionName = sub_str;

							ifObjectiveFunctionNameIsSet = true;
							ifParameterAlreadySet[key] = true;
							break;
						}

						case 4: {

							numberOfPartitions = std::stoi(sub_str);

							break;
						}


						}

					}
				}

			} /* end of if */

		} /* end of while */

		fclose(inp);

		if(!ifProblemDimensionSet){

			std::cout<<"ERROR: Number of optimization parameters is not set, did you set DIMENSION properly?\n";
			abort();

		}

		if(dimension>1000){

			std::cout<<"ERROR: Problem dimension is too large, did you set DIMENSION properly?\n";
			abort();

		}


		if(!ifObjectiveFunctionNameIsSet){

			std::cout<<"ERROR: Objective function is required, did you set OBJECTIVE_FUNCTION_EXECUTABLE_NAME properly?\n";
			abort();
		}

	}
	
	bool file_exist(std::string filename)
	{
        	return file_exist(filename.c_str());
	}

	bool file_exist(const char *fileName)
	{
        	std::ifstream infile(fileName);
        	return infile.good();
	}

	void getValuesFromString(std::string str, std::vector<std::string> &values,char delimiter){


        	assert(values.size() == 0);

        	if(str[0] == '{' || str[0] == '['){

                	str.erase(0,1);
        	}

        	if(str[str.length()-1] == '}' || str[str.length()-1] == ']'){

                	str.erase(str.length()-1,1);
        	}

        	while(1){

                	std::size_t found = str.find(delimiter);
                	if (found==std::string::npos) break;

                	std::string buffer;
                	buffer.assign(str,0,found);
                	str.erase(0,found+1);

                	values.push_back(buffer);

        	}

        	values.push_back(str);


	}


};


