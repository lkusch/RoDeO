


#include <fstream>
#include <stdio.h>
#include <math.h>
#include <string>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <sstream>
#include "su2_parameters.hpp"

SU2_Parameters KS;

/* calls the SU2_CFD for a given vector of dv */
int call_SU2_CFD_Solver(double *dv,
		double &OBJ,
		double* &CONS){ 

	int number_of_design_variables=KS.dimension;

	//Apply the design vector using SU2_DEF

	std::ifstream ifs("config_DEF.cfg");
	std::string basic_text;
	getline (ifs, basic_text, (char) ifs.eof());

	std::string dv_text = "DV_VALUE=";


	for(int i=0;i<number_of_design_variables-1;i++){
		dv_text+= std::to_string(dv[i])+",";

	}
	dv_text+= std::to_string(dv[number_of_design_variables-1])+"\n";

	std::ofstream su2_def_input_file;
	su2_def_input_file.open ("config_DEF_new.cfg");
	su2_def_input_file << basic_text+dv_text;
	su2_def_input_file.close();

        std::string run_def = "mpirun -n "+std::to_string(KS.numberOfPartitions)+" SU2_DEF config_DEF_new.cfg > su2def_output";
	system(run_def.c_str());

	// Calculate possible geometric constraints based on SU2_GEO
	
        int geo_data_count; 
        for (int cons = 0; cons < KS.numberOfConstraints; cons++){
          geo_data_count = -1;

	  if(KS.constraintNames[cons] == "AREA") geo_data_count = 0;
	  //TODO USER_DEFINED define a new geometric constraint here, geo_data_count defines the corresponding column in of_eval.csv (starting from 0!)
          if(geo_data_count!=-1){
		system("SU2_GEO config_CFD.cfg > su2geo_output");

		double *geo_data = new double[6];
		
		std::ifstream geo_outstream("of_eval.csv");
		std::string str;
		int count=0;
		while (std::getline(geo_outstream, str))
		{

			count++;
			if(count == 2){
				str.erase(std::remove(str.begin(), str.end(), ','), str.end());
				std::stringstream ss(str);

				for(int i=0;i<6;i++){
					ss >> geo_data[i];
				}
			}

		}

		//TODO FORMAT change if output file changes (this is the old version for of_func.dat)
		/*double *geo_data = new double[10];

		std::ifstream geo_outstream("of_func.dat");
		std::string str;
		int count=0;
		while (std::getline(geo_outstream, str))
		{

			count++;
			if(count == 4){
				str.erase(std::remove(str.begin(), str.end(), ','), str.end());
				std::stringstream ss(str);
				for(int i=0;i<10;i++){
					ss >> geo_data[i];
				}
			}

		}
		*/
		CONS[cons] = geo_data[geo_data_count];
          }
        }

	//Run CFD simulation and collect results for objective function and constraints

        std::string run_cfd = "parallel_computation.py -f config_CFD.cfg -n "+std::to_string(KS.numberOfPartitions)+" > su2cfd_output";
	system(run_cfd.c_str());

        int numRead;
        double factor = 1.0; 

        for (int cons = 0; cons <= KS.numberOfConstraints; cons++){
          
          numRead = -1;
          if ((cons==0 && KS.objectiveFunctionName == "LIFT") || (cons!=0 && KS.constraintNames[cons-1] == "LIFT")){
             numRead = 8; 
             factor = 1.0; 
          }
          if ((cons==0 && KS.objectiveFunctionName == "DRAG") || (cons!=0 && KS.constraintNames[cons-1] == "DRAG")){
             numRead = 7; 
             factor = 1.0; 
          }
          if ((cons==0 && KS.objectiveFunctionName == "MOMENT_Z") || (cons!=0 && KS.constraintNames[cons-1] == "MOMENT_Z")){
             numRead = 12; 
             factor = 1.0; 
          }
	  //TODO USER_DEFINED: add another objective/constraint function here, numRead defines the column in the history file (starting from 0!)
	  

          if(numRead !=-1){
		//TODO FORMAT: The output format differs, this implementation only goes with .csv format!

          	std::ifstream forces_outstream;
                forces_outstream.open("history.vtk");
                if (forces_outstream.fail()) {
			forces_outstream.open("history.csv");
                        if (forces_outstream.fail()){
                            forces_outstream.open("history.dat");
			    if(forces_outstream.fail()){
				    std::cout<<"Failure: History of CFD run not found."<<std::endl;
		 	    }
                        }
		}

          	std::string line, last_line;
          	while (std::getline(forces_outstream, line)) {
            		if (!line.empty()) last_line = line;
          	}

          	std::istringstream ss( last_line );
          	int counter=0;
	
          	while (ss)
          	{
            		std::string s;
            		if (!getline( ss, s, ',' )) break;

	    		if(counter==numRead){

                		if (cons==0) OBJ = factor*std::stod(s);
                		else CONS[cons-1] = factor*std::stod(s);
            		}
	    		counter++;
          	}
          }

	  
        }


	return 0;
}

bool newdesign(double *x1, double *x2){
	bool samedesign=true;
	for(int i=0; i<KS.dimension; i++){
		if(fabs(x1[i]-x2[i])>1E-14){
		       	samedesign=false;
		}
	}
	return !samedesign;
}

int main(void){

	// read parameters for SU2
	KS.read();

	int n_dv = KS.dimension;
	int n_cons = KS.numberOfConstraints;

	double *x = new double[n_dv];
	FILE *inp = fopen("dv.dat","r");
	for(int i=0; i<n_dv; i++){
    		fscanf(inp,"%lf",&x[i]);
	}
	fclose(inp);

	// dv_old.dat serves to store the former value of the design to avoid unnecessary recalculations
        std::ifstream dv_in;
        dv_in.open("dv_old.dat");
        if (dv_in.fail()) {
		std::ofstream dv_out ("dv_old.dat");

                for(unsigned int i=0; i<n_dv; i++){
			dv_out << "9999 "; 
                }

                dv_out.close();
	}
	dv_in.close();
	double *xOld = new double[n_dv];
	FILE *inpOld = fopen("dv_old.dat","r");
	for(int i=0; i<n_dv; i++){
    		fscanf(inpOld,"%lf",&xOld[i]);
	}
	fclose(inpOld);

	std::string copy_dv = "cp dv.dat dv_old.dat";
	system(copy_dv.c_str());

	if(newdesign(x,xOld)){
		std::cout<<"Calculating CFD Solution (SU2) ...\n";

		double result;
		double *constraintValue = new double[n_cons];
		call_SU2_CFD_Solver(x, result, constraintValue);

		FILE *outp = fopen("objFunVal.dat","w");
		fprintf(outp, "%15.10f ", result);
		for(int i=0; i<n_cons; i++){
			fprintf(outp,"%15.10f ", constraintValue[i]);
		}
		fclose(outp);
	}

	return 0;
}
