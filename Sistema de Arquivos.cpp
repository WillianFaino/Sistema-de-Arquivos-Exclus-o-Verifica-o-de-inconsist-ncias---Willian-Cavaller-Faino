#include <bits/stdc++.h>
#include <unistd.h>

using namespace std;

struct fileEntry{
	uint8_t fileAttr;
	char shortFileName[9];
	char ext[4];
	uint16_t creationT;
	uint16_t creationD;
	uint16_t lastAccess;
	long long int firstBlockN;
	uint16_t lastModifyD;
	int fileSizeInBytes;
	
	char lfn[32];
	char* fullname=NULL;
	
};typedef struct fileEntry fe;



fe *entryVec;
int entryN;
long long int tamImg;
int16_t bytespb;
int rBlocos;
long long int dataBlocos;
long long int blocos;
long long int rootDirStart;
long long int rootDirSize;
int firstBlockCurrentDir;
int blockSizeCurrentDir;
char* filename;



fe fileEntryRead(long long int addr){
	FILE* img = fopen(filename,"rb");
	fe entry;
	//cout<<"addr: "<<addr<<endl;	
	

	fseek(img, addr, SEEK_SET);
	fread(&entry.fileAttr, 1, 1, img);
	//cout<<"fileAttr: "<<(int)(entry.fileAttr)<<endl;
	if(entry.fileAttr==3){
		fread(&entry.lfn, 1, 31, img);
		entry.lfn[31] = 0;
	}else{
		fread(&entry.shortFileName, 1, 8, img);
		fread(&entry.ext, 1, 3, img);
		fread(&entry.creationT, 2, 1, img);
		fread(&entry.creationD, 2, 1, img);
		fread(&entry.lastAccess, 2, 1, img);
		fread(&entry.firstBlockN, 8, 1, img);
		fread(&entry.lastModifyD, 2, 1, img);
		fread(&entry.fileSizeInBytes, 4, 1, img);
	}
	
	fclose(img);
	
	
	return entry;
}

void formatar(long long int _dataBlocos, int16_t _bytespb){
	//informações do boot sector
	bytespb = _bytespb;
	dataBlocos = _dataBlocos;
	rBlocos = (ceil((double)dataBlocos/((double)bytespb*8)))+1;
	blocos = dataBlocos+rBlocos;
	tamImg = blocos*bytespb;
	rootDirStart = rBlocos;
	rootDirSize=1;
	
	
	
	
	
	//-------------------------------------------------------------------------------------------------------------------------------------------
	
	//criação da imagem
	FILE* img = fopen(filename,"w"); //abre o arquivo de imagem em modo de escrita, nesse modo, é aberto um arquivo totalmente novo, sobrescrevendo qualquer arquivo anterior.
	
	int8_t temp = 0;
	
	for(int i = 0;i<tamImg;i++){
		fwrite(&temp, 1, 1, img); //escreve o vetor temp (que é 0) em cada posição da imagem, com intuito de zerar todo o sistema de arquivos.
	}

	fclose(img); 
	//-------------------------------------------------------------------------------------------------------------------------------------------
	
	//escrevendo informações do boot sector
	int um=1;
	img = fopen(filename,"r+");
	fwrite(&bytespb,2,1,img); //escreve o boot sector na imagem.
	//cout<<bytespb<<endl;
	fwrite(&rBlocos,4,1,img); //escreve o boot sector na imagem.
	//cout<<rBlocos<<endl;
	fwrite(&dataBlocos,8,1,img); //escreve o boot sector na imagem.
	//cout<<dataBlocos<<endl;
	fwrite(&rootDirStart,8,1,img); //escreve o boot sector na imagem.
	//cout<<rootDirStart<<endl;
	fwrite(&rootDirSize,8,1,img); //escreve o boot sector na imagem.
	//cout<<rootDirSize<<endl;
	fseek(img, bytespb, SEEK_SET);
	fwrite(&um, 1, 1, img);
	fclose(img);
}

