/*
 * Copyright (C) 2021 Useerobot Ltd.All rights reserved.
 * @Author       : Zola
 * @Description  : 
 * @Date         : 2021-05-08 20:39:51
 * @LastEditTime : 2022-01-27 12:11:45
 * @Project      : UM_path_planning
 */
#include "common_function/MapFunction.h"
#include "common_function/logger.h"
#include "um_chassis/chassisBase.h"
#include "navigation_algorithm/AlongWall.h"
#include "navigation_algorithm/RoadPlanning.h"
#include "navigation_algorithm/BlockPlanning.h"
extern useerobot::Maze _maze;

namespace useerobot{

    cornerIdex corner_index;
    extern int areaClean;
    PRO process; 
    static Grid lastPoint; 
    static Grid pre_lastPoint; 
    static Grid lastOb;
    extern share_map_t* current_full_map;
    chassisBase chassisMap;
    
    extern RoadAim road_aim;
    extern RoadAim last_road_aim;
    extern vector <Grid> deleteOb;
    extern vector <Grid> boundPoint;
    extern boundary limit;
    extern ROAD_STATE roadState;
    extern bool left_charger_index;
    extern share_path_planning_t* current_path_planning;  
    Point::Point() {}

    Point::Point(int x, int y, int n)
    {
        this->x = x;
        this->y = y;
        this->n = n; // 0->无障碍; 1->有障碍; -1->未知区域; 2->清扫过区域
        F = 0;
        G = 0;
        H = 0;
        parent = NULL;
    }

    Point::Point(const Point &p)
    {
        x = p.x;
        y = p.y;
        n = p.n;
        F = p.F;
        G = p.G;
        H = p.H;
        parent = p.parent;
    } 
    
    bool Point::operator==(const Point & p)
    {
        return x == p.x && y == p.y && n == p.n && F == p.F && G == p.G && H == p.H && parent == p.parent;
    }  
    
    Maze::Maze() {}

    Maze::Maze(int rows, int cols)
    {
        this->rows = rows;
        this->cols = cols;
    }

    Maze::Maze(const Maze &m)
    {
        // rows = m.rows;
        // cols = m.cols;
        // startPoint = m.startPoint;
        // endPoint = m.endPoint;
        // Map = m.Map;
        // recordMap = m.recordMap;
        // Map.resize(m.rows);
        // recordMap.resize(m.rows);
        // for (unsigned int i = 0; i < Map.size(); i++)
        //     {Map[i].resize(m.cols);
        //     recordMap[i].resize(m.cols);}
        // for (int i = 0; i<rows; i++)
        //     for (int j = 0; j<cols; j++)
        //         {
        //             Map[i][j] = new Point(*m.Map[i][j]);
        //             recordMap[i][j] = new Point(*m.recordMap[i][j]);
        //         }

    }

    Maze::~Maze()
    {

    }

    void Maze::setMaze(int rows, int cols)
    {
        Map.resize(rows);
        for (unsigned int i = 0; i < Map.size(); i++)
            Map[i].resize(cols);

        recordMap.resize(rows);
        for (unsigned int i = 0; i < recordMap.size(); i++)
            recordMap[i].resize(cols);            
    }

    void Maze::setGlobalMapState(int x,int y,int state)
    {
        printf("@300=%04d,%04d,00%d\n",x,y,state);

        if (x < 0)
            x = abs(x) + SIZE;
        else   
            x = abs(SIZE - x);
        if (y < 0)
            y = abs(y) + SIZE;    
        else
            y = abs(SIZE - y);   

        Map[x][y]->n = state;
    }

