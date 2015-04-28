#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <stdio.h>   /* required for file operations */
#include <time.h>

FILE *fr;            /* declare the file pointer */
#define size 1024
#define DFA_win_length 512

//const int size = 10;

float mean(float in[], int length);
float variance(float in[]);
float std(float in[], int length, int opt);
float *cumsum(float in[]);
float min(float in[], int length);
float max(float in[], int length);
// http://code.metager.de/source/xref/gnu/octave/scripts/signal/hurst.m
// float hurstexponent(float in[]);
float hurst(float in[]);
float *lin_reg(float x[], float y[], int length);
float dfa(float data[]); // order is 1
float *hjorth(float data[]);
float detrendfluctuation(float data[]);


int main() {
 int i=0;
 float chunk[size];
 char line[80];
 float df;
 time_t start, stop;
 /*
 double x_test[15]={88.6,71.6,93.3,84.3,80.6,75.2,69.7,82.0,69.4,83.3,79.6,82.6,80.6,83.5,76.3};
 double y_test[15]={20.0,16.0,19.8,18.4,17.1,15.5,14.7,17.1,15.4,16.3,15.0,17.2,16.0,17.0,14.4};
 */
 /*
float chunk[size];
 for (i = 0; i < size; i++){
    chunk[i] = i;    // sensor on analog pin 0
  }
 */
 
fr = fopen ("col1.csv", "rt");  /* open the file for reading */
   /* elapsed.dta is the name of the file */
   /* "rt" means open the file for reading text */

while(fgets(line, 80, fr) != NULL)
   {
	 /* get a line, up to 80 chars from fr.  done if NULL */
	 sscanf (line, "%e", &df);
	 chunk[i]=df;
	 i++;
	 /* convert the string to a long int */
	 //printf ("%e\n", elapsed_seconds);
   }

//printf("%d\n",i);   
   
fclose(fr);  /* close the file prior to exiting the routine */
time(&start);
int t;
for(t=0; t<10000; t++)
{

float m = mean(chunk,size);
float v = variance(chunk);
float h = hurst(chunk);

//double *f = lin_reg(x_test,y_test);
//float  d = dfa(chunk);
float *hj = hjorth(chunk);

float detf = detrendfluctuation(chunk);
//float he = hurstexponent(chunk);
//printf ("Hurst Exponent:%e\n",he);

//printf ("mean:%.2e, variance:%.2e, hurst:%.2e, dfa:%.2e, hjorth complexity:%.2e,  hjorth mobility:%.2e\n",m,v,h,detf,hj[0],hj[1]); 
printf ("mean:%e, variance:%e, hurst:%e, dfa:%e, hjorth complexity:%e,  hjorth mobility:%e\n",m,v,h,detf,hj[0],hj[1]); 
/*
float test[5]={1,2,3,4,5};
printf ("mean:%e,std:%e\n",mean(test,5),std(test,5,1));
*/
}

time(&stop);
printf("Finished 10000 iterations in about %.0f seconds. \n", difftime(stop, start));
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
	float s = std(in,size,0);
	float app[size];
	float *cs;
	int i;
	for (i = 0; i < size; i++){
				app[i]=in[i]-m;
	}
	cs = cumsum(app);
	float RS = (max(cs,size)-min(cs,size))/s;
	float H = log(RS)/log(size);
	return H;
}	

float min(float in[], int length){
	float min = INT_MAX;
	int i;
	for (i = 0; i < length; i++){
				if (in[i] < min){
					min = in[i];
					}
	}
	return min;
	}
	
