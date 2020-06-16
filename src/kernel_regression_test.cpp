#include "kernel_regression.hpp"
#include "kernel_regression_test.hpp"
#include "auxilliary_functions.hpp"
#include <armadillo>

using namespace arma;

void testAllKernelRegression(void){

	testcalculateMetric();
	testcalculateMetricAdjoint();
	testcalculateMahalanobisMatrix();
	testcalculateLossFunctions();
	testcalculateLossFunctionsAdjoint();
	testcalculateGaussianKernel();
	testcalculateGaussianKernelAdjoint();
	testcalculateKernelRegressionWeights();
	testcalculateKernelRegressionWeightsAdjoint();
	testcalculateLossFunctionAdjointL2();
	testcalculateLossFunctionAdjointL1();
	testKernelRegressionTrain();


}
/* This function generates a test data for kernel regression using N data points with "dim" dimensional random
 * data. Functional values are generated by a randomized polynomial of degree "dim".
 */

void generateRandomTestDataForKernelRegression(int dim, int N){


	mat testData(N,dim+1,fill::zeros);
	mat X(N,dim,fill::randn);
	vec y(N, fill::zeros);
	/* we generate coefficients for a random polynomial */

	vec coeffsPolynomial(dim, fill::zeros);

	coeffsPolynomial = generateRandomVector(-10.0,10.0,dim);

	for(int i=0; i<N; i++){
		rowvec x = X.row(i);
		y(i) = calculatePolynomial(x, coeffsPolynomial);

	}

	for(int i=0; i<dim; i++){

		testData.col(i) = X.col(i);
	}
	testData.col(dim) = y;
	testData.save("testData.csv",arma::csv_ascii);

}



void testcalculateMahalanobisMatrix(void){

	printf("%s:",__func__ );

	int dim = generateRandomInt(4,8);
	int N =  generateRandomInt(100,200);

	generateRandomTestDataForKernelRegression(dim,N);

	KernelRegressionModel2 testModel("testData",dim);
	testModel.initializeMahalanobisMatrixRandom();
	testModel.calculateMahalanobisMatrix();
	mat lowerDiagTest = testModel.lowerDiagonalMatrix;

	mat mahalanabisMatTest =  lowerDiagTest * trans(lowerDiagTest);
	bool passTest = checkMatrix(mahalanabisMatTest,testModel.mahalanobisMatrix);

	abortIfFalse(passTest,__FILE__, __LINE__);

	printf("\t passed\n");

	remove("testData.csv");


}


void testcalculateKernelRegressionWeights(void){

	printf("%s:",__func__ );

	mat kernelMat(3,3);
	kernelMat(0,0) = 1.0; kernelMat(0,1) = 2.0; kernelMat(0,2) = 3.0;
	kernelMat(1,0) = 2.0; kernelMat(1,1) = 1.0; kernelMat(1,2) = 4.0;
	kernelMat(2,0) = 3.0; kernelMat(2,1) = 4.0; kernelMat(2,2) = 1.0;



	KernelRegressionModel2 testModel("None",2);
	testModel.initializeSurrogateModel();
	testModel.Ntraining =3;

	mat weights = testModel.calculateKernelRegressionWeights(kernelMat);


	bool passTest = checkValue(weights(0,1),2.0/5.0);
	passTest = checkValue(weights(0,2),3.0/5.0);
	abortIfFalse(passTest,__FILE__, __LINE__);
	passTest = checkValue(weights(1,0),2.0/6.0);
	abortIfFalse(passTest,__FILE__, __LINE__);
	passTest = checkValue(weights(1,2),4.0/6.0);
	abortIfFalse(passTest,__FILE__, __LINE__);
	passTest = checkValue(weights(2,0),3.0/7.0);
	abortIfFalse(passTest,__FILE__, __LINE__);
	passTest = checkValue(weights(2,1),4.0/7.0);
	abortIfFalse(passTest,__FILE__, __LINE__);
	/* check row sums */
	passTest = checkValue(sum(weights.row(0)),1.0);
	abortIfFalse(passTest,__FILE__, __LINE__);
	passTest = checkValue(sum(weights.row(1)),1.0);
	abortIfFalse(passTest,__FILE__, __LINE__);
	passTest = checkValue(sum(weights.row(2)),1.0);
	abortIfFalse(passTest,__FILE__, __LINE__);

	printf("\t passed\n");
}



