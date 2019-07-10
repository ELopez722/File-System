//
// Main_driver.cpp
//
// Edgar Lopez
// OS CS
// 5/11/18
//
// File system development
//
// contents of sector will be back + frwd indicators.
// string of char (for user data only )
// (for dir only) free, 31 list long of type, name, link and size.

#include<iostream>
#include<vector>
#include<iomanip>
#include<cstdlib>
#include<string>
#include<fstream>
using namespace std;



struct workSec
// information of current sector we are in
{
    int position = 0;    // current sector we are working on
    char mode = 'X';     // I-input, O-output, U-update, X-dir use not needed.
    string name = "Root";// name of file we are in;
    int bytes = 0;   // number of bytes used in user data
};

struct sector
// used to represent a dir or user data.
{
    int bck = 0; // indicator of previous sector.
    int frwd = 0; // indicator of next sector.
    int free = 0; // used by first block only to know location of next free sector
    int filler = 0; // used to know base of extended dir/data.
    int UserDataPos  = 0; // (Data use)to know location of data
    string FileName;  // (used to contain name of current dir/data)
    vector<char> data = vector<char>(504, ' ');     // (Data use only) contains data content
    vector<char> type = vector<char>(31, 'F'); // (dir use only) list of type to know what it contains. D or U
    vector<string> name = vector<string>(31);  // (dir use only) name of files in dir
    vector<int> link = vector<int>(31, 0);    // (dir use only) position of sector
    vector<int> DataSize = vector<int>(31, 0);    // number of bytes used in sector
};

// find the next free sector and erases it from free list
int FreeSector(int a[])
{
    for(int i = 0; i < 500; i++)
    {
        if(a[i] == 1)
        {
            a[i] = 0;
            return i;
        }
    }
    return -1;
}

//strip string of leading white space.
void StripSpace(string& a)
{
    int i, length = a.size();
    for(i = 0; i < length && a[i] == ' ' ; i++);
    a = a.substr(i);
}

// returns next token of a string
string TokenCreator(string& s)
{
    StripSpace(s);
    string token;
    int i, length = s.size();
    for(i = 0; i < length && s[i] != ' ' && s[i] != '/' ; i++);
    token = s.substr(0, i);
    s = s.substr(i);
    StripSpace(s);
    return token;
}


int seek(sector disk[], string task, workSec inUse)
{
    if(inUse.mode == 'X')
    {
        cout << "please enter user file to use this command." << endl;
        return -1;
    }
    if(inUse.mode == 'I' || inUse.mode == 'U')
    {
        string base =  TokenCreator(task);
        string offset = TokenCreator(task);
        int offsetInt = 0;
        if(offset[0] != '0')
        {
            int numTemp;
            // stoi was not working so i just manually converted the string to a num.
            for(int i = 1; i < offset.size(); i++)
            {
                numTemp = offset[i] - 48;
                offsetInt = (offsetInt * 10) + numTemp;
            }
        }
        else
            offsetInt  = 0;
        if(offset[0] == '-')
            offsetInt = offsetInt * -1;
        // place pointer at whatever base the user types.
        if(base == "-1")
        {
            while(disk[inUse.position].filler > 0)
                inUse.position = disk[inUse.position].bck;
            disk[inUse.position].UserDataPos = 0;
        }
        else if(base == "0")
        {
            while(disk[inUse.position].UserDataPos == 504 && disk[inUse.position].frwd != 0)
                inUse.position = disk[inUse.position].frwd;
        }
        else if(base == "+1")
        {
            while(disk[inUse.position].frwd != 0)
                inUse.position = disk[inUse.position].frwd;
            disk[inUse.position].UserDataPos = disk[inUse.position].DataSize[0];
        }
        else
        {
            cout << "Enter proper data for this command." <<endl;
            return -1;
        }
        int i;
        if(offsetInt > 0)
        {
            i = 0;
            do{
                for( i; i < offsetInt && disk[inUse.position].UserDataPos < 504; i++)
                {
                    disk[inUse.position].UserDataPos = disk[inUse.position].UserDataPos + 1;
                }
                if(disk[inUse.position].frwd == 0)
                    return 0;
            }while(i < offsetInt && (inUse.position = disk[inUse.position].frwd));
            return 0 ;
        }
        else if(offsetInt < 0)
        {
            i = 0;
            do{
                for( i; i > offsetInt && disk[inUse.position].UserDataPos > 0 ; i-- )
                {
                    disk[inUse.position].UserDataPos = disk[inUse.position].UserDataPos - 1;
                }
            }while(disk[inUse.position].filler > 0 && (inUse.position = disk[inUse.position].bck ));
        }
        return 0;


    }
    else
    {
        cout << "current mode not permitted to write to file. " << endl;
        return -1;
    }
}


