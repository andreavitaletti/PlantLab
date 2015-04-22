#include <stdio.h>
#include <math.h>
#include <limits.h>
#define size 1000
#define DFA_win_length 100

//const int size = 10;

float mean(float in[], int length);
float variance(float in[]);
float std(float in[]);
float *cumsum(float in[]);
float min(float in[]);
float max(float in[]);
// http://code.metager.de/source/xref/gnu/octave/scripts/signal/hurst.m
float hurst(float in[]);
float *lin_reg(float x[], float y[], int length);
float dfa(float data[]); // order is 1
float *hjorth(float data[]);

int main() {
 int i;
 /*
 double x_test[15]={88.6,71.6,93.3,84.3,80.6,75.2,69.7,82.0,69.4,83.3,79.6,82.6,80.6,83.5,76.3};
 double y_test[15]={20.0,16.0,19.8,18.4,17.1,15.5,14.7,17.1,15.4,16.3,15.0,17.2,16.0,17.0,14.4};
 */
 
float chunk[size];
 for (i = 0; i < size; i++){
    chunk[i] = i;    // sensor on analog pin 0
  }

float m = mean(chunk,size);
float v = std(chunk);
float h = hurst(chunk);
//double *f = lin_reg(x_test,y_test);
float  d = dfa(chunk);
float *hj = hjorth(chunk);

printf ("mean:%f, std:%f,hurst:%f,dfa:%f, hjorth complexity:%f,  hjorth mobility:%f\n",m,v,h,d,hj[0],hj[1]); 
return 0;

}

float mean(float in[], int length)
{
  float sum = 0;
  int i;
  for (i = 0; i < length; i++){
    sum = sum + in[i];    // sensor on analog pin 0
  }
  
  return sum/length;
  
}

float *cumsum(float in[]){
	static float app[size];
	app[0]=in[0];
	int i;
	for (i = 1; i < size; i++){
				app[i]=app[i-1]+in[i];
	}
	return app;
}	

// http://code.metager.de/source/xref/gnu/octave/scripts/signal/hurst.m
float hurst(float in[]){
	float m = mean(in,size);
	float s = std(in);
	float app[size];
	float *cs;
	int i;
	for (i = 0; i < size; i++){
				app[i]=in[i]-m;
	}
	cs = cumsum(app);
	float RS = (max(cs)-min(cs))/s;
	float H = log(RS)/log(size);
	return H;
}	

float min(float in[]){
	float min = INT_MAX;
	int i;
	for (i = 0; i < size; i++){
				if (in[i] < min){
					min = in[i];
					}
	}
	return min;
	}
	
float max(float in[]){
	float max = INT_MIN;
	int i;
	for (i = 0; i < size; i++){
				if (in[i] > max){
					max = in[i];
					}
	}
	return max;
	}

float variance(float in[])
{
  float sum = 0;
  int i;
  float mu = mean(in,size);
  for (i = 0; i < size; i++){  
    sum = sum + pow((in[i]-mu),2);    // sensor on analog pin 0
  }
  
  return sum/(size-1);
  
}

// std (x) = sqrt ( 1/(N-1) SUM_i (x(i) - mean(x))^2 )

float std(float in[])
{
  float sum = 0;
  float mu = mean(in,size);
  int i;
  for (i = 0; i < size; i++){  
    sum = sum + pow((in[i]-mu),2);    // sensor on analog pin 0
  }
  
  return sqrt(sum/(size-1));
  
}

float *hjorth(float data[])
{
  int i;
  static float coeff[2];
  float dxV[size-1],ddxV[size-2],d2[size];
  for (i = 0; i < size-1; i++){
	  dxV[i]=pow(data[i+1]-data[i],2);
  }
  for (i = 0; i < size-2; i++){
	  ddxV[i]=pow(dxV[i+1]-dxV[i],2);
  }
  for (i = 0; i < size; i++){
	  d2[i]=data[i]*data[i];
  }
  float mx2 = mean(d2,size);
  float mdx2 = mean(dxV,size-1);
  float mddx2 = mean(ddxV,size-2);

  float mob = mdx2 / mx2;
  float complexity = sqrt(mddx2 / mdx2 - mob);
  float mobility = sqrt(mob);  
  
  coeff[0] = complexity;
  coeff[1] = mobility;
  
  return coeff;
  
}