void testcalculateKernelRegressionWeightsAdjoint(void){

	printf("%s:",__func__ );

	int N = generateRandomInt(5,10);
	KernelRegressionModel2 testModel("None",2);

	testModel.Ntraining = N;

	mat kernelMat(N,N,fill::randn);
	mat kernelMatb(N,N,fill::zeros);

	kernelMat = abs(kernelMat);
	kernelMat = kernelMat*trans(kernelMat);

	for(int i=0; i<N; i++){

		kernelMat(i,i) = 1.0;
	}


	mat weights = testModel.calculateKernelRegressionWeights(kernelMat);

	mat weightsb(N,N,fill::zeros);

	weightsb(3,4) = 1.0;

	mat weightsFromAdj = testModel.calculateKernelRegressionWeightsAdjoint(kernelMat, kernelMatb, weightsb);


	bool passTest = checkMatrix(weights,weightsFromAdj);
	abortIfFalse(passTest,__FILE__, __LINE__);


	const double eps = kernelMat(3,4)*0.0001;
	kernelMat(3,4) += eps;
	kernelMat(4,3) = kernelMat(3,4);
	mat weightsPlus = testModel.calculateKernelRegressionWeights(kernelMat);

	kernelMat(3,4) -= 2*eps;
	kernelMat(4,3) = kernelMat(3,4);
	mat weightsMinus = testModel.calculateKernelRegressionWeights(kernelMat);

	mat fdValuesMat = (weightsPlus - weightsMinus)*(1.0/(2*eps) );

	passTest = checkValue(fdValuesMat(3,4),kernelMatb(3,4));
	abortIfFalse(passTest,__FILE__, __LINE__);

	kernelMat(3,0) += eps;
	kernelMat(0,3) = kernelMat(3,0);
	weightsPlus = testModel.calculateKernelRegressionWeights(kernelMat);

	kernelMat(3,0) -= 2*eps;
	kernelMat(0,3) = kernelMat(3,0);
	weightsMinus = testModel.calculateKernelRegressionWeights(kernelMat);

	fdValuesMat = (weightsPlus - weightsMinus)*(1.0/(2*eps) );

	passTest = checkValue(fdValuesMat(3,4),kernelMatb(3,0));
	abortIfFalse(passTest,__FILE__, __LINE__);

	printf("\t passed\n");

}

void testcalculateLossFunctions(void){


	std::string testDataFileName = "testcalculateLossFunctions.csv";

	printf("%s:",__func__ );

	mat testData(3,3,fill::zeros);

	testData(0,0) = 0.0; testData(0,1) = 1.0; testData(0,2) = 1.0;
	testData(1,0) = 1.0; testData(1,1) = 0.0; testData(1,2) = 2.0;
	testData(2,0) = 3.0; testData(2,1) = 2.0; testData(2,2) = 3.0;

	testData.save(testDataFileName,arma::csv_ascii);


	KernelRegressionModel2 testModel("testcalculateLossFunctions",2);
	testModel.initializeSurrogateModel();

	remove(testDataFileName.c_str());


	/* for the test x and y coordinates in the testData do not matter, we give the weights manually */
	mat weights(3,3);
	weights(0,0) = 0.0; weights(0,1) = 3.0/4.0; weights(0,2) = 1.0/4.0;
	weights(1,0) = 1.0/2.0; weights(1,1) = 0.0; weights(1,2) = 1.0/2.0;
	weights(2,0) = 1.0/5.0; weights(2,1) = 4.0/5.0; weights(2,2) = 0.0;

	/* first  point : 2*3/4 + 3*1/4 = 2.25, Squared error = (1-2.25)^2 = 1.5625
	 * second point : 1*1/2 + 3*1/2 = 2     Squared error = 0
	 * third  point : 1*1/5 + 2*4/5 = 1.8   Squared error = (3-1.8)^2  = 1.44
	 * total loss = 3.0025
	 *
	 * */


	double lossFuncVal = testModel.calculateL2Loss(weights);

	bool passTest = checkValue(lossFuncVal,3.0025,10E-6);

	abortIfFalse(passTest,__FILE__, __LINE__);


	/* first  point : 2*3/4 + 3*1/4 = 2.25, Abs error = (1-2.25) = 1.25
	 * second point : 1*1/2 + 3*1/2 = 2     Abs error = 0
	 * third  point : 1*1/5 + 2*4/5 = 1.8   Abs error = (3-1.8)  = 1.2
	 * total loss = 2.45
	 *
	 * */


	lossFuncVal = testModel.calculateL1Loss(weights);

	passTest = checkValue(lossFuncVal,2.45,10E-6);

	abortIfFalse(passTest,__FILE__, __LINE__);

	printf("\t passed\n");

}


/* this is a randomized test, depends on the testcalculateLossFunctions */