    void Maze::InputRecord(int x,int y,int state)
    {

        if (IsWall() == 2
             && (process == ROAD
                 || process == PLAN
                 || (process == BOUND && !virWall))){
                
            if (state == 1 || state == 99){
                filter.push_back({x,y});
            }
            if (state == 2){
                Grid tmp = {x,y};
                if (protectPoint.addAngle || find(filter.begin(), filter.end(),tmp) != filter.end()){
                    FRIZY_LOG(LOG_DEBUG,"guolvle");
                    return;
                }
            }  
        }
        else{
            if (!filter.empty() && process != ROAD){
                FRIZY_LOG(LOG_DEBUG,"clear filter");
                filter.clear();
            }
        }

        if (state == 99)
            return;
            

        // if (IsWall() == 2 && state == 2
        //      && (process == ROAD || process == PLAN))
        // {
        //     if (x == lastPoint.x && y == lastPoint.y
        //         || (x == pre_lastPoint.x && y == pre_lastPoint.y))
        //     {
        //         FRIZY_LOG(LOG_DEBUG,"filter1");
        //         return;
        //     }
        // }

        

        // int tempi = boundPoint.size() - 1;
        // if (IsWall() == 2 && process == BOUND && state == 2 
        //     && tempi >= 2
        //     && !virWall)
        // {
        //     if ((boundPoint[tempi].x == x && boundPoint[tempi].y == y)
        //         || (boundPoint[tempi-1].x == x && boundPoint[tempi-1].y == y))
        //     {
        //         FRIZY_LOG(LOG_DEBUG,"filter2");
        //         return;
        //     }
        // }
        
        //
        if (IsWall() == 0 && process == ROAD && state == 2
            //&& (road_aim.kind == searchUnclean && GetMapState(x,y,2) == 1)
            && ((road_aim.kind == recharge && GetMapState(x,y,2) != 2)
                || ((road_aim.kind == searchUnclean || road_aim.kind == searchBound) && GetMapState(x,y,2) == 1))

            && (lastOb.x != x || lastOb.y != y))
        {  
            Grid temp;
            temp.x = x,temp.y = y;
            
			if (road_aim.x == last_road_aim.x && road_aim.y == last_road_aim.y)
			{
                if (deleteOb.size() <= 5)
                {
                    FRIZY_LOG(LOG_DEBUG,"addd.%d.%d",temp.x,temp.y);
                    deleteOb.push_back(temp);
                }
			}
            else
            {
                FRIZY_LOG(LOG_DEBUG,"newde.%d.%d",temp.x,temp.y);
                deleteOb.clear();
                deleteOb.push_back(temp);
			}  
        }

        lastOb.x = x,lastOb.y = y;  


        printf("@300=%04d,%04d,00%d\n",x,y,state);

        if (x < 0)
            x = abs(x) + SIZE;
        else   
            x = abs(SIZE - x);

        if (y < 0)
            y = abs(y) + SIZE;    
        else
            y = abs(SIZE - y);   

        recordMap[x][y]->n = state;
    }

    int8_t Maze::VirBound(int16_t x,int16_t y)
    {
        if (GetMapState(x,y,1) == 3 || GetMapState(x,y,1) == 4)
        {
            printf("VirBound.%d.%d\n",x,y);
            return 1;
        }
        else
            return 0;
    } 

    int Maze::GetMapState(int x,int y,int mapkind)
    {

        if (x > 200) x = 200;
        if (x < -200) x = -200;
        if (y > 200) y = 200;
        if (y < -200) y = -200;


        if (mapkind == 1)
        {
            if(x < 0)
                x = abs(x) + SIZE;
            else 
                x = SIZE - x;
            if(y < 0)
                y = abs(y) + SIZE;
            else
                y = SIZE - y;                      
            return Map[x][y] -> n;
        }
          

        if (mapkind == 2)
        {
            if(x < 0)
                x = abs(x) + SIZE;
            else 
                x = SIZE - x;
            if(y < 0)
                y = abs(y) + SIZE;
            else
                y = SIZE - y;                       
            return recordMap[x][y]->n; 
        }
    }


