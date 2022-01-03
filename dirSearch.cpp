#include<Windows.h>
#include"fileSearch.h"
#include<string>
#include<vector>
#include<map>
#include <comdef.h>
#include<iostream>
#include <algorithm>
using namespace std;
vector<string> duplicateFolderPaths;
map<string,map<string,map<string,string>>> constPathContents;
void printFolderContents(map<string,map<string,string>> vect)
{
    map<string,map<string,string>>::iterator itr;
    for(itr=vect.begin();itr!=vect.end();++itr)
    {
        cout<<itr->first<<endl;
        map<string,string>::iterator innerItr;
        for(innerItr=itr->second.begin();innerItr!=itr->second.end();++innerItr)
        {
            cout<<innerItr->first<<" : "<<innerItr->second<<endl;
        }
    }
    cout<<endl;
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

void buildConstPathDetails(char* searchPath)
{
  map<string,map<string,string>> fileContents;
    char* subPath= "*";
    char * fileName = new char[strlen(searchPath) + strlen(subPath) + 1];
    strcpy(fileName,searchPath);
    strcat(fileName,subPath);
    WIN32_FIND_DATA findFileData;
    HANDLE hFindFile;
    hFindFile = FindFirstFile(fileName,&findFileData);
    /*if(INVALID_HANDLE_VALUE == hFindFile)
    { return fileContents;
    }*/
    int file=0,folder=0;
    while(FindNextFile(hFindFile,&findFileData))
    {
        if(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
        {
            char* subDir = findFileData.cFileName;
            if(!(strcmp(subDir,"..")&&strcmp(subDir,".")))
            {
                continue;
            }
            folder++;
            char* constSubDir =  new char[strlen(searchPath) + strlen(subDir) + 2];
            strcpy(constSubDir,searchPath);
            strcat(constSubDir,subDir);
            strcat(constSubDir,"/");
            buildConstPathDetails(constSubDir);
            string folderInd = "folder";
            folderInd+=to_string(folder);
            map<string,string> fileDetails;
            fileDetails.insert({"path",constSubDir});
            fileContents.insert({folderInd,fileDetails});
        }
        else
        {
        /*wcout<<"File name: "<<searchPath<<findFileData.cFileName<<" and file type: "<<findFileData.dwFileAttributes<<endl;*/
        file++;
            char* constFileName = findFileData.cFileName;
            char* currentPath =  new char[strlen(searchPath) + strlen(constFileName) + 1];
            strcpy(currentPath,searchPath);
            strcat(currentPath,constFileName);
            FILE *f;
            char md5Sum[33];
            f=fopen(currentPath,"rb");
            CalcHash(f,md5Sum);
            fclose(f);
            string hashValue = md5Sum;
            string fileInd="file";
            fileInd+=to_string(file);
            map<string,string> fileDetails;
            string fName = constFileName;
            string fSize = to_string(findFileData.nFileSizeLow);
            fileDetails.insert({"ext",fName.substr(fName.find_last_of(".") + 1)});
            fileDetails.insert({"size",fSize});
            fileDetails.insert({"content",hashValue});
            fileContents.insert({fileInd,fileDetails});
        }
    }
    FindClose(hFindFile);
    map<string,string> counts;
    counts.insert({"file",to_string(file)});
    counts.insert({"folder",to_string(folder)});
    fileContents.insert({"counts",counts});
    constPathContents.insert({searchPath,fileContents}); 
    return; 
}
map<string,map<string,string>> directoryDetails(char* searchPath,int maxFolderCount,int maxFileCount)
{
    map<string,map<string,string>> fileContents;
    char* subPath= "*";
    char * fileName = new char[strlen(searchPath) + strlen(subPath) + 1];
    strcpy(fileName,searchPath);
    strcat(fileName,subPath);
    WIN32_FIND_DATA findFileData;
    HANDLE hFindFile;
    hFindFile = FindFirstFile(fileName,&findFileData);
    /*if(INVALID_HANDLE_VALUE == hFindFile)
    { return fileContents;
    }*/
    int file=0,folder=0;
    while(FindNextFile(hFindFile,&findFileData))
    {
        if(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
        {
            char* subDir = findFileData.cFileName;
            if(!(strcmp(subDir,"..")&&strcmp(subDir,".")))
            {
                continue;
            }
            folder++;
            char* constSubDir =  new char[strlen(searchPath) + strlen(subDir) + 2];
            strcpy(constSubDir,searchPath);
            strcat(constSubDir,subDir);
            strcat(constSubDir,"/");
            string folderInd = "folder";
            folderInd+=to_string(folder);
            map<string,string> fileDetails;
            fileDetails.insert({"path",constSubDir});
            fileContents.insert({folderInd,fileDetails});
        }
        else
        {
        /*wcout<<"File name: "<<searchPath<<findFileData.cFileName<<" and file type: "<<findFileData.dwFileAttributes<<endl;*/
        file++;
            char* constFileName = findFileData.cFileName;
            char* currentPath =  new char[strlen(searchPath) + strlen(constFileName) + 1];
            strcpy(currentPath,searchPath);
            strcat(currentPath,constFileName);
            /*FILE *f;
            char md5Sum[33];
            f=fopen(currentPath,"rb");
            CalcHash(f,md5Sum);
            fclose(f);
            string hashValue = md5Sum;*/
            string pathName = currentPath;
            string fileInd="file";
            fileInd+=to_string(file);
            map<string,string> fileDetails;
            string fName = constFileName;
            string fSize = to_string(findFileData.nFileSizeLow);
            fileDetails.insert({"ext",fName.substr(fName.find_last_of(".") + 1)});
            fileDetails.insert({"size",fSize});
            fileDetails.insert({"content",pathName});
            fileContents.insert({fileInd,fileDetails});
        }
        if(folder>maxFolderCount || file>maxFileCount)
        {
            break;
        }
    }
    FindClose(hFindFile);
    map<string,string> counts;
    counts.insert({"file",to_string(file)});
    counts.insert({"folder",to_string(folder)});
    fileContents.insert({"counts",counts});
    return fileContents;
}
bool compareDirectories(char* path1,char* path2)
{
    map<string,map<string,map<string,string>>>::iterator itrConstPath = constPathContents.find(path1);
    map<string,map<string,string>> dir1Details = itrConstPath->second;
    map<string,map<string,string>>::iterator it1=dir1Details.find("counts");
    map<string,string>::iterator dir1Folder = it1->second.find("folder");
    map<string,string>::iterator dir1File = it1->second.find("file");
    int dir1FolderCount = stoi(dir1Folder->second);
    int dir1FileCount = stoi(dir1File->second);
    map<string,map<string,string>> dir2Details = directoryDetails(path2,dir1FolderCount,dir1FileCount);
    map<string,map<string,string>>::iterator it2=dir2Details.find("counts");
    map<string,string>::iterator dir2Folder = it2->second.find("folder");
    map<string,string>::iterator dir2File = it2->second.find("file");
    int dir2FolderCount = stoi(dir2Folder->second);
    int dir2FileCount = stoi(dir2File->second);     
    map<string,bool> folderContentsFlag;
    if(dir1FileCount!=dir2FileCount)
    {
        return false;
    }
    if(dir2FolderCount!=dir1FolderCount)
    {
        return false;
    }
    for(pair<string,map<string,string>> itr: dir1Details)
    {
        folderContentsFlag.insert({itr.first,false});
    }
    for(pair<string,map<string,string>> itr: dir2Details)
    {
        map<string,string> dir2Map =itr.second;
        map<string,string>::iterator content2Ptr = dir2Map.find("content");
        if(content2Ptr!=dir2Map.end())
        {
        char md5Sum[33];
        FILE *f;
        f=fopen(content2Ptr->second.c_str(),"rb");
        CalcHash(f,md5Sum);
        fclose(f);
        string hashValue = md5Sum;
        content2Ptr->second=hashValue;
        }
        else
        {
            continue;
        }
        string key = itr.first;
        bool matchFound =false;
        if(key.find("file")!=-1)
        {
            for(pair<string,bool> flagItr:folderContentsFlag)
            {
                if(!flagItr.second && flagItr.first.find("file")!=-1)
                {
                map<string,map<string,string>>::iterator fileItr =dir1Details.find(flagItr.first);
                map<string,string> dir1Map = fileItr->second;
                if(dir2Map==dir1Map)
                {
                    matchFound=true;
                    flagItr.second=true;
                    break;
                }
                }
            }
            if(!matchFound)
            {
                return false;
            }
        }
    }
    for(pair<string,map<string,string>> itr: dir2Details)
    {
        map<string,string> dir2Map =itr.second;
        map<string,string>::iterator dir2PathKey = dir2Map.find("path");
        char *dir2Path =const_cast<char*>(dir2PathKey->second.c_str());
        string key = itr.first;
        bool matchFound =false;
        if(key.find("folder")!=-1)
        {
            for(pair<string,bool> flagItr:folderContentsFlag)
            {
                if(!flagItr.second && flagItr.first.find("folder")!=-1)
                {
                map<string,map<string,string>>::iterator fileItr =dir1Details.find(flagItr.first);
                map<string,string> dir1Map = fileItr->second;
                map<string,string>::iterator dir1PathKey = dir1Map.find("path");
                char *dir1Path = new char[dir1PathKey->second.length() + 1];
                strcpy(dir1Path, dir1PathKey->second.c_str());
                if(compareDirectories(dir1Path,dir2Path))
                {
                    matchFound=true;
                    flagItr.second=true;
                    break;
                }
                }
            }
            if(!matchFound)
            {
                return false;
            }
        }
    }
    return true;
}

void findDuplicateFolderPaths(char* constPath,char* varPath)
{
    if(strcmp(constPath,varPath)==0)
    {
        return;
    }
    string currentPath=varPath;
    for(int i=0;i<duplicateFolderPaths.size();++i)
    {
        if(strstr(currentPath.c_str(),duplicateFolderPaths[i].c_str()))
        {
            /*cout<<"subfolder of already matched folder"<<endl;*/
            return;
        }
    }
    if(compareDirectories(constPath,varPath))
    {
        duplicateFolderPaths.push_back(varPath);
    }
    char* subPath= "*";
    char * fileName = new char[strlen(varPath) + strlen(subPath) + 1];
    strcpy(fileName,varPath);
    strcat(fileName,subPath);
    WIN32_FIND_DATA findFileData;
    HANDLE hFindFile;
    hFindFile = FindFirstFile(fileName,&findFileData);
    if(INVALID_HANDLE_VALUE == hFindFile)
    {
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
            char* constSubDir =  new char[strlen(varPath) + strlen(subDir) + 2];
            strcpy(constSubDir,varPath);
            strcat(constSubDir,subDir);
            strcat(constSubDir,"/");
            findDuplicateFolderPaths(constPath,constSubDir);
        }
    }
  /*printFolderContents(currenFolderContent);*/
  FindClose(hFindFile);
  return;
}
int main()
{
    buildConstPathDetails("D:/JNIExample/");
    for(auto itr : constPathContents)
    {
        printFolderContents(itr.second);
    }
    /*findDuplicateFolderPaths("D:/JNIExample/","D:/");
    findDuplicateFolderPaths("D:/JNIExample/","C:/"); */
    for(string s : duplicateFolderPaths)
    {
        cout<<s<<endl;
    }
    return 0;
}

JNIEXPORT jobject JNICALL Java_fileSearch_getDuplicateDir
  (JNIEnv *env, jclass cls, jstring path){
     char* pathToFind = (char *)env->GetStringUTFChars(path, NULL);
    buildConstPathDetails(pathToFind);
    /*for(auto itr : constPathContents)
    {
    printFolderContents(itr.second);
    }*/
    findDuplicateFolderPaths(pathToFind,"D:/");
    findDuplicateFolderPaths(pathToFind,"C:/"); 
jclass java_util_ArrayList      = static_cast<jclass>(env->NewGlobalRef(env->FindClass("java/util/ArrayList")));
jmethodID  java_util_ArrayList_     = env->GetMethodID(java_util_ArrayList, "<init>", "(I)V");
  jmethodID java_util_ArrayList_add  = env->GetMethodID(java_util_ArrayList, "add", "(Ljava/lang/Object;)Z");
    jobject result = (*env).NewObject(java_util_ArrayList,java_util_ArrayList_,duplicateFolderPaths.size());
    for (size_t i = 0; i < duplicateFolderPaths.size(); i++) {
     char* str = (char*) static_cast<char*>(&duplicateFolderPaths[i][0]);
     jstring _str = (*env).NewStringUTF(str);
     (*env).CallBooleanMethod(result,java_util_ArrayList_add, _str);
    }
  return result;
  }