/*

Name: Manas Singh
CS 3377

Purpose: This is a threads program

*/
#include <string>
#include <fstream>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <vector>
#include <pthread.h>

#include "LineInfo.h"

using namespace std;

unsigned totalLineCount = 0;
pthread_mutex_t lock;

struct SingLinesThreadInfoStruct {
    unsigned* singerNoIdPtr;
    vector<string> lyricLinesVector;
    unsigned noOfLyricLines;
};

void GetLyricsLinesFromFileToVector(const string& filename, vector<string>& linesVector, unsigned& noOfLines) {
    ifstream inputFile(filename.c_str());
    if (inputFile.fail()) {
        string errorStr = "Error : Could not open file " + filename;
        throw domain_error(LineInfo(errorStr, __FILE__, __LINE__));
    }
    string line;
    while (getline(inputFile, line)) {
        linesVector.push_back(line);
        noOfLines++;
    }
    inputFile.close();
}

// Declare the global mutex variable
pthread_mutex_t lineCountLock;

void* SingLinesThread(void* threadArg) {
    SingLinesThreadInfoStruct* infoStructPtr = (SingLinesThreadInfoStruct*)threadArg;
    unsigned singerNo = *(infoStructPtr->singerNoIdPtr);
    for (unsigned lineNo = singerNo - 1; lineNo < infoStructPtr->noOfLyricLines; lineNo += 4) {
        // Lock the mutex to access the shared variable
        if ((pthread_mutex_lock(&lineCountLock)) != 0)
            throw domain_error(LineInfo("pthread_mutex_lock", __FILE__, __LINE__));

        // Update the shared variable
        totalLineCount++;

        // Sing the line
        cout << "Singer " << singerNo << " singing : " << infoStructPtr->lyricLinesVector[lineNo] << endl;

        // Unlock the mutex
        if ((pthread_mutex_unlock(&lineCountLock)) != 0)
            throw domain_error(LineInfo("pthread_mutex_unlock", __FILE__, __LINE__));
        usleep(100000);
    }
    return NULL;
}


int main(int argc, const char* argv[]) {

    try {
        if (argc != 3) {
            string errorStr = "Usage : ";
            errorStr = errorStr.append("\n./singers <lyrics filename.txt> <number of singers>");
            errorStr = errorStr.append("\nExample : \n./singers PopLife.txt 4/n ");
            throw domain_error(LineInfo(errorStr, __FILE__, __LINE__));
        }

        string lyricsFilenameStr(argv[1]);
        unsigned noOfSingers = stoi(argv[2]);
        vector<string> lyricLinesVector;
        unsigned noOfLyricLines = 0;

        GetLyricsLinesFromFileToVector(lyricsFilenameStr, lyricLinesVector, noOfLyricLines);

        cout << endl << endl;
        cout << "Song Lyrics File Name is : " << lyricsFilenameStr << endl;
        cout << "Number of lyric lines is : " << noOfLyricLines << endl;
        cout << endl << endl;

        pthread_t* singersThreadIdPtr;
        unsigned* singersNoIdPtr;
        SingLinesThreadInfoStruct* singLinesThreadInfoStructPtr;

        singersThreadIdPtr = new pthread_t[noOfSingers];
        singersNoIdPtr = new unsigned[noOfSingers];
        singLinesThreadInfoStructPtr = new SingLinesThreadInfoStruct[noOfSingers];

        pthread_mutex_t lineCountLock;

        if ((pthread_mutex_init(&lineCountLock, NULL)) != 0)
            throw domain_error(LineInfo("pthread_mutex_init", __FILE__, __LINE__));

        // Create independent threads each of which will execute the pthread function.
        for (unsigned singerNo = 0; singerNo < noOfSingers; ++singerNo) {
            unsigned* singerNoIdPtr = new unsigned;
            *singerNoIdPtr = singerNo + 1;

            singLinesThreadInfoStructPtr[singerNo].singerNoIdPtr = singerNoIdPtr;
            singLinesThreadInfoStructPtr[singerNo].lyricLinesVector = lyricLinesVector;
            singLinesThreadInfoStructPtr[singerNo].noOfLyricLines = noOfLyricLines;

            if ((pthread_mutex_lock(&lineCountLock)) != 0)
                throw domain_error(LineInfo("pthread_mutex_lock", __FILE__, __LINE__));

            cout << endl << "create thread " << *singerNoIdPtr << endl;

            if ((pthread_mutex_unlock(&lineCountLock)) != 0)
                throw domain_error(LineInfo("pthread_mutex_unlock", __FILE__, __LINE__));

            int threadCreateReturn = pthread_create(
                &(singersThreadIdPtr[singerNo]),
                NULL,
                SingLinesThread,
                (void*)&singLinesThreadInfoStructPtr[singerNo]);

            if (threadCreateReturn != 0)
                throw domain_error(LineInfo("pthread_create", __FILE__, __LINE__));
        }

        for (unsigned singerNo = 0; singerNo < noOfSingers; ++singerNo)
            pthread_join(singersThreadIdPtr[singerNo], NULL);

        cout << endl;
        cout << "Program done " << endl;
        cout << "Total Line Count sung : " << totalLineCount << endl;
        cout << endl;

        if ((pthread_mutex_destroy(&lineCountLock)) != 0)
            throw domain_error(LineInfo("pthread_mutex_destroy", __FILE__, __LINE__));
    }
    catch (exception& e) {
        cout << e.what() << endl;
        exit(EXIT_FAILURE);
    }
    
    exit(EXIT_SUCCESS);
}