void read(sector disk[], string task, workSec inUse, bool saveedData)
{   // skip any output if we are working through saved file

    if(inUse.mode == 'X')
    {
        cout << "please enter user file to use this command." << endl;
        return;
    }
    if(inUse.mode == 'I' || inUse.mode == 'U')
    {
        string temp =  TokenCreator(task);
        int numGiven = 0, numTemp;
        // stoi was not working so i just manually converted the string to a num.
        for(int i = 0; i < temp.size(); i++)
        {
            numTemp = temp[i] - 48;
            numGiven = (numGiven * 10) + numTemp;
        }

        int readPointer = disk[inUse.position].UserDataPos;
        int i = 0;
        do{      //while we are within the limits of the sector length of data keep reading.
                for(i; readPointer < 504 && readPointer < disk[inUse.position].DataSize[0] && i < numGiven; i++)
                {
                    if(!saveedData)
                        cout << disk[inUse.position].data[readPointer];
                    readPointer++;
                }
                disk[inUse.position].UserDataPos = readPointer;

                if(readPointer == 504 && disk[inUse.position].frwd != 0)
                {
                    if(i != 0)
                    {   // if there is more to the file link to it and read from the start.
                        inUse.position = disk[inUse.position].frwd;
                        readPointer = 0;
                        disk[inUse.position].UserDataPos = 0;
                    }
                    else
                    {   //if we havent read anything yet then we need to read from the start pointer location
                        inUse.position = disk[inUse.position].frwd;
                        readPointer = disk[inUse.position].UserDataPos;
                    }
                }
                if(readPointer == disk[inUse.position].DataSize[0])
                {   // end of file reached.
                    if(!saveedData)
                    {
                        cout << endl << "We have reached the end of the data file with " << numGiven - i
                            << " bytes remaining." << endl;
                    }
                    return;
                }
        }while( i < numGiven);
        cout << endl << endl;
    }
    else{
        if(!saveedData)
            cout << "current mode not permitted to write to file. " << endl;
        return;
    }
}

