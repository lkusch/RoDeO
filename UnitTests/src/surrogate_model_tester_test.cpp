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
#include "surrogate_model_tester.hpp"
#include "matrix_vector_operations.hpp"
#include "bounds.hpp"
#include "random_functions.hpp"
#include "standard_test_functions.hpp"
#include "test_defines.hpp"


#ifdef TEST_SURROGATE_MODEL_TESTER




class SurrogateTesterTest : public ::testing::Test {
protected:
	void SetUp() override {


		linearTestFunction.function.filenameTestData = "linearTestData.csv";
		linearTestFunction.function.filenameTrainingData =  "linearTrainingData.csv";
		linearTestFunction.function.numberOfTrainingSamples = 50;
		linearTestFunction.function.numberOfTestSamples = 100;


		testFunction.function.filenameTestData = "testData.csv";
		testFunction.function.filenameTrainingData =  "trainingData.csv";
		testFunction.function.numberOfTrainingSamples = 50;
		testFunction.function.numberOfTestSamples = 100;

		testFunction.function.filenameTrainingDataHighFidelity = "trainingData.csv";
		testFunction.function.filenameTrainingDataLowFidelity = "trainingDataLowFi.csv";
		testFunction.function.numberOfTrainingSamplesLowFi = 100;

	}

	void TearDown() override {}

	SurrogateModelTester surrogateTester;
	HimmelblauFunction testFunction;
	LinearTestFunction1 linearTestFunction;

};


TEST_F(SurrogateTesterTest, setSurrogateModel){


	surrogateTester.setName("testModel");
	surrogateTester.setFileNameTrainingData("trainingData.csv");
	surrogateTester.setFileNameTestData("test.csv");
	surrogateTester.setSurrogateModel(LINEAR_REGRESSION);

	bool ifSurrogateModelSpecified = surrogateTester.isSurrogateModelSpecified();
	ASSERT_EQ(ifSurrogateModelSpecified,true);

}

TEST_F(SurrogateTesterTest, setBoxConstraints){

	unsigned int dim = 2;
	vec lowerBounds(dim, fill::zeros);
	vec upperBounds(dim, fill::ones);
	Bounds boxConstraints(lowerBounds , upperBounds);

	SurrogateModelTester surrogateTester;
	surrogateTester.setBoxConstraints(boxConstraints);

	Bounds boxConstraintsGet = surrogateTester.getBoxConstraints();

	unsigned int dimBoxConstraints = boxConstraintsGet.getDimension();
	ASSERT_EQ(dimBoxConstraints,2);

}


TEST_F(SurrogateTesterTest, performSurrogateModelTestMultiLevelOnlyFunctionalValues){


	Bounds boxConstraints;
	boxConstraints.setDimension(2);
	boxConstraints.setBounds(-6.0, 6.0);

	surrogateTester.setBoxConstraints(boxConstraints);
	testFunction.function.generateTrainingSamplesMultiFidelity();
	testFunction.function.generateTestSamples();



	surrogateTester.setName("testModel");
	surrogateTester.setFileNameTrainingData(testFunction.function.filenameTrainingData);
	surrogateTester.setFileNameTrainingDataLowFidelity(testFunction.function.filenameTrainingDataLowFidelity);

	surrogateTester.setFileNameTestData(testFunction.function.filenameTestData);
	surrogateTester.setNumberOfTrainingIterations(1000);

	surrogateTester.setSurrogateModel(MULTI_LEVEL);
	surrogateTester.setDisplayOn();
	surrogateTester.performSurrogateModelTest();

	mat results;
	results.load("surrogateTestResults.csv", csv_ascii);

	vec SE = results.col(4);

	EXPECT_LT(mean(SE), 100000);

	remove("surrogateTestResults.csv");
	remove(testFunction.function.filenameTrainingDataHighFidelity.c_str());
	remove(testFunction.function.filenameTrainingDataLowFidelity.c_str());
	remove(testFunction.function.filenameTestData.c_str());

}



