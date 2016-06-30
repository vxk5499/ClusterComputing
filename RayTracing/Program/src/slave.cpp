//This file contains the code that the master process will execute.

#include <iostream>
#include <mpi.h>
#include <math.h>
#include "RayTrace.h"
#include "slave.h"

void slaveVertical(ConfigData* data, float* pixels2)
{
    //Start the computation time timer.
	
	
	
    double computationStart = MPI_Wtime();
	
	int c = 3 * (data->width) * data->height + 1 ;
	
	//variables to find quotient, remainder and starting point.

	int processor_width,slave_start,slave_stop,tag;

	float i1;
	
	processor_width=(data->width)/(data->mpi_procs);  // Calculates the amount of width processed by each processor.
	slave_start=((data->mpi_rank)-1)*processor_width; //starting point for the slave processor
	slave_stop=(data->mpi_rank)*(processor_width);	  //ending point for the slave processor
	tag=0;
	//std::cout << "Entered slave ";
    //Render the scene.
    for( int i = 0; i < data->height; ++i )
    {
		//std::cout << "Entered slave i";
        for( int j = slave_start; j <= slave_stop; j++)
        {
			//std::cout << "Entered slave j"  << i<<"	"<<j<<std::endl;
            int row = i;
            int column = j;

            //Calculate the index into the array.
            int baseIndex = 3 * ( row * data->width + column );

            //Call the function to shade the pixel.
            shadePixel(&(pixels2[baseIndex]),row,j,data);
			
        }
    }
	
	

    //Stop the comp. timer
    double computationStop = MPI_Wtime();
    double computationTime = computationStop - computationStart;
	i1=(float)computationTime;
	pixels2[3*(data->height-1)*(slave_stop-1)+3]=i1;
	
    MPI_Send(pixels2,c,MPI_FLOAT,0,tag,MPI_COMM_WORLD);
    //After receiving from all processes, the communication time will
    //be obtained.
  

    //Print the times and the c-to-c ratio
	//This section of printing, IN THIS ORDER, needs to be included in all of the
	//functions that you write at the end of the function.
    //std::cout << "Total Computation Time: " << computationTime << " seconds" << std::endl;
   // std::cout << "Total Communication Time: " << communicationTime << " seconds" << std::endl;
    //double c2cRatio = communicationTime / computationTime;
   // std::cout << "C-to-C Ratio: " << c2cRatio << std::endl;
}

void slaveCyclic(ConfigData* data)
{
    //Start the computation time timer.
	
	int cyclesperprocessor=data->height/(data->mpi_procs * data->cycleSize);// Amount of times the processor is called
	int requiredheight=cyclesperprocessor*data->cycleSize;//the height that will be processed by all processors
	
	float* pixels3 = new float[3 * (data->width) * (requiredheight) + 1];

	
   	 double computationStart = MPI_Wtime(); 
	
	int c = 3 * (data->width) * (requiredheight)+1 ;
	
	//variables to find quotient, remainder and starting point.

	int tag;

	float i1;
	
	//processor_height=(data->height)/(data->mpi_procs);
	
	tag=0;
	
	int baseIndex=0;
	
    //Render the scene.
    for( int i = 0; i <cyclesperprocessor; ++i )
    {
		
		int k=0;
		int rstart = (i*data->mpi_procs*data->cycleSize)+(data->mpi_rank*(data->cycleSize));
		int row=rstart;
		
	    while(k<data->cycleSize)
		{
					
        	for( int j = 0; j < data->width ; ++j)
        		{
					
            		int column = j;		       
       

            		//Call the function to shade the pixel.
            		shadePixel(&(pixels3[baseIndex]),row,j,data);
					baseIndex=baseIndex+3;				
		
			}
		
		k=k+1;
		row=row+1;
		}	
			
        
    }
	
	

    //Stop the comp. timer
    double computationStop = MPI_Wtime();
    double computationTime = computationStop - computationStart;
	i1=(float)computationTime;
	pixels3[3 * (data->width) * (requiredheight) ]= i1;
	
    MPI_Send(pixels3,c,MPI_FLOAT,0,tag,MPI_COMM_WORLD);
	

  

}

