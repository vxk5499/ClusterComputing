//This file contains the code that the master process will execute.

#include <iostream>
#include <mpi.h>
#include <math.h>

#include "RayTrace.h"
#include "master.h" 

void masterMain(ConfigData* data)
{
    //Depending on the partitioning scheme, different things will happen.
    //You should have a different function for each of the required 
    //schemes that returns some values that you need to handle.
    
    //Allocate space for the image on the master.
    float* pixels = new float[3 * data->width * data->height];
	
    
    //Execution time will be defined as how long it takes
    //for the given function to execute based on partitioning
    //type.
    double renderTime = 0.0, startTime, stopTime;

	//Add the required partitioning methods here in the case statement.
	//You do not need to handle all cases; the default will catch any
	//statements that are not specified. This switch/case statement is the
	//only place that you should be adding code in this function. Make sure
	//that you update the header files with the new functions that will be
	//called.
	//It is suggested that you use the same parameters to your functions as shown
	//in the sequential example below.
    switch (data->partitioningMode)
    {
        case PART_MODE_NONE:
            //Call the function that will handle this.
            startTime = MPI_Wtime();
            masterSequential(data, pixels);
            stopTime = MPI_Wtime();
            break;
			
			
		case PART_MODE_STATIC_STRIPS_VERTICAL:
		
		//Call the function that will handle strips verticals
		
            startTime = MPI_Wtime();
            masterVertical(data, pixels);
            stopTime = MPI_Wtime();
            break;

		
		case PART_MODE_STATIC_CYCLES_HORIZONTAL:
		
		//Call the function that will Cycles Horizontal
			
			startTime = MPI_Wtime();
            masterCyclic(data, pixels);
            stopTime = MPI_Wtime();
            break;

		case PART_MODE_STATIC_BLOCKS:
		
		//Call this function that will handle blocks
		
			startTime = MPI_Wtime();
            masterBlock(data, pixels);
            stopTime = MPI_Wtime();
            break;
			
		case PART_MODE_DYNAMIC:
		
			startTime = MPI_Wtime();
            masterDynamic(data, pixels);
            stopTime = MPI_Wtime();
            break;	



        default:
		
            std::cout << "This mode (" << data->partitioningMode;
            std::cout << ") is not currently implemented." << std::endl;
            break;
    }

    renderTime = stopTime - startTime;
    std::cout << "Execution Time: " << renderTime << " seconds" << std::endl << std::endl;

    //After this gets done, save the image.
    std::cout << "Image will be save to: ";
    std::string file = generateFileName(data);
    std::cout << file << std::endl;
    savePixels(file, pixels, data);

    //Delete the pixel data.
    delete[] pixels; 
}

void masterSequential(ConfigData* data, float* pixels)
{
    //Start the computation time timer.
    double computationStart = MPI_Wtime();

    //Render the scene.
    for( int i = 0; i < data->height; ++i )
    {
        for( int j = 0; j < data->width; ++j )
        {
            int row = i;
            int column = j;

            //Calculate the index into the array.
            int baseIndex = 3 * ( row * data->width + column );
			
			std::cout<<"base index"<<baseIndex<<std::endl;

            //Call the function to shade the pixel.
            shadePixel(&(pixels[baseIndex]),row,j,data);
        }
    }

    //Stop the comp. timer
    double computationStop = MPI_Wtime();
    double computationTime = computationStop - computationStart;
	
	

    //After receiving from all processes, the communication time will
    //be obtained.
    double communicationTime = 0.0;

    //Print the times and the c-to-c ratio
	//This section of printing, IN THIS ORDER, needs to be included in all of the
	//functions that you write at the end of the function.
    std::cout << "Total Computation Time: " << computationTime << " seconds" << std::endl;
    std::cout << "Total Communication Time: " << communicationTime << " seconds" << std::endl;
    double c2cRatio = communicationTime / computationTime;
    std::cout << "C-to-C Ratio: " << c2cRatio << std::endl;
}

