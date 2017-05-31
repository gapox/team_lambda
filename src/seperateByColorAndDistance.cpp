#include <ros/ros.h>

#include <pcl_conversions/pcl_conversions.h>
#include <pcl/point_types.h>
#include <pcl/filters/voxel_grid.h>
#include <sensor_msgs/PointCloud2.h>
#include <pcl/ModelCoefficients.h>
#include <pcl/sample_consensus/method_types.h>
#include <pcl/sample_consensus/model_types.h>
#include <pcl/segmentation/sac_segmentation.h>
#include <pcl/filters/extract_indices.h>

ros::Publisher pubR;
ros::Publisher pubG;
ros::Publisher pubB;
ros::Publisher pubY;
ros::Publisher pubK;

int getCol(int r, int g, int b){
	int lab;
	if(r*0.5>g && r*0.5>b){
		//RED OBJECT
		lab=1;
	}else if(g*0.8>r && g*0.8>b){
		//GREEN OBJECT
		lab=2;
	}else if(b*0.6>r && b*0.6>g){
		//BLUE OBJECT
		lab=3;
	}else if(r*0.5>b && g*0.5>b){
		//YELLOW OBJECT
		lab=4;
		
	}else if(r<10 && g<10 && b<10){
		//BLACK OBJECT
		lab=0;
	}else{
		lab=-1;
	}
	return lab;
}



void callback(const pcl::PCLPointCloud2ConstPtr& cloud_blob) {
	pcl::PCLPointCloud2::Ptr cloud_filtered_blob (new pcl::PCLPointCloud2);
	pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud_filtered (new pcl::PointCloud<pcl::PointXYZRGB>);
	pcl::PointCloud<pcl::PointXYZRGB>::Ptr colorR (new pcl::PointCloud<pcl::PointXYZRGB>);
	pcl::PointCloud<pcl::PointXYZRGB>::Ptr colorG (new pcl::PointCloud<pcl::PointXYZRGB>);
	pcl::PointCloud<pcl::PointXYZRGB>::Ptr colorB (new pcl::PointCloud<pcl::PointXYZRGB>);
	pcl::PointCloud<pcl::PointXYZRGB>::Ptr colorY (new pcl::PointCloud<pcl::PointXYZRGB>);
	pcl::PointCloud<pcl::PointXYZRGB>::Ptr colorK (new pcl::PointCloud<pcl::PointXYZRGB>);
	pcl::PCLPointCloud2::Ptr cloud_colorR (new pcl::PCLPointCloud2);
	pcl::PCLPointCloud2::Ptr cloud_colorG (new pcl::PCLPointCloud2);
	pcl::PCLPointCloud2::Ptr cloud_colorB (new pcl::PCLPointCloud2);
	pcl::PCLPointCloud2::Ptr cloud_colorY (new pcl::PCLPointCloud2);
	pcl::PCLPointCloud2::Ptr cloud_colorK (new pcl::PCLPointCloud2);

	// Create the filtering object: downsample the dataset using a leaf size of 1cm

	pcl::fromPCLPointCloud2 (*cloud_blob, *cloud_filtered);

	pcl::PointCloud<pcl::PointXYZRGB>::iterator b1;
	for (b1 = cloud_filtered->points.begin(); b1 < cloud_filtered->points.end(); b1++){
		int label=getCol(b1->r,b1->g,b1->b);
		if(label==-1)continue;
		pcl::PointXYZRGB newpoint;
		newpoint.x = b1->x;
		newpoint.y = b1->y;
		newpoint.z = b1->z;
		newpoint.r = b1->r;
		newpoint.g = b1->g;
		newpoint.b = b1->b;
		switch(label){
			case 0:colorK->push_back(newpoint);
			break;
			case 1:colorR->push_back(newpoint);
			break;
			case 2:colorG->push_back(newpoint);
			break;
			case 3:colorB->push_back(newpoint);
			break;
			case 4:colorY->push_back(newpoint);
			break;
			default:printf("Unknown point cloud color label!\n");
			break;
		}
	}
	pcl::toPCLPointCloud2(*colorR, *cloud_colorR);
	pcl::toPCLPointCloud2(*colorG, *cloud_colorG);
	pcl::toPCLPointCloud2(*colorB, *cloud_colorB);
	pcl::toPCLPointCloud2(*colorY, *cloud_colorY);
	pcl::toPCLPointCloud2(*colorK, *cloud_colorK);
	cloud_colorR->header.frame_id= cloud_blob->header.frame_id;
	cloud_colorG->header.frame_id= cloud_blob->header.frame_id;
	cloud_colorB->header.frame_id= cloud_blob->header.frame_id;
	cloud_colorY->header.frame_id= cloud_blob->header.frame_id;
	cloud_colorK->header.frame_id= cloud_blob->header.frame_id;
	pubR.publish(*cloud_colorR);
	pubG.publish(*cloud_colorG);
	pubB.publish(*cloud_colorB);
	pubY.publish(*cloud_colorY);
	pubK.publish(*cloud_colorK);

}



int main (int argc, char** argv) {

  // Initialize ROS
  ros::init (argc, argv, "seperate");
  ros::NodeHandle nh;

  // Create a ROS subscriber for the input point cloud
  ros::Subscriber sub = nh.subscribe ("input", 1, callback);

  // Create a ROS publisher for the output point cloud
  pubR = nh.advertise<sensor_msgs::PointCloud2> ("pclR", 1);
  pubG = nh.advertise<sensor_msgs::PointCloud2> ("pclG", 1);
  pubB = nh.advertise<sensor_msgs::PointCloud2> ("pclB", 1);
  pubY = nh.advertise<sensor_msgs::PointCloud2> ("pclY", 1);
  pubK = nh.advertise<sensor_msgs::PointCloud2> ("pclK", 1);

  
  ros::Rate r(5);
  // Spin
  ros::spin ();
  
 
}