float max(float in[], int length){
	float max = INT_MIN;
	int i;
	for (i = 0; i < length; i++){
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

float std(float in[], int length, int opt)
{
  float sum = 0;
  float mu = mean(in,length);
  int i;
  for (i = 0; i < length; i++){  
    sum = sum + pow((in[i]-mu),2);    // sensor on analog pin 0
  }
  if (opt==0){
	return sqrt(sum/(length-1));
	}
  else {
	return sqrt(sum/length);
  }
}

float *hjorth(float data[])
{
  int i;
  static float coeff[2];
  float dxV[size],ddxV[size],d2[size];
  dxV[0]=data[0];
  for (i = 0; i < size-1; i++){
	  dxV[i+1]=data[i+1]-data[i];
  }
  ddxV[0]=dxV[0];
  for (i = 0; i < size-1; i++){
	  ddxV[i+1]=dxV[i+1]-dxV[i];
  }
  for (i = 0; i < size; i++){
	  d2[i]=pow(data[i],2);
	  dxV[i]=pow(dxV[i],2);
	  ddxV[i]=pow(ddxV[i],2);
  }
  float mx2 = mean(d2,size);
  float mdx2 = mean(dxV,size);
  float mddx2 = mean(ddxV,size);
  
  //printf("%e ,%e ,%e \n",mx2,mdx2,mddx2);

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

float hurstexponent(float data[])
{
	int ind;
	float m = mean(data,size);
	float yapp[size];
	float d[size];
	float len[size];
	
	for (ind = 0; ind < size; ind++){
		yapp[ind]=data[ind]-m;
	} 
	float *y2=cumsum(yapp);
	d[1]=(max(y2,size)-min(y2,size))/std(data,size,1);
	//printf("d(1)-->%e\n",d[1]);

	len[1]=size;
	int i=1;
	int rnr=size,rnc;
	while (rnr>=16)
	{
		i=i+1;
		int j;
		rnr=floor(size/(pow(2,(i-1)))); //row
		rnc=floor(size/rnr); //column
		float y[rnr*rnc];
		for(j=0; j<rnr*rnc; j++) y[j]=data[j]; //y=reshape(xV(1:rnr*rnc),rnr,rnc);
		float n2=rnr; // n2=size(y,1);
		
		float s[rnc];
	
		
		for (j=0;j<rnc;j++) // the columns
			{	
				
			float appo[rnr];
			int k=0,r;
			for (r = j*rnr; r < (j+1)*rnr; r++) // the rows
				{ 	
					appo[k]=y[r];
					k=k+1;
				}
			s[j]=std(appo,rnr,1);
			}
		int index[rnc];
		int is_empty = 1;
		for (j=0;j<rnc;j++)  
		{
			if (s[j]!=0) {
				index[j]=1;
				is_empty=0;
			}
			else index[j]=0;
		}
		if (!is_empty)
		{
			float yappo[rnr*rnc];
			float y2[rnc];
			int yappo_size=0, y2_index=0,j,r,summa=0;
			for (j=0;j<rnc;j++) // the columns
			{	
				if(index[j])
				{
					for (r = j*rnr; r < (j+1)*rnr; r++) // the rows
					{ 	
						yappo[yappo_size]=y[r];
						yappo_size=yappo_size+1;
					}
				}
			}
			len[i]=n2;
			float myappo = mean(yappo,yappo_size);
			float syappo = std(yappo,yappo_size,1);
			
			printf("d(i)-->%e,%e\n",myappo,syappo);
			
			int y2_size=0;
			for (j=0;j<rnc;j++) // the columns
			{	
				if(index[j])
				{
					for (r = j*rnr; r < (j+1)*rnr; r++) // the rows
					{ 	
						summa=summa+(y[r]-myappo);
					}
					y2[y2_size] = summa;
					y2_size=y2_size+1;
				}
			}
			d[i]=(max(y2,y2_size)-min(y2,y2_size))/syappo;
			
			
			
		}
		else
		{
		}
		
	
	}
	
	float dlog[i],blog[i];
	int j;
	for (j=i;j>=1;j--) dlog[i-j]=log2(d[j]); //d=[1..i]
	//for (j=0;j<i;j++) printf("d(1)-->%.4e\n",dlog[j]);
	for (j=i;j>=1;j--) blog[i-j]=log2(len[j]); 
	//for (j=0;j<i;j++) printf("d(1)-->%.4e\n",blog[j]);
	
	float *p1=lin_reg(blog,dlog,i); 
	//printf("-->%.4e,%.4e\n",p1[0],p1[1]);

	return p1[1];
	

}

float detrendfluctuation(float data[]) // order is 1
{
	
	int ind;
	float m = mean(data,size);
	float yapp[size];
	float *y;
	float x[size];
	float er[size];
	float y2[size];
	float d[size];
	float len[size];
	for (ind = 0; ind < size; ind++){
		yapp[ind]=data[ind]-m;
		x[ind]=ind+1;
	} 
	y=cumsum(yapp); // y=cumsum(xV-mean(xV));
	float *f =lin_reg(x,y,size); // p1=polyfit([1:n]',y,pord);
	//printf("-->%e,%e\n",f[0],f[1]);
	for (ind = 0; ind < size; ind++){ 
			 er[ind]=y[ind]-(f[1]*(ind+1)+f[0]);//y-polyval(p1,[1:n]');
	}
	float d1=0;
	for (ind = 0; ind < size; ind++){ 
			d1=d1+er[ind]*er[ind]; //er=y-polyval(p1,[1:n]');
			y2[ind]=y[ind];  // y2=y;
	}
	d[1]=sqrt(d1/size);
	len[1]=size;
	//printf("d(1)-->%e\n",d1);
	
	int i=1;
	int rnr=size,rnc;
	/*
	for (ind = 0; ind < 10; ind++){ 
			printf("-->%e\n",y2[ind]);  // y2=reshape(y(1:rnr*rnc),rnr,rnc);
		}
		*/
	
	while (rnr>=16)
	{
		i=i+1;
		rnr=floor(size/(pow(2,(i-1)))); //row
		rnc=floor(size/rnr); //column
		
		//printf("rnr:%d,rnc:%d\n",rnr,rnc);
		
		
		int j,r;
		float xxx[rnr],yyy[rnr],pro[rnr*rnc];
		
		
		for(j=0; j<rnr; j++) xxx[j]=j+1; //[1:rnr]
		for(j=0; j<rnr*rnc; j++) pro[j]=1;
		for(j=0; j<rnr*rnc; j++) y2[j]=y[j];
		
		for (j=0;j<rnc;j++) // the columns
			{
				int ki=0;
				for (r = j*rnr; r < (j+1)*rnr; r++) // the rows
				{ 	
					yyy[ki]=y2[r];
					ki=ki+1; //y2(:,j)
				}
				float *p1=lin_reg(xxx,yyy,rnr); //p1=polyfit([1:rnr]',y2(:,j),pord);
				//printf("-->%.4e,%.4e\n",p1[0],p1[1]);
				ind=1;
				for (r = j*rnr; r < (j+1)*rnr; r++) // the rows
				{ 	
					pro[r]=p1[1]*(ind)+p1[0]; //pro(rnr*(j-1)+1:rnr*j)=polyval(p1,[1:rnr]');
					ind = ind+1;
				}
				
			}
		for (j=0;j<rnc*rnr;j++) er[j]=y[j]-pro[j]; // er=y(1:rnr*rnc)-pro;
		float prod = 0;
		for (j=0;j<rnc*rnr;j++) prod = prod+er[j]*er[j];
		d[i]=sqrt(prod/(rnr*rnc)); //d(i)=sqrt(er'*er/(rnr*rnc));
		len[i]=rnr;
		
		
		
	}
	/*
	int j;
	for (j=1;j<=i;j++) printf("d(1)-->%.4e\n",d[j]);
	*/
	
	//dlog=log2(d(end:-1:1));
	//blog=log2(len(end:-1:1));

	float dlog[i],blog[i];
	int j;
	for (j=i;j>=1;j--) dlog[i-j]=log2(d[j]); //d=[1..i]
	//for (j=0;j<i;j++) printf("d(1)-->%.4e\n",dlog[j]);
	for (j=i;j>=1;j--) blog[i-j]=log2(len[j]); 
	//for (j=0;j<i;j++) printf("d(1)-->%.4e\n",blog[j]);
	
	float *p1=lin_reg(blog,dlog,i); 
	//printf("-->%.4e,%.4e\n",p1[0],p1[1]);

	return p1[1];
	
	
	
	
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