void load(){
	if( access( filename, F_OK ) == 0 ) {
	} else {
		formatar(50, 512);
		return;
	}

	FILE* img = fopen(filename,"r");

	fseek(img, 0, SEEK_SET);
	fread(&bytespb,2,1,img); //escreve o boot sector na imagem.
	//cout<<bytespb<<endl;
	fread(&rBlocos,4,1,img); //escreve o boot sector na imagem.
	//cout<<rBlocos<<endl;
	fread(&dataBlocos,8,1,img); //escreve o boot sector na imagem.
	//cout<<dataBlocos<<endl;
	fread(&rootDirStart,8,1,img); //escreve o boot sector na imagem.
	//cout<<rootDirStart<<endl;
	fread(&rootDirSize,8,1,img); //escreve o boot sector na imagem.
	//cout<<rootDirSize<<endl;
	blocos = dataBlocos+rBlocos;
	tamImg = blocos*bytespb;
	firstBlockCurrentDir = rootDirStart;
	blockSizeCurrentDir = rootDirSize;
	
	fclose(img);
}

void dirRead(){
	int start=firstBlockCurrentDir*bytespb;
	int end=start+blockSizeCurrentDir*bytespb;
	
	entryVec = (fe*)malloc((end-start/32)*sizeof(fe));
	fe *lfnVec = NULL;
	entryN = 0;
	int lfnN = 0;
	int lastValEntrynotLFN=0;
	
	for (int i=start;i<end;i+=32){
		fe temp=fileEntryRead(i);
		if(temp.fileAttr==1 ||temp.fileAttr==2){
			lastValEntrynotLFN=entryN;
			if(entryN!=0){
				int count=0;
				if(lfnN!=0){
					int lfnSize = (lfnN*31)+1;
					entryVec[entryN-1].fullname = (char*)malloc(lfnSize+4);
					
					for (int j=0;j<lfnN;j++){
						for (int k=0;k<31;k++){
							if(lfnVec[j].lfn[k]!=0){
								entryVec[entryN-1].fullname[count]=lfnVec[j].lfn[k];
								count++;
							}
						}
					}
					
				}else{
					entryVec[entryN-1].fullname = (char*)malloc(13);
					for (int j=0;j<8;j++){
						if(entryVec[entryN-1].shortFileName[j]!=0){
							entryVec[entryN-1].fullname[count]=entryVec[entryN-1].shortFileName[j];
							count++;
						}
					}	
				}
				if(entryVec[entryN-1].fileAttr == 2){
					entryVec[entryN-1].fullname[count]='.';
					count++;
					
					for (int j=0;j<3;j++){
						if(entryVec[entryN-1].ext[j]!=0){
							entryVec[entryN-1].fullname[count]=entryVec[entryN-1].ext[j];
							count++;
						}
					}
				}
				entryVec[entryN-1].fullname[count]=0;
				
				lfnVec = NULL;
				lfnN = 0;
			}
		}else if(temp.fileAttr==3){
			if(lfnVec==NULL){
				lfnVec = (fe*)malloc(end-start);
			}
			
			lfnVec[lfnN] = temp;
			lfnN++;
		}
		
	
		entryVec[entryN]=temp;
		entryN = entryN + 1;
	}

	if(entryN!=0){
		int count=0;
		if(lfnN!=0){
			int lfnSize = (lfnN*31)+1;
			entryVec[lastValEntrynotLFN].fullname = (char*)malloc(lfnSize+4);
			
			for (int j=0;j<lfnN;j++){
				for (int k=0;k<31;k++){
					if(lfnVec[j].lfn[k]!=0){
						entryVec[lastValEntrynotLFN].fullname[count]=lfnVec[j].lfn[k];
						count++;
					}
				}
				
			}
			
		}else{
			entryVec[lastValEntrynotLFN].fullname = (char*)malloc(13);
			for (int j=0;j<8;j++){
				if(entryVec[lastValEntrynotLFN].shortFileName[j]!=0){
					entryVec[lastValEntrynotLFN].fullname[count]=entryVec[lastValEntrynotLFN].shortFileName[j];
					count++;
				}
			}	
		}
		if(entryVec[lastValEntrynotLFN].fileAttr == 2){
			entryVec[lastValEntrynotLFN].fullname[count]='.';
			count++;
			
			for (int j=0;j<3;j++){
				if(entryVec[lastValEntrynotLFN].ext[j]!=0){
					entryVec[lastValEntrynotLFN].fullname[count]=entryVec[lastValEntrynotLFN].ext[j];
					count++;
				}
			}
		}
		entryVec[lastValEntrynotLFN].fullname[count]=0;
		lfnVec = NULL;
		lfnN = 0;
		
	}
}