void testcalculateLossFunctionsAdjoint(void){

	std::string testDataFileName = "testcalculateLossFunctions.csv";

	int N= generateRandomInt(50,100);
	int dim= generateRandomInt(5,10);

	printf("%s:",__func__ );

	mat testData(N,dim+1,fill::randn);

	testData.save(testDataFileName,arma::csv_ascii);

	KernelRegressionModel2 testModel("testcalculateLossFunctions",dim);
	testModel.initializeSurrogateModel();

	remove(testDataFileName.c_str());


	unsigned int Ntraining = testModel.Ntraining;

	mat weights = generateRandomWeightMatrix(Ntraining);
	mat weightsb(Ntraining,Ntraining, fill::zeros);


	/* Testing the L2 Loss function */

	double lossFuncVal = testModel.calculateL2Loss(weights);
	double lossFuncValFromAdj = testModel.calculateL2LossAdjoint(weights, weightsb);



	bool passTest = checkValue(lossFuncVal,lossFuncValFromAdj);
	abortIfFalse(passTest,__FILE__, __LINE__);

	const double eps = 0.00001;
	for(unsigned int i=0; i<Ntraining; i++){

		for(unsigned int j=0; j<Ntraining; j++){
			weights(i,j)+=eps;
			double fplus  = testModel.calculateL2Loss(weights);
			weights(i,j)-=2*eps;
			double fminus = testModel.calculateL2Loss(weights);
			double fdValue = (fplus-fminus)/(2*eps);

			weights(i,j)+=eps;

			passTest = checkValue(weightsb(i,j),fdValue);
			abortIfFalse(passTest,__FILE__, __LINE__);


		}
	}



	/* Testing the L1 Loss function */

	lossFuncVal = testModel.calculateL1Loss(weights);
	lossFuncValFromAdj = testModel.calculateL1LossAdjoint(weights, weightsb);

	/* first check primal value */
	passTest = checkValue(lossFuncVal,lossFuncValFromAdj);
	abortIfFalse(passTest,__FILE__, __LINE__);


	for(unsigned int i=0; i<Ntraining; i++){

		for(unsigned int j=0; j<Ntraining; j++){

			weights(i,j)+=eps;
			double fplus  = testModel.calculateL1Loss(weights);
			weights(i,j)-=2*eps;
			double fminus = testModel.calculateL1Loss(weights);
			double fdValue = (fplus-fminus)/(2*eps);
			weights(i,j)+=eps;

			passTest = checkValue(weightsb(i,j),fdValue);
			abortIfFalse(passTest,__FILE__, __LINE__);

		}
	}

	printf("\t passed\n");


}



void testcalculateGaussianKernel(void){

	printf("%s:",__func__ );

	KernelRegressionModel2 testModel("None",2);
	testModel.initializeSurrogateModel();

	rowvec x1(3);
	rowvec x2(3);

	x1(0) = -1.2; x1(1) = 1.0;  x1(2) = 0.75;
	x2(0) =  9.2; x2(1) = 1.3;  x2(2) = -1.05;

	mat M(3,3);
	mat L(3,3); L.eye();


	L(1,0) = 1.2;
	L(2,0) = 0.9;
	L(2,1) = 1.05;

	M = L*trans(L);

	testModel.mahalanobisMatrix = M;
	testModel.sigmaGaussianKernel = 10.0;

	double kernelVal = testModel.calculateGaussianKernel(x1,x2);

	bool passTest = checkValue(kernelVal,0.025525887997848,10E-6);

	abortIfFalse(passTest,__FILE__, __LINE__);

	printf("\t passed\n");
}