    void Maze::RecordMap(Sensor sensor,Grid cur)
    {
        virWall = sensor.rightInfrared;
        
        int tempw = sensor.size/2;
            
        //设置障碍点
        

        if (sensor.bump || (sensor.obs && roadState != roadTrue) || sensor.cliff || IsWall() != 0)
        {
            //printf("setob v:%d\n",virWall);

            if (protectPoint.addAngle && abs(protectPoint.addAngle - cur.addAngle) > 60){
                printf("setob v:%d\n",virWall);
                protectPoint.addAngle = 0;  
            }
                

            if (IsWall() == 1)
            {
                printf("yan2.%f\n",cur.forward);

                if (cur.forward >= 315 || cur.forward <= 45)
                {
                    cur.forward = 270;
                }
                else if (cur.forward > 45 && cur.forward <= 135)
                {
                    cur.forward = 0;
                }
                else if (cur.forward > 135 && cur.forward <= 225)
                {
                    cur.forward = 90;
                }
                else
                {
                    cur.forward = 180;
                }
            }
            else if (IsWall() == 2)
            {
                printf ("yan1.%f\n",cur.forward);
                if (cur.forward >= 315 || cur.forward <= 45)
                {
                    cur.forward = 90;
                }
                else if (cur.forward > 45 && cur.forward <= 135)
                {
                    cur.forward = 180;
                }
                else if (cur.forward > 135 && cur.forward <= 225)
                {
                    cur.forward = 270;
                }
                else
                {
                    cur.forward = 0;
                }			
            }

            if (++obCount > 98){
                obCount = 0;
            }

            if (IsWall() != 0 && obCount % 3 != 0){

            }
            else{

            FRIZY_LOG(LOG_DEBUG,"obagg.%f",cur.forward);
        
            if ((cur.forward >= 0 && cur.forward <= 22) || cur.forward >= 338)
            {    
                InputRecord(cur.x+1,cur.y,2); 
            }

            if (cur.forward > 157 && cur.forward < 202)
            {
                InputRecord(cur.x-1,cur.y,2); 
            }	

            if (cur.forward > 77 && cur.forward < 112)
            {
                InputRecord(cur.x,cur.y-1,2); 
            }

            if (cur.forward > 245 && cur.forward < 292)
            {
                InputRecord(cur.x,cur.y+1,2); 
            }	

            if (cur.forward > 22 && cur.forward <= 77)
            {
                //recordMap[cur.x+1][cur.y-1]->n = 2;
                InputRecord(cur.x+1,cur.y-1,2); 
            }

            if (cur.forward >= 112 && cur.forward <= 157)
            {
                //recordMap[cur.x-1][cur.y-1]->n = 2;
                InputRecord(cur.x-1,cur.y-1,2); 
            }	

            if (cur.forward >= 202 && cur.forward <= 245)
            {
                //recordMap[cur.x-1][cur.y+1]->n = 2;
                InputRecord(cur.x-1,cur.y+1,2); 
            }

            if (cur.forward >= 292 && cur.forward < 338)
            {
                //recordMap[cur.x+1][cur.y+1]->n = 2;
                InputRecord(cur.x+1,cur.y+1,2); 
            }	
            }	
        }
        
        //设置已清扫点
        if (lastPoint.x != cur.x || lastPoint.y != cur.y)
        {
            int tmp = 0;
            protectPoint.addAngle = 0;
            for (int x = -100;x < 100;x++)
            {
                for (int y = -100;y < 100;y++)
                {
                    if (GetMapState(x,y,2) != 0){
                       ++tmp; 
                    }

                    if (GetMapState(x+1,y,2) == 1 && GetMapState(x-1,y,2) == 1
                        && GetMapState(x,y+1,2) == 1 && GetMapState(x,y-1,2) == 1
                        && GetMapState(x,y,2) == 0 && !IsWall() && !areaClean)
                        {
                            FRIZY_LOG(LOG_DEBUG,"buqi1");
                            InputRecord(x,y,1);
                        } 

                    if (GetMapState(x,y,2) == 0 && !IsWall() && !areaClean
                         && ((GetMapState(x+1,y,2) == 1 && x+1 == limit.up && GetMapState(x,y-1,2) == 1 && GetMapState(x,y+1,2) == 1)
                            || (GetMapState(x-1,y,2) == 1 && x-1 == limit.down && GetMapState(x,y-1,2) == 1 && GetMapState(x,y+1,2) == 1)
                            || (GetMapState(x,y+1,2) == 1 && y+1 == limit.left && GetMapState(x+1,y,2) == 1 && GetMapState(x-1,y,2) == 1)
                            || (GetMapState(x,y-1,2) == 1 && y-1 == limit.right && GetMapState(x+1,y,2) == 1 && GetMapState(x-1,y,2) == 1)))
                        {
                            FRIZY_LOG(LOG_DEBUG,"buqi2");
                            InputRecord(x,y,1);
                        }
                }
            }
            mapArea = tmp * 0.0225;
            FRIZY_LOG(LOG_DEBUG,"lastPoint.%d.%d,pre.%d.%d,area.%d,%d",cur.x,cur.y,pre_lastPoint.x,pre_lastPoint.y,tmp,mapArea);

            if (process == ROAD 
                && (road_aim.x != cur.x || road_aim.y != cur.y))
            {
                if (GetMapState(cur.x,cur.y,2) == 2 && !areaClean){
                    FRIZY_LOG(LOG_DEBUG,"tupo  %d %d",cur.x,cur.y);
                    InputRecord(cur.x,cur.y,1);
                }
                else{
                    InputRecord(cur.x,cur.y,99);
                    printf("@300=%04d,%04d,00%d\n",cur.x,cur.y,4);
                } 
            }
            
            else if (process == PLAN)
            {
                if (cur.forward < 5 || cur.forward > 355
                    || (cur.forward > 175 && cur.forward < 180))
                {
                    for (int i = -1*tempw;i <= tempw;i++){
                                                
                        InputRecord(cur.x,cur.y+i,1); 
                    }
                }
                else
                    InputRecord(cur.x,cur.y,1);                   
            }
            else
                InputRecord(cur.x,cur.y,1);
        }
        if (IsWall())
            pre_lastPoint = lastPoint;

        lastPoint = cur;
        
    }

