#include <ros/ros.h>

#include "std_msgs/MultiArrayLayout.h"
#include "std_msgs/MultiArrayDimension.h"
#include <sensor_msgs/image_encodings.h>

#include "std_msgs/Float64MultiArray.h"
#include <detection_msgs/Detection.h>


int siz=50;
int sts=3;
int descLen=3*sts*sts;
double** createSmaller(int **orgF, int orgX, int orgY){
	double **smlF=new double* [siz];
	int **ctr = new int* [siz];
	double stpX=(siz*1.0)/(orgX*1.0);
	double stpY=(siz*1.0)/(orgY*1.0);
	double x=0;
	for(int i=0;i<orgX; i++){
		smlF[(int)x]=new double [siz];
		ctr[(int)x]=new int [siz];
		double y=0;
		for(int j=0;j<orgY;j++){
			smlF[(int)x][(int)y]+=orgF[i][j];
			ctr[(int)x][(int)y]++;
			y+=stpY;
		}
		x+=stpX;
	}
	for(int i=0;i<siz; i++){
		for(int j=0;j<siz;j++){
			//printf("%f ",smlF[i][j]);fflush(stdout);
			smlF[i][j]/=(ctr[i][j]+1);
		}
		//printf("\n");fflush(stdout);
	}
	return smlF;
}
double ** blur(double **smlF){
	double **bl=new double* [siz];
	for(int i=0;i<siz; i++){
		bl[i]=new double [siz];
		for(int j=0;j<siz;j++){
			if(i==0 || j==0 || i==siz-1 || j==siz-1)
				bl[i][j]=smlF[i][j];
			else{
				bl[i][j]= smlF[i-1][j-1]/16.0 + smlF[i][j-1]/8.0+smlF[i+1][j-1]/16.0+smlF[i-1][j]/8.0+smlF[i][j]/4.0+smlF[i+1][j]/8.0+smlF[i-1][j+1]/16.0+smlF[i][j+1]/8.0+smlF[i+1][j+1]/16.0;
			}
		}
	}
	return bl;
}

void normalize(double **smlF){
	double min=256;
	double max=0;
	for(int i=0;i<siz; i++)
		for(int j=0;j<siz;j++)
			smlF[i][j]/=255;
	
	double diff=(max-min);
	for(int i=0;i<siz; i++){
		for(int j=0;j<siz;j++){
			smlF[i][j]=((smlF[i][j]-min)/diff);
		}
	}
}
void norm(double **smlF){
	double min=256;
	double max=0;
	for(int i=0;i<siz; i++){
		for(int j=0;j<siz;j++){
			if(smlF[i][j]<min)min=smlF[i][j];
			if(smlF[i][j]>max)max=smlF[i][j];
		}
	}
	double diff=(max-min);
	for(int i=0;i<siz; i++){
		for(int j=0;j<siz;j++){
			smlF[i][j]=((smlF[i][j]-min)/diff);
		}
	}
}
double ** diff(double **a, double **b){
	double **bl=new double* [siz];
	for(int i=0;i<siz; i++){
		bl[i]=new double [siz];
		for(int j=0;j<siz;j++){
			bl[i][j]=a[i][j]-b[i][j];
		}
	}
	return bl;
}
std_msgs::Float64MultiArray getDescriptorVector(double **smlF){
	std_msgs::Float64MultiArray array;
	array.data.clear();
	int stp=siz/sts;
	for(int i=0;i<sts; i++){
		for(int j=0;j<sts;j++){
			double avg=0;
			double dx=0;
			double dy=0;
			for(int k=i*stp+1;k<(i+1)*stp;k++){
				for(int l=j*stp+1;l<(j+1)*stp;l++){
					avg+=smlF[k][l];
					dx+=smlF[k][l]-smlF[k-1][l];
					dy+=smlF[k][l]-smlF[k][l-1];
				}
			}
			avg/=((stp-1)*(stp-1));
			dx/=((stp-1)*(stp-1));
			dy/=((stp-1)*(stp-1));
			array.data.push_back(avg);
			array.data.push_back(dx);
			array.data.push_back(dy);
			
		}
	}
	array.layout.data_offset=3*descLen;
	return array;
}


ros::Publisher pub;
void callback(const detection_msgs::DetectionConstPtr  det) {
	sensor_msgs::Image image=det->image;
	int sizeX=det->width;
	int sizeY=det->height;
	printf("Detekcija velikosti:\nw:%d\nh:%d\n",sizeX,sizeY);
	int **chR = new int* [sizeX];
	int **chG = new int* [sizeX];
	int **chB = new int* [sizeX];
	for (int x = 0; x < sizeX; x++){
		chR[x] = new int [sizeY];
		chG[x] = new int [sizeY];
		chB[x] = new int [sizeY];
		for (int y = 0; y < sizeY; y++){
			int index = ((sizeX * y) + x)*3;
			chR[x][y] = image.data[index];
			chG[x][y] = image.data[index+1];
			chB[x][y] = image.data[index+2];
		}
	}
	double **smlR=createSmaller(chR, sizeX, sizeY); 
	normalize(smlR);
	norm(smlR);
	smlR=blur(smlR);
	std_msgs::Float64MultiArray descR=getDescriptorVector(smlR);
	
	double **smlG=createSmaller(chG, sizeX, sizeY); 
	normalize(smlG);
	norm(smlG);
	smlG=blur(smlG);
	std_msgs::Float64MultiArray descG=getDescriptorVector(smlG);
	
	double **smlB=createSmaller(chR, sizeX, sizeY); 
	normalize(smlB);
	norm(smlB);
	smlB=blur(smlB);
	std_msgs::Float64MultiArray descB=getDescriptorVector(smlB);
	
	double **diffRG=diff(smlR, smlG);
	norm(diffRG);
	diffRG=blur(diffRG);
	std_msgs::Float64MultiArray descRG=getDescriptorVector(diffRG);
	
	double **diffGB=diff(smlG, smlB); 
	norm(diffGB);
	diffGB=blur(diffGB);
	std_msgs::Float64MultiArray descGB=getDescriptorVector(diffGB);
	
	double **diffBR=diff(smlB, smlR); 
	norm(diffBR);
	diffRG=blur(diffBR);
	std_msgs::Float64MultiArray descBR=getDescriptorVector(diffBR);
	std_msgs::Float64MultiArray desc;
	for(int i=0;i<descLen;i++){
		desc.data.push_back(descR.data[i]);
		desc.data.push_back(descG.data[i]);
		desc.data.push_back(descB.data[i]);
		desc.data.push_back(descRG.data[i]);
		desc.data.push_back(descGB.data[i]);
		desc.data.push_back(descBR.data[i]);
	}
	
		
	printf(" Velikosti:%d\n\n", desc.layout.data_offset);
	pub.publish(desc);
}


int main (int argc, char** argv) {

	// Initialize ROS

	ros::init (argc, argv, "descriptorMaker");
	ros::NodeHandle nh;

	// Create a ROS subscriber for the input point cloud
	ros::Subscriber sub = nh.subscribe ("faceImg", 1, callback);

	// Create a ROS publisher for the output point cloud
	pub = nh.advertise<std_msgs::Float64MultiArray> ("faceDesc", 1);


	ros::Rate r(5);
	// Spin
	ros::spin ();


	}
