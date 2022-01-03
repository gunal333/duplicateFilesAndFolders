#include"fileSearch.h"
#include<Windows.h>
#include<string>
#include<vector>
#include <comdef.h>
#include<iostream>
using namespace std;

/*vector<string> get_all_files_names_within_folder(string folder)
{
    vector<string> names;
    string search_path = folder + "/*.*";
    WIN32_FIND_DATA fd; 
    HANDLE hFind =FindFirstFile(search_path.c_str(), &fd); 
    if(hFind != INVALID_HANDLE_VALUE) { 
        do { 
            // read all (real) files in current folder
            // , delete '!' read other 2 default folder . and ..
            if(! (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) {
                _bstr_t b(fd.cFileName);
                char * fileName =b;
                names.push_back(fileName);
            }
        }while(::FindNextFile(hFind, &fd)); 
        ::FindClose(hFind); 
    } 
    return names;
}*/
vector<string> duplicateByNameAndSize;
typedef struct searchFileAttributes
{
 WIN32_FIND_DATA findFileData;
char md5sum[33];
}sFileAttr;

sFileAttr *fileAttr = NULL;

bool extensionChecker(char* fileName1,char* fileName2)
{
  string fName1 = fileName1;
  string fName2 = fileName2;
  if(fName1.substr(fName1.find_last_of(".") + 1) == fName2.substr(fName2.find_last_of(".") + 1)) {
    return true;
  } else {
    return false;
  }
}
int CalcHash(FILE *f,char *md5sum)
{
  HCRYPTPROV hProv;
  HCRYPTHASH hHash;
  unsigned char buf[1024];
  unsigned char hsh[16];
  unsigned long sz;
  char byt[3];
  int rc,err,i;
  size_t fsz;
  rc=CryptAcquireContext(&hProv,NULL,MS_STRONG_PROV,PROV_RSA_FULL,0);
  if(!rc){
    err=GetLastError();
    if(err==0x80090016){
      //first time using crypto API, need to create a new keyset
      rc=CryptAcquireContext(&hProv,NULL,MS_STRONG_PROV,PROV_RSA_FULL,CRYPT_NEWKEYSET);
      if(!rc){
        err=GetLastError();
        return 0;
      }
    }
  }
  CryptCreateHash(hProv,CALG_MD5,0,0,&hHash);
  while((fsz=fread(buf,1,1024,f))!=0){
    CryptHashData(hHash,(unsigned char *)buf,fsz,0);
  }
  sz=16;
  CryptGetHashParam(hHash,HP_HASHVAL,hsh,&sz,0);
  md5sum[0]=0;
  for(i=0;i<sz;i++){
    sprintf(byt,"%.2X",hsh[i]);
    strcat(md5sum,byt);
  }
  CryptDestroyHash(hHash);
  CryptReleaseContext(hProv,0);
  return 1;
}

void findDuplicateContentFiles()
{
    for (size_t i = 0; i < duplicateByNameAndSize.size(); i++) {
    FILE *f;
    char md5Sum[33];
    char* path = &duplicateByNameAndSize[i][0]; 
    f=fopen(path,"rb");
    CalcHash(f,md5Sum);
    if(strcmp(fileAttr->md5sum,md5Sum)!=0)
    {
        duplicateByNameAndSize.erase(duplicateByNameAndSize.begin()+i);
    }
    fclose(f);
    }
}

void findFileDetails(const char* fileToSearch)
{
     if (fileAttr != NULL)
        return;
    fileAttr = new sFileAttr;
 HANDLE hFindFile;
    hFindFile = FindFirstFile(fileToSearch,&(fileAttr->findFileData));
    if(INVALID_HANDLE_VALUE == hFindFile)
    {
        cout<<"Error in finding file"<<endl;
        return;
    }
    FILE *f;
    f=fopen(fileToSearch,"rb");
    CalcHash(f,fileAttr->md5sum);
    fclose(f);
    return;
}
void directorySearch(char* constPath,char* searchPath)
{
    
    char* subPath= "*";
    char * fileName = new char[strlen(searchPath) + strlen(subPath) + 1];
    strcpy(fileName,searchPath);
    strcat(fileName,subPath);
    WIN32_FIND_DATA findFileData;
    HANDLE hFindFile;
    hFindFile = FindFirstFile(fileName,&findFileData);
    if(INVALID_HANDLE_VALUE == hFindFile)
    {
        /*cout<<"Error in finding file"<<endl;*/
        return;
    }
    while(FindNextFile(hFindFile,&findFileData))
    {
        if(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
        {
            char* subDir = findFileData.cFileName;
            if(!(strcmp(subDir,"..")&&strcmp(subDir,".")))
            {
                continue;
            }
            char* constSubDir =  new char[strlen(searchPath) + strlen(subDir) + 2];
            strcpy(constSubDir,searchPath);
            strcat(constSubDir,subDir);
            strcat(constSubDir,"/");
            directorySearch(constPath,constSubDir);
        }
        else
        {
            
        /*wcout<<"File name: "<<searchPath<<findFileData.cFileName<<" and file type: "<<findFileData.dwFileAttributes<<endl;*/
        if(extensionChecker(findFileData.cFileName,fileAttr->findFileData.cFileName) && findFileData.nFileSizeLow==fileAttr->findFileData.nFileSizeLow)
        {
            char* constFileName = findFileData.cFileName;
            char* currentPath =  new char[strlen(searchPath) + strlen(constFileName) + 1];
            strcpy(currentPath,searchPath);
            strcat(currentPath,constFileName);
            if(strcmp(constPath,currentPath)!=0)
              {
            duplicateByNameAndSize.push_back(currentPath);
              }
        }
        }
    }
    FindClose(hFindFile);
    return;
}
JNIEXPORT jobject JNICALL Java_fileSearch_getDuplicateFiles
  (JNIEnv *env, jclass cls, jstring path){
    const char* pathToFind = env->GetStringUTFChars(path, NULL);
    char* pToFind=(char*)pathToFind;
    findFileDetails(pathToFind);
    directorySearch(pToFind,"D:/");
    directorySearch(pToFind,"C:/");
    findDuplicateContentFiles(); 
jclass java_util_ArrayList      = static_cast<jclass>(env->NewGlobalRef(env->FindClass("java/util/ArrayList")));
jmethodID  java_util_ArrayList_     = env->GetMethodID(java_util_ArrayList, "<init>", "(I)V");
jmethodID  java_util_ArrayList_size = env->GetMethodID (java_util_ArrayList, "size", "()I");
 jmethodID java_util_ArrayList_get  = env->GetMethodID(java_util_ArrayList, "get", "(I)Ljava/lang/Object;");
  jmethodID java_util_ArrayList_add  = env->GetMethodID(java_util_ArrayList, "add", "(Ljava/lang/Object;)Z");
    jobject result = (*env).NewObject(java_util_ArrayList,java_util_ArrayList_,duplicateByNameAndSize.size());
    for (size_t i = 0; i < duplicateByNameAndSize.size(); i++) {
     char* str = (char*) static_cast<char*>(&duplicateByNameAndSize[i][0]);
     jstring _str = (*env).NewStringUTF(str);
     (*env).CallBooleanMethod(result,java_util_ArrayList_add, _str);
    }
  return result;
  }
int main()
{
    findFileDetails("D:/systeminfo.txt");
    directorySearch("D:/systeminfo.txt","D:/");
    directorySearch("D:/systeminfo.txt","C:/");
    findDuplicateContentFiles();
    for (size_t i = 0; i < duplicateByNameAndSize.size(); i++) {
     cout<<duplicateByNameAndSize[i]<<endl;
    }
    return 0;
}