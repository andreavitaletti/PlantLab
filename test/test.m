source('HurstExponent.m');
source('DetrendedFluctuation.m');
source('HjorthParameters.m');
load('H6Ch1_Post.mat');
[r,c]=size(H6Ch1_Post);
#for i=[1:c]
for i=[1:1]
	col=H6Ch1_Post(:,i);
	m=mean(col);
	v=var(col);
	i=iqr(col);
	s=skewness(col);
	k=kurtosis(col);
	[HP_mob,HP_comp]=HjorthParameters(col);
	HE=HurstExponent(col);
	dfa=DetrendedFluctuation(col);
	printf("%e,%e,%e,%e,%e,%e,%e,%e,%e\n",m,v,i,s,k,HP_mob,HP_comp,HE,dfa)
endfor