// http://stackoverflow.com/questions/6846467/where-can-i-find-source-code-to-do-a-simple-linear-regression
float *lin_reg(float x[], float y[], int length)
{
  int i;
  static float coeff[2]; /* coeff[0] y-intercept of best fit line coeff[1] slope of best fit line */
  //double   b;                                 /* y-intercept of best fit line  */
  //double   m;                                 /* slope of best fit line        */
  //double   n = 0.0;                           /* number of data points         */
  //double   r;                                 /* correlation coefficient       */
float   sumx = 0.0;                        /* sum of x                      */
float   sumx2 = 0.0;                       /* sum of x**2                   */
float   sumxy = 0.0;                       /* sum of x * y                  */
float   sumy = 0.0;                        /* sum of y                      */
float   sumy2 = 0.0;                       /* sum of y**2                   */
for (i = 0; i < length; i++){  

      sumx  += x[i];                           /* compute sum of x              */
      sumx2 += x[i] * x[i];                       /* compute sum of x**2           */
      sumxy += x[i] * y[i];                       /* compute sum of x * y          */
      sumy  = sumy + y[i];                           /* compute sum of y              */
      sumy2 += y[i] * y[i];                       /* compute sum of y**2           */
                                          
}
/*---------------------------------------------------------------------------*/
/*  Compute least-squares best fit straight line.                            */
/*---------------------------------------------------------------------------*/

	coeff[1] = (length * sumxy  -  sumx * sumy) /        /* b - compute slope                 */
       (length * sumx2 - pow(sumx,2));

	coeff[0] = (sumy * sumx2  -  sumx * sumxy) /    /* a - compute y-intercept           */
      (length * sumx2  -  pow(sumx,2));


//   r = (sumxy - sumx * sumy / n) /          /* compute correlation coeff     */
//            sqrt((sumx2 - sqr(sumx)/n) *
//            (sumy2 - sqr(sumy)/n));

return coeff;
}

float dfa(float data[]) // order is 1
{
	int n=floor(size/DFA_win_length);
	int N1=n*DFA_win_length;
	float y[N1];
	float Yn[N1];
	float sum;
	float fitcoef0[n];
	float fitcoef1[n];
	int i,j;
	for (i = 0; i < N1; i++){ 
		y[i]=0.0;
		Yn[i]=0.0;
		sum = sum + data[i];
	}
	float mean1 = sum/N1;
	
	sum = 0.0;
	y[0]=data[0]-mean1;
	for (i = 1; i < N1; i++){ 
		y[i]=y[i-1]+data[i]-mean1;		
	}
	
	float xx[DFA_win_length];
	for (i = 0; i < DFA_win_length; i++){ 
			xx[i]=i+1;
		}
	
	for (j = 0; j < n; j++){ 
		float yy[DFA_win_length];
		int l=0;
		for (i = j*DFA_win_length; i < (j+1)*DFA_win_length; i++){ 
			yy[l]=y[i];
			l=l+1;
		}
		float *f =lin_reg(xx,yy,DFA_win_length);
		fitcoef0[j]=f[0];
		fitcoef1[j]=f[1];
	}

	for (j = 0; j < n; j++){ 
		int app = 1;
		for (i = j*DFA_win_length; i < (j+1)*DFA_win_length; i++){ 
			Yn[i]=fitcoef1[j]*app+fitcoef0[j];
			app = app + 1;
		}
	}
	
	float sum1 = 0.0;
	for (j = 0; j < N1; j++){
		sum1 = sum1 + pow((y[j]-Yn[j]),2);
	}
	
	sum1 = sum1/N1;
	return sqrt(sum1);
	}