int Write (sector disk[], string task, workSec &inUse, int OpenSpace[])
{   /// Test to make sure we are within user file and are in correct mode.
    /// once we confirmed we are in the right file userDataPos will be our pointer.
    if(inUse.mode == 'X')
    {
        cout << "please enter user file to use this command." << endl;
        return -1;
    }
    int base, i;
    if(inUse.mode == 'O' || inUse.mode == 'U')
    {
        string temp =  TokenCreator(task);
        int numGiven = 0, numTemp;
        // stoi was not working so i just manually converted the string to a num.
        for(int i = 0; i < temp.size(); i++)
        {
            numTemp = temp[i] - 48;
            numGiven = (numGiven * 10) + numTemp;
        }

        string userData = task;
        if(userData[0] != '\'' || userData[userData.size() - 1] != '\'')
        {
            cout << "enter proper data to preform command" << endl;
            return -1;
        }

        // write pointer keeps track of file space once its reached its limit
        // if no existing linked files create new user file link it to old one and resume process,
        // other wise move to next file. once we reached end of user data fill remaining n bytes with ' ';
        // once we finished writting return;
        int WritePointer = disk[inUse.position].UserDataPos;
        base = inUse.position;
        i = 0;
        do{
                for( i; WritePointer < 504 && i < numGiven; i++)
                {
                    if(i >= userData.size() - 2)
                    {
                        disk[inUse.position].data[WritePointer] = ' ';
                    }
                    else
                        disk[inUse.position].data[WritePointer] = userData[i+1];
                    WritePointer++;
                }
                //make current bytes equal to what we have written if its not already there.
                if(disk[inUse.position].DataSize[0] < WritePointer)
                    disk[inUse.position].DataSize[0] = WritePointer;

                disk[inUse.position].UserDataPos = WritePointer;
                inUse.bytes = WritePointer;

                if(WritePointer == 504)
                {
                    if(inUse.mode == 'U' && disk[inUse.position].frwd != 0 && i != 0)
                    {   // changing data as we move though the user file
                        inUse.position = disk[inUse.position].frwd;
                        WritePointer = 0;
                        disk[inUse.position].UserDataPos = 0;
                    }

                    else if(disk[inUse.position].frwd != 0)
                    {   // looking for the end of the datafile to start writing
                        inUse.position = disk[inUse.position].frwd;
                        WritePointer = disk[inUse.position].UserDataPos;
                    }
                    else
                    {   // if we have reached end of data file create new data file and link it.
                        ///root dir contains the next free sector, must follow with placing next free sec in root.
                        int dataFileNew = disk[0].free;
                        if(dataFileNew == -1)
                        {// ran out of memory
                            cout << "not enough memory to create new dir/data. Please delete some data and retry process." << endl;
                            return -1;
                        }
                        disk[0].free = FreeSector(OpenSpace);
                        // place all info needed to link and for data file to run properly.
                        disk[inUse.position].frwd = dataFileNew;
                        disk[dataFileNew].bck = inUse.position;
                        disk[dataFileNew].UserDataPos = 0;
                        disk[dataFileNew].DataSize[0] = 0;
                        disk[dataFileNew].filler = disk[inUse.position].filler + 1;
                        inUse.position = dataFileNew;
                        WritePointer = disk[inUse.position].UserDataPos;
                    }
                }
        }while(i < numGiven);
    }
    else{
        cout << "current mode not permitted to write to file. " << endl;
        return -1;
    }
    inUse.position = base;
    for( i = 0; i < 31 && disk[disk[base].bck].name[i] != inUse.name; i++);

    disk[disk[base].bck].DataSize[i] = inUse.bytes;
    return 0;
}

void Close(sector disk[], workSec &inUse)
{
    ///close and go back to base of last dir.
    ///replace information inUse with the previous dir. name, position, mode 'X' for dir.
    ///since you can not open a dir/data in a user data you will always go back to a dir
    ///for case of closing root inUse will still point at root.
    // Use filler to know how many times it needs to go back to the base of the current dir.
    do{
        inUse.position = disk[inUse.position].bck;
    }while (disk[inUse.position].filler > 0);

    inUse.mode = 'X';
    inUse.name = disk[inUse.position].FileName;
    inUse.bytes = 0;
    return;
}

int Open(sector disk[], string task, workSec &inUse)
{
    /// first make sure you are in a dir before you look at other data. you can only open from a dir.
    /// then look through current dir we are and all extended dir it is linked to if any.
    /// looking for a file to match its name. if user file is found open in the mode
    /// given by user. if dir is found make it the current dir(ignore the mode).
    if(inUse.mode != 'X')
    {
        cout << "Not a viable path, please try again." << endl;
        return -1;
    }
    string tokenMode = TokenCreator(task);
    string tokenTarget = TokenCreator(task);
    int i, position = inUse.position;

    do{
        for(i = 0; (i < 31) && (disk[position].name[i] != tokenTarget); i++);
    }while( (i == 31) && (position = disk[position].frwd));

    if(i == 31)
    {
        cout << "No file found with that name. " << endl;
        return -1;
    }


    if(disk[position].type[i] == 'D')
    {
        inUse.mode = 'X';
    }
    else if (!(tokenMode == "I" || tokenMode == "O" || tokenMode == "U"))
    {
        cout << "please enter proper mode for user data to be placed in. " << endl;
        return -1;
    }
    else
    {   // place first and only char from string into mode.
        inUse.mode = tokenMode[0];
    }
    inUse.name = disk[position].name[i];
    inUse.bytes = disk[position].DataSize[i];
    inUse.position = disk[position].link[i];

    if(inUse.mode != 'X')
    {// making sure we place the pointer for the data files in the right location
        if(inUse.mode == 'O')
        {   // for open place at end of file. loop through making sure we find the last
            // file and place the pointer at the end.
            workSec temp = inUse;
            while(disk[temp.position].DataSize[0] == 504 && disk[temp.position].frwd != 0)
            {
                // max out userPos to make note that we should look at next data
                // file for current pos "aka" end of file since we are opening in 'O' mode
                disk[temp.position].UserDataPos = 504;
                temp.position = disk[temp.position].frwd;
            }
            // max bytes will be end of file location.
            disk[temp.position].UserDataPos = disk[temp.position].DataSize[0];
        }
        else
        {   // for I and U place index at start of file.
            disk[inUse.position].UserDataPos = 0;
        }
    }

    // if we are trying to open multi files recuse through open till we hit our target
    // or can not go any further.
    if(task[0] == '/')
    {
        tokenMode += " ";
        tokenMode += task.substr(1);
        Open(disk, tokenMode, inUse);
    }
    return 0;
}

