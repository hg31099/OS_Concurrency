#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<errno.h>
#include<sys/ipc.h>
#include<semaphore.h>
#include <sched.h>

#define N 5
#define stud 5
#define staff 5
int cust;
time_t end_time;/*end time*/
sem_t mutex,customers,barbers;/*Three semaphors*/
int count=0;/*The number of customers waiting for haircuts*/

void barber(void *arg);
void customer(void *arg);
pthread_attr_t tattr;
int newprio = 20;
struct sched_param param;
int main(int argc,char *argv[])
{
	cust=stud+staff;
	pthread_t id1;
	pthread_t ida[cust];
	int status=0;
	printf("BARBER opens: %ld \n",time(NULL));   		
	end_time=time(NULL)+60;/*Barber Shop Hours is 20s*/
	int i=0,j=0;
	int sdc[stud]={0,2,4,6,8};
	int stc[staff]={0,2,4,6,8};
	int sp=0;	
	/*Semaphore initialization*/
	sem_init(&mutex,0,1);
	sem_init(&customers,0,0);
	sem_init(&barbers,0,1);
	/*Barber_thread initialization*/
	status=pthread_create(&id1,NULL,(void *)barber,NULL);
	if(status!=0) perror("create barbers is failure!\n");
	int prev;
	/*Customer_thread initialization*/
	pthread_attr_init (&tattr);
	pthread_attr_getschedparam (&tattr, &param);
	param.sched_priority = newprio;
	pthread_attr_setschedparam (&tattr, &param);
	if(sdc[0]<=stc[0])
	{
		sleep(sdc[0]);
		printf("student thread creation \n");
		status=pthread_create(&ida[0],&tattr,(void *)customer,NULL);
		if(status!=0) perror("create customers is failure!\n");
		i++;
		prev=sdc[0];
	}
	else
	{
		sleep(stc[0]);
		printf("staff thread creation \n");
		status=pthread_create(&ida[0],NULL,(void *)customer,NULL);
		if(status!=0) perror("create customers is failure!\n");
		j++;
		prev=stc[0];
	}	
	while(i<stud || j<staff)
	{
		if(i<stud && sdc[i]<=stc[j])
		{
			sp=sdc[i]-prev;
			prev=sdc[i];
			sleep(sp);
			printf("student thread creation \n");
			status=pthread_create(&ida[i+j],&tattr,(void *)customer,NULL);
			if(status!=0) perror("create customers is failure!\n");
			i++;
			sleep(1);
		}
		else
		{
			sp=stc[j]-prev;
			prev=stc[j];
			sleep(sp);
			printf("staff thread creation \n");
			status=pthread_create(&ida[i+j],NULL,(void *)customer,NULL);
			if(status!=0) perror("create customers is failure!\n");
			j++;
			sleep(1);
		}
	}	
	/*Customer_thread first blocked*/
	for(i=0;i<cust;i++)
	{
		pthread_join(ida[i],NULL);
	}
	pthread_join(id1,NULL);

	exit(0);
}

void barber(void *arg)/*Barber Process*/
{
	while(time(NULL)<end_time || count>0)
	{
		sem_wait(&customers);/*P(customers)*/            
		sem_wait(&mutex);/*P(mutex)*/
		count--;
		printf("Barber:cut hair,count is:%d.\n",count);
		printf("start: %ld ",time(NULL));   		
		sem_post(&mutex);/*V(mutex)*/
		sem_post(&barbers);/*V(barbers)*/
		sleep(5);     
		printf("end: %ld \n",time(NULL)); 
	}
}

void customer(void *arg)/*Customers Process*/
{
	//while(time(NULL)<end_time)
	//{
		sem_wait(&mutex);/*P(mutex)*/
		if(count<N)
		{
			count++;
			printf("Customer:add count,count is:%d\n",count);
			sem_post(&mutex);/*V(mutex)*/
			sem_post(&customers);/*V(customers)*/
			sem_wait(&barbers);/*P(barbers)*/
		}
		else
			/*V(mutex)*/
			/*If the number is full of customers,just put the mutex lock let go*/
			sem_post(&mutex);
	//}
}