struct DateTime{
	unsigned int ano;
	unsigned int mes;
	unsigned int dia;
	unsigned int hora;
	unsigned int min;
	unsigned int sec;
};typedef struct DateTime DateTime;

int8_t *ToBin(unsigned int val, int nBits){
	int8_t *bits = (int8_t*)malloc(16);
	for(int i=nBits-1;i>=0;i--){
		bits[i] = val%2;
		val /= 2;
	}
	/*for(int i=0;i<nBits;i++){
		printf("%d ",bits[i]);
	}*/
	return bits;
}

int ToDec(int8_t * bin, int bitStart, int bitEnd){
	int val = 0;
	int mult = 1;
	for(int i=bitEnd-1;i>=bitStart;i--){
		val += bin[i]*mult;
		mult *= 2;
	}
	return val;
}

DateTime ConvertDate(int16_t dataVal){
	int8_t *bits = ToBin(dataVal, 16);
	DateTime data;
	data.ano = 1900 + ToDec(bits, 0, 7);
	data.mes = ToDec(bits, 7, 11);
	data.dia = ToDec(bits, 11, 16);

	return data;
}

DateTime ConvertTime(int16_t timeVal){
	int8_t *bits = ToBin(timeVal, 16);
	DateTime time;
	time.hora = ToDec(bits, 0, 5);
	time.min = ToDec(bits, 5, 11);
	time.sec = ToDec(bits, 11, 16)*2;

	return time;
}

void printCurrentDirFiles(bool fullInfo){
	dirRead();


	for(int i=0;i<entryN;i++){
		if(entryVec[i].fileAttr<1 || entryVec[i].fileAttr>3){
			continue;
		}
		printf("%s\n", entryVec[i].fullname);
		if(fullInfo){
			if(entryVec[i].fileAttr == 1)
				printf("Tipo: Diretorio\n");
			else if(entryVec[i].fileAttr == 2)
				printf("Tipo: Arquivo\n");
			
			

			DateTime dataCriacao = ConvertDate(entryVec[i].creationD);
			printf("Data de criacao: %d/%d/%d\n", dataCriacao.dia, dataCriacao.mes, dataCriacao.ano);
			DateTime horaCriacao = ConvertTime(entryVec[i].creationT);
			printf("Hora de criacao: %d:%d:%d\n", horaCriacao.hora, horaCriacao.min, horaCriacao.sec);
			DateTime ultimoAcesso = ConvertDate(entryVec[i].lastAccess);
			printf("Ultimo acesso: %d/%d/%d\n", ultimoAcesso.dia, ultimoAcesso.mes, ultimoAcesso.ano);
			DateTime ultimaMod = ConvertDate(entryVec[i].lastModifyD);
			printf("Ultima modificacao: %d/%d/%d\n", ultimaMod.dia, ultimaMod.mes, ultimaMod.ano);

			printf("Tamanho do arquivo: %d\n", entryVec[i].fileSizeInBytes);
			
		}
		printf("\n");
	}
}

