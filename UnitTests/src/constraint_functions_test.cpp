/*
 * RoDeO, a Robust Design Optimization Package
 *
 * Copyright (C) 2015-2023 Chair for Scientific Computing (SciComp), RPTU
 * Homepage: http://www.scicomp.uni-kl.de
 * Contact:  Prof. Nicolas R. Gauger (nicolas.gauger@scicomp.uni-kl.de) or Dr. Emre Özkaya (emre.oezkaya@scicomp.uni-kl.de)
 *
 * Lead developer: Emre Özkaya (SciComp, RPTU)
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
 * General Public License along with RoDeO.
 * If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Emre Özkaya, (SciComp, RPTU)
 *
 *
 *
 */

#include<gtest/gtest.h>
#include "constraint_functions.hpp"
#include "matrix_vector_operations.hpp"
#include "test_functions.hpp"
#include "standard_test_functions.hpp"
#include "auxiliary_functions.hpp"
#include "design.hpp"
#include "test_defines.hpp"




#ifdef TEST_CONSTRAIN_FUNCTION


class ConstraintFunctionTest : public ::testing::Test {
protected:
	void SetUp() override {

		filenameTrainingData = himmelblauFunction.function.filenameTrainingData;
		filenameTrainingDataLowFi = himmelblauFunction.function.filenameTrainingDataLowFidelity;
		filenameTrainingDataHiFi  = himmelblauFunction.function.filenameTrainingDataHighFidelity;

		constraintFunTest.setDimension(2);

		vec lb(2); lb.fill(-6.0);
		vec ub(2); ub.fill(6.0);

		constraintFunTest.setParameterBounds(lb,ub);

		definition.designVectorFilename = "dv.dat";
		definition.executableName = "himmelblau";
		definition.outputFilename = "objFunVal.dat";
		definition.name= "himmelblau";
		definition.nameHighFidelityTrainingData = filenameTrainingData;
		definition.setDefinition("constraint1 > 10.0");



	}

	void TearDown() override {



	}

	ConstraintFunction constraintFunTest;
	HimmelblauFunction himmelblauFunction;
	ConstraintDefinition definition;
	mat trainingData;
	mat trainingDataLowFi;

	std::string filenameTrainingData;
	std::string filenameTrainingDataLowFi;
	std::string filenameTrainingDataHiFi;



	void setDefinitionForCase1(void){}

	void setDefinitionForCase2(void){

		definition.modelHiFi = AGGREGATION;
	}
	void setDefinitionForCase3(void){

		definition.modelHiFi = TANGENT;
	}

	void setDefinitionForCase4(void){
		definition.ifMultiLevel = true;
		definition.executableNameLowFi = "himmelblauLowFi";
		definition.outputFilenameLowFi = "objFunValLowFi.dat";
		definition.nameLowFidelityTrainingData = filenameTrainingDataLowFi;
	}


};





TEST_F(ConstraintFunctionTest, testConstructor) {

	ASSERT_FALSE(constraintFunTest.ifGradientAvailable);
	ASSERT_FALSE(constraintFunTest.ifInitialized);
	ASSERT_FALSE(constraintFunTest.ifSurrogateModelIsDefined);
	ASSERT_TRUE(constraintFunTest.getDimension() == 2);
	ASSERT_TRUE(constraintFunTest.ifParameterBoundsAreSet);
	ASSERT_TRUE(constraintFunTest.getID() == -1);

}

TEST_F(ConstraintFunctionTest, setInequalityType) {

	constraintFunTest.setInequalityType(">");

	std::string type = constraintFunTest.getInequalityType();

	ASSERT_EQ(type,">");


}

TEST_F(ConstraintFunctionTest, setInequalityTargetValue) {

	constraintFunTest.setInequalityTargetValue(4.4);

	double value = constraintFunTest.getInequalityTargetValue();

	ASSERT_EQ(value,4.4);


}

TEST_F(ConstraintFunctionTest, bindSurrogateModelCase1) {

	setDefinitionForCase1();
	constraintFunTest.setParametersByDefinition(definition);
	//	objFunTest.setDisplayOn();
	constraintFunTest.bindSurrogateModel();
	ASSERT_TRUE(constraintFunTest.ifSurrogateModelIsDefined);
}


