<<<<<<< HEAD
void VISystem::InitializeSystem(string _images_path, string _ground_truth_dataset, string _ground_truth_path, string _depth_path) {
    // Check if depth images are available
    if (_depth_path != "")
        depth_available_ = true;
=======
#include "../include/VISystem.hpp"

namespace vi
{

    VISystem::VISystem(int argc, char *argv[])
    {
        ros::init(argc, argv, "vi_slam");  // Initialize ROS
        initialized = false;
        distortion_valid = false;
        depth_available = false;
        num_keyframes = 0;
    }

    VISystem::~VISystem() 
    {
        cout << "SLAM System shutdown ..." << endl;
        /*
        frames_.clear();
        keyframes_.clear();
        camera_model_->~CameraModel();
        tracker_->~Tracker();
        visualizer_->~Visualizer();

        delete camera_model_;
        delete tracker_;
        delete visualizer_;
        */
    }
>>>>>>> 84cbb66b923993e288999b89010ddf5874cf420e


    void VISystem::InitializeSystem(string _outputPath, string _depthPath, string _calPath, Point3d _iniPosition, Point3d _iniVelocity, float _iniYaw, Mat image)
    {
        // Check if depth images are available
        /*if (_depth_path != "")
            depth_available = true;
        */
        Calibration(_calPath);
        
    
        // Obtain parameters of camera_model
        K = camera_model->GetK();
        w_input = camera_model->GetInputWidth();
        h_input = camera_model->GetInputHeight();
        map1 = camera_model->GetMap1();
        map2 = camera_model->GetMap2();
        fx = camera_model->GetK().at<float>(0,0);
        fy = camera_model->GetK().at<float>(1,1);
        cx = camera_model->GetK().at<float>(0,2);
        cy = camera_model->GetK().at<float>(1,2);
        distortion_valid = camera_model->IsValid();
       

        // Obtain ROI for distorted images
        if (distortion_valid)
        {
            CalculateROI(image);
            cout<< "distortion detected"<<endl;
            cout << "Input width = "<<w_input<<"\t"<< " Output width = "<<  w <<endl;
            cout << "Input height = "<<h_input<<"\t"<< " Output height = "<< h<< endl;
        }
        else{
            w = w_input;
            h = h_input;
        }
            

        // Initialize tracker system
        /*tracker_ = new Tracker(depth_available_);
        tracker_->InitializePyramid(w_, h_, K_);
        tracker_->InitializeMasks();
        */

        // Initialize map
        //map_ = new Map(); //para el futuro :)

        // Initialize output visualizer


        // Cheking if the number of depth images are greater or lower than the actual number of images

        initialized = true;
        cout << "Initializing system ... done" << endl << endl;
        cout << "Ouput file in : "<< _outputPath<<endl;
        outputFile.open(_outputPath); // agregar fecha automticamente

        // Posicion inicial
        position = _iniPosition;
        velocity = _iniVelocity;
        RPYOrientation.z = _iniYaw;
        // IMU 
        imuCore.createPublisher(1.0/(camera_model->imu_frecuency));
        imuCore.initializate(_iniYaw); // Poner yaw inicial del gt
        imuCore.setImuInitialVelocity(_iniVelocity);
     
        // Camera
        num_max_keyframes = camera_model->min_features; // Solo almacenar 10 keyframes
        min_features = camera_model->min_features;
        start_index = camera_model->start_index;

        camera.initializate(camera_model->detector, camera_model->matcher, w, h, camera_model->num_cells, camera_model->length_patch );

    }

    void VISystem::CalculateROI(Mat image) {
        // Load first image
        Mat distorted, undistorted;
        distorted = image;
        remap(distorted, undistorted, map1, map2, INTER_LINEAR);

        // Find middle x and y of image (supposing a symmetrical distortion)
        int x_middle = (undistorted.cols - 1) * 0.5;
        int y_middle = (undistorted.rows - 1) * 0.5;
        
        Point p1, p2;    
        p1.x = 0;
        p1.y = 0;
        p2.x = undistorted.cols - 1;
        p2.y = undistorted.rows - 1;

        // Search x1_ROI distance to crop
        while (undistorted.at<uchar>(y_middle, p1.x) == 0)
            p1.x++;

        // Search x2_ROI distance to crop
        while (undistorted.at<uchar>(y_middle, p2.x) == 0)
            p2.x--;

        // Search y1_ROI distance to crop
        while (undistorted.at<uchar>(p1.y, x_middle) == 0)
            p1.y++;

        // Search y2_ROI distance to crop
        while (undistorted.at<uchar>(p2.y, x_middle) == 0)
            p2.y--;

        // Considering an error margin
        p1.x += 5;
        p2.x -= 5;
        p1.y += 5;
        p2.y -= 5;

        ROI = Rect(p1,p2);
        
        // Update w_ and h_ with ROI dimentions
        w = p2.x - p1.x;
        h  = p2.y - p1.y;
    }

    void VISystem::Calibration(string _calibration_path) 
    {
        cout << "Reading calibration xml file";
        camera_model = new CameraModel();
        camera_model ->GetCameraModel(_calibration_path);
        w = camera_model->GetOutputWidth();
        h = camera_model->GetOutputHeight();

        if (w%2!=0 || h%2!=0) {
            cout << "Output image dimensions must be multiples of 32. Choose another output dimentions" << endl;
            cout << "Exiting..." << endl;
            exit(0);
        }
    }

<<<<<<< HEAD
void VISystem::CalculateROI() {
    // Load first image
    Mat distorted, undistorted;
    distorted = imread(images_list_[0], CV_LOAD_IMAGE_GRAYSCALE);
    remap(distorted, undistorted, map1_, map2_, INTER_LINEAR);
=======
    void VISystem::AddFrame(Mat _currentImage, vector <Point3d> _imuAngularVelocity, vector <Point3d> _imuAcceleration)
    {
        imuCore.setImuData(_imuAngularVelocity, _imuAcceleration); // primeras medidas
        imuCore.estimate();
        camera.Update(_currentImage);
        camera.addKeyframe();
        num_keyframes = camera.frameList.size();
        if (camera.frameList.size()> 1) // primera imagen agregada
        {
            drawKeypoints(camera.frameList[camera.frameList.size()-2]->grayImage, camera.frameList[camera.frameList.size()-2]->nextGoodMatches , outputLastImage, Scalar(0,0, 255), DrawMatchesFlags::DEFAULT);
            drawKeypoints(camera.frameList[camera.frameList.size()-1]->grayImage, camera.frameList[camera.frameList.size()-1]->prevGoodMatches, outputCurrentImage, Scalar(0,0, 255), DrawMatchesFlags::DEFAULT);
            if (camera.frameList[camera.frameList.size()-1]->prevGoodMatches.size() < min_features)
            {
                //min_features = camera.frameList[camera.frameList.size()-1]->prevGoodMatches.size();
                camera.printStatistics();
                
            }
            if (num_keyframes > num_max_keyframes)
            {
                FreeLastFrame();
            }
        }
        cout << "frame List Size = "<< camera.frameList.size()<<endl;
        
        
>>>>>>> 84cbb66b923993e288999b89010ddf5874cf420e

    }
    
<<<<<<< HEAD
    // Update w_ and h_ with ROI dimentions
    w_ = p2.x - p1.x;
    h_ = p2.y - p1.y;
}


// Gauss-Newton using Foward Compositional Algorithm - Using features
void VISystem::EstimatePoseFeatures(Frame* _previous_frame, Frame* _current_frame) {
    // Gauss-Newton Optimization Options
    float epsilon = 0.001;
    float intial_factor = 10;
    int max_iterations = 10;
    float error_threshold = 0.005;
    int first_pyramid_lvl = 0;
    int last_pyramid_lvl = 0;
    float z_factor = 0.002;

    // Variables initialization
    float error         = 0.0;
    float initial_error = 0.0;    
    float last_error    = 0.0;

    // Initial pose and deltapose (assumes little movement between frames)
    Mat deltaMat = Mat::zeros(6,1,CV_32FC1);
    Sophus::Vector<float, SE3::DoF> deltaVector;
    for (int i=0; i<6; i++)
        deltaVector(i) = 0;

    SE3 current_pose = SE3(SO3::exp(SE3::Point(0.0, 0.0, 0.0)), SE3::Point(0.0, 0.0, 0.0));

    // Sparse to Fine iteration
    // Create for() WORKED WITH LVL 2
    for (int lvl = first_pyramid_lvl; lvl>=last_pyramid_lvl; lvl--) {
        // lvl = 0;
        // Initialize error   
        error = 0.0;
        last_error = 50000.0;
        float factor = intial_factor * (lvl + 1);

        // Obtain image 1 and 2
        Mat image1 = _previous_frame->images_[lvl].clone();
        Mat image2 = _current_frame->images_[lvl].clone();

        // Obtain points and depth of initial frame 
        Mat candidatePoints1  = _previous_frame->candidatePoints_[lvl].clone();
        Mat candidatePoints2  = _current_frame->candidatePoints_[lvl].clone();    
        
        //candidatePoints1 = AddPatchPointsFeatures(candidatePoints1, lvl);
        // cout << candidatePoints1.rows << endl;
        // Mat imageWarped = Mat::zeros(image1.size(), CV_8UC1);
        // ObtainImageTransformed(image1, candidatePoints1, candidatePoints1, imageWarped);             
        // DebugShowWarpedPerspective(image1, image2, imageWarped, lvl);

        // Obtain gradients           
        Mat gradientX1 = Mat::zeros(image1.size(), CV_16SC1);
        Mat gradientY1 = Mat::zeros(image1.size(), CV_16SC1);
        gradientX1 = _previous_frame->gradientX_[lvl].clone();
        gradientY1 = _previous_frame->gradientY_[lvl].clone();

        // Obtain intrinsic parameters 
        Mat K = K_[lvl];

        // Optimization iteration
        for (int k=0; k<max_iterations; k++) {
=======
    void VISystem::AddFrame(Mat _currentImage, vector <Point3d> _imuAngularVelocity, vector <Point3d> _imuAcceleration, vector <Point3d> _gtRPY) 
    {
        imuCore.setImuData(_imuAngularVelocity, _imuAcceleration); // primeras medidas
        imuCore.estimate(_gtRPY);
        camera.Update(_currentImage);
        camera.addKeyframe();
        num_keyframes = camera.frameList.size();
        if (camera.frameList.size()> 1) // primera imagen agregada
        {
            if (camera.frameList[camera.frameList.size()-1]->prevGoodMatches.size() < min_features)
            {
                ///min_features = camera.frameList[camera.frameList.size()-1]->prevGoodMatches.size();
                camera.printStatistics();
            }
            if (num_keyframes > num_max_keyframes)
            {
                FreeLastFrame();
            }
        }
        cout << "frame List Size = "<< camera.frameList.size()<<endl;
        drawKeypoints(camera.frameList[camera.frameList.size()-2]->grayImage, camera.frameList[camera.frameList.size()-2]->nextGoodMatches , outputLastImage, Scalar(0,0, 255), DrawMatchesFlags::DEFAULT);
        drawKeypoints(camera.frameList[camera.frameList.size()-1]->grayImage, camera.frameList[camera.frameList.size()-1]->prevGoodMatches, outputCurrentImage, Scalar(0,0, 255), DrawMatchesFlags::DEFAULT);
    }

    void VISystem::FreeLastFrame()
    {
        camera.frameList.erase(camera.frameList.begin());
        cout << "Num keyFrames = "<< camera.frameList.size()<<endl;
    }
    /*
    // Gauss-Newton using Foward Compositional Algorithm - Using features
    void VISystem::EstimatePoseFeatures(Frame* _previous_frame, Frame* _current_frame) {
        // Gauss-Newton Optimization Options
        float epsilon = 0.001;
        float intial_factor = 10;
        int max_iterations = 10;
        float error_threshold = 0.005;
        int first_pyramid_lvl = 0;
        int last_pyramid_lvl = 0;
        float z_factor = 0.002;

        // Variables initialization
        float error         = 0.0;
        float initial_error = 0.0;    
        float last_error    = 0.0;

        // Initial pose and deltapose (assumes little movement between frames)
        Mat deltaMat = Mat::zeros(6,1,CV_32FC1);
        Sophus::Vector<float, SE3::DoF> deltaVector;
        for (int i=0; i<6; i++)
            deltaVector(i) = 0;

        SE3 current_pose = SE3(SO3::exp(SE3::Point(0.0, 0.0, 0.0)), SE3::Point(0.0, 0.0, 0.0));

        // Sparse to Fine iteration
        // Create for() WORKED WITH LVL 2
        for (int lvl = first_pyramid_lvl; lvl>=last_pyramid_lvl; lvl--) {
            // lvl = 0;
            // Initialize error   
            error = 0.0;
            last_error = 50000.0;
            float factor = intial_factor * (lvl + 1);

            // Obtain image 1 and 2
            Mat image1 = _previous_frame->images_[lvl].clone();
            Mat image2 = _current_frame->images_[lvl].clone();

            // Obtain points and depth of initial frame 
            Mat candidatePoints1  = _previous_frame->candidatePoints_[lvl].clone();
            Mat candidatePoints2  = _current_frame->candidatePoints_[lvl].clone();    
>>>>>>> 84cbb66b923993e288999b89010ddf5874cf420e
            
            //candidatePoints1 = AddPatchPointsFeatures(candidatePoints1, lvl);
            // cout << candidatePoints1.rows << endl;
            // Mat imageWarped = Mat::zeros(image1.size(), CV_8UC1);
            // ObtainImageTransformed(image1, candidatePoints1, candidatePoints1, imageWarped);             
            // DebugShowWarpedPerspective(image1, image2, imageWarped, lvl);

            // Obtain gradients           
            Mat gradientX1 = Mat::zeros(image1.size(), CV_16SC1);
            Mat gradientY1 = Mat::zeros(image1.size(), CV_16SC1);
            gradientX1 = _previous_frame->gradientX_[lvl].clone();
            gradientY1 = _previous_frame->gradientY_[lvl].clone();

            // Obtain intrinsic parameters 
            Mat K = K_[lvl];

            // Optimization iteration
            for (int k=0; k<max_iterations; k++) {
                
                // Warp points with current pose and delta pose (from previous iteration)
                SE3 deltaSE3;
                Mat warpedPoints = Mat(candidatePoints1.size(), CV_32FC1);


                //warpedPoints = WarpFunctionOpenCV(candidatePoints1, current_pose, lvl);
                warpedPoints = WarpFunction(candidatePoints1, current_pose, lvl);
                // Mat imageWarped = Mat::zeros(image1.size(), CV_8UC1);
                // ObtainImageTransformed(image1, candidatePoints1, warpedPoints, imageWarped);    
                // imshow("warped", imageWarped);
                // waitKey(0);
                // Computation of Jacobian and Residuals
                Mat Jacobians;
                Mat Residuals;      

                int num_valid = 0;
                for (int i=0; i<candidatePoints1.rows; i++) {
                    Mat Residual = Mat(1,1,CV_32FC1);        
                    Mat Jacobian_row = Mat::zeros(1,6,CV_32FC1);
                    Mat Jw = Mat::zeros(2,6,CV_32FC1);
                    Mat Jl = Mat::zeros(1,2,CV_32FC1);
                    
                    // Point in frame 1            
                    float x1 = candidatePoints1.at<float>(i,0);
                    float y1 = candidatePoints1.at<float>(i,1);
                    float z1 = candidatePoints1.at<float>(i,2);
                    // Points of warped frame
                    float x2 = warpedPoints.at<float>(i,0);
                    float y2 = warpedPoints.at<float>(i,1);
                    float z2 = warpedPoints.at<float>(i,2);

                    float inv_z2 = 1 / z2;

                    // Check if warpedPoints are out of boundaries
                    if (y2>0 && y2<image2.rows && x2>0 && x2<image2.cols) {
                        if (z2!=0) {
                            if (inv_z2<0) 
                                inv_z2 = 0;
                            num_valid++;
                            Jw.at<float>(0,0) = fx_[lvl] * inv_z2;
                            Jw.at<float>(0,1) = 0.0;
                            Jw.at<float>(0,2) = -(fx_[lvl] * x2 * inv_z2 * inv_z2) * z_factor;
                            Jw.at<float>(0,3) = -(fx_[lvl] * x2 * y2 * inv_z2 * inv_z2);
                            Jw.at<float>(0,4) = (fx_[lvl] * (1 + x2 * x2 * inv_z2 * inv_z2));   
                            Jw.at<float>(0,5) = - fx_[lvl] * y2 * inv_z2;

                            Jw.at<float>(1,0) = 0.0;
                            Jw.at<float>(1,1) = fy_[lvl] * inv_z2;
                            Jw.at<float>(1,2) = -(fy_[lvl] * y2 * inv_z2 * inv_z2) * z_factor;
                            Jw.at<float>(1,3) = -(fy_[lvl] * (1 + y2 * y2 * inv_z2 * inv_z2));
                            Jw.at<float>(1,4) = fy_[lvl] * x2 * y2 * inv_z2 * inv_z2;
                            Jw.at<float>(1,5) = fy_[lvl] * x2 * inv_z2;

                        
                            // Intensities
                            int intensity1 = image1.at<uchar>(y1,x1);
                            int intensity2 = image2.at<uchar>(round(y2),round(x2));

                            Residual.at<float>(0,0) = intensity2 - intensity1;
                            
                            Jl.at<float>(0,0) = gradientX1.at<short>(y1,x1);
                            Jl.at<float>(0,1) = gradientY1.at<short>(y1,x1);

                            Jacobian_row = Jl * Jw;
                            // cout << "Residual: " << Residual.at<float>(0,0) << endl;
                            // cout << "Jl: " << Jl << endl;                
                            // cout << "Jw: " << Jw << endl;                                
                            // cout << "Jacobian: " << Jacobian_row << endl;
                            // cout << endl;
                            
                            Jacobians.push_back(Jacobian_row);
                            Residuals.push_back(Residual);
                        }
                    }
                }
                // cout << "Valid points found: " << num_valid << endl;
                // DebugShowJacobians(Jacobians, warpedPoints, w_[lvl], h_[lvl]);

                // Computation of Weights (Identity or Tukey function)
                Mat W = IdentityWeights(Residuals.rows);
                //Mat W = TukeyFunctionWeights(Residuals);

                // Computation of error
                float inv_num_residuals = 1.0 / Residuals.rows;
                Mat ResidualsW = Residuals.mul(W);
                Mat errorMat =  inv_num_residuals * Residuals.t() * ResidualsW;
                error = errorMat.at<float>(0,0);
    
                if (k==0)
                    initial_error = error;

                // Break if error increases
                if (error >= last_error || k == max_iterations-1 || abs(error - last_error) < epsilon) {
                    // cout << "Pyramid level: " << lvl << endl;
                    // cout << "Number of iterations: " << k << endl;
                    // cout << "Initial-Final Error: " << initial_error << " - " << last_error << endl << endl;

                    // if (lvl == last_pyramid_lvl) {
                    //     DebugShowJacobians(Jacobians, warpedPoints, w_[lvl], h_[lvl]);
                    //     Mat imageWarped = Mat::zeros(image1.size(), CV_8UC1);
                    //     ObtainImageTransformed(image1, candidatePoints1, warpedPoints, imageWarped);             
                    //     DebugShowWarpedPerspective(image1, image2, imageWarped, lvl);
                    // }

                    // Reset delta
                    deltaMat = Mat::zeros(6,1,CV_32FC1);
            
                    for (int i=0; i<6; i++)
                        deltaVector(i) = 0;

                    break;
                }

                last_error = error;

                // Checking dimentions of matrices
                // cout << "Jacobians dimentions: " << Jacobians.size() << endl;
                // cout << "Weights dimentions: " << W.size() << endl;
                // cout << "Residuals dimentions: " << Residuals.size() << endl;
                
                // Computation of new delta (DSO-way)
                // LS ls;
                // ls.initialize(Residuals.rows);
                // for (int i=0; i<Residuals.rows; i++) {
                //     Mat61f jacobian;
                //     cv2eigen(Jacobians.row(i), jacobian);
                    
                //     ls.update(jacobian, Residuals.at<float>(i,0), W.at<float>(i,0));
                // }
                // ls.finish();
                // // Solve LS system
                // float LM_lambda = 0.2;
                // Mat61f b = -ls.b;
                // Mat66f A = ls.A;
                // deltaVector = A.ldlt().solve(b);

                // Computation of new delta (Kerl-way)            
                // Multiplication of W to Jacobian
                for (int i=0; i<Jacobians.rows; i++) {
                    float wi = W.at<float>(i,0);
                    Jacobians.row(i) = wi * Jacobians.row(i);
                }

                Residuals = Residuals.mul(1);  // Workaround to make delta updates larger
                Mat A = Jacobians.t() * Jacobians;                    
                Mat b = -Jacobians.t() * Residuals.mul(W);

                //cout << b << endl;
                deltaMat = A.inv() * b;
                //cout << A.inv() << endl;
                //cout << A << endl;
                

                // Convert info from eigen to cv
                for (int i=0; i<6; i++)
                    deltaVector(i) = deltaMat.at<float>(i,0);

                // Update new pose with computed delta
                current_pose = current_pose * SE3::exp(deltaVector);
                //cout << current_pose.matrix() << endl;
                
            }

            // Scale current_pose estimation to next lvl
            if (lvl !=0) {
                Mat31f t = 2 * current_pose.translation();

                Quaternion quaternion = current_pose.unit_quaternion();

                quaternion.x() = quaternion.x() * 2;
                quaternion.y() = quaternion.y() * 2;
                quaternion.z() = quaternion.z() * 2;
                
                current_pose = SE3(quaternion, t);
            }
            
            //current_pose = SE3(current_pose.unit_quaternion() * 2, current_pose.translation() * 2);
        }

        _previous_frame->rigid_transformation_ = current_pose;

    }
    */
}