int SecDelete(sector disk[], string task, workSec &inUse, int OpenSpace[])
{   /// use open as a helper to access the file that they wish to be deleted if file
    /// is not found then exit without doing anything. if location is found then work on deleting
    /// all files within current file. recurse through SecDelete to delete the files. on the way out
    /// of the function clear data and free up space in open space to be reused.
    workSec temp = inUse;
    string tempName, mode = "I ";
    int x, i = Open(disk, mode += task, temp);
    if(i == -1)
        return i;
    int position = temp.position;
    do{// loop though and open file that is furthest in by recursive calling
        for(i = 0; i < 31 ; i++)
        {
            if(disk[position].type[i] != 'F')
            {
                x = SecDelete(disk, disk[position].name[i], temp, OpenSpace);
                if(x == -1)
                {//if file name given is bad return
                    return x;
                }
                else
                {//delete the file you just recursed from
                    disk[position].type[i] = 'F';
                    disk[position].name[i] = "";
                    disk[position].DataSize[i] = 0;
                    //for(int j; j < 504; j++)
                    //    disk[position].data[j] = ' ';
                }
            }
        }// if dir has more then 31 files then erase them all
    }while( (i == 31) && (disk[position].frwd != 0) && (position = disk[position].frwd));

    do{// erase the linked dir/user files.
            x = disk[position].filler;
            disk[position].frwd = 0;
            tempName = disk[position].FileName;
            disk[position].FileName = "";
            OpenSpace[position] = 1;
            position = disk[position].bck;
    }while(x > 0);
    //delete yourself from the previous file.
    for(i = 0; i < 31 && disk[position].name[i] != tempName ;i++ );
    disk[position].type[i] = 'F';
    disk[position].name[i] = "";
    disk[position].DataSize[i] = 0;
    //for(int j; j < 504; j++)
    //    disk[position].data[j] = ' ';

    return 0;
}

