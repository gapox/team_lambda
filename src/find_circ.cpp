#include <ros/ros.h>

#include <pcl_conversions/pcl_conversions.h>
#include <pcl/point_types.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/ModelCoefficients.h>
#include <pcl/sample_consensus/method_types.h>
#include <pcl/sample_consensus/model_types.h>
#include <pcl/segmentation/sac_segmentation.h>
#include <pcl/filters/extract_indices.h>
#include <visualization_msgs/Marker.h>


char *namespaces[]={"undefined","red", "green", "blue", "yellow", "black"};
int ids[]={0,1,2,3,4,5};
int thisNode=0;
ros::Publisher pub;
void callback(const pcl::PCLPointCloud2ConstPtr& cloud_blob) {
  pcl::PCLPointCloud2::Ptr cloud_filtered_blob (new pcl::PCLPointCloud2);
  pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud_filtered (new pcl::PointCloud<pcl::PointXYZRGB>);
  pcl::PointCloud<pcl::PointXYZRGB>::Ptr color_filtered (new pcl::PointCloud<pcl::PointXYZRGB>);
  pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud_outliers (new pcl::PointCloud<pcl::PointXYZRGB>);

  // Create the filtering object: downsample the dataset using a leaf size of 1cm
  pcl::fromPCLPointCloud2 (*cloud_blob, *cloud_filtered);
	
	pcl::ModelCoefficients::Ptr coefficients (new pcl::ModelCoefficients ());
	pcl::PointIndices::Ptr inliers (new pcl::PointIndices ());
	// Create the segmentation object
	pcl::SACSegmentation<pcl::PointXYZRGB> seg;
	// Optional
	seg.setOptimizeCoefficients (true);
	// Mandatory
	seg.setModelType (pcl::SACMODEL_CIRCLE3D);
	seg.setMethodType (pcl::SAC_RANSAC);
	seg.setMaxIterations (1000);
	seg.setDistanceThreshold (0.02);
	seg.setRadiusLimits(0.04,0.1);
	seg.setInputCloud (cloud_filtered);

	seg.segment(*inliers, *coefficients);

	if (inliers->indices.size () == 0) return;
	if(coefficients->values[1]<-0.28 || coefficients->values[1]>-0.08) return;
	if(sqrt(coefficients->values[1]*coefficients->values[1]+coefficients->values[0]*coefficients->values[0]+coefficients->values[2]*coefficients->values[2])>2) return;//so far away from me
	printf("%s Center: %f, %f, %f\n",namespaces[thisNode],coefficients->values[0],coefficients->values[1],coefficients->values[2]);
	
	pcl::ExtractIndices<pcl::PointXYZRGB> extract;

	extract.setInputCloud(cloud_filtered);
	extract.setIndices(inliers);
	//remove the plane
	//extract.setNegative(true);
	//show the plane only
	extract.setNegative(false);
	extract.filter(*cloud_outliers);

	
	
	visualization_msgs::Marker marker;
	marker.header.frame_id = "/base_link";
	marker.header.stamp = ros::Time::now();
	marker.ns = namespaces[thisNode];
	marker.id = ids[thisNode];
	marker.type = visualization_msgs::Marker::SPHERE;
	marker.action = visualization_msgs::Marker::ADD;
	marker.pose.position.x = (double)coefficients->values[2]-0.1;//KOLIKO PRED NJIM
	marker.pose.position.y = (double)-coefficients->values[0];//KOLIKO LEVO OD NJEGA
	marker.pose.position.z = (double)-coefficients->values[1]+0.3;//KOLIKO NAD NJIM
	marker.pose.orientation.x = 0.0;
	marker.pose.orientation.y = 0.0;
	marker.pose.orientation.z = 0.0;
	marker.pose.orientation.w = 1.0;
	marker.scale.x = 0.05;
	marker.scale.y = 0.05;
	marker.scale.z = 0.05;
	marker.color.a = 1.0; // Don't forget to set the alpha!
	marker.color.r = 0.1;
	marker.color.g = 0.1;
	marker.color.b = 1.0;
	
	pub.publish(marker);
}



int main (int argc, char** argv) {

	// Initialize ROS
	if(argc >1){
		if(streq(argv[1], "red")==0)
			thisNode=1;
		else if(streq(argv[1], "green")==0)
			thisNode=2;
		else if(streq(argv[1], "blue")==0)
			thisNode=3;
		else if(streq(argv[1], "yellow")==0)
			thisNode=4;
		else if(streq(argv[1], "black")==0)
			thisNode=5;
		else
			thisNode=0;
		printf("Started node! Its color is:%s\n", namespaces[thisNode]);
	}
	else
		thisNode=0;
	ros::init (argc, argv, "find_circ");
	ros::NodeHandle nh;

	// Create a ROS subscriber for the input point cloud
	ros::Subscriber sub = nh.subscribe ("input", 1, callback);

	// Create a ROS publisher for the output point cloud
	pub = nh.advertise<visualization_msgs::Marker> ("marker", 1);


	ros::Rate r(5);
	// Spin
	ros::spin ();


	}