int freeBlock(){
	int addr=bytespb;
	int bloco=0;

	FILE* img = fopen(filename,"r");
	fseek(img, addr, SEEK_SET);

	for (int i=0;i<dataBlocos/8;i++){
		int8_t byte;
		fread(&byte, 1, 1, img);
		int8_t *bits = ToBin(byte, 8);
		
		for (int j=7;j>=0;j--){
			if(bits[j]==0){
				fclose(img);
				return bloco+rBlocos;
			}else{
				bloco++;
			}
		}
		
	}
	
	fclose(img);

	return -1;
}

void fillBlock(int blockn){
	blockn-=rBlocos;
	int addr=bytespb;
	int bloco=0;
	int nbits=blockn%8;
	int nbytes=blockn/8;


	FILE* img = fopen(filename,"r+");
	fseek(img, addr+nbytes, SEEK_SET);
	int byte;
	fread(&byte, 1, 1, img);
	fseek(img, addr+nbytes, SEEK_SET);
	int8_t *bits = ToBin(byte, 8);
	bits[7-nbits] = 1;
	byte = ToDec(bits, 0, 8);
	fwrite(&byte, 1, 1, img);
	fclose(img);
}

void clearBlock(int blockn){
	blockn-=rBlocos;
	int addr=bytespb;
	int bloco=0;
	int nbits=blockn%8;
	int nbytes=blockn/8;


	FILE* img = fopen(filename,"r+");
	fseek(img, addr+nbytes, SEEK_SET);
	int byte;
	fread(&byte, 1, 1, img);
	fseek(img, addr+nbytes, SEEK_SET);
	int8_t *bits = ToBin(byte, 8);
	bits[7-nbits] = 0;
	byte = ToDec(bits, 0, 8);
	fwrite(&byte, 1, 1, img);
	fclose(img);
}

int16_t TimeConvert(tm* time){
	int hora = time->tm_hour;
	int min = time->tm_min;
	int sec = time->tm_sec/2;

	int8_t *bits = (int8_t*)malloc(16);
	int8_t *bitsH = ToBin(hora,5);
	int8_t *bitsM = ToBin(min,6);
	int8_t *bitsS = ToBin(sec,5);

	int c=0;
	for(int i=0;i<5;i++){
		bits[c]=bitsH[i];
		c++;
	}
	for(int i=0;i<6;i++){
		bits[c]=bitsM[i];
		c++;
	}
	for(int i=0;i<5;i++){
		bits[c]=bitsS[i];
		c++;
	}

	for(int i=0;i<16;i++){
		//printf("%d ",bits[i]);
	}
	return ToDec(bits,0,16);
}

int16_t DataConvert(tm* data){
	int ano = data->tm_year;
	int mes = data->tm_mon+1;
	int dia = data->tm_mday;

	int8_t *bits = (int8_t*)malloc(16);
	int8_t *bitsY = ToBin(ano,7);
	int8_t *bitsM = ToBin(mes,4);
	int8_t *bitsD = ToBin(dia,5);

	int c=0;
	for(int i=0;i<7;i++){
		bits[c]=bitsY[i];
		c++;
	}
	for(int i=0;i<4;i++){
		bits[c]=bitsM[i];
		c++;
	}
	for(int i=0;i<5;i++){
		bits[c]=bitsD[i];
		c++;
	}

	for(int i=0;i<16;i++){
		//printf("%d ",bits[i]);
	}
	return ToDec(bits,0,16);
}

fe createDirEntry(char* name, char* ext){
	fe entry;
	entry.fileAttr = 1;
	for (int i=0;i<9;i++){
		if(i<=strlen(name)-1){
			entry.shortFileName[i]=name[i];
		}else{
			entry.shortFileName[i]=0;
		}
		//printf("%c", name[i]);
	}
	//cout<<endl;

	for (int i=0;i<4;i++){
		if(i<=strlen(ext)-1){
			entry.ext[i]=ext[i];
		}else{
			entry.ext[i]=0;
		}
		//printf("%c", entry.ext[i]);
	}
	//cout<<endl;


	time_t t = time(0);
    tm* now = localtime(&t);

	entry.creationT = TimeConvert(now);
	entry.creationD = DataConvert(now);
	entry.lastAccess = DataConvert(now);
	entry.lastModifyD = DataConvert(now);
	entry.firstBlockN = freeBlock();
	entry.fileSizeInBytes = 0;

	return entry;
}