void masterVertical(ConfigData* data, float* pixels)
{
	float* pixels1 = new float[3 * data->width * data->height+1];
	
    //Start the computation time timer.
    double computationStart = MPI_Wtime();
	
	//variables to find quotient, remainder and starting point.
	int processor_width,remaining_width,master_start;
	
	processor_width=(data->width)/(data->mpi_procs);  // Calculates the amount of width processed by each processor.
	remaining_width=(data->width)%(data->mpi_procs);  // Calculates the remainder		
	master_start=(data->mpi_procs-1)*processor_width; // Starting point for the master processor
	
	//float* pixels = new float[3 * data->width * data->height];
	
	
    //Render the scene including remainder portions.
    for( int i = 0; i < data->height; ++i )
    {
        for( int j = master_start; j < data->width; ++j )
        {
            int row = i;
            int column = j;

            //Calculate the index into the array.
            int baseIndex = 3 * ( row * data->width + column );
			
			

            //Call the function to shade the pixel.
            shadePixel(&(pixels[baseIndex]),row,j,data);
        }
    } 
	double computationStop = MPI_Wtime();
    double computationTime = computationStop - computationStart;
	
	
	int tag=0;
	MPI_Status status;
	int y;
	
	
	//Receive values from the slaves and render image
	
	double communicationTime = 0.0;
	for(int i=1;i<data->mpi_procs;i++)
	{
	//k=(3*data->height*y+y)+3;
	int c = 3 * data->width * data->height+1;
	
	double communicationStart = MPI_Wtime();

	MPI_Recv(pixels1,c,MPI_FLOAT,i,tag,MPI_COMM_WORLD,&status);
	
	double communicationStop = MPI_Wtime();
	
	double communicationtemp=communicationStop-communicationStart;
	communicationTime=communicationTime+communicationtemp;
			
	int render_start,render_end,processor_width,remaining_width,tag;
	
	
	processor_width=(data->width)/(data->mpi_procs);
	remaining_width=(data->width)%(data->mpi_procs);
	render_start=(i-1)*processor_width;
	render_end=(i)*processor_width;
	
	tag=0;
    //Render the scene
    for( int k = 0; k < data->height; ++k )
    {
        for( int j = render_start; j < render_end; ++j )
        {
			int row = k;
            int column = j;

            //Calculate the index into the array.
            int t = 3 * ( row * data->width + column );

			pixels[t]=pixels1[t];
			pixels[t+1]=pixels1[t+1]; 
			pixels[t+2]=pixels1[t+2];
			
        }
		
    }

		int temp=pixels1[3*(data->height-1)*(render_end-1)+3];
		if(temp>computationTime)
		computationTime=temp;

		//std::cout << "Computation Time: " << computationTime << " seconds" << std::endl;
	} 

    //Stop the comp. timer
   

    //After receiving from all processes, the communication time will
    //be obtained.
    

    //Print the times and the c-to-c ratio
	//This section of printing, IN THIS ORDER, needs to be included in all of the
	//functions that you write at the end of the function.
    std::cout << "Total Computation Time: " << computationTime << " seconds" << std::endl;
    std::cout << "Total Communication Time: " << communicationTime << " seconds" << std::endl;
    double c2cRatio = communicationTime / computationTime;
    std::cout << "C-to-C Ratio: " << c2cRatio << std::endl;
}

void masterDynamic(ConfigData* data, float* pixels)
{

	int nofblocksrow=data->height/data->dynamicBlockHeight;
	int nofblockscol=data->width/data->dynamicBlockWidth;
	
	int remainingrows=data->height-nofblocksrow;
	int remainingcol=data->width-nofblockscol;
	
	int qrow=nofblocksrow*data->dynamicBlockHeight;
	int qcol=nofblockscol*data->dynamicBlockWidth;
	
	int nofblocks=nofblocksrow*nofblockscol;
	
	int arraysize=nofblocks*4;
	int row=0;
	int column=0;
	int *queue=new int[arraysize];
	int k=0;
	
	
	for(int i=0;i<nofblocks;i++)
	{
		MPI_Status status;
		int r=i/nofblockscol;
		int t=i%nofblockscol;
		int rstart=r*data->dynamicBlockHeight;
		int rend=(r+1)*data->dynamicBlockHeight;
		int columnstart=t*data->dynamicBlockWidth;
		int columnstop=(t+1)*data->dynamicBlockWidth;
		int c=(columnstop-columnstart)*(rend-rstart)*3;
		queue[k]=rstart;
		queue[k+1]=rend;
		queue[k+2]=columnstart;
		queue[k+3]=columnstop;
		k=k+4;
		int *pixels1=new int[c];
		int tag=0;
		
		if(i<data->mpi_procs){
		int *q1=new int[4];
		q1[0]=rstart;
		q1[1]=rend;
		q1[2]=columnstart;
		q1[3]=columnstop;
		MPI_Send(q1,4,MPI_INT,0,tag,MPI_COMM_WORLD);
		}
		else{
		MPI_Recv(pixels1,c,MPI_FLOAT,i,tag,MPI_COMM_WORLD,&status);
		int y=status.MPI_SOURCE;
		int *q1=new int[4];
		q1[0]=rstart;
		q1[1]=rend;
		q1[2]=columnstart;
		q1[3]=columnstop;
		MPI_Send(q1,4,MPI_INT,0,tag,MPI_COMM_WORLD);		
		}
		
	}
	
	
}


