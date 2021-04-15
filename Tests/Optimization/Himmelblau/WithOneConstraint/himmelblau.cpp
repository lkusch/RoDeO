#include<stdio.h>
#include<math.h>
#include <unistd.h>
double Himmelblau(double *x){
	
	return pow( (x[0]*x[0]+x[1]-11.0), 2.0 ) + pow( (x[0]+x[1]*x[1]-7.0), 2.0 );


}
double constraint(double *x){
		return x[0]+x[1];
}

int main(void){

double x[2];
FILE *inp = fopen("dv.dat","r");
fscanf(inp,"%lf",&x[0]);
fscanf(inp,"%lf",&x[1]);
fclose(inp);
double result = Himmelblau(x);
double constraintValue = constraint(x);
FILE *outp = fopen("objFunVal.dat","w");
fprintf(outp,"%15.10f %15.10f\n",result,constraintValue);
fclose(outp);
return 0;
}