    MapFunction::MapFunction(/* args */)
    {
    }
    
    MapFunction::~MapFunction()
    {
    }
    bool MapFunction::getVirtualWall()
    {
        FRIZY_LOG(LOG_INFO, "START TO GET THE VIRTUALWALL");
    }
    void MapFunction::fittingMap()
    {
        current_pos = GetCurGridPose();
        FRIZY_LOG(LOG_INFO, "START TO FITTING THE MAP");
    }
    

    void Maze::InitRecordMap()
    {
        FRIZY_LOG(LOG_INFO,"  init the record map \n");
        FRIZY_LOG(LOG_DEBUG,"  the record map size is %d * %d\n",_maze.cols,_maze.rows);
        for(int i =0; i < _maze.cols;i++)
        {
            for(int j =0; j < _maze.rows;j++)
            {
                recordMap[i][j]->n = 0;
            }
        }        
    }
    void Maze::globalMapUpdate()
    {

        FRIZY_LOG(LOG_DEBUG,"map update");

        //vector <cellIndex> pathArrary_planning;
        // core::share_mem_lock(current_full_map);
        core::share_mem_sync sync(current_full_map);
        // FRIZY_LOG(LOG_DEBUG, "share_mem_sync ok");

        int _cols = current_full_map->map_width;
        int _rows = current_full_map->map_height;


        // FRIZY_LOG(LOG_DEBUG, "current_full_map->map_width = %d",_cols);
        // FRIZY_LOG(LOG_DEBUG, "current_full_map->map_height = %d",_rows);
        // int decompress_size;
        // int current_map_size;
            
        
        // decompress_size = MAP_DATA_LIMIT*sizeof(uint8_t);

        // uint8_t *map_buf = NULL;

        // map_buf = (uint8_t *)malloc(decompress_size);
        

        //map_buf = current_full_map->data;


        for(int i = 2; i < _cols-2;i=i+3){
            for(int j = 2; j<_rows-2;j=j+3){ 
         
                caculateMapState(i,j,current_full_map->data);

            }
        }
        
       
        for(int i = 0 ;i<201;i++)
        {
            for(int j = 0;j<201;j++)
            {                
                if(_maze.Map[SIZE+99-j][SIZE+99-i]->n == 2)
                {
                    remap_sum = 0;
                    for(int a = -1 ; a< 2; a++)
                    {
                        for(int b =-1 ;b <2;b++)
                        {
                            if(_maze.Map[SIZE+99-j+a][SIZE+99-i+b]->n == 2)
                            {
                               remap_sum++; 
                            }
                        }
                    }
                    if(remap_sum < 2)
                    {
                        _maze.Map[SIZE+99-j][SIZE+99-i]->n = 0;
                    }
                }
            }
        }

    
        // free(map_buf);
        // map_buf = NULL;
       //free(current_full_map->data);

        //current_full_map->data = NULL;
    }

    void Maze::localMapUpdate()
    {
        FRIZY_LOG(LOG_INFO, "local map update from SLAM  mapservice");
    }