void testcalculateGaussianKernelAdjoint(void){

	printf("%s:",__func__ );

	int dim = generateRandomInt(4,8);
	rowvec xi(dim,fill::randn);
	rowvec xj(dim,fill::randn);

	KernelRegressionModel2 testModel("None",dim);
	testModel.initializeSurrogateModel();
	testModel.initializeSigmaRandom();
	testModel.initializeMahalanobisMatrixRandom();
#if 0
	testModel.printHyperParameters();
#endif
	double kernelValfromPrimal = testModel.calculateGaussianKernel(xi, xj);

	double kernelValfromAdj = testModel.calculateGaussianKernelAdjoint(xi, xj,1.0);


	bool passTest = checkValue(kernelValfromPrimal,kernelValfromAdj,kernelValfromAdj/1000.0);
	abortIfFalse(passTest,__FILE__, __LINE__);

	const double eps = 0.0001;

	/* check derivatives w.r.t. Mahalanobis matrix entries */
	for(int i=0; i<dim; i++){

		for(int j=0; j<dim; j++){

			testModel.mahalanobisMatrix(i,j) += eps;
			double fplus  = testModel.calculateGaussianKernel(xi,xj);
			testModel.mahalanobisMatrix(i,j) -= 2*eps;

			double fminus = testModel.calculateGaussianKernel(xi,xj);
			testModel.mahalanobisMatrix(i,j) += eps;
			double fdValue = (fplus-fminus)/(2.0*eps); /* central finite difference approximation */
			passTest = checkValue(fdValue, testModel.mahalanobisMatrixAdjoint(i,j));
			abortIfFalse(passTest,__FILE__, __LINE__);

		}


	}


	testModel.sigmaGaussianKernel += eps;
	double fplus   = testModel.calculateGaussianKernel(xi,xj);
	testModel.sigmaGaussianKernel -= 2*eps;
	double fminus  = testModel.calculateGaussianKernel(xi,xj);
	double fdValue = (fplus-fminus)/(2.0*eps);
	passTest = checkValue(fdValue, testModel.sigmaGaussianKernelAdjoint);
	abortIfFalse(passTest,__FILE__, __LINE__);

	printf("\t passed\n");

}

void testcalculateMetric(void){

	printf("%s:",__func__ );

	rowvec x1(3);
	rowvec x2(3);

	mat M(3,3);
	mat L(3,3); L.eye();

	L(1,0) = 1.2;
	L(2,0) = 0.9;
	L(2,1) = 1.05;

	M = L*trans(L);

	x1(0) = -1.2; x1(1) = 1.0;  x1(2) = 0.75;
	x2(0) =  9.2; x2(1) = 1.3;  x2(2) = -1.05;

	double value = calculateMetric(x1,x2,M);

	bool passTest = checkValue(value,89.3077,0.000000001);

	abortIfFalse(passTest,__FILE__, __LINE__);

	printf("\t passed\n");
}

/* This is a randomized test */
void testcalculateMetricAdjoint(void){

	printf("%s:",__func__ );

	int dim = generateRandomInt(3,10);

	mat M(dim,dim,fill::randn);
	rowvec x1(dim,fill::randn);
	rowvec x2(dim,fill::randn);

	/* we make sure that M is spd */
	M = M*trans(M);

	mat Mb(dim,dim,fill::zeros);

	double metricVal = calculateMetric(x1,x2,M);
	double metricValfromAdjoint = calculateMetricAdjoint(x1,x2,M,Mb,1.0);

	bool passTest = checkValue(metricVal,metricValfromAdjoint);
	abortIfFalse(passTest,__FILE__, __LINE__);

	/* we check all derivatives by central finite differences */
	const double eps = 0.00001;
	for(int i=0; i<dim; i++){

		for(int j=0; j<dim; j++){

			M(i,j)+= eps;
			double fplus  = calculateMetric(x1,x2,M);
			M(i,j)-= 2*eps;
			double fminus = calculateMetric(x1,x2,M);
			M(i,j)+= eps;
			double fdValue = (fplus-fminus)/(2*eps);

			passTest = checkValue(Mb(i,j),fdValue);

			abortIfFalse(passTest,__FILE__, __LINE__);
		}

	}

	printf("\t passed\n");
}



void testcalculateLossFunctionAdjointL2(void){

	printf("%s:",__func__ );

	int dim = generateRandomInt(4,8);
	int N =  generateRandomInt(5,10);

	mat testData(N,dim+1,fill::randn);

	testData.save("testData.csv",arma::csv_ascii);

	KernelRegressionModel2 testModel("testData",dim);
	testModel.initializeSurrogateModel();
	testModel.initializeMahalanobisMatrixRandom();

	double lossFunc = testModel.calculateLossFunction();
	double lossFuncFromAdj = testModel.calculateLossFunctionAdjoint();

	bool passTest = checkValue(lossFunc,lossFuncFromAdj);
	abortIfFalse(passTest,__FILE__, __LINE__);

	/* check derivative w.r.t. to sigmaGaussianKernel */
	const double epsSigma = testModel.sigmaGaussianKernel*0.001;
	testModel.sigmaGaussianKernel += epsSigma;
	double fplus = testModel.calculateLossFunction();
	testModel.sigmaGaussianKernel -= 2*epsSigma;
	double fminus = testModel.calculateLossFunction();
	testModel.sigmaGaussianKernel += epsSigma;
	double fdValue = (fplus - fminus) /(2*epsSigma);
	passTest = checkValue(testModel.sigmaGaussianKernelAdjoint,fdValue);
	abortIfFalse(passTest,__FILE__, __LINE__);

	/* check derivatives w.r.t. to L (only lower diagonal)*/
	const double epsL = testModel.lowerDiagonalMatrix(0,0)*0.001;
	mat fdValues(dim,dim, fill::zeros);
	for(int i=0 ; i<dim; i++){

		for(int j=0; j<dim; j++){

			testModel.lowerDiagonalMatrix(i,j) += epsL;
			fplus = testModel.calculateLossFunction();
			testModel.lowerDiagonalMatrix(i,j) -= 2*epsL;
			fminus = testModel.calculateLossFunction();
			fdValues(i,j) = (fplus - fminus) /(2*epsL);
			testModel.lowerDiagonalMatrix(i,j) += epsL;
		}


	}


	passTest = checkMatrix(testModel.lowerDiagonalMatrixAdjoint, fdValues);
	abortIfFalse(passTest,__FILE__, __LINE__);

	printf("\t passed\n");
}