int Create(sector disk[], string task, workSec &inUse, int OpenSpace[])
{
    if(inUse.mode != 'X')
    {// test to make sure you are in a dir
        cout << "can only create new dir/data in a dir" << endl;
        return -1;
    }

    string tokenType = TokenCreator(task);
    if( tokenType != "U" && tokenType != "D" )
    {
        cout << "Enter type U for user or D for directory when using create." << endl;
        return -1;
    }
    string tokenName = TokenCreator(task);
    for(int i = 0; i < tokenName.length(); i++)
    {
        if(!(tokenName[i] >= 'a' && tokenName[i] <= 'z') && !(tokenName[i] >= 'A' && tokenName[i] <= 'Z')
           || !(tokenName.length() <= 9))
        {
            cout << "Enter Proper Name for Create." << endl;
            return -1;
        }
    }

    // searching for next available spot in current dir and extended dir or existing spot with same name
    int i, lastPosition, position = inUse.position;
    do{
        for(i = 0; (i < 31) && (disk[position].type[i] != 'F') && (disk[position].name[i] != tokenName);  i++);
         lastPosition = position;
    }while(i == 31 && (position = disk[position].frwd));

    //create new extended dir if full.
    if( position == 0 && i == 31)
    {   ///root dir contains the next free sector, must follow with placing next free sec in root.
        int dirNew = disk[0].free;
        if(dirNew == -1)
        {// ran out of memory
            cout << "not enough memory to create new dir/data. Please delete some data and retry process." << endl;
            return -1;
        }
        disk[0].free = FreeSector(OpenSpace);
        disk[dirNew].bck = lastPosition;        // linking new dir to tail end of previous one
        disk[lastPosition].frwd = dirNew;
        disk[dirNew].filler = disk[lastPosition].filler + 1;
        i = 0;
        position = dirNew;
    }

    //place new data/dir in position on disk at ith location
    if(disk[position].name[i] == tokenName)
    {
        int x = SecDelete(disk, tokenName, inUse, OpenSpace);
        if(x = -1)
            return x;
    }

    ///root dir contains the next free sector, must follow with placing next free sec in root.
    int secNew = disk[0].free;
    if(secNew == -1)
    {// ran out of memory
        cout << "not enough memory to create new dir/data. Please delete some data and retry process." << endl;
        return -1;
    }
    disk[0].free = FreeSector(OpenSpace);

    // place basic info into dir linking to new sec, and new sec back to dir
    disk[position].name[i] = tokenName;
    disk[position].link[i] = secNew;
    disk[position].DataSize[i] = 0;
    disk[secNew].UserDataPos = 0;
    disk[secNew].bck = position;
    disk[secNew].FileName = tokenName;
    // place basic info in working sector
    inUse.name = tokenName;
    inUse.position = secNew;
    disk[secNew].filler = 0;

    // if we have multiple dir to create we will call create recursivly
    if(tokenType == "D" || task[0] == '/')
    {
        disk[position].type[i] = 'D';
        inUse.mode = 'X';
        if(task[0] == '/')
        {
            tokenType += " ";
            tokenType += task.substr(1);
            Create(disk, tokenType, inUse, OpenSpace);
        }
    }
    else
    {
        disk[position].type[i] = 'U';
        inUse.mode = 'O';
        disk[position].DataSize[0] = 0;
    }
    return 0;
}

void printSpace(int x)
{
    for(int i = 0; i < x; i++)
        cout <<  " ";
}

void printFile(sector disk[], workSec inUse, int spaces, bool savedData, int counter[])
{   // dont preint while saved data is being recovered.
    if(savedData)
        return;
    int i;
    printSpace(spaces);
    cout << spaces + 1 << " " << inUse.name << endl;
    /* testing purposes. observe linkage of files
    printSpace(spaces);
    cout << " " << spaces + 1 << " Previous Block " << disk[inUse.position].bck << endl;
    printSpace(spaces);
    cout << " " << spaces + 1 << " Successor Block " << disk[inUse.position].frwd << endl;
    */
    if( inUse.mode != 'X')
    {
        counter[2] =+ 1;
        printSpace(spaces);
        int totalbytes = 0;
        cout << " " << spaces + 2 << " last free block " << inUse.bytes << endl;
        do{
            totalbytes =+ disk[inUse.position].DataSize[0];
        }while(inUse.position = disk[inUse.position].frwd);
        printSpace(spaces);
        cout << " " << spaces + 2 << " User File Size " << totalbytes << endl << endl;
        /* testing purposes. observe data.
        do{
        for(int i = 0; i < disk[inUse.position].DataSize[0]; i++)
            cout << disk[inUse.position].data[i];
            cout << endl;
        }while(inUse.position = disk[inUse.position].frwd);
        */
        return;

    }
    counter [1] =+ 1;
    printSpace(spaces);
    cout << " " << spaces + 2 << " Dir " << endl;
    workSec temp;
    do{
        for( i = 0; i < 31; i++)
        {
            if(disk[inUse.position].type[i] != 'F')
            {
                printSpace(spaces);
                cout << "  " << spaces + 3 << " Type " << disk[inUse.position].type[i] << endl;
                printSpace(spaces);
                cout << "  " << spaces + 3 << " Name " << disk[inUse.position].name[i] << endl << endl;
                /* testing purposes. observe links in dir
                printSpace(spaces);
                cout << "  " << 3 << " link " << disk[inUse.position].link[i] << endl << endl;*/
                temp.position = disk[inUse.position].link[i];
                temp.name = disk[inUse.position].name[i];
                temp.bytes = disk[inUse.position].DataSize[i];
                if(disk[inUse.position].type[i] == 'U')
                {
                    temp.mode = 'O';
                }
                else
                {
                    temp.mode = 'X';
                }
                printFile(disk, temp, spaces + 3, savedData, counter);
            }
            else
                counter[0] =+ 1;
        }
    }while(i == 31 && (inUse.position = disk[inUse.position].frwd));

    return;
}