    void Maze::caculateMapState(int i,int j, uint8_t *map_buf)
    {
        int times1 = 0;
        int times2 = 0;
        int times3 = 0;
        static int ss = 1;

        // if (ss && IsWall()){

        //     for(int m = 0;m < 100000;++m){
        //         // if (map_buf[m] == 255){
        //         //     ss = 0;
        //         //     FRIZY_LOG(LOG_DEBUG,"cunzai %d",m);
        //         //     break;
        //         // }
        //         FRIZY_LOG(LOG_DEBUG,"cunzai %d",map_buf[m]);
        //     }

        //     for(int i = -50 ;i<=50;i++)
        //     {
        //         for(int j = -50;j<=50;j++){

        //             if (_maze.GetMapState(i,j,1) == -1){
        //                 printf("@300=%04d,%04d,00%d\n",i,j,4);
        //             }
        //             if (_maze.GetMapState(i,j,1) == 2){
        //                 printf("@300=%04d,%04d,00%d\n",i,j,2);
        //             }

        //         }   
        //     } 
        //     //FRIZY_LOG(LOG_DEBUG,"nothing..");    
        // }
        


        for(int a = 0;a < 3 ;a++)
        {
            // if (_maze.Map[SIZE+99-(i-2)/3][SIZE+99-(j-2)/3]->n!= 2)
            if(_maze.Map[SIZE+99-(j-2)/3][SIZE+99-(i-2)/3]->n != 3 && _maze.Map[SIZE+99-(j-2)/3][SIZE+99-(i-2)/3]->n != 4)
            {
                // if (_maze.Map[SIZE+99-(j-2)/3][SIZE+99-(i-2)/3]->n!= 2)

                if (_maze.Map[SIZE+99-(j-2)/3][SIZE+99-(i-2)/3]->n == 2)
                    _maze.Map[SIZE+99-(j-2)/3][SIZE+99-(i-2)/3]->n = 0;


                for (int b = 0;b < 3;b++)
                {
                    if(map_buf[(i+b)*map_rows+j+a] == 100)
                    {   
                        if(times1 > 1)
                        {
                            //FRIZY_LOG(LOG_DEBUG,"input ob!");
                            _maze.Map[SIZE+99-(j-2)/3][SIZE+99-(i-2)/3]->n = 2;
                        }
                        times1++;
                    }
                    else if (map_buf[(i+b)*map_rows+j+a] == 0){

                        if(times3 > 1)
                        {
                            //FRIZY_LOG(LOG_DEBUG,"input ob!");
                            _maze.Map[SIZE+99-(j-2)/3][SIZE+99-(i-2)/3]->n = 0;

                        }
                        times3++;                                               
                    }
                    else if(map_buf[(i+b)*map_rows+j+a] == 255)
                    {   
                        if(times2 > 1)
                        {
                            // _maze.Map[SIZE+99-(i-2)/3][SIZE+99-(j-2)/3]->n = 2;
                            _maze.Map[SIZE+99-(j-2)/3][SIZE+99-(i-2)/3]->n = -1;

                        }
                        times2++;
                    } 
 
                }
          
          
            }

        }

    } 
    
    void Maze::mapUpdateStart()
    {
       FRIZY_LOG(LOG_INFO, "mapservice thread start");
       mapupdate_thread_ = std::make_shared<std::thread>(&Maze::mapService, this);
    }

    void Maze::mapService()
    {
        while(1)
        {
            // FRIZY_LOG(LOG_DEBUG, "globalmap update");
            usleep(2000*1000);
            globalMapUpdate();
        }
    }

