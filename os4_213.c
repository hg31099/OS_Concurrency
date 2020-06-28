#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<errno.h>
#include<sys/ipc.h>
#include<semaphore.h>
#include <sched.h>
#define k 3
#define N 5
#define cust 7
#define close 20
sem_t waiting_room_mutex;
sem_t barber_room_mutex;
sem_t barber_chair_free;
sem_t sleepy_barbers;
sem_t barber_chairs[k];
int barber_chair_states[k] = {0, 0, 0};
int num_waiting_chairs_free = N;
time_t end_time;/*end time*/


int cc=cust;
int customer( ) {
// try to make it into waiting room
	sem_wait(&waiting_room_mutex);
	if (num_waiting_chairs_free == 0) {
		sem_post(&waiting_room_mutex);
		return 0;
	}
	num_waiting_chairs_free--; // grabbed a chair
	sem_post(&waiting_room_mutex);
	// now, sem_wait until there is a barber chair free
	sem_wait(&barber_chair_free);
	// a barber chair is free, so release waiting room chair
	sem_wait(&waiting_room_mutex);
	sem_wait(&barber_room_mutex);
	num_waiting_chairs_free++;
	sem_post(&waiting_room_mutex);
	// now grab a barber chair
	int mychair;
	for (int i=0; i<k; i++) 
	{
		if (barber_chair_states[i] == 0) 
		{ // 0 = empty chair
			mychair = i;
			break;
		}
	}
	barber_chair_states[mychair] = 1; // 1 = haircut needed
	sem_post(&barber_room_mutex);
	// now wake up barber, and sleep until haircut done
	sem_post(&sleepy_barbers);
	sem_wait(&barber_chairs[mychair]);
	// great! haircut is done, let's leave. barber........bye
	// has taken care of the barber_chair_states array.
	sem_post(&barber_chair_free);
	return 1;
}
void barber() {
	while(time(NULL)<end_time) 
	{
		// sem_wait for a customer
		if(cc!=0)
		{
			sem_wait(&sleepy_barbers);
			// find the customer
			sem_wait(&barber_room_mutex);
			int mychair;
			for (int i=0; i<k; i++) 
			{
				if (barber_chair_states[i] == 1) 
				{
					mychair = i;
					break;
				}
			}
			barber_chair_states[mychair] = 2; // 2 = cutting hair
			sem_post(&barber_room_mutex);
			// CUT HAIR HERE
			printf("Start %ld\n",time(NULL));
			printf("At chair %d\n",mychair+1);
			sleep(5);
			printf("End %ld\n",time(NULL));
			cc--;
			if(cc==0)
			break;
			// now wake up customer

			if(cc!=0 && time(NULL)<end_time)
			{
				sem_wait(&barber_room_mutex);
				barber_chair_states[mychair] = 0; // 0 = empty chair
				
				sem_post(&barber_room_mutex);
				// all done, we'll loop and sleep again	
			}
			sem_post(&barber_chairs[mychair]);
		}
	}
}
int main(int argc,char *argv[])
{
	pthread_t idb[k];
	pthread_t ida[cust];
	int status=0;
	printf("BARBER opens: %ld \n",time(NULL));   		
	end_time=time(NULL)+close;/*Barber Shop Hours is 20s*/
	int i;
	int a[cust]={0,4,7,8,9,10,11};
	int sp=0;	
	/*Semaphore initialization*/
	sem_init(&waiting_room_mutex,0,1);
	sem_init(&barber_room_mutex,0,1);
	sem_init(&barber_chair_free,0,k);
	sem_init(&sleepy_barbers,0,0);
	for(i=0;i<k;i++)
	{
		sem_init(&barber_chairs[i],0,0);		
	}
	/*Barber_thread initialization*/
	for(i=0;i<k;i++)
	{
		status=pthread_create(&idb[i],NULL,(void *)barber,NULL);
	    if(status!=0) perror("create barbers is failure!\n");		
	}
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
	for(i=0;i<k;i++)
	{
		pthread_join(idb[i],NULL);
	}
	
	exit(0);
}

