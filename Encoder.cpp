#include <iostream>
#include <fstream>
#include <cstring> // for std::strlen
#include <cstddef> // for std::size_t -> is a typedef on an unsinged int
#include <iomanip>
#include <sstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> //for uint16_t
#include <unistd.h> //for getopt()

#define MAX_KEY_FILE_SIZE 65533

#ifdef DEBUG
#define DEBUG_STDERR(x) (std::cerr << x)
#define DEBUG_STDOUT(x) (std::cout << x)
#else 
#define DEBUG_STDERR(x)
#define DEBUG_STDOUT(x)
#endif

int keyChecker(unsigned char* keyFile);

using namespace std ;

int main( int argc, char **argv )
{

	string keyFileName = "";
	string outputFileName = "output";
	string inputFileName = "";
	int c;
	bool decryptFlag = false;
	bool encryptFlag = false;

	opterr = 0;

	while ((c = getopt (argc, argv, "dek:o:i:")) != -1){
		switch (c)
		{
		case 'd':
			decryptFlag = true;
			break;

		case 'e':
			encryptFlag = true;
			break;
			
		case 'k':
			keyFileName = optarg;
			break;

		case 'o':
			outputFileName = optarg;
			break;

		case 'i':
			inputFileName = optarg;
			break;

		case '?':
			if (optopt == 'k' || optopt == 'o' || optopt == 'i'){
				cerr << "Option " << optopt << "requires an argument. " << endl;
			}
			else if (isprint (optopt)){
				cerr << "Unknown option " << static_cast<char>(optopt) << endl;
			}

			else{
				cerr << "Unknown option characer " << static_cast<char>(optopt) << "." << endl;
			}
			return 1;

			break;

		default:
			abort();
		}
	}

	ifstream keyFile;
	ifstream inputFile;
	
	ofstream outFile;
	
	size_t keyFileSize = 0;
	size_t inputFileSize = 0;

	cout << "Opening file input file: " << inputFileName  << endl;
	inputFile.open( inputFileName.c_str(), ios::in|ios::binary);
	if(!inputFile.is_open()){
		cerr << "Error opening: " << inputFileName << endl;
		return 1;
	}
	
	cout << "Opening key file name: " << keyFileName << endl;
	keyFile.open(keyFileName.c_str(), ios::in|ios::binary);
	if(!keyFile.is_open()){
		cerr << "Error opening: " << keyFileName << endl;
		return 1;
	}
	
	cout << "Opening output file name: " << outputFileName << endl;
	outFile.open( outputFileName.c_str(), ios::out|ios::binary);
	if(!outFile.is_open()){
		cerr << "Error opening: " << outputFileName << endl;
		return 1;
	}

	unsigned char* keyFileData = 0;
	unsigned char* inputFileData = 0;
	
	keyFile.seekg(0, ios::end); // set the pointer to the end
	inputFile.seekg(0, ios::end);
	
	keyFileSize = keyFile.tellg() ; // get the length of the file
	inputFileSize = inputFile.tellg();

	cout << "Size of Key file: " << keyFileSize << endl;
	cout << "Size of Input file: " << inputFileSize << endl;
	
	keyFile.seekg(0, ios::beg); // set the pointer to the beginning
	inputFile.seekg(0, ios::beg);
	
	keyFileData = new unsigned char[ keyFileSize+1 ]; //  for the '\0'
	inputFileData = new unsigned char[ inputFileSize+1 ];
	
	keyFile.read( (char*)keyFileData, keyFileSize );
	inputFile.read( (char*)inputFileData, inputFileSize);

	keyFileData[keyFileSize] = '\0' ; // set '\0'
	inputFileData[inputFileSize] = '\0';

	if(keyChecker(keyFileData)){
		cerr << "Key file is not compatible" << endl;
		return 1;
	}

	#ifdef DEBUG
	cout << " Encryptor Data size: " << strlen((char*)keyFileData) << "\n";
	cout << " File Data size: " << strlen((char*)inputFileData) << "\n";
	#endif
	
	//Decrypt Mode
	if( decryptFlag == true && encryptFlag == false ) {
		size_t decryptedBytes = 0;
		uint16_t encryptedData;
		
		uint8_t temp;
		
		for ( size_t i = 0; i < inputFileSize; i+=2) {
			DEBUG_STDOUT( "val of i: " << i << " ," << (unsigned int)inputFileData[i] << " " << (unsigned int)inputFileData[i+1] << " " << endl);

			encryptedData = (unsigned int)(inputFileData[i+1] << 8) + (unsigned int)(inputFileData[i]);  

			DEBUG_STDOUT( "final val: " << encryptedData << endl);
			
			temp = keyFileData[encryptedData];
			outFile.write(reinterpret_cast<const char *>(&temp), sizeof(temp));
			decryptedBytes++;
		}
		
		if ( decryptedBytes != inputFileSize/2) {
			cout << " Not all data was encrypted" << endl;
			cout << "Encrypted Bytes: " << decryptedBytes << endl << "Input file size: " << inputFileSize << endl;
		}
	}

	//Encrypt Mode
	else if ( encryptFlag == true && decryptFlag == false) {
		size_t encryptedBytes = 0;

		for ( size_t i = 0; i < inputFileSize; i++ ) {
			for ( uint16_t j = 0; j < MAX_KEY_FILE_SIZE; j++) { //for ( size_t j = 0; j < keyFileSize; j++)
				if ( inputFileData[i] == keyFileData[j] ) {

					DEBUG_STDOUT( "Data To Encrypt: " << hex << "0x" << (int)inputFileData[i]<< " At location: " << i << endl);
					DEBUG_STDOUT( "Ensure match-up: " << hex << "0x" << inputFileData[i] << " " << hex <<"0x"<< keyFileData[j] << endl);
					DEBUG_STDOUT( "Locations: " << i << " " << j << endl << endl);

					outFile.write(reinterpret_cast<const char *>(&j), sizeof(j));//uint16_t));//sizeof(j));
					encryptedBytes++;
					j = MAX_KEY_FILE_SIZE+1;//j = keyFileSize+1;
				}
			}
		}

		if ( encryptedBytes != inputFileSize) {
			cout << " Not all data was encrypted" << endl;
			cout << "Encrypted Bytes: " << encryptedBytes << endl << "Input file size: " << inputFileSize << endl;
		}
	}

	else {
		cerr << "Please select either Encrypt (-e) or Decrypt (-d)" << endl;
		return 1;
	}

	cout << endl << "Finshed Successfully" << endl;
	
	outFile.close();
	
	return 0;
}

int keyChecker(unsigned char* keyFileData)
{
	int val = 0;
	for(int i = 0; i < MAX_KEY_FILE_SIZE; i++)
	{
		if(val == keyFileData[i]){
			DEBUG_STDOUT( val << " "<< i << endl );
			val++;
			i = 0;
			if(val == 255){
				cout << "Key File is compatible" << endl;
				return 0;
			}
		}
	}

	//Not compatible
	if (val != 255){
		cout << "Value of val: " << val << endl;
		return 1;
	}

	return 1; //shouldn't get here
}