void masterCyclic(ConfigData* data, float* pixels)
{

	 //Start the computation time timer.
	 
	double computationStart = MPI_Wtime();
	
	float* pixels1 = new float[3 * data->width * data->height];
	
   
   
	
	
	int remaining_height; // Calcutates the remaining height that is to be processed
	int cyclesperprocessor=data->height/(data->mpi_procs * data->cycleSize);// Amount of times the processor is called
	int requiredheight=cyclesperprocessor*data->cycleSize;//the height that will be processed by all processors
	
	remaining_height=(data->height)-(requiredheight*data->mpi_procs);  // Calculates the remainder		
	
	
	
    //Render the scene.
       for( int i = 0; i <cyclesperprocessor; ++i )
    {
		int k=0;
		int rstart = (i*data->mpi_procs*data->cycleSize)+(data->mpi_rank*(data->cycleSize));
		int row=rstart;
	    while(k<(data->cycleSize))
		{
		        for( int j = 0; j < data->width ; ++j)
        		{
					    
	
			
          	  	    
            		int column = j;
					int baseIndex = 3 * ( row * data->width + column );
					 
            		//Call the function to shade the pixel.
            		shadePixel(&(pixels[baseIndex]),row,j,data);
					
					
		
				}
		
			k=k+1;
			row=row+1;
		}	
			
        
    }
	
	int processor_height=data->height/data->mpi_procs; //calcuates remaining height
	
	int t=processor_height*data->mpi_procs;
	
	  
	
	if(remaining_height>0)
	
	{
		//Remaining pixels are rendered
		
		for( int i =(data->height-remaining_height); i < data->height; ++i )
		{
	
			for( int j = 0; j < data->width; ++j )
			{
				int row = i;
				int column = j;

				//Calculate the index into the array.
				int baseIndex = 3 * ( row * data->width + column );
				
				//std::cout << " baseIndex "<<baseIndex << std::endl;

				//Call the function to shade the pixel.
				shadePixel(&(pixels[baseIndex]),row,j,data);
			
				//std::cout << "Computation T Remainder: " << baseIndex<< std::endl;
			}
		} 
	
	}
	
	//Stop the comp. timer
	double computationStop = MPI_Wtime();
    double computationTime = computationStop - computationStart;
	
	
	int tag=0;
	MPI_Status status;
	int y;
	
	
	
	
	double communicationTime = 0.0;
	
	//Renders the values obtained from the slaves
	
	for(int i=1;i < data->mpi_procs;i++)
{
	
	int c = (3 * (data->width) * (requiredheight)+1) ;
	
	double communicationStart = MPI_Wtime();

	MPI_Recv(pixels1,c,MPI_FLOAT,i,tag,MPI_COMM_WORLD,&status);
	
	double communicationStop = MPI_Wtime();
	
	double communicationtemp=communicationStop-communicationStart;
	communicationTime=communicationTime+communicationtemp;
			
	
	
	
	
	
	tag=0;
	int res=0;
    //Render the scene.
    for( int k = 0; k <cyclesperprocessor ; ++k )
    {
		int x=0;
		int rstart = (k*data->mpi_procs*data->cycleSize)+(i*(data->cycleSize));
		int row = rstart;
	while(x<data->cycleSize)
		{
		

			for( int j = 0; j < data->width; ++j )
			{
			
            		int column = j;

            //Calculate the index into the array.
           		 int t = 3 * ( row * data->width + column );

			pixels[t]=pixels1[res];
			pixels[t+1]=pixels1[res+1]; 
			pixels[t+2]=pixels1[res+2];
			res=res+3;		
			
			}
		x=x+1;
		row=row+1;
   		}
		
		
		
   } 
		double temp1=pixels1[c-1]; 
		if(temp1>computationTime)
		computationTime=temp1;

}

   
   

    //After receiving from all processes, the communication time will
    //be obtained.  

    //Print the times and the c-to-c ratio
	//This section of printing, IN THIS ORDER, needs to be included in all of the
	//functions that you write at the end of the function.
    std::cout << "Total Computation Time: " << computationTime << " seconds" << std::endl;
    std::cout << "Total Communication Time: " << communicationTime << " seconds" << std::endl;
    double c2cRatio = communicationTime / computationTime;
    std::cout << "C-to-C Ratio: " << c2cRatio << std::endl;
}


