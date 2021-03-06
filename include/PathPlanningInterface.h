/*
 * Copyright (C) 2021 Useerobot. All rights reserved.
 * @Author       : Zola
 * @Description  : 
 * @Date         : 2021-05-06 17:14:52
 * @LastEditTime : 2022-01-25 17:10:50
 * @Project      : UM_path_planning
 */

#pragma once
#ifndef PATH_PLANNING_INTERFACE_H
#define PATH_PLANNING_INTERFACE_H
// #include <boost/bind.hpp>
#include  <string>
// #include  "lidar/lidar_shm.h"
#include  "um_chassis/shmCom.h"
#include  "semaphore.h"
#include  <sys/shm.h>
#include  <sys/sem.h>
#include  "um_chassis/uart.h"
#include  "common_function/logger.h"
#include  "common_function/MotionControl.h"
#include  "navigation_algorithm/GlobalPlanning.h"
#include  "common_function/Parse.h"
#include  "message/navigation_msg.h"
#include  "message/navigation_rpc_server.h"
#include  "message/navigation_server_handler.h"
#include  "core/log.h"
#include  "core/utils.h"
#include "navigation_algorithm/UTurnPlanning.h"
#include "common_function/ExceptionHanding.h"
// #include "common_function/lz4.h"
#include "navigation_algorithm/AlongWall.h"
#include "RemoteControllerTask.h"
#include "navigation_algorithm/InternalSpiralPlanning.h"
#include "um_chassis/logger.h"
#include "um_chassis/logger_rc.h"
// #include "boost/heap/d_ary_heap.hpp"


using namespace core;

namespace useerobot
{
extern RoadSensorData_t sensordata;
extern current_pose_t* current_pose;
extern share_path_planning_t* current_path_planning;
extern current_pose_t share_pose;
extern WallFallowPara_t WallFallowPara;
extern bool cleanTaskOverIndex;
extern RoadAim record_charge_aim;
// extern share_map_t* current_full_map;
extern PRO process;
extern CHASSIS_CONTROL GLOBAL_CONTROL;
extern bool getBlockInfoIdenx;
extern bool getPointInfoIndex;
extern bool getForbbidenInfoIndex;
extern RoadAim road_aim;
extern int calibration_time;
extern bool gyro_stat_resp_state;
typedef void(*fun)(struct RobotCleanReq &rev);
// typedef std::function<void(struct RobotCleanReq *)> robot_clean_cb;


extern cornerIdex corner_index;
struct SystemStatusData
{
    double battery; //????????????
    double batteryVoltage; //????????????
    bool chargerStatus; //????????????
    double speed; //?????????????????????
    double totalMileage;//??????????????????
    double uptime;//?????????????????????
};


enum MAP_TYPE
{
  MAP = 1,
  RECORDMAP=2,
  LOCALMAP =3,
};
enum ESCAPE_STATE
{   
  ESCAPE_IDLE = 0,      
  ESCAPE_ING = 1,      
  ESCAPE_FAILED = 2,
};

typedef enum LidarState{
  NORMAL = 0, 
  COVERED = 1, // ??????
  WILD = 2, //??????
}lidat_state_t;

struct Robotstate 
{
  int robot_state;
  ESCAPE_STATE escape_state;
};
class PathPlanningInterface
{
  public:
    PathPlanningInterface();
    ~PathPlanningInterface();
    /**
     * @description: ?????????????????????
     * @event: 
     * @param {*}
     * @return  {0 -??????}
     */
    void cleanCycleInit(); 
    /**
     * @description: ????????????
     * @event: 
     * @param {*}
     * @return {0 -??????}
     */
    void cleanStartRun(); 
    /**
     * @description: ??????????????????
     * @event: 
     * @param {*}
     * @return {0 -??????}
     */
    void cleanCycleDestroy();
    void AppControl(Sensor sensor,Grid cur,Trouble trouble);
    void handle_navigation_reqeust(struct RobotCleanReq *req);
    void cleanPlanningRun(struct RobotCleanReq &rev); //??????????????????
    void SLAMPathPLanning();//????????????
    void Init();
   // void InitRecordMap();// ?????????recordmap;
    void SetMap(int cols,int rows,int maptype);
    void getRobotState();
    void stop_task();
    void clear_state();
    void suspendClean();//????????????
    void resumeClean();//????????????
    void stopClean();//????????????
    int SignManage(Sensor sensor,Grid cur);
    void gamepadControl();
    void divideRoomToBlock(BlockCorner &selectBlockCorner,RobotType &robotShape); //????????????
    void judgePosition();//?????????????????????????????????
    Point* judgeBlockCorner(const Point &startPoint, const BlockCorner &selectBlockCorner, RobotType &robotShape);//????????????????????????????????????
    void pointClean(Point &sweepPoint,RobotType &robotShape);  //????????????
    void autoClean(RobotType &robotShape);//????????????
    void reLocalization(RobotType &robotShape);//???????????????
    void appointRoomClean(BlockCorner &selectBlockCorner,RobotType &robotShape); //??????????????????
    void appointBlockClean(BlockCorner &selectBlockCorner,RobotType &robotShape); //??????????????????
    void movingCharge();  //??????
    int  chargeFunction(int charger_x,int charger_y); //????????????
    void alongWallClean();//????????????
    void updateRobotstate();
    void ClearRecodrdMap();
    void LeaveCharger();//????????????
    void randomClean();
    void escapescene();//????????????
    void setChargeMode(RoadAim charge_aim);
    void rechargeTask(); // ????????????
    RobotPlanningState getRobotPlanningState();
    static void sensorMemCallBack(RoadSensorData_t sensorData);
    void RectangleTest(Sensor sensor,Grid cur);
    void uploadArea();
    void InitGlobalMap();
    void updateEscapeState(ESCAPE_STATE state);
    void mapCorrect(float angle);
    // void _SensorDataCallback(UserSensorData_t *data);
    // void _ImuDataCallback(UserImuData_t *data);
    useerobot::MotionControl *motion_control;
    void *shm = NULL;
    // share_use_sensor_t* shared = NULL;
    std::string server_address;
    std::shared_ptr<NavigationRpcServer> server;
    std::shared_ptr<NavigationServerHandler> handler;
    std::shared_ptr<RemoteControllerTask> remote_controller;
    int count;
    bool success;
    float lastPara;
    RobotType cur_RobotType;
    UTurnPlanning arch;
    blockPlanning block;
    RoadPlanning road;
    MotionControl motion;
    EscapePlan escape;
	  chassisBase chassis;
    AlongWall _wall;
    InternalSpiralPlanning internal_planning;
    