    void Maze::setMapArea()
    {
        FRIZY_LOG(LOG_DEBUG, "the blockconer info is %f , %f ,%f ,%f"
        ,current_planning_info.cleanBlock->bottomLeftCorner.x,
        current_planning_info.cleanBlock->bottomRightCorner.y,
        current_planning_info.cleanBlock->topLeftCorner.y,
        current_planning_info.cleanBlock->topRightCorner.x); 

        auto bottomLeftCorner_x = current_planning_info.cleanBlock->bottomLeftCorner.x *100 /15;
        auto bottomLeftCorner_y = current_planning_info.cleanBlock->bottomLeftCorner.y *100 /15;
        auto bottomRightCorner_x = current_planning_info.cleanBlock->bottomRightCorner.x *100 /15;
        auto bottomRightCorner_y = current_planning_info.cleanBlock->bottomRightCorner.y *100 /15; 

        auto topLeftCorner_x = current_planning_info.cleanBlock->topLeftCorner.x *100 /15;
        auto topLeftCorner_y = current_planning_info.cleanBlock->topLeftCorner.y *100 /15;
        auto topRightCorner_x = current_planning_info.cleanBlock->topRightCorner.x *100 /15;
        auto topRightCorner_y = current_planning_info.cleanBlock->topRightCorner.y *100 /15;

        // for test 
        // int bottomLeftCorner_x = 2 ;
        // int bottomRightCorner_x = 8 ;
        // int bottomLeftCorner_y = -8 ;
        // int topLeftCorner_y =  -2 ;
        for(int i = bottomLeftCorner_x ; i <bottomRightCorner_x; i++ )
        {
            setGlobalMapState(i,bottomLeftCorner_y,3);
            setGlobalMapState(i,topLeftCorner_y,3);
            // FRIZY_LOG(LOG_DEBUG,"_maze block point : d%,d%",i,bottomLeftCorner_y);
            
        }
        for(int i = bottomLeftCorner_y ; i <topLeftCorner_y; i++ )
        {
            setGlobalMapState(bottomLeftCorner_x,i,3);
            setGlobalMapState(bottomRightCorner_x,i,3);
            FRIZY_LOG(LOG_DEBUG,"_maze block point : %f,%f",bottomLeftCorner_x,i);
        }
              
    }
    void Maze::setForbindenInfo(bool type)
    {
        for(int i = 0 ; i < MAX_BLOCK_NBR;i++){

            //if(current_path_planning->prohibitedBlock.Block[i].isNew == 1)
            
            current_planning_info.forbidenArea.Block[i].bottomLeftCorner = current_path_planning->prohibitedBlock.Block[i].bottomLeftCorner;
            current_planning_info.forbidenArea.Block[i].bottomRightCorner = current_path_planning->prohibitedBlock.Block[i].bottomRightCorner;
            current_planning_info.forbidenArea.Block[i].topLeftCorner = current_path_planning->prohibitedBlock.Block[i].topLeftCorner;
            current_planning_info.forbidenArea.Block[i].topRightCorner = current_path_planning->prohibitedBlock.Block[i].topRightCorner;
                //planning_info->forbidenArea.Block[i].isNew = 1;
                    
            FRIZY_LOG(LOG_DEBUG, "forbin2 is %f  %f  %f %f"
            ,current_planning_info.forbidenArea.Block[i].bottomLeftCorner.x
            ,current_planning_info.forbidenArea.Block[i].bottomRightCorner.x
            ,current_planning_info.forbidenArea.Block[i].topLeftCorner.x
            ,current_planning_info.forbidenArea.Block[i].topRightCorner.x);                      
                
        }

        forbidenPoint.clear();
                    
        float seat_x = current_planning_info.charger_seat_position.x;
        float seat_y = current_planning_info.charger_seat_position.y;

        for(int i = -200; i < 200;i++)
        {
            for(int j = -200; j < 200;j++)
            {
                if (!seatPoint.empty()){

                    int seatX = round(seat_x/0.15);
                    int seatY = round(seat_y/0.15);
                    if (i <= seatX+2 && i >= seatX-2 && j <= seatY+2 && j >= seatY-2)
                        continue;
                }

                if (GetMapState(i,j,1) == 4){
                    InputRecord(i,j,0);
                    setGlobalMapState(i,j,0);
                }
                // if(_maze.Map[i][j]->n == 4)
                // {
                //     _maze.recordMap[i][j]->n = 0;
                //     printf("@300=%04d,%04d,00%d\n",i,j,0);
                //     _maze.Map[i][j]->n = 0;
                // }
            }
        }      

        FRIZY_LOG(LOG_DEBUG,"set jinqu %f %f  %d %d"
         ,seat_x,seat_y,int(round(seat_x/0.15)),int(round(seat_y/0.15)));
         
        // set the forbbinden info for Map
        //type = true;
        if(type == true)
        {
            seatPoint.clear();
            left_charger_index = false;
            FRIZY_LOG(LOG_DEBUG,"shezhiyici");

            _maze.Seat = {int(round(seat_x/0.15)),int(round(seat_y/0.15))};

            for(int i = -2;i <= 2;i++)
            {
                for(int j = -2;j <= 2; j++)
                {
                    int seatX = round(seat_x/0.15);
                    int seatY = round(seat_y/0.15);
                    // if (pow(i*i + j*j,0.5) > 2.1)
                    //     continue;

                    setGlobalMapState(i+seatX,j+seatY,4);
                }
            }

            for(float i = -25;i <= 25; i=i+5){
                for (float j = -25;j <= 25; j=j+5){

                    float tmp = pow(i*i + j*j,0.5);

                    if (tmp > 26){
                        continue;
                    }
                    
                    seatPoint.push_back({i/100+seat_x,j/100+seat_y});
                }
            }           
        }


        for(int i = 0; i< MAX_BLOCK_NBR ;i++)
        {   
            FRIZY_LOG(LOG_DEBUG,"isNew: %d",current_planning_info.forbidenArea.Block[i].isNew);
            //if (current_planning_info.forbidenArea.Block[i].isNew == 1)
            {
                Point2d leftDown = {current_planning_info.forbidenArea.Block[i].bottomLeftCorner.x
                                ,current_planning_info.forbidenArea.Block[i].bottomLeftCorner.y};
                Point2d leftUp = {current_planning_info.forbidenArea.Block[i].topLeftCorner.x
                                ,current_planning_info.forbidenArea.Block[i].topLeftCorner.y};
                Point2d rightDown = {current_planning_info.forbidenArea.Block[i].bottomRightCorner.x
                                ,current_planning_info.forbidenArea.Block[i].bottomRightCorner.y};
                Point2d rightUp = {current_planning_info.forbidenArea.Block[i].topRightCorner.x
                                ,current_planning_info.forbidenArea.Block[i].topRightCorner.y};                                

                FRIZY_LOG(LOG_DEBUG,"forbindenInfo leftDown:%f %f rightDown:%f %f leftUp:%f %f rightUp:%f %f"
                ,leftDown.x,leftDown.y,rightDown.x,rightDown.y,leftUp.x,leftUp.y,rightUp.x,rightUp.y);

                if (fabs(leftDown.x) < 0.0001 && fabs(leftDown.y) < 0.0001 && fabs(rightDown.x) < 0.0001 && fabs(rightDown.y) < 0.0001
                    && fabs(leftUp.x) < 0.0001 && fabs(leftUp.y) < 0.0001 && fabs(rightUp.x) < 0.0001 && fabs(rightUp.y) < 0.0001)
                    continue;
                
                if (fabs(leftDown.y - rightDown.y) <= 0.10){

                    FRIZY_LOG(LOG_DEBUG,"xuan1");


                    for(float x = min(leftDown.x,rightDown.x); x <= max(leftDown.x,rightDown.x); x += 0.05)
                    {
                        
                        forbidenPoint.push_back({x,leftDown.y});
                        forbidenPoint.push_back({x,leftUp.y});

                    }   
                    
                    for(float y = min(leftDown.y,leftUp.y); y <= max(leftDown.y,leftUp.y) ;y += 0.05)
                    {
                        forbidenPoint.push_back({leftDown.x,y});
                        forbidenPoint.push_back({rightDown.x,y});
                    }

                    //map
                    for(int x = min(leftDown.x,rightDown.x)/0.15; x <= max(leftDown.x,rightDown.x)/0.15 ;++x)
                    {       
                        for(int y = min(leftDown.y,leftUp.y)/0.15; y <= max(leftDown.y,leftUp.y)/0.15 ;++y) 
                        {   
                            setGlobalMapState(x,y,4);
                            FRIZY_LOG(LOG_DEBUG,"forbbiden_area point1 : %d,%d",x,y);
                        }
                    }
                }
                else if (fabs(leftDown.x - rightDown.x) <= 0.10){

                    FRIZY_LOG(LOG_DEBUG,"xuan2");
                    for(float y = min(rightDown.y,leftDown.y); y <= max(rightDown.y,leftDown.y); y += 0.05)
                    {
                        
                        forbidenPoint.push_back({leftDown.x,y});
                        forbidenPoint.push_back({leftUp.x,y});

                    }
                    
                    for(float x = min(leftDown.x,leftUp.x); x <= max(leftDown.x,leftUp.x) ;x += 0.05)
                    {
                        forbidenPoint.push_back({x,leftDown.y});
                        forbidenPoint.push_back({x,rightDown.y});
                    }

                    //map
                    for(int x = min(leftDown.x,leftUp.x)/0.15; x <= max(leftDown.x,leftUp.x)/0.15 ;++x)
                    {       
                        for(int y = min(rightDown.y,leftDown.y)/0.15; y <= max(rightDown.y,leftDown.y)/0.15 ;++y) 
                        {   
                            setGlobalMapState(x,y,4);
                            FRIZY_LOG(LOG_DEBUG,"forbbiden_area point2 : %d,%d",x,y);
                        }
                    }
                }
                //????????? 
                else   
                {

                    auto tmp_math_data = caculateSineCose(current_planning_info.forbidenArea.Block[i]);
                    auto tmp_normalize_point = coordinatenormalize(current_planning_info.forbidenArea.Block[i],tmp_math_data);
                    
                    FRIZY_LOG (LOG_DEBUG,"xuan3 : %f,%f",tmp_math_data.first,tmp_math_data.second);

                    for(double j = current_planning_info.forbidenArea.Block[i].bottomLeftCorner.x; j< tmp_normalize_point.first+0.05 ;j=j+0.05)
                    {
                        NormalizeConer tmp_bottom_data = {j,current_planning_info.forbidenArea.Block[i].bottomLeftCorner.y};
                        NormalizeConer tmp_top_data = {j,tmp_normalize_point.second};
                        tmp_bottom_data = antCoordinatenormalize(current_planning_info.forbidenArea.Block[i],tmp_bottom_data,tmp_math_data);
                        tmp_top_data = antCoordinatenormalize(current_planning_info.forbidenArea.Block[i],tmp_top_data,tmp_math_data);  
                        forbidenPoint.push_back(tmp_bottom_data);
                        forbidenPoint.push_back(tmp_top_data);
                    }

                    for(double m = current_planning_info.forbidenArea.Block[i].bottomLeftCorner.y; m< tmp_normalize_point.second+0.05 ;m=m+0.05)
                    {
                        NormalizeConer tmp_bottom_data = {current_planning_info.forbidenArea.Block[i].bottomLeftCorner.x,m};
                        NormalizeConer tmp_top_data = {current_planning_info.forbidenArea.Block[i].bottomRightCorner.x,m}; 
                        tmp_bottom_data = antCoordinatenormalize(current_planning_info.forbidenArea.Block[i],tmp_bottom_data,tmp_math_data);
                        tmp_top_data = antCoordinatenormalize(current_planning_info.forbidenArea.Block[i],tmp_top_data,tmp_math_data); 
                        forbidenPoint.push_back(tmp_bottom_data);
                        forbidenPoint.push_back(tmp_top_data);
                    }

                    //map
                    for(int j = current_planning_info.forbidenArea.Block[i].bottomLeftCorner.x*100/15; j< (tmp_normalize_point.first+0.15)*100/15 ;j++)
                    {       
                        for(double m = current_planning_info.forbidenArea.Block[i].bottomLeftCorner.y*100/15; m< (tmp_normalize_point.second+0.15)*100/15 ;m++) 
                        {    
                            NormalizeConer tmp_data = {j*0.15,m*0.15};
                            tmp_data = antCoordinatenormalize(current_planning_info.forbidenArea.Block[i],tmp_data,tmp_math_data);
                            setGlobalMapState(int(tmp_data.first),int(tmp_data.second),4);
                            FRIZY_LOG (LOG_DEBUG,"forbbiden_area point1 : %d,%d",int(tmp_data.first),int(tmp_data.second));
                        }
                    }
                }
            }
        }
        FRIZY_LOG(LOG_DEBUG,"set the forbindenInfo OK");

    }
    