fe createFileEntry(char* name, char* ext, int size, int firstBlock){
	fe entry;
	entry.fileAttr = 2;
	
	for (int i=0;i<9;i++){
		if(i<=strlen(name)-1){
			entry.shortFileName[i]=name[i];
		}else{
			entry.shortFileName[i]=0;
		}
		//printf("%c", name[i]);
	}
	

	for (int i=0;i<4;i++){
		if(i<=strlen(ext)-1){
			entry.ext[i]=ext[i];
		}else{
			entry.ext[i]=0;
		}
		//printf("%c", entry.ext[i]);
	}
	


	time_t t = time(0);
    tm* now = localtime(&t);

	entry.creationT = TimeConvert(now);
	entry.creationD = DataConvert(now);
	entry.lastAccess = DataConvert(now);
	entry.lastModifyD = DataConvert(now);
	entry.firstBlockN = firstBlock;
	entry.fileSizeInBytes = size;

	return entry;
}

void dirCreation(char* name, char* ext){
	int start=firstBlockCurrentDir*bytespb;
	int end=start+blockSizeCurrentDir*bytespb;
	int newEntryAddr;
	


	entryVec = (fe*)malloc(end-start);
	entryN = 0;
	
	
	
	for(int i=start;i<end;i+=32){
		fe temp=fileEntryRead(i);

		if(temp.fileAttr==1 ||temp.fileAttr==2){
			continue;
		}else if(temp.fileAttr==3){
			continue;
		}else{
			newEntryAddr=i;
			break;
		}
		
	}
	
	fe de = createDirEntry(name, ext);
	fillBlock(de.firstBlockN);
	FILE* img = fopen(filename,"r+");
	fseek(img, newEntryAddr, SEEK_SET);
	fwrite(&de.fileAttr, 1, 1, img);
	//printf("file attr %d\n", de.fileAttr);


	for(int i=0;i<8;i++){
		fwrite(&de.shortFileName[i], 1, 1, img);
		//cout<<de.shortFileName[i];
	}
	//cout<<endl;

	for(int i=0;i<3;i++){
		fwrite(&de.ext[i], 1, 1, img);
		//cout<<de.ext[i];
	}
	//cout<<endl;
		
	fwrite(&de.creationT, 2, 1, img);
	// cout<<"creation t:"<<de.creationT<<endl;
	fwrite(&de.creationD, 2, 1, img);
	// cout<<"creation d:"<<de.creationD<<endl;
	fwrite(&de.lastAccess, 2, 1, img);
	// cout<<"last acc:"<<de.lastAccess<<endl;
	fwrite(&de.firstBlockN, 8, 1, img);
	// cout<<"first block n:"<<de.firstBlockN<<endl;
	fwrite(&de.lastModifyD, 2, 1, img);
	// cout<<"last modify d:"<<de.lastModifyD<<endl;
	fwrite(&de.fileSizeInBytes, 4, 1, img);
	// cout<<"file size in:"<<de.fileSizeInBytes<<endl;
	fclose(img);
}

void gotoDir(char* name){
	if(strcmp(name, "root")==0){
		firstBlockCurrentDir=rootDirStart;
		rootDirSize = 1;
		return;
	}
	dirRead();
	int rightEntry=-1; 

	for(int i=0;i<entryN;i++){
		if(strcmp(name, entryVec[i].fullname)==0){
			rightEntry=i;
			break;
		}
	}

	if(rightEntry==-1){
		printf("ERRO!\n >>>Diretorio nao encontrado<<<");
		return;
	}

	firstBlockCurrentDir = entryVec[rightEntry].firstBlockN;
	blockSizeCurrentDir = 1;
}

