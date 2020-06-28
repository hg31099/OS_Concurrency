#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<errno.h>
#include<sys/ipc.h>
#include<semaphore.h>
#include <sched.h>

#define N 5
#define cust 6
#define close 20
time_t end_time;/*end time*/
sem_t mutex,customers,barbers;/*Three semaphors*/
int count=0;/*The number of customers waiting for haircuts*/

void barber(void *arg);
void customer(void *arg);

int main(int argc,char *argv[])
{
	
	pthread_t id1;
	pthread_t ida[cust];
	int status=0;
	printf("BARBER opens: %ld \n",time(NULL));   		
	end_time=time(NULL)+close;/*Barber Shop Hours is 20s*/
	int i;
	int a[cust]={0,7,9,11,13,15};
	int sp=0;	
	/*Semaphore initialization*/
	sem_init(&mutex,0,1);
	sem_init(&customers,0,0);
	sem_init(&barbers,0,1);
	
	/*Barber_thread initialization*/
	status=pthread_create(&id1,NULL,(void *)barber,NULL);
	if(status!=0) perror("create barbers is failure!\n");
	sleep(a[0]);
	/*Customer_thread initialization*/
	status=pthread_create(&ida[0],NULL,(void *)customer,NULL);
	if(status!=0) perror("create customers is failure!\n");
	for(i=1;i<cust;i++)
	{
		sp=a[i]-a[i-1];
		sleep(sp);
		status=pthread_create(&ida[i],NULL,(void *)customer,NULL);
	       if(status!=0) perror("create customers is failure!\n");	
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
	time_t sec=time(NULL);

	while(sec<end_time || count>0)
	{
		if(count!=0)
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
		sec=time(NULL);
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
