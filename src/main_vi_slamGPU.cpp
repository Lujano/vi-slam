#include <iostream>
#include <fstream>
#include "DataReader.hpp"
#include "Visualizer.hpp"
#include "VISystemGPU.hpp"
#include "opencv2/cudafeatures2d.hpp"
#include "opencv2/xfeatures2d/cuda.hpp"
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/calib3d.hpp"
#include <ctime>

#include <ros/ros.h>
#include <ros/console.h>

#include <std_msgs/Int32.h>
#include <geometry_msgs/Vector3.h>
#include <geometry_msgs/Quaternion.h>

using namespace cv;
using namespace std;
using namespace vi;
cuda::DeviceInfo device_info;

int main( int argc, char** argv ){
    const String keys =
        "{help h usage ? |      | print this message   }"
        "{gtFile     |       |  groundtruth file} "
        "{imagesPath   |       | data image directory} "
        "{imuFile   |       | data imu directory} "
        "{calibrationFile   |       | settings} "
        "{outputFile   |       | outputFile} ";

    cv::CommandLineParser parser(argc, argv, keys);
        if (parser.has("help"))
        {
            cout<< "parse the file path"<<endl;
            return 0;
        }
    cout << "===================================================" << endl;
    int n_cuda_devices = cuda::getCudaEnabledDeviceCount();
    if (n_cuda_devices > 0) {
        cuda::setDevice(0);
    } else {
        cout << "No CUDA device detected" << endl;
        cout << "Exiting..." << endl;
        return -1;
    }

    string gtFile = parser.get<string>("gtFile");
    string imagesPath = parser.get<string>("imagesPath");
    string imuFile = parser.get<string>("imuFile");
    string calibrationFile = parser.get<string>("calibrationFile");
    string outputFile = parser.get<string>("outputFile");
    char separator = ',';

    // Leer Dataset
    cout << "Gt file "<< imuFile<<endl;
    DataReader Data(imagesPath, imuFile, gtFile, separator);

       
    int j = 210;
    Data.UpdateDataReader(j-1, j);
    VISystemGPU visystem(argc, argv);
    visystem.InitializeSystemGPU( calibrationFile, Data.gtPosition[0], Data.gtLinearVelocity[0], Data.gtRPY[0], Data.image1);
    cout << "Initializate System"<<endl;
    Quaterniond qinit = toQuaternion(Data.gtRPY[0].x, Data.gtRPY[0].y, Data.gtRPY[0].z);

    //VisualizerVector3 rqt_error("error", 10.0);
    
    //
    //VisualizerMarker visualizer_gtIMU("gtIMU_poses", "/my_frame", 2000, ARROW, 0, Point3f(0.5, 0.5, 0.5),Point3f(0.0, 0.0, 1.0)); //azul
    VisualizerMarker visualizer_gtCam("gtCam_poses", "/my_frame", 2000, ARROW, 0, Point3f(0.5, 0.5, 0.5),Point3f(0.0, 1.0, 0.0));
    //VisualizerMarker visualizer_estIMU("estIMU_poses", "/my_frame", 2000, ARROW, 0, Point3f(0.5, 0.5, 0.5),Point3f(1.0, 0.0, 0.0));
    VisualizerMarker visualizer_estCam("estCam_poses", "/my_frame", 2000, ARROW, 0, Point3f(0.5, 0.5, 0.5),Point3f(0.5, 0.5, 0.5));

    //VisualizerMarker visualizer_est("est_poses", "/my_frame", 2000, ARROW, 0, Point3f(1.0, 1.0, 1.0),Point3f(0.0, 1.0, 0.0));
    //VisualizerFrame visualizerFrame("current_frame", 90);
    //VisualizerFrame visualizerFrame2("current_frame2", 90);
    //VisualizerVector3 velocidad_groundtruth("velocidad_groundtruth", 1200);
    //VisualizerVector3 velocidad_estimado("velocidad_estimado", 1200);
    //VisualizerVector3 error_angular ("error_angular", 1200);
    //VisualizerVector3 error_velocidad ("error_velocidad", 1200);
    //VisualizerVector3 error_posicion ("error_posicion", 1200);
    //VisualizerVector3 posicion_estimada ("posicion_estimada", 1200);
    //VisualizerVector3 posicion_groundtruth ("posicion_gt", 1200);
    //VisualizerVector3 angulo_estimado ("angulo_estimado", 1200);
   // VisualizerVector3 residual_angEst ("residual_angEst", 1200);
    //VisualizerVector3 residual_angGt ("residual_angGt", 1200);
    //VisualizerVector3 angulo_groundtruth ("angulo_groundtruth",1200);
    Mat frame1;
    Mat frame2;
    Point3f error;
    double position[3];
    double orientation[4];

    double position2[3];
    double orientation2[4];

   
   
    vector<KeyPoint> vectorMatches;
    Quaterniond qGt_initial;
    
    //Gt para camara 
    Quaterniond qOrientationCamGT;
    Point3d RPYOrientationCamGT;
    Point3d positionCamGT;




    std::ofstream outputFilecsv;

    Point3d zero;
    
    outputFilecsv.open("/home/lujano/Documents/outputVISlam.csv", std::ofstream::out | std::ofstream::trunc);
    while(j <Data.indexLastData)
    {  // Cambiar por constant
        Mat finalImage, finalImage2;
        Data.UpdateDataReader(j, j+1);
        j = j+1;
        visystem.AddFrameGPU(Data.image2, Data.imuAngularVelocity, Data.imuAcceleration);
       
        positionCamGT = Data.gtPosition.back()+visystem.imu2camTranslation;
        RPYOrientationCamGT =rotationMatrix2RPY(visystem.imu2camRotation*RPY2rotationMatrix(toRPY(Data.gtQuaternion.back()) ));
        qOrientationCamGT = toQuaternion(RPYOrientationCamGT.x, RPYOrientationCamGT.y, RPYOrientationCamGT.z);



         //visualizer_gtIMU.UpdateMessages(zero, Data.gtQuaternion.back());
         visualizer_gtCam.UpdateMessages(zero, qOrientationCamGT);
         //visualizer_estIMU.UpdateMessages(zero, visystem.qOrientationImu);
         visualizer_estCam.UpdateMessages(zero, visystem.qOrientationCam);
         
         cout<< " Current time = "<< Data.currentTimeMs <<" ms " <<endl;
        
        outputFilecsv <<  visystem.positionCam.x<<","
        <<visystem.positionCam.y<<","
        <<visystem.positionCam.z<<","
        <<visystem.qOrientationCam.x <<","
        <<visystem.qOrientationCam.y <<","
        <<visystem.qOrientationCam.z <<","
        <<visystem.qOrientationCam.w <<","
        <<  positionCamGT.x <<","
        <<  positionCamGT.y <<","
        <<  positionCamGT.z <<","
        <<  qOrientationCamGT.x <<","        
        <<  qOrientationCamGT.y <<","
        <<  qOrientationCamGT.z <<","
        <<  qOrientationCamGT.w
        <<endl;
        
       /*
        outputFilecsv <<  visystem.positionImu.x<<","
        <<visystem.positionImu.y<<","
        <<visystem.positionImu.z<<","
        <<visystem.qOrientationImu.x <<","
        <<visystem.qOrientationImu.y <<","
        <<visystem.qOrientationImu.z <<","
        <<visystem.qOrientationImu.w <<","
        <<  Data.gtPosition.back().x <<","
        <<  Data.gtPosition.back().y <<","
        <<  Data.gtPosition.back().z <<","
        <<  Data.gtQuaternion.back().x <<","        
        <<  Data.gtQuaternion.back().y <<","
        <<  Data.gtQuaternion.back().z <<","
        <<  Data.gtQuaternion.back().w
        <<endl;
        */


        //visualizer_est.UpdateMessages(position2, orientation2);
            
            

                
            //vector3d.UpdateMessages(acc_diff);
                
        }
        /*
        //cout << "tamData"<< Data.gtRPY.size()<<endl;
        Point3d error_ang_est, error_ang_gt, error_ang;
        Point3d error_vel_est, error_vel_gt, error_vel;
        Point3d error_pos_est, error_pos_gt, error_pos;
        error_ang_est.x = imuCore.rpyAnglesWorld[imuCore.quaternionWorld.size()-1].x-imuCore.rpyAnglesWorld[0].x;
        error_ang_est.y = imuCore.rpyAnglesWorld[imuCore.quaternionWorld.size()-1].y-imuCore.rpyAnglesWorld[0].y;
        error_ang_est.z = imuCore.rpyAnglesWorld[imuCore.quaternionWorld.size()-1].z-imuCore.rpyAnglesWorld[0].z;
        
        error_ang_gt.x = Data.gtRPY[Data.gtRPY.size()-1].x-Data.gtRPY[0].x;
        error_ang_gt.y = Data.gtRPY[Data.gtRPY.size()-1].y-Data.gtRPY[0].y;
        error_ang_gt.z = Data.gtRPY[Data.gtRPY.size()-1].z-Data.gtRPY[0].z;

        error_ang = error_ang_est-error_ang_gt;

        //Velocidad
        //velocity = velocity+imuCore.velocity;
       // velocidad_groundtruth.UpdateMessages(Data.gtLinearVelocity[0]);
       // velocidad_estimado.UpdateMessages(velocity);
        error_vel_gt = Data.gtLinearVelocity[Data.gtLinearVelocity.size() -1]-Data.gtLinearVelocity[0];
        error_vel_est = imuCore.velocity;
        error_vel =  error_vel_gt - error_vel_est;
        int n = Data.gtPosition.size()-1;
        error_pos_est = imuCore.position;
        error_pos_gt =  Data.gtPosition[Data.gtPosition.size()-1] - Data.gtPosition[0] ; 
        //cout << "Size" <<Data.gtPosition.size() <<endl;
        error_pos = error_pos_est-error_pos_gt;
        int size = Data.gtPosition.size();
        if (size > 10) size = 10;
        for (int jj = 0; jj < size ; jj++)
        {
           accx << imuCore.accelerationWorld[jj].x<<endl;
           accy << imuCore.accelerationWorld[jj].y<<endl;
           accz << imuCore.accelerationWorld[jj].z<<endl;
           angxGt << Data.gtRPY[jj].x<<endl;
           angyGt << Data.gtRPY[jj].y<<endl;
           angzGt << Data.gtRPY[jj].z<<endl;
           angxEst << imuCore.rpyAnglesWorld[jj].x<<endl;
           angyEst << imuCore.rpyAnglesWorld[jj].y<<endl;
           angzEst << imuCore.rpyAnglesWorld[jj].z<<endl;
           biasx << Data.accBias[jj].x<<endl;
           biasy << Data.accBias[jj].y<<endl;
           biasz << Data.accBias[jj].z<<endl;
           velx << Data.gtLinearVelocity[jj].x << endl;
           vely << Data.gtLinearVelocity[jj].y << endl;
           velz << Data.gtLinearVelocity[jj].z << endl;
           posx << Data.gtPosition[jj].x <<endl;
           posy << Data.gtPosition[jj].y <<endl;
           posz << Data.gtPosition[jj].z <<endl;

        }

        
       */


	/*
	if(error_vel_gt.x>0.01)
		error_vel.x = error_vel.x/error_vel_gt.x*100;
	else
		error_vel.x = 0.0;
	if(error_vel_gt.x>0.01)
		error_vel.y = error_vel.y/error_vel_gt.y*100;
	else
		error_vel.y = 0.0;
	if(error_vel_gt.x>0.01)
		error_vel.z = error_vel.z/error_vel_gt.z*100;
	else
		error_vel.z = 0.0;
*/
        //error_angular.UpdateMessages(error_ang*180/M_PI);
        //error_velocidad.UpdateMessages(error_vel);
        //error_posicion.UpdateMessages(error_pos);
       
        //posicion_estimada.UpdateMessages(error_pos_est);
        //posicion_groundtruth.UpdateMessages(error_pos_gt);



        
        //visualizer_gt.UpdateMessages(position, orientation);
        //visualizer_est.UpdateMessages(position, orientation2);

       
        /*
        cout << " diffx = " << imuCore.accelerationWorld[9].x
        <<" diffy = "<<imuCore.accelerationWorld[9].y
        <<" diffz = "<<imuCore.accelerationWorld[9].z
        << " Current time = "<< Data.currentTimeMs <<" ms "
        << "accx = "  << Data.accBias.x
        << "timestep = "<< imuCore.timeStep

        
        
	<<endl;*/
      


        

        
        
      
       

    
    




    return 0;

}

// COMPILE COMMAND
//g++ -g main_vi_slam.cpp GroundTruth.cpp -o main_vi_slam.out `pkg-config opencv --cflags --libs`

// RUN COMMAND
//./main_vi_slam.out -filePath=../../../../Documents/EuroDataset/state_groundtruth_estimate0/data.csv
// rosrun vi_slam vi_slam -filePath=../../../../Documents/EuroDataset/state_groundtruth_estimate0/data.csv 
