#include <iostream>
#include <fstream>
#include "DataReader.hpp"
#include "Visualizer.hpp"
#include "Camera.hpp"
#include "Imu.hpp"
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

int main( int argc, char** argv ){
    
    const String keys =
        "{help h usage ? |      | print this message   }"
        "{filePath     |       | data input directory} "
        "{imagePath   |       | data image directory} "
        "{imuPath   |       | data imu directory} ";

    cv::CommandLineParser parser(argc, argv, keys);
        if (parser.has("help"))
        {
            cout<< "parse the file path"<<endl;
            return 0;
        }
    string filePath = parser.get<string>("filePath");
    string imagePath = parser.get<string>("imagePath");
    string imuPath = parser.get<string>("imuPath");
    char separator = ',';

    cout<<"File "<<filePath<<endl;

    DataReader Data(imagePath, imuPath, filePath, separator);

    /*
    GroundTruth groundTruth(filePath, separator);

    cout<< groundTruth.getFileName()<<"\n"
    << groundTruth.getCharSeparator()<<"\n"
    <<"número de filas"<<groundTruth.getRows()
    <<"número de columnas"<<groundTruth.getCols()<< endl;

    cout <<"Data ="<< groundTruth.getGroundTruthData(0, 1)<<endl;;
    ImageReader imageReader(imagePath);
    imageReader.searchImages();
    */

    ros::init(argc, argv, "vi_slam");  // Initialize ROS

    //VisualizerVector3 rqt_error("error", 10.0);
   VisualizerMarker visualizer_gt("gt_poses", "/my_frame", 2000, ARROW, 0, Point3f(1.0, 1.0, 1.0),Point3f(0.0, 0.0, 1.0));
   VisualizerMarker visualizer_est("est_poses", "/my_frame", 2000, ARROW, 0, Point3f(1.0, 1.0, 1.0),Point3f(0.0, 1.0, 0.0));
    VisualizerFrame visualizerFrame("current_frame", 90);
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
    orientation[3] = 1.0;
    int min_points;

    Imu imuCore(Data.timeStepImu/1000000000.0);

    //-- Paso 4: Calcular la matriz Esencial
    // Parametros intrisecos de la camara
    double fx, fy, focal, cx, cy;
    fx = 458.654;
    fy = 457.296;
    cx =  367.215;
    cy = 248.375;
    /*
    fx = 7.188560000000e+02;

    fy = 7.188560000000e+02;
    cx =  6.071928000000e+02;
    cy =  1.852157000000e+02;
    */
    focal = fx;
    Mat E, R, t; // matriz esencial

    std::vector<Point2f> points1_OK, points2_OK; // Puntos finales bajos analisis
    std::vector<Point2f> aux_points1_OK, aux_points2_OK;
    vector<int> point_indexs;
    Mat odometry = Mat::zeros(1, 3, CV_64F); // Matriz vacia de vectores de longitud 3 (comienza con 000)
    Mat R_p ; // matriz de rotacion temporal
    Mat traslation; 

    int i = 0;
    vector<KeyPoint> vectorMatches;
    Quaterniond qGt_initial;

    Data.UpdateDataReader(0, 1);
    int n = Data.gtPosition.size()-1;
    position2[0] = Data.gtPosition[n].x;   //x
    position2[1] = Data.gtPosition[n].y;   //x
    position2[2] = Data.gtPosition[n].z;
    qGt_initial.x = Data.gtQuaternion[n].x;   //x
    qGt_initial.y = Data.gtQuaternion[n].y; // y
    qGt_initial.z = Data.gtQuaternion[n].z; // z
    qGt_initial.w = Data.gtQuaternion[n].w; // w
    Point3d angle_gt_initial = toRPY(qGt_initial);
    imuCore.setImuData(Data.imuAngularVelocity, Data.imuAcceleration); // primeras medidas
    imuCore.initializate(angle_gt_initial.z); // Poner yaw inicial del gt
    imuCore.setImuInitialVelocity(Data.gtLinearVelocity[0]);
    imuCore.estimate(Data.gtRPY);
    
    Point3d velocity ;//= Data.gtLinearVelocity[9];
    velocity = Data.gtLinearVelocity[0];
    velocity.x = 0.0;
    velocity.y = 0.0;
    velocity.z = 0.0;
    min_points = 250;


    std::ofstream accx ("/home/lujano/Documents/Reporte/Testfiltraje/Procesamiento/Filtraje/accx.out");
    std::ofstream accy ("/home/lujano/Documents/Reporte/Testfiltraje/Procesamiento/Filtraje/accy.out");
    std::ofstream accz ("/home/lujano/Documents/Reporte/Testfiltraje/Procesamiento/Filtraje/accz.out");


    std::ofstream angxGt ("/home/lujano/Documents/Reporte/Testfiltraje/Procesamiento/Filtraje/angxGt.out");
    std::ofstream angyGt ("/home/lujano/Documents/Reporte/Testfiltraje/Procesamiento/Filtraje/angyGt.out");
    std::ofstream angzGt ("/home/lujano/Documents/Reporte/Testfiltraje/Procesamiento/Filtraje/angzGt.out");

    std::ofstream angxEst ("/home/lujano/Documents/Reporte/Testfiltraje/Procesamiento/Filtraje/angxEst.out");
    std::ofstream angyEst ("/home/lujano/Documents/Reporte/Testfiltraje/Procesamiento/Filtraje/angyEst.out");
    std::ofstream angzEst ("/home/lujano/Documents/Reporte/Testfiltraje/Procesamiento/Filtraje/angzEst.out");

    

    std::ofstream velx ("/home/lujano/Documents/Reporte/Testfiltraje/Procesamiento/Filtraje/velx.out");
    std::ofstream vely ("/home/lujano/Documents/Reporte/Testfiltraje/Procesamiento/Filtraje/vely.out");
    std::ofstream velz ("/home/lujano/Documents/Reporte/Testfiltraje/Procesamiento/Filtraje/velz.out");

    std::ofstream posx ("/home/lujano/Documents/Reporte/Testfiltraje/Procesamiento/Filtraje/posx.out");
    std::ofstream posy ("/home/lujano/Documents/Reporte/Testfiltraje/Procesamiento/Filtraje/posy.out");
    std::ofstream posz ("/home/lujano/Documents/Reporte/Testfiltraje/Procesamiento/Filtraje/posz.out");


    std::ofstream biasx ("/home/lujano/Documents/Reporte/Testfiltraje/Procesamiento/Filtraje/biasx.out");
    std::ofstream biasy ("/home/lujano/Documents/Reporte/Testfiltraje/Procesamiento/Filtraje/biasy.out");
    std::ofstream biasz ("/home/lujano/Documents/Reporte/Testfiltraje/Procesamiento/Filtraje/biasz.out");
   
    //Camera camera(USE_ORB, USE_BRUTE_FORCE_HAMMING, Data.image1.cols, Data.image1.rows);
    int j = 1;
     while(j <Data.indexLastData)
    {  // Cambiar por constant
        Mat finalImage, finalImage2;
        Data.UpdateDataReader(j, j+1);
        j = j+1;
      
        
        Data.image1.copyTo(finalImage);
	    Data.image2.copyTo(finalImage2);
       //camera.Update(Data.image2);
        //camera.addKeyframe();
        /*
        if (camera.frameList.size()> 1) // primera imagen agregada
        {
            if (camera.frameList[camera.frameList.size()-1]->prevGoodMatches.size() < min_points)
            {
                cout << "Puntos minimos AAAAA = "<< min_points<<endl;
                min_points = camera.frameList[camera.frameList.size()-1]->prevGoodMatches.size();
                camera.printStatistics();
                 cout<< " Current time = "<< Data.currentTimeMs <<" ms "<<endl;
            }
       
        //drawKeypoints( frame1, aux, frame1, Scalar(0, 0, 255), DrawMatchesFlags::DEFAULT);
        drawKeypoints(camera.frameList[camera.frameList.size()-2]->grayImage, camera.frameList[camera.frameList.size()-2]->nextGoodMatches , finalImage, Scalar(0,0, 255), DrawMatchesFlags::DEFAULT);
        drawKeypoints(camera.frameList[camera.frameList.size()-1]->grayImage, camera.frameList[camera.frameList.size()-1]->prevGoodMatches, finalImage2, Scalar(0,0, 255), DrawMatchesFlags::DEFAULT);

       visualizerFrame.UpdateMessages(finalImage);
        }
        //visualizerFrame2.UpdateMessages(finalImage2);
        
        //imwrite("/home/lujano/Documents/Imagen1.png", finalImage);
        //imwrite("/home/lujano/Documents/Imagen2.png", finalImage2);
       
      
        E = findEssentialMat(points1_OK, points2_OK, focal, Point2d(cx, cy), RANSAC, 0.999, 1.0, noArray());
        int p;
        p = recoverPose(E, points1_OK, points2_OK, R, t, focal, Point2d(cx, cy), noArray()   );
        int k = 0;
        if (j == 0){
            traslation = Mat::zeros(3, 1, CV_64F);
            R_p = Mat::eye(Size(3, 3), CV_64F);
        }
        else{
            traslation = traslation +R_p*t;
            R_p = R*R_p;
          
        //}

       
        */
      
       
        velocity = velocity+imuCore.velocity;

        Point3d angle_diff;
        Point3d angle_gt, angle_ft, gravity_imu;
        double elapsed_f;
        
        
        imuCore.setImuData(Data.imuAngularVelocity, Data.imuAcceleration);
        clock_t t1 = clock(); 
        //imuCore.setImuBias(Data.accBias, Data.angBias);
        imuCore.setImuInitialVelocity(velocity);
        imuCore.estimate(Data.gtRPY);

        orientation2[0] = imuCore.quaternionWorld[imuCore.quaternionWorld.size()-1].x;
        orientation2[1] = imuCore.quaternionWorld[imuCore.quaternionWorld.size()-1].y;
        orientation2[2] = imuCore.quaternionWorld[imuCore.quaternionWorld.size()-1].z;
        orientation2[3] = imuCore.quaternionWorld[imuCore.quaternionWorld.size()-1].w;

        orientation[0] = Data.gtQuaternion[Data.gtQuaternion.size()-1].x;   //x
        orientation[1] = Data.gtQuaternion[Data.gtQuaternion.size()-1].y; // y
        orientation[2] = Data.gtQuaternion[Data.gtQuaternion.size()-1].z; // z
        orientation[3] = Data.gtQuaternion[Data.gtQuaternion.size()-1].w; // w

        position[0] = Data.gtPosition[Data.gtPosition.size()-1].x;   //x
        position[1] = Data.gtPosition[Data.gtPosition.size()-1].y;   //x
        position[2] = Data.gtPosition[Data.gtPosition.size()-1].z;   //x

        //imuCore.printStatistics(); // imprime el tiempo de computo del filtro
        
        
        Quaterniond orien;
        for ( int ii = 0 ; ii< 10;ii++)
        {
            
            orien= toQuaternion(-175.0*M_PI/180, -68.0*M_PI/180, 0.0 );
            orientation2[0] =  orien.x;//imuCore.quaternionWorld[ii].x;
            orientation2[1] = orien.y;//imuCore.quaternionWorld[ii].y;
            orientation2[2] = orien.z;//imuCore.quaternionWorld[ii].z;
            orientation2[3] = orien.w;//imuCore.quaternionWorld[ii].w;
            position2[0] = imuCore.position.x;   //x
            position2[1] = imuCore.position.y;   //x
            position2[2] = imuCore.position.z;   //x
         
            position[0] = 0.0;Data.gtPosition[ii].x;   //x
            position[1] = 0.0;Data.gtPosition[ii].y;   //x
            position[2] = 0.0;Data.gtPosition[ii].z;   //x
            orientation[0] = Data.gtQuaternion[ii].x;   //x
            orientation[1] = Data.gtQuaternion[ii].y; // y
            orientation[2] = Data.gtQuaternion[ii].z; // z
            orientation[3] = Data.gtQuaternion[ii].w; // w
           

           

            angle_diff.x = computeDiff(Data.gtRPY[ii].x, imuCore.rpyAnglesWorld[ii].x);
            angle_diff.y = computeDiff(Data.gtRPY[ii].y, imuCore.rpyAnglesWorld[ii].y);
            angle_diff.z = computeDiff(Data.gtRPY[ii].z, imuCore.rpyAnglesWorld[ii].z);

            angle_diff = angle_diff*180/M_PI;

            Point3d f_angles = toRPY360(imuCore.rpyAnglesWorld[ii]); // pasar a representacion 0-360
            Point3d gt_angles = toRPY360(Data.gtRPY[ii]);
            //angulo_estimado.UpdateMessages(f_angles*180/M_PI );
            //angulo_groundtruth.UpdateMessages(gt_angles*180/M_PI );
            //error_angular.UpdateMessages(angle_diff);
            visualizer_gt.UpdateMessages(position2, orientation);
            visualizer_est.UpdateMessages(position2, orientation2);
            
            

                
            //vector3d.UpdateMessages(acc_diff);
                
        }
        
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
        Point3d vacio;
        //posicion_estimada.UpdateMessages(error_pos_est);
        //posicion_groundtruth.UpdateMessages(error_pos_gt);



        
        //visualizer_gt.UpdateMessages(position, orientation);
        //visualizer_est.UpdateMessages(position, orientation2);
        clock_t t2 = clock(); 
        double elapsed_time= double(t2- t1) / CLOCKS_PER_SEC;

       
        /*
        cout << " diffx = " << imuCore.accelerationWorld[9].x
        <<" diffy = "<<imuCore.accelerationWorld[9].y
        <<" diffz = "<<imuCore.accelerationWorld[9].z
        << " Current time = "<< Data.currentTimeMs <<" ms "
        << "accx = "  << Data.accBias.x
        << "timestep = "<< imuCore.timeStep

        
        
	<<endl;*/
        cout<< " Current time = "<< Data.currentTimeMs <<" ms " <<endl;



        

        
        
      
       

       
        
        
     }
    
    


     accx.close();
     accy.close();
     accz.close();

     velx.close();
     vely.close();
     velz.close();
     biasx.close();
     biasy.close();
     biasz.close();
     angxGt.close();
     angyGt.close();
     angzGt.close();

     angxEst.close();
     angyEst.close();
     angzEst.close();
     posx.close();
     posy.close();
     posz.close();



    return 0;

}

// COMPILE COMMAND
//g++ -g main_vi_slam.cpp GroundTruth.cpp -o main_vi_slam.out `pkg-config opencv --cflags --libs`

// RUN COMMAND
//./main_vi_slam.out -filePath=../../../../Documents/EuroDataset/state_groundtruth_estimate0/data.csv
// rosrun vi_slam vi_slam -filePath=../../../../Documents/EuroDataset/state_groundtruth_estimate0/data.csv 