TEST_F(ConstraintFunctionTest, checkFeasibility){

	constraintFunTest.setParametersByDefinition(definition);
	bool ifFeasible = constraintFunTest.checkFeasibility(10.4);
	ASSERT_TRUE(ifFeasible);
	ifFeasible = constraintFunTest.checkFeasibility(9.9);
	ASSERT_FALSE(ifFeasible);

}



TEST_F(ConstraintFunctionTest, readOutputDesign){

	Design d(2);
	d.setNumberOfConstraints(1);

	std::ofstream readOutputTestFile;
	readOutputTestFile.open ("readOutputTestFile.txt");
	readOutputTestFile << "2.144\n";
	readOutputTestFile.close();

	constraintFunTest.setFileNameReadInput("readOutputTestFile.txt");
	constraintFunTest.setEvaluationMode("primal");
	constraintFunTest.setID(0);
	constraintFunTest.readOutputDesign(d);

	double readValue = d.constraintTrueValues(0);
	EXPECT_EQ(readValue,  2.144);


	remove("readOutputTestFile.txt");

}



TEST_F(ConstraintFunctionTest, evaluate){
	Design d(2);

	rowvec dvInput(2);
	dvInput(0) = 2.1;
	dvInput(1) = -1.9;
	d.designParameters = dvInput;
	d.setNumberOfConstraints(1);

	compileWithCpp("himmelblau.cpp", definition.executableName);

	constraintFunTest.setParametersByDefinition(definition);
	constraintFunTest.setID(0);
	constraintFunTest.setEvaluationMode("primal");
	constraintFunTest.evaluateDesign(d);

	EXPECT_EQ(d.constraintTrueValues(constraintFunTest.getID()),  73.74420);

	remove(definition.designVectorFilename.c_str());
	remove(definition.outputFilename.c_str());
	remove(definition.executableName.c_str());

}

TEST_F(ConstraintFunctionTest, evaluateDesignAdjoint){

	Design d(2);

	rowvec dvInput(2);
	dvInput(0) = 2.1;
	dvInput(1) = -1.9;
	d.designParameters = dvInput;
	d.setNumberOfConstraints(1);

	compileWithCpp("himmelblauAdjoint.cpp", definition.executableName);


	constraintFunTest.setParametersByDefinition(definition);
	constraintFunTest.setID(0);
	constraintFunTest.setEvaluationMode("adjoint");
	constraintFunTest.evaluateDesign(d);


	EXPECT_EQ(d.constraintTrueValues(0),  73.74420);

	rowvec constraintGradient = d.constraintGradients[0];

	EXPECT_EQ(constraintGradient(0),  -73.896);
	EXPECT_EQ(constraintGradient(1),  -7.176);

	remove(definition.designVectorFilename.c_str());
	remove(definition.outputFilename.c_str());
	remove(definition.executableName.c_str());





}


TEST_F(ConstraintFunctionTest, evaluateDesignTangent){

	Design d(2);

	rowvec dvInput(2);
	dvInput(0) = 2.1;
	dvInput(1) = -1.9;
	d.designParameters = dvInput;
	rowvec diffDirection(2);
	diffDirection(0) = 1.0;
	diffDirection(1) = 0.0;
	d.tangentDirection = diffDirection;

	compileWithCpp("himmelblauTangent.cpp", definition.executableName);

	constraintFunTest.setID(0);
	constraintFunTest.setParametersByDefinition(definition);
	constraintFunTest.setEvaluationMode("tangent");
	constraintFunTest.evaluateDesign(d);

	EXPECT_EQ(d.trueValue,  73.74420);
	EXPECT_EQ(d.tangentValue,  -73.896);


	remove(definition.designVectorFilename.c_str());
	remove(definition.outputFilename.c_str());
	remove(definition.executableName.c_str());



}