// takes in string and assigns task
int fileSystem(sector disk[], string task, bool savedData, workSec &inUse, int OpenSpace[])
{
    string token;
    token = TokenCreator(task);
    int x, counter[3];
    counter[0] = 0, counter[1] = 0, counter[2] = 0;
    if(token == "CREATE")
    {
        x = Create(disk, task, inUse, OpenSpace);
        //Close(disk, inUse);
        printFile(disk, inUse, 0, savedData, counter);
       // return x;
    }
    else if(token == "OPEN")
    {
        x = Open(disk, task, inUse);
        printFile(disk, inUse, 0, savedData, counter);
    }
    else if(token == "CLOSE")
    {
        Close(disk, inUse);
        x = 0;
        printFile(disk, inUse, 0, savedData, counter);
    }
    else if(token == "DELETE")
    {
        x = SecDelete(disk, task, inUse, OpenSpace);
        printFile(disk, inUse, 0, savedData, counter);
    }
    else if(token == "READ")
    {
        read(disk, task, inUse, savedData);
        x = 0;
    }
    else if(token == "WRITE")
    {
        x = Write(disk, task, inUse, OpenSpace);
        printFile(disk, inUse, 0, savedData, counter);

    }
    else if(token == "SEEK")
    {
        x = seek(disk, task, inUse);
    }
    else if(token == "quit")
    {

        exit(0);
    }
    else
    {   if(!savedData)
            cout << "Please enter a reasonable task with proper parameter. or to exit type \"quit\": " << endl;
        x = -1;
        return x;
    }
    if(!savedData)
    {
      //  cout << "amount of free blocks = " << counter[0] << endl;
       // cout << "amount of dir blocks = " << counter[1] << endl;
        //cout << "amount of user data blocks = " << counter[2] << endl << endl;
    }
    return x;
}


int main()
{
    // always start program by creating the disk
    // and free table.
    // create the sectors and set the open list to all open except the first
    int OpenSpace[500], a[3];
    sector disk[500];
    for(int i = 2; i < 500; i++ ) // mark the root dir and first free space as taken
            OpenSpace[i] = 1;
    disk[0].free = 1;    // the first free space is the 2nd in array when program starts
    disk[0].FileName = "Root"; // keep the name of dir within  the data field.

    string userString;
    workSec inUse;
    // get task from user. while you still receive
    // data send it to file system to process task.

    fstream savedFile;
    fstream newSave;
    savedFile.open("savedData.txt");
    newSave.open("savedData.txt", ios::out | ios::app);
    if(savedFile)
    {
        string input;
        while(getline(savedFile,input))
        {
            if(savedFile)
                fileSystem(disk, input, true, inUse, OpenSpace);
        }
        cout << "File system recovered" << endl;
    }
    else
        cout << "File System Created." << endl;

    printFile(disk, inUse, 0, false, a);
    cout << "Please enter a task: ";
    getline(cin, userString);
    int save;
    while(userString != "\0")
    {
        StripSpace(userString);
        save = fileSystem(disk, userString, false, inUse, OpenSpace);
        if(save == 0)
        {
            newSave << userString << endl;
        }
        cout << "Please enter a task: ";
        getline(cin, userString);
    }

}