TEST_F(SurrogateTesterTest, performSurrogateModelTestLinearRegression){

	Bounds boxConstraints;
	boxConstraints.setDimension(2);
	boxConstraints.setBounds(-3.0, 3.0);

	surrogateTester.setBoxConstraints(boxConstraints);

	linearTestFunction.function.generateTrainingSamples();
	linearTestFunction.function.generateTestSamples();

	surrogateTester.setName("testModel");
	surrogateTester.setFileNameTrainingData(linearTestFunction.function.filenameTrainingData);
	surrogateTester.setFileNameTestData(linearTestFunction.function.filenameTestData);

	surrogateTester.setSurrogateModel(LINEAR_REGRESSION);

	//	surrogateTester.setDisplayOn();
	surrogateTester.performSurrogateModelTest();

	mat results;
	results.load("surrogateTestResults.csv", csv_ascii);

	vec SE = results.col(4);

	EXPECT_LT(mean(SE), 0.00001);

	remove("surrogateTestResults.csv");
	remove(linearTestFunction.function.filenameTrainingData.c_str());
	remove(linearTestFunction.function.filenameTestData.c_str());



}


TEST_F(SurrogateTesterTest, performSurrogateModelTestOrdinaryKriging){


	Bounds boxConstraints;
	boxConstraints.setDimension(2);
	boxConstraints.setBounds(-6.0, 6.0);

	surrogateTester.setBoxConstraints(boxConstraints);

	testFunction.function.generateTrainingSamples();
	testFunction.function.generateTestSamples();

	surrogateTester.setName("testModel");
	surrogateTester.setFileNameTrainingData(testFunction.function.filenameTrainingData);
	surrogateTester.setFileNameTestData(testFunction.function.filenameTestData);

	surrogateTester.setNumberOfTrainingIterations(1000);


	surrogateTester.setSurrogateModel(ORDINARY_KRIGING);
	//	surrogateTester.setDisplayOn();


	surrogateTester.performSurrogateModelTest();

	mat results;
	results.load("surrogateTestResults.csv", csv_ascii);

	vec SE = results.col(4);

	EXPECT_LT(mean(SE), 100000);

	remove("surrogateTestResults.csv");
	remove(testFunction.function.filenameTrainingData.c_str());
	remove(testFunction.function.filenameTestData.c_str());


}


TEST_F(SurrogateTesterTest, performSurrogateModelTestUniversalKriging){


	Bounds boxConstraints;
	boxConstraints.setDimension(2);
	boxConstraints.setBounds(-6.0, 6.0);

	surrogateTester.setBoxConstraints(boxConstraints);

	testFunction.function.generateTrainingSamples();
	testFunction.function.generateTestSamples();

	surrogateTester.setName("testModel");
	surrogateTester.setFileNameTrainingData(testFunction.function.filenameTrainingData);
	surrogateTester.setFileNameTestData(testFunction.function.filenameTestData);

	surrogateTester.setNumberOfTrainingIterations(1000);


	surrogateTester.setSurrogateModel(UNIVERSAL_KRIGING);
	//	surrogateTester.setDisplayOn();


	surrogateTester.performSurrogateModelTest();

	mat results;
	results.load("surrogateTestResults.csv", csv_ascii);

	vec SE = results.col(4);

	EXPECT_LT(mean(SE), 100000);

	remove("surrogateTestResults.csv");
	remove(testFunction.function.filenameTrainingData.c_str());
	remove(testFunction.function.filenameTestData.c_str());

}

TEST_F(SurrogateTesterTest, performSurrogateModelTestTangentModel){


	Bounds boxConstraints;
	boxConstraints.setDimension(2);
	boxConstraints.setBounds(-6.0, 6.0);

	surrogateTester.setBoxConstraints(boxConstraints);
	testFunction.function.generateTrainingSamplesWithTangents();
	testFunction.function.generateTestSamples();

	surrogateTester.setName("testModel");
	surrogateTester.setFileNameTrainingData(testFunction.function.filenameTrainingData);
	surrogateTester.setFileNameTestData(testFunction.function.filenameTestData);
	surrogateTester.setNumberOfTrainingIterations(1000);

	surrogateTester.setSurrogateModel(TANGENT);
//	surrogateTester.setDisplayOn();
	surrogateTester.performSurrogateModelTest();

	mat results;
	results.load("surrogateTestResults.csv", csv_ascii);

	vec SE = results.col(4);

	EXPECT_LT(mean(SE), 100000);

	remove("surrogateTestResults.csv");
	remove(testFunction.function.filenameTrainingData.c_str());
	remove(testFunction.function.filenameTestData.c_str());

}


#endif