    bool autodividerooms;
    RobotType *_robotShape;

    GAMEPAD_CONTROL CURRENT_GAMEPAD_STATE;
    
    aStar astar;
    dwa dwaRun;
    useerobot::MapFunction   map_function;
    // RoadSensorData_t *sensordata; 

    Robotstate *robotstate;
    int *shareSensorState;
    int rechargeindex;
    bool blockCleanIndex = false;
    bool pointCleanIdex = false;
    bool fixPointCleanIndex = false;
    
    pointCoordinate blockCenterPoint;
    pointCoordinate pointCleanPosition;
    RoadAim tmp_aim;
    int last_map_area;
    bool escape_index  =false;
    bool last_escape_index = false;
  private:
    RobotPlanningStateData robot_current_state;
    RobotPlanningAreaData  current_clean_area;
    RobotCleanScene robot_current_clean_scene;
    RobotCleanReq rev_message;
    bool exit_ ;

    Grid current_pos;
    Sensor current_sensor;
    
    Planning_info current_planning_info;
    AlongWall Alongwall_task;
    SystemStatusData *system_status_data;
    // share_map_t* current_full_map;
    bool thread_index = false;
    pthread_t task_thread;
    std::shared_ptr<std::thread> prepare_thread_ = nullptr;
    std::shared_ptr<std::thread> slamPathPLanning_thread_ = nullptr;
    std::shared_ptr<std::thread> rondomPLanning_thread_ = nullptr;
    std::shared_ptr<std::thread> leaveCharger_thread_ = nullptr;
    

    bool slamPathPLanning_state_index = false;
    bool leaveCharger_task_state = false;
    bool fisrt_correcting_map = true;
    bool last_cleanTaskOverIndex =false;
    bool remap_recharge_index = false;
    bool escape_fail_index = false;
    bool correct_map_resume_recharge_index = false;
    bool gyo_calibration_index =true;
    bool gyo_message_ok_index = true;
    int gyro_stat_resp_cycles = 0;
    // bool batvolume_Index = true;

};
}
#endif