void slaveBlock(ConfigData* data)
{
	 double computationStart = MPI_Wtime(); 
	 
	int pixels_processed=(data->width)*(data->height)/data->mpi_procs;// Amount of pixels to be processed by each processor
	int height_Block=sqrt(pixels_processed); // Width and height of block

	int r=sqrt(data->mpi_procs); // Num of blocks per row or column
	int remaining_height=data->width-(height_Block*r); // remaining height to be processed
	int c=3 * (height_Block+remaining_height) * (height_Block+remaining_height) + 1;
	float* pixels4=new float[c];
	int k=0;//determines whether the block has to process remainder
	int y=0;//determines whether the block has to process remainder
	int tag=0;
	if((data->mpi_rank+1)%r==0)
	k=1;
	if((data->mpi_rank+1)>(r*(r-1)))
	y=1;
	
	int r_start=height_Block*(data->mpi_rank/r);//starting row index
	int r_stop=(height_Block*((data->mpi_rank/r)+1))+(y*(remaining_height));//ending row index
	int c_start=((data->mpi_rank)%r)*height_Block;//starting  column index
	int c_stop=((((data->mpi_rank)%r)+1)*height_Block)+(k*(remaining_height));//ending column index
	
	
	int baseIndex=0;
	
	for(int i=r_start;i<r_stop;i++)
	{
		for(int j=c_start;j<c_stop;j++)
		{
		
			int row = i;
            int column = j;
			
					
			shadePixel(&(pixels4[baseIndex]),row,j,data);
			baseIndex=baseIndex+3;
		}
	}
	
	double computationStop = MPI_Wtime();
    double computationTime = computationStop - computationStart;
	
	float i1;
	i1=(float)computationTime;
	
	pixels4[c-1]= i1;
	std::cout<<"Rank:"<<data->mpi_rank<<" Comp time:"<<pixels4[c-1]<<" c:"<<c<<std::endl;
	
	MPI_Send(pixels4,c,MPI_FLOAT,0,tag,MPI_COMM_WORLD);
}

void slaveDynamic(ConfigData* data)
{
	int *pixels1=new int[4];
	int tag=0;
	MPI_Status status;
	
	
	MPI_Recv(pixels1,4,MPI_FLOAT,0,tag,MPI_COMM_WORLD,&status);
	int rstart=pixels1[0];
	int rend=pixels1[1];
	int cstart=pixels1[2];
	int cend=pixels1[3];
	int c=(rend-rstart)*(cend-cstart)*3;
	float *pixels4=new float[(rend-rstart)*(cend-cstart)*3];
	int baseIndex=0;
	for(int i=rstart;i<rend;i++)
	{
		for(int j=cstart;j<cend;j++)
		{
		
			int row = i;
            int column = j;	
			
					
			shadePixel(&(pixels4[baseIndex]),row,j,data);
			baseIndex=baseIndex+3;
		}
	}
	
	MPI_Send(pixels4,c,MPI_FLOAT,0,tag,MPI_COMM_WORLD);
}



void slaveMain(ConfigData* data)
{
	
	
	float* pixels2 = new float[(3 * (data->width) * data->height)+1];
	
	double startTime, stopTime;
    //Depending on the partitioning scheme, different things will happen.
    //You should have a different function for each of the required 
    //schemes that returns some values that you need to handle.
    switch (data->partitioningMode)
    {
        case PART_MODE_NONE:		
        //The slave will do nothing since this means sequential operation.
            break;
		case PART_MODE_STATIC_STRIPS_VERTICAL:
	    //Call the function that will handle this.
            startTime = MPI_Wtime();
            slaveVertical(data, pixels2);
            stopTime = MPI_Wtime();
            break;

		case PART_MODE_STATIC_CYCLES_HORIZONTAL:
		//Call the function that will handle Cycles horizontal
	    startTime = MPI_Wtime();
            slaveCyclic(data);
            stopTime = MPI_Wtime();
            break;

		case PART_MODE_STATIC_BLOCKS:
		//Call the function that will handle Static blocks
		
			startTime = MPI_Wtime();
            slaveBlock(data);
            stopTime = MPI_Wtime();
            break;
			
		case PART_MODE_DYNAMIC:
		
			startTime = MPI_Wtime();
            slaveDynamic(data);
            stopTime = MPI_Wtime();
            break;	


        default:
            std::cout << "This mode (" << data->partitioningMode;
            std::cout << ") is not currently implemented." << std::endl;
            break;
    }

   
}