void testcalculateLossFunctionAdjointL1(void){

	printf("%s:",__func__ );

	int dim = generateRandomInt(4,8);
	int N =  generateRandomInt(5,10);

	mat testData(N,dim+1,fill::randn);

	testData.save("testData.csv",arma::csv_ascii);

	KernelRegressionModel2 testModel("testData",dim);
	testModel.lossFunctionType = L1_LOSS_FUNCTION;
	testModel.initializeSurrogateModel();

	double lossFunc = testModel.calculateLossFunction();
	double lossFuncFromAdj = testModel.calculateLossFunctionAdjoint();

	bool passTest = checkValue(lossFunc,lossFuncFromAdj);
	abortIfFalse(passTest);

	/* check derivative w.r.t. to sigmaGaussianKernel */
	const double epsSigma = testModel.sigmaGaussianKernel*0.0001;
	testModel.sigmaGaussianKernel += epsSigma;
	double fplus = testModel.calculateLossFunction();
	testModel.sigmaGaussianKernel -= 2*epsSigma;
	double fminus = testModel.calculateLossFunction();
	testModel.sigmaGaussianKernel += epsSigma;
	double fdValue = (fplus - fminus) /(2*epsSigma);
	passTest = checkValue(testModel.sigmaGaussianKernelAdjoint,fdValue);
	abortIfFalse(passTest);

#if 0
	printf("sigmaGaussianKernelAdj = %10.7f, FD Approx. = %10.7f\n",testModel.sigmaGaussianKernelAdj,fdValue);
#endif


	/* check derivatives w.r.t. to mahalanobisMatrix */
	const double epsL = testModel.lowerDiagonalMatrix(0,0)*0.0001;
	mat fdValues(dim,dim);
	for(int i=0 ; i<dim; i++){

		for(int j=0; j< dim; j++){

			testModel.lowerDiagonalMatrix(i,j) += epsL;
			fplus = testModel.calculateLossFunction();
			testModel.lowerDiagonalMatrix(i,j) -= 2*epsL;
			fminus = testModel.calculateLossFunction();
			fdValues(i,j) = (fplus - fminus) /(2*epsL);
			testModel.lowerDiagonalMatrix(i,j) += epsL;
		}


	}

	passTest = checkMatrix(testModel.lowerDiagonalMatrixAdjoint, fdValues);
	abortIfFalse(passTest,__FILE__, __LINE__);
#if 0
	printMatrix(testModel.lowerDiagonalMatrixAdjoint, "lowerDiagonalMatrixAdjoint");
	printMatrix(fdValues, "Finite Difference Values");
#endif
	printf("\t passed\n");
}

void testKernelRegressionTrain(void){

	printf("%s:",__func__ );

	int dim = generateRandomInt(4,8);
	int N =  generateRandomInt(100,200);

	mat testData(N,dim+1,fill::zeros);
	mat X(N,dim,fill::randn);
	vec y(N, fill::zeros);
	/* we generate coefficients for a random polynomial */

	vec coeffsPolynomial(dim, fill::zeros);

	coeffsPolynomial = generateRandomVector(-10.0,10.0,dim);

	for(int i=0; i<N; i++){
		rowvec x = X.row(i);
		y(i) = calculatePolynomial(x, coeffsPolynomial);

	}

	for(int i=0; i<dim; i++){

		testData.col(i) = X.col(i);
	}
	testData.col(dim) = y;
	testData.save("testData.csv",arma::csv_ascii);

	KernelRegressionModel2 testModel("testData",dim);
	testModel.initializeSurrogateModel();

	printMatrix(testData, "testData");


	testModel.train();



}

