/*
 * RoDeO, a Robust Design Optimization Package
 *
 * Copyright (C) 2015-2023 Chair for Scientific Computing (SciComp), TU Kaiserslautern
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
 * Authors: Emre Özkaya, (SciComp, TU Kaiserslautern)
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




#define TEST_CONSTRAIN_FUNCTION
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


	}

	void TearDown() override {



	}

	ConstraintFunction2 constraintFunTest;
	HimmelblauFunction himmelblauFunction;
	ObjectiveFunctionDefinition definition;
	mat trainingData;
	mat trainingDataLowFi;

	std::string filenameTrainingData;
	std::string filenameTrainingDataLowFi;
	std::string filenameTrainingDataHiFi;



	void setDefinitionForCase1(void){}

	void setDefinitionForCase2(void){

		definition.ifGradient = true;
	}
	void setDefinitionForCase3(void){

		definition.ifTangent  = true;
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

	constraintFunTest.setInequalityType(">");
	constraintFunTest.setInequalityTargetValue(10.0);
	bool ifFeasible = constraintFunTest.checkFeasibility(18.4);
	ASSERT_TRUE(ifFeasible);
	ifFeasible = constraintFunTest.checkFeasibility(8.1);
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

	constraintFunTest.setID(0);
	constraintFunTest.setParametersByDefinition(definition);
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




//
//
//TEST(testDriver, testConstraintDefinition){
//
//	std::string testDefinition = "constraint1 < 3.2";
//	ConstraintDefinition testConstraint(testDefinition);
//	ASSERT_EQ(testConstraint.value,3.2);
//	ASSERT_EQ(testConstraint.inequalityType,"<");
//	ASSERT_EQ(testConstraint.name,"constraint1");
//
//	std::string testDefinition2 = "constraint2 > 0.2";
//	ConstraintDefinition testConstraint2(testDefinition2);
//	ASSERT_EQ(testConstraint2.value,0.2);
//	ASSERT_EQ(testConstraint2.inequalityType,">");
//	ASSERT_EQ(testConstraint2.name,"constraint2");
//
//
//}
//
//
//TEST(testConstraintFunctions, testConstructor){
//
//	ConstraintFunction constraintFunTest("testConstraintFunction",4);
//	unsigned int dim = constraintFunTest.getDimension();
//	std::string name = constraintFunTest.getName();
//
//	ASSERT_EQ(dim,4);
//	ASSERT_TRUE(name.compare("testConstraintFunction") == 0);
//
//
//}
//
//TEST(testConstraintFunctions, testConstructorWithFunctionPointer){
//
//	ConstraintFunction constraintFunTest("testConstraintFunction",2);
//	constraintFunTest.setFunctionPointer(Himmelblau);
//
//
//	unsigned int dim = constraintFunTest.getDimension();
//	std::string name = constraintFunTest.getName();
//
//	ASSERT_EQ(dim,2);
//	ASSERT_TRUE(name.compare("testConstraintFunction") == 0);
//	ASSERT_TRUE(constraintFunTest.ifHasFunctionFunctionPointer());
//
//
//}
//
//
//TEST(testConstraintFunctions, checkFeasibility){
//
//	ConstraintFunction constraintFunTest("testConstraintFunction",4);
//
//	ConstraintDefinition def("testConstraintFunction > 18.2");
//
//	constraintFunTest.setParametersByDefinition(def);
//	bool ifFeasible = constraintFunTest.checkFeasibility(18.4);
//	ASSERT_TRUE(ifFeasible);
//	ifFeasible = constraintFunTest.checkFeasibility(18.1);
//	ASSERT_FALSE(ifFeasible);
//
//}
//
//
//TEST(testConstraintFunctions, testEvaluateExternal){
//
//	Design d(2);
//	d.generateRandomDesignVector(0.0,1.0);
//	d.saveDesignVector("dv.dat");
//
//	std::string compileCommand = "g++ himmelblau.cpp -o himmelblau -lm";
//	system(compileCommand.c_str());
//
//
//	ConstraintFunction constraintFunTest("testConstraintFunction",2);
//	constraintFunTest.setExecutableName("himmelblau");
//	constraintFunTest.evaluate(d);
//
//	std::ifstream testInput("objFunVal.dat");
//	double functionValue = 0.0;
//	double functionValueFromFunction = Himmelblau(d.designParameters.memptr());
//	testInput >> functionValue;
//	testInput.close();
//	double error = fabs(functionValue - functionValueFromFunction);
//	EXPECT_LT(error,10E-08);
//
//	remove("himmelblau");
//	remove("dv.dat");
//	remove("objFunVal.dat");
//}
//
//TEST(testConstraintFunctions, testreadEvaluateOutputWithoutMarker){
//
//	Design d(4);
//	d.setNumberOfConstraints(1);
//	std::ofstream readOutputTestFile;
//	readOutputTestFile.open ("readOutputTestFile.txt");
//	readOutputTestFile << "2.144\n";
//	readOutputTestFile.close();
//
//	ConstraintFunction constraintFunTest("testConstraintFunction",4);
//	constraintFunTest.setFileNameReadInput("readOutputTestFile.txt");
//	constraintFunTest.setID(0);
//
//	constraintFunTest.readEvaluateOutput(d);
//	EXPECT_EQ(d.constraintTrueValues(0),2.144);
//	remove("readOutputTestFile.txt");
//
//}
//
//TEST(testConstraintFunctions, testreadEvaluateOutputWithoutMarkerWithAdjoint){
//
//	Design d(4);
//	d.setNumberOfConstraints(1);
//	std::ofstream readOutputTestFile;
//	readOutputTestFile.open ("readOutputTestFile.txt");
//	readOutputTestFile << "2.144 1 2 3 4\n";
//	readOutputTestFile.close();
//
//	ConstraintFunction constraintFunTest("testConstraintFunction",4);
//	constraintFunTest.setFileNameReadInput("readOutputTestFile.txt");
//	constraintFunTest.setID(0);
//	constraintFunTest.setGradientOn();
//	constraintFunTest.readEvaluateOutput(d);
//	EXPECT_EQ(d.constraintTrueValues(0),2.144);
//	rowvec gradientResult = d.constraintGradients.front();
//	EXPECT_EQ(gradientResult(0),1);
//	EXPECT_EQ(gradientResult(1),2);
//	EXPECT_EQ(gradientResult(2),3);
//	EXPECT_EQ(gradientResult(3),4);
//
//	remove("readOutputTestFile.txt");
//}
//
//
//
//TEST(testConstraintFunctions, testreadEvaluateOutputWithMarkerWithoutAdjoint){
//
//	Design d(4);
//	d.setNumberOfConstraints(1);
//	std::ofstream readOutputTestFile;
//	readOutputTestFile.open ("readOutputTestFile.txt");
//	readOutputTestFile << "myConstraint = 2.144\n";
//	readOutputTestFile.close();
//
//
//	ConstraintFunction constraintFunTest("testConstraintFunction",4);
//	constraintFunTest.setFileNameReadInput("readOutputTestFile.txt");
//	constraintFunTest.setID(0);
//	constraintFunTest.setReadMarker("myConstraint");
//
//	constraintFunTest.readEvaluateOutput(d);
//	EXPECT_EQ(d.constraintTrueValues(0),2.144);
//	remove("readOutputTestFile.txt");
//
//
//
//}
//
//TEST(testConstraintFunctions, testreadEvaluateOutputWithMarkerWithAdjoint){
//
//	Design d(4);
//	d.setNumberOfConstraints(1);
//	std::ofstream readOutputTestFile;
//	readOutputTestFile.open ("readOutputTestFile.txt");
//	readOutputTestFile << "myConstraint = 2.144\nConstraintGradient = 1,2,3,4\n";
//	readOutputTestFile.close();
//
//	ConstraintFunction constraintFunTest("testConstraintFunction",4);
//	constraintFunTest.setFileNameReadInput("readOutputTestFile.txt");
//	constraintFunTest.setID(0);
//	constraintFunTest.setReadMarker("myConstraint");
//	constraintFunTest.setReadMarkerAdjoint("ConstraintGradient");
//
//	constraintFunTest.setGradientOn();
//
//	constraintFunTest.readEvaluateOutput(d);
//	EXPECT_EQ(d.constraintTrueValues(0),2.144);
//	rowvec gradientResult = d.constraintGradients.front();
//	EXPECT_EQ(gradientResult(0),1);
//	EXPECT_EQ(gradientResult(1),2);
//	EXPECT_EQ(gradientResult(2),3);
//	EXPECT_EQ(gradientResult(3),4);
//
//
//	remove("readOutputTestFile.txt");
//
//
//}
//
//
//
//
//
//
//TEST(testConstraintFunctions, testreadEvaluateThreeOutputsInTheSameFileWithMarkersAndTwoOfThemHasAdjoints){
//
//	Design d(4);
//	d.setNumberOfConstraints(3);
//	std::ofstream readOutputTestFile;
//	readOutputTestFile.open ("readOutputTestFile.txt");
//	readOutputTestFile << "Constraint1 = 2.144\nConstraint2 = -1\nConstraint2Gradient = 1,2,3,4\nConstraint3 = -1.6\nConstraint3Gradient = -1,-2,-3,-4";
//	readOutputTestFile.close();
//
//	/* first constraint has only value */
//	ConstraintFunction constraintFunTest("testConstraintFunction",4);
//	constraintFunTest.setFileNameReadInput("readOutputTestFile.txt");
//	constraintFunTest.setID(0);
//	constraintFunTest.setReadMarker("Constraint1");
//
//	/* second constraint has value and gradient*/
//	ConstraintFunction constraintFunTest2("testConstraintFunction2",4);
//	constraintFunTest2.setFileNameReadInput("readOutputTestFile.txt");
//	constraintFunTest2.setID(1);
//	constraintFunTest2.setGradientOn();
//	constraintFunTest2.setReadMarker("Constraint2");
//	constraintFunTest2.setReadMarkerAdjoint("Constraint2Gradient");
//
//	/* third constraint has value and gradient*/
//	ConstraintFunction constraintFunTest3("testConstraintFunction3",4);
//	constraintFunTest3.setFileNameReadInput("readOutputTestFile.txt");
//	constraintFunTest3.setID(2);
//	constraintFunTest3.setGradientOn();
//	constraintFunTest3.setReadMarker("Constraint3");
//	constraintFunTest3.setReadMarkerAdjoint("Constraint3Gradient");
//
//	constraintFunTest.readEvaluateOutput(d);
//	EXPECT_EQ(d.constraintTrueValues(0),2.144);
//
//	rowvec gradientResult = d.constraintGradients[0];
//	EXPECT_EQ(gradientResult(0),0);
//	EXPECT_EQ(gradientResult(1),0);
//	EXPECT_EQ(gradientResult(2),0);
//	EXPECT_EQ(gradientResult(3),0);
//
//
//
//	constraintFunTest2.readEvaluateOutput(d);
//	EXPECT_EQ(d.constraintTrueValues(1),-1);
//
//	gradientResult = d.constraintGradients[1];
//	EXPECT_EQ(gradientResult(0),1);
//	EXPECT_EQ(gradientResult(1),2);
//	EXPECT_EQ(gradientResult(2),3);
//	EXPECT_EQ(gradientResult(3),4);
//
//	constraintFunTest3.readEvaluateOutput(d);
//	EXPECT_EQ(d.constraintTrueValues(2),-1.6);
//
//	gradientResult = d.constraintGradients[2];
//	EXPECT_EQ(gradientResult(0),-1);
//	EXPECT_EQ(gradientResult(1),-2);
//	EXPECT_EQ(gradientResult(2),-3);
//	EXPECT_EQ(gradientResult(3),-4);
//
//	remove("readOutputTestFile.txt");
//}
//
//
//
//TEST(testConstraintFunctions, testreadEvaluateObjectiveFunctionAndConstraintInTheSameFile){
//
//	Design d(4);
//	d.setNumberOfConstraints(1);
//	std::ofstream readOutputTestFile;
//	readOutputTestFile.open ("readOutputTestFile.txt");
//	readOutputTestFile << "Constraint1 = 2.144\nObjectiveFunction = 1.789";
//	readOutputTestFile.close();
//
//
//	/* constraint has only value */
//	ConstraintFunction constraintFunTest("testConstraintFunction",4);
//	constraintFunTest.setFileNameReadInput("readOutputTestFile.txt");
//	constraintFunTest.setReadMarker("Constraint1");
//	constraintFunTest.setID(0);
//
//	/* objective function has only value */
//	ObjectiveFunction objFunTest("testObjFun",4);
//	objFunTest.setFileNameReadInput("readOutputTestFile.txt");
//	objFunTest.setReadMarker("ObjectiveFunction");
//
//
//	constraintFunTest.readEvaluateOutput(d);
//	EXPECT_EQ(d.constraintTrueValues(0),2.144);
//
//
//	objFunTest.readEvaluateOutput(d);
//
//	EXPECT_EQ(d.trueValue,1.789);
//	EXPECT_EQ(d.objectiveFunctionValue,1.789);
//
//	remove("readOutputTestFile.txt");
//
//}
//
//TEST(testConstraintFunctions, testreadEvaluateObjectiveFunctionAndConstraintInTheSameFileWithAdjoints){
//
//	Design d(4);
//	d.setNumberOfConstraints(1);
//	std::ofstream readOutputTestFile;
//	readOutputTestFile.open ("readOutputTestFile.txt");
//	readOutputTestFile << "Constraint1 = 2.144\nConstraint1Gradient = 1.2,2.2,3.1,4.1\nObjectiveFunction = 1.789\nObjectiveFunctionGradient = 0,1,2,3";
//	readOutputTestFile.close();
//
//	ObjectiveFunction objFunTest("testObjFun",4);
//	objFunTest.setFileNameReadInput("readOutputTestFile.txt");
//	objFunTest.setReadMarker("ObjectiveFunction");
//	objFunTest.setReadMarkerAdjoint("ObjectiveFunctionGradient");
//	objFunTest.setGradientOn();
//
//	objFunTest.readEvaluateOutput(d);
//
//	EXPECT_EQ(d.trueValue,1.789);
//
//	rowvec gradientResult = d.gradient;
//	EXPECT_EQ(gradientResult(0),0);
//	EXPECT_EQ(gradientResult(1),1);
//	EXPECT_EQ(gradientResult(2),2);
//	EXPECT_EQ(gradientResult(3),3);
//
//
//	ConstraintFunction constraintFunTest("testConstraintFunction",4);
//	constraintFunTest.setFileNameReadInput("readOutputTestFile.txt");
//	constraintFunTest.setReadMarker("Constraint1");
//	constraintFunTest.setReadMarkerAdjoint("Constraint1Gradient");
//	constraintFunTest.setGradientOn();
//	constraintFunTest.setID(0);
//
//	constraintFunTest.readEvaluateOutput(d);
//	EXPECT_EQ(d.constraintTrueValues(0),2.144);
//
//	gradientResult = d.constraintGradients[0];
//	EXPECT_EQ(gradientResult(0),1.2);
//	EXPECT_EQ(gradientResult(1),2.2);
//	EXPECT_EQ(gradientResult(2),3.1);
//	EXPECT_EQ(gradientResult(3),4.1);
//
//	remove("readOutputTestFile.txt");
//
//
//}

#endif