void masterBlock(ConfigData* data, float* pixels)
{

	double computationStart = MPI_Wtime();
	
	int pixels_processed=(data->width)*(data->height)/data->mpi_procs; // Amount of pixels to be processed by each processor
	int height_Block=sqrt(pixels_processed); // Width and height of block
	MPI_Status status;
	double communicationTime; 
	int r=sqrt(data->mpi_procs); // Num of blocks per row or column
	int remaining_height=data->width-(height_Block * r); // reamining height to be processed
	int c=(3 * (height_Block+remaining_height) * (height_Block+remaining_height) )+ 1;

	float* pixels1 = new float[c];
	int k=0;//determines whether the block has to process remainder
	int y=0;//determines whether the block has to process remainder
	if((data->mpi_rank+1)%r==0)
	k=1;
	if((data->mpi_rank+1)>(r*(r-1)))
	y=1;
	int r_start=height_Block*(data->mpi_rank/r);//starting row index
	int r_stop=(height_Block*((data->mpi_rank/r)+1))+(y*(remaining_height));//ending row index
	int c_start=((data->mpi_rank)%r)*height_Block;//starting  column index
	int c_stop=((((data->mpi_rank)%r)+1)*height_Block)+(k*(remaining_height));//ending column index
	
	//std::cout<<"pixels_processed:"<<pixels_processed<<" height of block:"<<height_Block<<" Remaining_height:"<<remaining_height<<"r:"<<r<<std::endl;
	
	//std::cout<<"Rank:"<<data->mpi_rank<<" r start:"<<r_start<<" r stop:"<<r_stop<<" c start:"<<c_start<<" c stop:"<<c_stop<<std::endl; 
	
	//render the image
	for(int i=r_start;i<r_stop;i++)
	{
		for(int j=c_start;j<c_stop;j++)
		{
		
			int row = i;
            int column = j;
			
			int baseIndex = 3 * ( row * data->width + column ); 
		   //std::cout<<"base index:"<<baseIndex<<std::endl;
		   
			
			shadePixel(&(pixels[baseIndex]),row,j,data);
		}
	}
	
	double computationStop = MPI_Wtime();
    double computationTime = computationStop - computationStart;
	
	//render image using values from slaves
	for(int i=1;i<data->mpi_procs;i++)
	{
	int k=0;
	int y=0;
	if((i+1)%r==0)
	k=1;
	if((i+1)>(r*(r-1)))
	y=1;
	int r1_start=height_Block*(i/r);
	int r1_stop=(height_Block*((i/r)+1))+(y*(remaining_height));
	int c1_start=((i)%r)*height_Block;
	int c1_stop=((((i)%r)+1)*height_Block)+(k*(remaining_height));
	int tag=0;
		
	double communicationStart = MPI_Wtime();

	
	MPI_Recv(pixels1,c,MPI_FLOAT,i,tag,MPI_COMM_WORLD,&status);
	//std::cout<<"I:"<<i<<" r start:"<<r_start<<" r stop:"<<r_stop<<" c start:"<<c_start<<" c stop:"<<c_stop<<std::endl; 
	std::cout<<"I:"<<i<<" value"<<pixels1[c-1]<<" c:"<<c<<std::endl;
	
	double communicationStop = MPI_Wtime();
	
	double communicationtemp=communicationStop-communicationStart;
	communicationTime=communicationTime+communicationtemp;
	
	int res=0;
	
		for(int p=r1_start;p<r1_stop;p++)
		{
			for(int j=c1_start;j<c1_stop;j++)
			{
		
			int row = p;
            int column = j;
			
			int t = 3 * ( row * data->width + column );
			
			//std::cout<<" Row:"<<row<<" Column:"<<column<<std::endl;
			//std::cout<<"base index:"<<t<<std::endl;
			//std::cout<<"res:"<<res<<std::endl;
			
			pixels[t]=pixels1[res];
			pixels[t+1]=pixels1[res+1];
			pixels[t+2]=pixels1[res+2];
			res=res+3;
			
			}
	
	
	
	
	
		}
	float temp=pixels1[c-1];
	
	//std::cout<<"I:"<<i<<" value"<<pixels1[c]<<" c:"<<c<<std::endl;
	if(temp>computationTime)
	computationTime=temp;

	
	
	
	}
	std::cout << "Total Computation Time: " << computationTime << " seconds" << std::endl;
    std::cout << "Total Communication Time: " << communicationTime << " seconds" << std::endl;
    double c2cRatio = communicationTime / computationTime;
    std::cout << "C-to-C Ratio: " << c2cRatio << std::endl;
}


