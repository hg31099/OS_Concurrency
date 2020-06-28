#include <bits/stdc++.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

using namespace std;
time_t sec;

static int currTime=0;
class request{
    int type;             //0-writer 1-reader
    string name;          //thread name 
    int Arrival;           
    int Exec;             //exec time of thread
    int StartTime;          
    int EndTime;
public:
    request() {}
    request(int type, const string &name, int arrival, int exec) : type(type), name(name), Arrival(arrival),
                                                                   Exec(exec) {}
    int getStartTime() const {
        return StartTime;
    }

    void setStartTime(int startTime) {
        StartTime = startTime;
    }

    int getEndTime() const {
        return EndTime;
    }

    void setEndTime(int endTime) {
        EndTime = endTime;
    }

    int getType() const {
        return type;
    }

    void setType(int type) {
        request::type = type;
    }

    const string &getName() const {
        return name;
    }

    void setName(const string &name) {
        request::name = name;
    }

    int getArrival() const {
        return Arrival;
    }

    void setArrival(int arrival) {
        Arrival = arrival;
    }

    int getExec() const {
        return Exec;
    }

    void setExec(int exec) {
        Exec = exec;
    }

};

request rq[9];
void *reader(void *);
void *writer(void *);

int readcount=0,writecount=0;
sem_t readTry,rmutex,wmutex,resource;
int ret = sem_init(&readTry, 0, 1);
int ret1 = sem_init(&rmutex, 0, 1); //avoid race for entry and exit
int ret2 = sem_init(&wmutex, 0, 1);
int ret3 = sem_init(&resource, 0, 1);
void *reader(void *i)
{
        //<ENTRY Section>
    int j = *((int *) i);
    //cout<<"#"<<j<<" "<<currTime <<endl;
    sem_wait(&readTry);            //Indicate a reader is trying to enter
    sem_wait(&rmutex);              //lock entry section to avoid race condition with other readers
    readcount++;                 //report yourself as a reader and increment read count
    if (readcount == 1)          //checks if you are first reader
        sem_wait(&resource);          //if you are first reader, lock  the resource
    sem_post(&rmutex);                //indicate you are done trying to access the resource
    sem_post(&readTry);              //release entry section for other readers

    //<CRITICAL Section>
    //reading is performed
    cout << "\n-------------------------";
    cout<<"\nStart Time of "<<rq[j].getName()<<" : "<<currTime-1;
    cout << "\nreader-" << rq[j].getName() << " is reading";    
    int localtime=currTime;
    while(rq[j].getExec()+localtime>currTime)
    {}
    cout<<"\nEnd Time of "<<rq[j].getName()<<" : "<<currTime-1;
    //<EXIT Section>
    sem_wait(&rmutex);                  //reserve exit section - avoids race condition with readers
    readcount--;                 //indicate you're leaving
    if (readcount == 0)          //checks if you are last reader leaving
        sem_post(&resource);              //if last, you must release the locked resource
    sem_post(&rmutex);                //release exit section for other readers
    return i;
}

void *writer(void *i)
{
    //<ENTRY Section>
    int j = *((int *) i);
    //cout<<j<<" "<<currTime <<endl;
    sem_wait(&wmutex);                  //reserve entry section for writers - avoids race conditions
    writecount++;                //report yourself as a writer entering
    if (writecount == 1)         //checks if you're first writer
        sem_wait(&readTry);               //if you're first, then you must lock the readers out. Prevent them from trying to enter Critical section writer pref.
    sem_post(&wmutex);                  //release entry section

    //<CRITICAL Section>
    sem_wait(&resource);                //reserve the resource for yourself - prevents other writers from simultaneously editing the shared resource

    cout << "\n-------------------------";
    cout<<"\nStart Time of "<<rq[j].getName()<<" : "<<currTime-1;
    cout << "\n writer-" << rq[j].getName() << "is writing";
    int localtime=currTime;
    while(rq[j].getExec()+localtime>currTime){}
    cout<<"\nEnd Time of "<<rq[j].getName()<<" : "<<currTime-1;

    //writing is performed
    sem_post(&resource);                //release file

    //<EXIT Section>
    sem_wait(&wmutex);                  //reserve exit section
    writecount--;                //indicate you're leaving
    if (writecount == 0)         //checks if you're the last writer
        sem_post(&readTry);               //if you're last writer, you must unlock the readers. Allows them to try enter CS for reading
    sem_post(&wmutex);
    return i;
}


int main() {
    fstream fin("./input.txt");
    pthread_t p[9];
    for(int i=0; i<4; i++)
    {
        for(int j=0; j<9; j++)
        {
            switch(i) {
                case 0:{
                    int t;
                    fin >> t;
                    rq[j].setType(t);
                    break;}
                case 1:{
                    string name;
                    fin >> name;
                    rq[j].setName(name);
                    break;}
                case 2:{
                    int x;
                    fin >>x;
                    rq[j].setArrival(x);
                    break;}
                case 3:{
                    int x;
                    fin >>x;
                    rq[j].setExec(x);
                    break;}
            }
        }
    }
    for(int j=0; j<9; j++) {
        cout << rq[j].getType() << "| " << rq[j].getName() << " " << rq[j].getArrival() << " " << rq[j].getExec()
             << endl;
    }
    while(currTime<100)
    {
        //cout<<currTime<<endl;
        for(int j=0; j<9; j++)
        {
            if(currTime==rq[j].getArrival())
            {
                /*if(j!=0)
                {
                    sleep(rq[j].getArrival()-rq[j-1].getArrival())
                }*/
                if(rq[j].getType()==1)
                {
                    int x = j;
                    void *pointer = &x;
                     
                     cout<<endl<<"Reader thread creation"<<rq[j].getName()<<endl;
                    pthread_create(&p[j],NULL,reader,pointer);
                     if(j<8 && currTime==rq[j+1].getArrival())  sleep(1);     
                }
                else if(rq[j].getType()==0)
                {
                    int x = j;
                    void *pointer = &x;
                    
                     cout<<endl<<"Writer thread creation"<<rq[j].getName()<<endl;
                     pthread_create(&p[j],NULL,writer,pointer);
                
                    if(j<8 && currTime==rq[j+1].getArrival())  sleep(1);     
                }
            }
        }
        currTime++;
        sleep(1);
    }
    for(int j=0; j<9; j++) {
        pthread_join(p[j], NULL);
    }
    return 0;
}
