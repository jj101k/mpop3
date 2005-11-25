#include <string.h>

char hexchar[]={'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};

int hex_from_binary(char *outstring, char const *instring, unsigned long bytes) {
	int i;
	for(i=0;i<bytes;i++) {
		outstring[i*2]=hexchar[(instring[i]&240)>>4];
		outstring[(i*2)+1]=hexchar[instring[i]&15];
	}
	outstring[2*bytes]=0;
	return 1;
}

char super_chomp_n(char *istring, unsigned long int len) {
	if(!istring || !len) return 0;
	if(istring[len-1]=='\n') {
		istring[len-1]=0;
		if(len>1 && istring[len-2]=='\r') {
			istring[len-2]=0;
		}
		return '\n';
	} else if(istring[len-1]=='\r') {
		istring[len-1]=0;
		return '\r';
	} else {
		return 0;
	}
}
char super_chomp(char *istring) {
	return super_chomp_n(istring, strlen(istring));
}