TEST_F(ConstraintFunctionTest, addDesignToData){


	himmelblauFunction.function.generateTrainingSamples();
	trainingData = himmelblauFunction.function.trainingSamples;

	Design d(2);

	rowvec dvInput(2);
	dvInput(0) = 2.1;
	dvInput(1) = -1.9;
	d.setNumberOfConstraints(1);
	d.designParameters = dvInput;
	d.constraintTrueValues(0) = 2.4;

	constraintFunTest.setParametersByDefinition(definition);
	constraintFunTest.setID(0);
	constraintFunTest.setEvaluationMode("primal");
	constraintFunTest.initializeSurrogate();
	constraintFunTest.addDesignToData(d);

	mat newData;
	newData.load(filenameTrainingData, csv_ascii);

	ASSERT_TRUE(newData.n_rows == trainingData.n_rows+1);

	rowvec lastRow = newData.row(newData.n_rows-1);
	ASSERT_EQ(lastRow(0),2.1);
	ASSERT_EQ(lastRow(1),-1.9);
	ASSERT_EQ(lastRow(2),2.4);

	remove(filenameTrainingData.c_str());


}

TEST_F(ConstraintFunctionTest, addDesignToDataWithTangent){

	himmelblauFunction.function.generateTrainingSamplesWithTangents();
	trainingData = himmelblauFunction.function.trainingSamples;

	Design d(2);
	rowvec dvInput(2);
	dvInput(0) = 2.1;
	dvInput(1) = -1.9;
	d.setNumberOfConstraints(1);
	d.designParameters = dvInput;
	d.constraintTrueValues(0) = 89.0;
	d.constraintTangent(0) = 11.4;
	rowvec direction(2);
	direction(0) = 0.5;
	direction(1) = 0.6;
	d.constraintDifferentiationDirectionsMatrix.row(0) = direction;

	setDefinitionForCase3();
	constraintFunTest.setParametersByDefinition(definition);
	constraintFunTest.setID(0);
	constraintFunTest.setEvaluationMode("tangent");
	constraintFunTest.initializeSurrogate();
	constraintFunTest.addDesignToData(d);

	mat newData;
	newData.load(filenameTrainingData, csv_ascii);

	ASSERT_TRUE(newData.n_rows == trainingData.n_rows+1);

	rowvec lastRow = newData.row(newData.n_rows-1);
	ASSERT_EQ(lastRow(0),2.1);
	ASSERT_EQ(lastRow(1),-1.9);
	ASSERT_EQ(lastRow(2),89.0);
	ASSERT_EQ(lastRow(3),11.4);
	ASSERT_EQ(lastRow(4),0.5);
	ASSERT_EQ(lastRow(5),0.6);

	remove(filenameTrainingData.c_str());


}

TEST_F(ConstraintFunctionTest, addDesignToDataWithGradient){


	himmelblauFunction.function.generateTrainingSamplesWithAdjoints();
	trainingData = himmelblauFunction.function.trainingSamples;

	Design d(2);

	rowvec dvInput(2);
	dvInput(0) = 2.1;
	dvInput(1) = -1.9;
	d.setNumberOfConstraints(1);
	d.designParameters = dvInput;
	d.constraintTrueValues(0) = 89.0;
	rowvec gradient(2);
	gradient(0) = 33.5;
	gradient(1) = 56.6;
	d.constraintGradientsMatrix.row(0) = gradient;

	setDefinitionForCase2();
	constraintFunTest.setParametersByDefinition(definition);
	constraintFunTest.setID(0);
	constraintFunTest.setEvaluationMode("adjoint");
	constraintFunTest.initializeSurrogate();
	constraintFunTest.addDesignToData(d);

	mat newData;
	newData.load(filenameTrainingData, csv_ascii);

	ASSERT_TRUE(newData.n_rows == trainingData.n_rows+1);

	rowvec lastRow = newData.row(newData.n_rows-1);
	ASSERT_EQ(lastRow(0),2.1);
	ASSERT_EQ(lastRow(1),-1.9);
	ASSERT_EQ(lastRow(2),89.0);
	ASSERT_EQ(lastRow(3),33.5);
	ASSERT_EQ(lastRow(4),56.6);


	remove(filenameTrainingData.c_str());


}




#endif