bool checkBlock(int blockN){
	int addr=bytespb;
	int bloco=0;
	int nbits=blockN%8;
	int nbytes=blockN/8;


	FILE* img = fopen(filename,"r+");
	fseek(img, addr+nbytes, SEEK_SET);
	int byte;
	fread(&byte, 1, 1, img);
	fseek(img, addr+nbytes, SEEK_SET);
	int8_t *bits = ToBin(byte, 8);
	if(bits[7-nbits]==0){
		fclose(img);
		return true;
	}
	
	fclose(img);
	return false;
}

int findFreeBlocks(int blockN){
	for(int i=0;i<dataBlocos;i++){
		if(checkBlock(i)){
			int count=0;
			while(checkBlock(i)){
				count++;
				if(count==blockN){
					if(i+count-1>dataBlocos){
						//printf("ERRO!\n>>>Nao foram encontrados blocos livres suficientes para armazenamento<<<");
						break;
					}
					return i;
				}
			}
			i+=count-1;
		}
	}
	
	printf("ERRO!\n>>>Nao foram encontrados blocos livres suficientes para armazenamento<<<");
	return -1;
}

void bringFileFromHDtoFS(char* nome, char* ext){
	FILE* img;
	int j=0;

	char* name = (char*)malloc(100);
	for(int i=0;i<(strlen(nome)+strlen(ext)+2);i++){
		if(i<strlen(nome)){
			name[i]=nome[i];
		}
		if(i==strlen(nome)){
			name[i]='.';
		}
		if(i>strlen(nome)){
			name[i]=ext[j];
			j++;
		}
		if(i==(strlen(nome)+strlen(ext)+1)){
			name[i]=0;
		}
	}
	
	//printf("entro cabeca de pika\n");
	FILE* copy = fopen(name,"r");
	//printf("name = %s\n", name);
	fseek(copy, 0, SEEK_END);
	int fileSize = ftell(copy);

	int fileBlocksN = ceil((double)fileSize/(double)bytespb);
	//printf("blocks used: %d\n", fileBlocksN);
	int start=firstBlockCurrentDir*bytespb;
	int end=start+blockSizeCurrentDir*bytespb;
	int newEntryAddr;

	entryVec = (fe*)malloc(end-start);
	entryN = 0;
	
	for(int i=start;i<end;i+=32){
		fe temp=fileEntryRead(i);

		if(temp.fileAttr==1 ||temp.fileAttr==2){
			continue;
		}else if(temp.fileAttr==3){
			continue;
		}else{
			newEntryAddr=i;
			break;
		}
		
	}

	fe entry = createFileEntry(nome, ext, fileSize, findFreeBlocks(fileBlocksN)+rBlocos);
	

	img = fopen(filename, "r+");
	fseek(img, newEntryAddr, SEEK_SET);
	fwrite(&entry.fileAttr, 1, 1, img);
	fwrite(&entry.shortFileName, 1, 8, img);
	fwrite(&entry.ext, 1, 3, img);
	fwrite(&entry.creationT, 2, 1, img);
	fwrite(&entry.creationD, 2, 1, img);
	fwrite(&entry.lastAccess, 2, 1, img);
	fwrite(&entry.firstBlockN, 8, 1, img);
	fwrite(&entry.lastModifyD, 2, 1, img);
	fwrite(&entry.fileSizeInBytes, 4, 1, img);
	fclose(img);

	int8_t* newFile = (int8_t*)malloc(fileSize);
	copy = fopen(name, "r+");
	fseek(copy, 0, SEEK_SET);
	fread(newFile, 1, fileSize, copy);
	fclose(copy);

	long int fileAddr=(entry.firstBlockN)*bytespb;
	//printf("entry.firstBlockN: %d\n", entry.firstBlockN);
	img = fopen(filename, "r+");
	fseek(img, fileAddr, SEEK_SET);
	fwrite(newFile,1,fileSize,img);
	fclose(img);

	for (int i=entry.firstBlockN;i<entry.firstBlockN+fileBlocksN;i++){
		fillBlock(i);
	}
}

