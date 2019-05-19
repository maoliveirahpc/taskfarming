/* #################################################################################### */
/* #################################################################################### */
/* # Task farming application  12/2007                                                # */
/* #       2nd version - transfers only the tasks each slave requires                 # */
/* #                                                        Miguel Afonso Oliveira    # */
/* #################################################################################### */
/* #################################################################################### */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mpi.h>

void read_input( int *, char ***, FILE * );
void sync_input( int *, char ***, int );

int main( int argc, char **argv )
{
int  ntasks,itasks;
int  nprocs,myrank,status;
int  ntaskspproc;
char **tasks;
char *filenamein,*filenameout;
FILE *taskfilein,*taskfileout;

if ( status=MPI_Init(&argc,&argv) )
{
   printf("\n\n\tUnable to start MPI...\n\n");
   MPI_Abort(MPI_COMM_WORLD,11);
}
MPI_Comm_size(MPI_COMM_WORLD,&nprocs);
MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
if ( myrank == 0 )
{
   if ( argc != 2 )
   {
       printf("\n\n\tUsage: taskfarm taskfile\n\n");
       MPI_Abort(MPI_COMM_WORLD,12);
   }
   if ( (filenamein=(char *)malloc(strlen(*(argv+1)+1))) == NULL )
   {
       printf("\n\n\tUnable to create input file name.\n\n");
       MPI_Abort(MPI_COMM_WORLD,13);
   }   
   sprintf(filenamein,"%s",*(argv+1));
   if ( (taskfilein=fopen(filenamein,"r")) == NULL )
   {
       printf("\n\n\tUnable to open input file.\n\n");
       MPI_Abort(MPI_COMM_WORLD,14);
   }
   free(filenamein);
   read_input(&ntasks,&tasks,taskfilein);
   fclose(taskfilein);
   if ( (filenameout=(char *)malloc(strlen(*(argv+1))+6)) == NULL )
   {
       printf("\n\n\tUnable to create output file name.\n\n");
       MPI_Abort(MPI_COMM_WORLD,15);
   }   
   sprintf(filenameout,"%s.%s",*(argv+1),"read");
   if ( (taskfileout=fopen(filenameout,"w")) == NULL )
   {
       printf("\n\n\tUnable to open output file.\n\n");
       MPI_Abort(MPI_COMM_WORLD,16);
   }
   free(filenameout);
   itasks=0;
   while ( (*(tasks+itasks)) != NULL )
   {
       fprintf(taskfileout,"%s\n",*(tasks+itasks));
       ++itasks;
   }
   fclose(taskfileout);
   if ( fmod(ntasks,nprocs) != 0.0 )
   {
       printf("\n\n\tTasks count must be divisible by processor number.\n\n");
       MPI_Abort(MPI_COMM_WORLD,17);
   }
   ntaskspproc=ntasks/nprocs;
   if ( nprocs > 1 ) sync_input(&ntaskspproc,&tasks,myrank);
}
else
{
   sync_input(&ntaskspproc,&tasks,myrank);
}
for ( itasks=0 ; itasks < ntaskspproc ; ++itasks )
   system(*(tasks+itasks));
itasks=0;
while ( (*(tasks+itasks)) != NULL )
{
   free(*(tasks+itasks));
   (*(tasks+itasks))=NULL;
   ++itasks;
}
free(tasks);
MPI_Finalize();
return 0;
}

/* #################################################################################### */
/* # Read input                                                                       # */
/* #################################################################################### */

void read_input( int *totaltasks, char ***tasklist, FILE *taskfile )
{
int  c,itasks,newtask,length;
char **tasks;

tasks=NULL;
if ( (tasks=(char **)malloc(sizeof(char *))) == NULL )
{
   printf("\n\n\tError allocating new task entry...\n\n");
   MPI_Abort(MPI_COMM_WORLD,21);
}
(*(tasks))=NULL;
itasks=0;
newtask=1;
while ( (c=fgetc(taskfile)) != EOF )
{
   if ( newtask )
   {
       newtask=0;
       if ( c == '#' || c == ' ' )
           while ( c=fgetc(taskfile) , c != EOF && c != '\n' );
       if ( c == EOF ) break;
       if ( c == '\n' )
       {
            newtask=1;
       }
       else
       {
           ++itasks;
           if ( (tasks=(char **)realloc(tasks,(itasks+1)*sizeof(char *))) == NULL )
           {
               printf("\n\n\tError allocating new task entry...\n\n");
               MPI_Abort(MPI_COMM_WORLD,22);
           }
           (*(tasks+itasks))=NULL;
           if ( ((*(tasks+itasks-1))=(char *)calloc(2,sizeof(char))) == NULL )
           {
               printf("\n\n\tError allocating new task entry...\n\n");
               MPI_Abort(MPI_COMM_WORLD,23);
           }
           (*(*(tasks+itasks-1)))=c;
           (*(*(tasks+itasks-1)+1))='\0';
       }
   }
   else
   {
       if ( c == '\n' )
           newtask=1;
       else
       {
           length=strlen(*(tasks+itasks-1));
           if ( ((*(tasks+itasks-1))=(char *)realloc(*(tasks+itasks-1), \
                                     (length+2)*sizeof(char))) == NULL )
           {
               printf("\n\n\tError allocating new task entry...\n\n");
               MPI_Abort(MPI_COMM_WORLD,24);
           }
           (*(*(tasks+itasks-1)+length))=c;
           (*(*(tasks+itasks-1)+length+1))='\0';
       }
   }
}
(*totaltasks)=itasks;
(*tasklist)=tasks;
}

/* #################################################################################### */
/* # Synchronize input                                                                # */
/* #################################################################################### */

void sync_input( int *ntasksperproc, char ***tasklist, int myrank )
{
int        ntasks,itasks,dest,length;
char       **tasks;
MPI_Status status;

if ( myrank == 0 ) ntasks=(*ntasksperproc);
MPI_Bcast(&ntasks,1,MPI_INT,0,MPI_COMM_WORLD);
if ( myrank == 0 )
{
   tasks=(*tasklist);
   itasks=0;
   while ( (*(tasks+itasks)) != NULL )
   {
       dest=itasks/ntasks;
       if (dest != 0 )
       {
           length=strlen(*(tasks+itasks));
           MPI_Send(&length,1,MPI_INT,dest,0,MPI_COMM_WORLD);
           MPI_Send(*(tasks+itasks),length+1,MPI_CHAR,dest,0,MPI_COMM_WORLD);
       }
       ++itasks;
   }
}
else
{
   tasks=NULL;
   if ( (tasks=(char **)calloc(ntasks+1,sizeof(char *))) == NULL )
   {
       printf("\n\n\tError allocating new task entry...\n\n");
       MPI_Abort(MPI_COMM_WORLD,31);
   }
   for ( itasks=0 ; itasks < ntasks ; ++itasks )
   {
       MPI_Recv(&length,1,MPI_INT,0,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
       if ( ((*(tasks+itasks))=(char *)calloc(length+1,sizeof(char))) == NULL )
       {
           printf("\n\n\tError allocating new task entry...\n\n");
           MPI_Abort(MPI_COMM_WORLD,32);
       }
       MPI_Recv((*(tasks+itasks)),length+1,MPI_CHAR,0,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
   }
   (*(tasks+ntasks))=NULL;
   (*tasklist)=tasks;
   (*ntasksperproc)=ntasks;
}
}

/* #################################################################################### */
/* #################################################################################### */
/* #                                          END                                     # */
/* #################################################################################### */
/* #################################################################################### */
