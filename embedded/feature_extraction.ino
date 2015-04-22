//#include <analogShield.h>   //Include to use analog shield.
//#include <SPI.h>	//required for ChipKIT but does not affect Arduino
#include <stdio.h>
#include <math.h>
#include <limits.h>

#define chunk_size 50
#define DFA_win_length 25

float mean(float in[], int length);
float variance(float in[]);
float stdev(float in[]);
float *cumsum(float in[]);
float minn(float in[]);
float maxx(float in[]);
float hurst(float in[]);
float *lin_reg(float x[], float y[], int length);
float dfa(float data[]); // order is 1
float *hjorth(float data[]);

float chunk[chunk_size];

void setup()
{
  Serial.begin(9600);  
  Serial.println("START");
}

unsigned int count = 0;
void loop()
{
  /*
  count = analog.read(0);  //read in on port labeled 'IN0'
  Serial.println(count);
  */
  
  int i;
  //for (i = 0; i < chunk_size; i++){
  for (i = 0; i < chunk_size; i++){
      chunk[i] = i;    // sensor on analog pin 0
      //Serial.println(i);
      //delay(500);
  }
  
  
  Serial.println("CHUNK OK");
  
  

  float m = mean(chunk,chunk_size);
  float v = stdev(chunk);
  float h = hurst(chunk);
  //float *f = lin_reg(x_test,y_test);
  float  d = dfa(chunk);
  float *hj = hjorth(chunk);
  
  Serial.println(m,3);
  Serial.println(v,3);
  Serial.println(h,3);
  Serial.println(d,3);
  Serial.println(hj[0],3); //complexity
  Serial.println(hj[1],3); //mobility 


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

float variance(float in[])
{
  float sum = 0;
  int i;
  float mu = mean(in,chunk_size);
  for (i = 0; i < chunk_size; i++){  
    sum = sum + pow((in[i]-mu),2);    // sensor on analog pin 0
  }
  return sum/(chunk_size-1);
}

float stdev(float in[])
{
  float sum = 0;
  float mu = mean(in,chunk_size);
  int i;
  for (i = 0; i < chunk_size; i++){  
    sum = sum + pow((in[i]-mu),2);    // sensor on analog pin 0
  }
  return sqrt(sum/(chunk_size-1));
}

float *cumsum(float in[]){
	static float app[chunk_size];
	app[0]=in[0];
	int i;
	for (i = 1; i < chunk_size; i++){
            app[i]=app[i-1]+in[i];
	}
	return app;
}

float minn(float in[]){
	float min = INT_MAX;
	int i;
	for (i = 0; i < chunk_size; i++){
		if (in[i] < min){
		  min = in[i];
		}
	}
	return min;
}

float maxx(float in[]){
	float max = INT_MIN;
	int i;
	for (i = 0; i < chunk_size; i++){
				if (in[i] > max){
					max = in[i];
					}
	}
	return max;
}

// http://code.metager.de/source/xref/gnu/octave/scripts/signal/hurst.m
float hurst(float in[]){
	float m = mean(in,chunk_size);
	float s = stdev(in);
	float app[chunk_size];
	float *cs;
	int i;
	for (i = 0; i < chunk_size; i++){
				app[i]=in[i]-m;
	}
	cs = cumsum(app);
	float RS = (maxx(cs)-minn(cs))/s;
	float H = log(RS)/log(chunk_size);
	return H;
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
	int n=floor(chunk_size/DFA_win_length);
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

float *hjorth(float data[])
{
  int i;
  static float coeff[2];
  float dxV[chunk_size-1],ddxV[chunk_size-2],d2[chunk_size];
  for (i = 0; i < chunk_size-1; i++){
	  dxV[i]=pow(data[i+1]-data[i],2);
  }
  for (i = 0; i < chunk_size-2; i++){
	  ddxV[i]=pow(dxV[i+1]-dxV[i],2);
  }
  for (i = 0; i < chunk_size; i++){
	  d2[i]=data[i]*data[i];
  }
  float mx2 = mean(d2,chunk_size);
  float mdx2 = mean(dxV,chunk_size-1);
  float mddx2 = mean(ddxV,chunk_size-2);

  float mob = mdx2 / mx2;
  float complexity = sqrt(mddx2 / mdx2 - mob);
  float mobility = sqrt(mob);  
  
  coeff[0] = complexity;
  coeff[1] = mobility;
  
  return coeff;
  
}