void bringFileFromFStoHD(char* nome, char* ext){
	dirRead();
	int rightEntry=-1; 
	int j=0;

	char* name = (char*)malloc(100);
	for(int i=0;i<(strlen(nome)+strlen(ext)+2);i++){
		if(i<strlen(nome)){
			name[i]=nome[i];
		}
		if(i==strlen(nome)){
			name[i]='.';
		}
		if(i>strlen(nome)){
			name[i]=ext[j];
			j++;
		}
		if(i==(strlen(nome)+strlen(ext)+1)){
			name[i]=0;
		}
	}

	for(int i=0;i<entryN;i++){
		if(strcmp(name, entryVec[i].fullname)==0){
			rightEntry=i;
			break;
		}
	}

	if(rightEntry==-1){
		printf("ERRO!\n >>>Diretorio nao encontrado<<<");
		return;
	}

	int firstBlockCurFile = entryVec[rightEntry].firstBlockN;
	int blockSizeCurFile = ceil((double)entryVec[rightEntry].fileSizeInBytes/(double)bytespb);

	int8_t* newFile = (int8_t*)malloc(entryVec[rightEntry].fileSizeInBytes);
	FILE* img = fopen(filename, "r");
	fseek(img, firstBlockCurFile*bytespb, SEEK_SET);
	fread(newFile, 1, entryVec[rightEntry].fileSizeInBytes, img);
	fclose(img);

	FILE* file = fopen(entryVec[rightEntry].fullname, "w");
	fseek(file, 0, SEEK_SET);
	fwrite(newFile, 1, entryVec[rightEntry].fileSizeInBytes, file);
	fclose(file);
}

void removeFile(char* nome, char* ext){
	dirRead();
	int start=firstBlockCurrentDir*bytespb;
	int rightEntry=-1; 
	int j=0;

	char* name = (char*)malloc(100);
	for(int i=0;i<(strlen(nome)+strlen(ext)+2);i++){
		if(i<strlen(nome)){
			name[i]=nome[i];
		}
		if(i==strlen(nome)){
			name[i]='.';
		}
		if(i>strlen(nome)){
			name[i]=ext[j];
			j++;
		}
		if(i==(strlen(nome)+strlen(ext)+1)){
			name[i]=0;
		}
	}

	for(int i=0;i<entryN;i++){
		if(strcmp(name, entryVec[i].fullname)==0){
			rightEntry=i;
			break;
		}else if(strcmp(nome,entryVec[i].fullname)==0){
			rightEntry=i;
			break;
		}
	}

	if(rightEntry==-1){
		printf("ERRO!\n >>>Diretorio nao encontrado<<<");
		return;
	}

	int firstBlockCurFile = entryVec[rightEntry].firstBlockN;
	int blockSizeCurFile = ceil((double)entryVec[rightEntry].fileSizeInBytes/(double)bytespb);


	if(entryVec[rightEntry].fileAttr==1){
		int blocoAtual=firstBlockCurrentDir;
		fe *aux = (fe*)malloc(entryN*sizeof(fe));

		for(int i=0;i<entryN;i++){
			aux[i]=entryVec[i];
		}

		for(int i=firstBlockCurFile*bytespb; i < (blockSizeCurFile+firstBlockCurFile)*bytespb;i+=32){
			fe temp=fileEntryRead(i);
			
			if(temp.fileAttr!=0){
				firstBlockCurrentDir=temp.firstBlockN;
				blockSizeCurrentDir=ceil((double)temp.fileSizeInBytes/(double)bytespb);
				cout<<"sfn: "<<temp.shortFileName<<endl;
				dirRead();
				printCurrentDirFiles(true);
				removeFile(temp.shortFileName, temp.ext);
			}
		}

		firstBlockCurrentDir=blocoAtual;
		for(int i=0;i<entryN;i++){
			entryVec[i]=aux[i];
		}
	}


	int8_t* newFile = (int8_t*)malloc(entryVec[rightEntry].fileSizeInBytes);
	FILE* img = fopen(filename, "r+");
	memset(newFile, 0, entryVec[rightEntry].fileSizeInBytes);
	fseek(img, firstBlockCurFile*bytespb, SEEK_SET);

	fwrite(newFile, 1, entryVec[rightEntry].fileSizeInBytes, img);
	fclose(img);

	for (int i=firstBlockCurFile;i<(blockSizeCurFile+firstBlockCurFile);i++){
		clearBlock(i);
	}

	int8_t* only0 = (int8_t*)malloc(32);
	
	memset(only0, 0, 32);
	img = fopen(filename, "r+");
	fseek(img, start+rightEntry*32, SEEK_SET);
	fwrite(only0, 32, 1, img);
	fclose(img);
}