    NormalizeConer Maze::coordinatenormalize(block &_block, Sine_cose_math & rotate_date)
    {
        float normalizeCorner_x;
        float normalizeCorner_y;
    
            
        float _sin_date = rotate_date.first;
        float _cos_date = rotate_date.second;

        normalizeCorner_x = (_block.topRightCorner.x - _block.bottomLeftCorner.x)*_cos_date
        -(_block.topRightCorner.y -_block.bottomLeftCorner.y)*_sin_date +_block.bottomLeftCorner.x;

        normalizeCorner_y = (_block.topRightCorner.y - _block.bottomLeftCorner.y)*_cos_date
        +(_block.topRightCorner.x -_block.bottomLeftCorner.x)*_sin_date +_block.bottomLeftCorner.y;

        NormalizeConer _normalizeConer = {normalizeCorner_x,normalizeCorner_y};

        return _normalizeConer;
    }
    NormalizeConer Maze::antCoordinatenormalize(block &_block,NormalizeConer &_Coner,Sine_cose_math &rotate_date)
    {
        float ant_normalizeCorner_x;
        float ant_normalizeCorner_y;
        
        float _sin_date = -rotate_date.first;
        float _cos_date = rotate_date.second;
        ant_normalizeCorner_x = (_Coner.first - _block.bottomLeftCorner.x)*_cos_date-(_Coner.second -_block.bottomLeftCorner.y)*_sin_date +_block.bottomLeftCorner.x;
        ant_normalizeCorner_y = (_Coner.second - _block.bottomLeftCorner.y)*_cos_date+(_Coner.first -_block.bottomLeftCorner.x)*_sin_date +_block.bottomLeftCorner.y;

        
        NormalizeConer ant_corner = {ant_normalizeCorner_x/0.15,ant_normalizeCorner_y/0.15};     
        return  ant_corner;          
    }

    Sine_cose_math Maze::caculateSineCose(block &_block)
    {
        float y_length = fabs(_block.bottomRightCorner.y - _block.bottomLeftCorner.y);
        float x_length = fabs(_block.bottomRightCorner.x -_block.bottomLeftCorner.x);
        float re_length =  hypot(y_length,x_length);
        float _sin_date = y_length/re_length;
        float _cos_date = x_length/re_length;
        Sine_cose_math _sine_cose_date = {_sin_date,_cos_date};
        return _sine_cose_date;
    }
}


