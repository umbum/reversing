#include "windows.h"

int main(){
	MessageBox(NULL,
				"Make me think your HD is a CD-Rom.",
				"abex' 1st crackme",
				MB_OK);


	if(((GetDriveType("c:\\") - 1) - 1) == ((((0 + 1) + 1) + 1))){
		MessageBox(NULL,
				"Success",
				"Success",
				MB_OK);
	}else{
		MessageBox(NULL,
				"Failed",
				"Failed",
				MB_OK);
	}
	return 0;
	
}