int main(int argc, char **argv){

	int op;
	filename = (char*)malloc(100);

	cout<<"Sistema de Arquivos - Alocacao Contigua e gerenciamento de blocos livre por bitmap"<<endl<<endl;
	
	cout<<"Nome do arquivo a ser acessado: ";
	cin>>filename;

	load();

	while(true){
		cout<<"Menu:"<<endl<<endl;
		cout<<"0) Listar arquivos armazenados no diretorio atual."<<endl;
		cout<<"1) Criar diretorio."<<endl;
		cout<<"2) Copiar arquivo do disco rigido para o sistema de arquivos."<<endl;
		cout<<"3) Copiar arquivo do sistema de arquivos para o disco rigido."<<endl;
		cout<<"4) Remover arquivo."<<endl;
		cout<<"5) Ir para diretorio."<<endl;
		cout<<"6) Formatar sistema de arquivos."<<endl;
		cout<<"7) Finalizar execucao."<<endl;

		cin>>op;

		if(op==0){
			cout<<"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n----------------------------------------------------------------------------------------------------------------------------------";
			cout<<endl;
			printCurrentDirFiles(true);
		}else if(op==1){
			char* n = (char*)malloc(100);
			cout<<"Nome do diretorio a ser criado: ";
			cin>>n;
			dirCreation(n, " ");

		}else if(op==2){
			char* n = (char*)malloc(100);
			char* e = (char*)malloc(10);
			
			cout<<"Nome do arquivo a ser movido: ";
			cin>>n;
			cout<<"\nExtensao do arquivo: ";
			cin>>e;
			bringFileFromHDtoFS(n, e);
		
		}else if(op==3){
			printCurrentDirFiles(true);
			char* n = (char*)malloc(100);
			char* e = (char*)malloc(10);
			
			cout<<"Nome do arquivo a ser movido: ";
			cin>>n;
			cout<<"\nExtensao do arquivo: ";
			cin>>e;
			bringFileFromFStoHD(n, e);

		}else if(op==4){
			printCurrentDirFiles(true);
			char* n = (char*)malloc(100);
			char* e = (char*)malloc(10);
			
			cout<<"Nome do arquivo a ser removido: ";
			cin>>n;
			cout<<"\nExtensao do arquivo: ";
			cin>>e;
			removeFile(n,e);

		}else if(op==5){
			char* n = (char*)malloc(100);
			cout<<"Nome do diretorio a ser acessado: ";
			cin>>n;
			gotoDir(n);

		}else if(op==6){
			long int db;
			long int bpb;
			
			cout<<"Quantidade de blocos de dados: ";
			cin>>db;
			cout<<"Quantidade de bytes por bloco: ";
			cin>>bpb;

			formatar(db, bpb);
			load();

		}else if(op==7){
			break;

		}else{
		cout<<"Operacao invalida\n\n\n";
	}
	}


	return 0;
}
