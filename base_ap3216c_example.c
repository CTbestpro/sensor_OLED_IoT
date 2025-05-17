/*
 * Copyright (c) 2023 Beijing HuaQing YuanJian Education Technology Co., Ltd
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>              // 标准输入输出库，用于sprintf等函数
#include <unistd.h>             // UNIX标准函数库，提供sleep()函数
#include <string.h>             // 字符串操作函数库
#include "ohos_init.h"          // OpenHarmony系统初始化头文件
#include "cmsis_os2.h"          // CMSIS-RTOS2接口头文件
#include "hal_bsp_ap3216c.h"    // AP3216C三合一传感器驱动
#include "hal_bsp_ssd1306.h"    // SSD1306 OLED显示屏驱动
#include "hal_bsp_pcf8574.h"    // PCF8574 I2C扩展芯片驱动

osThreadId_t SensorTask_ID;     // 定义任务句柄用于传感器显示任务
#define TASK_STACK_SIZE 1024    // 定义任务栈大小为1024字节

void SensorDisplayTask(void)
{
    uint16_t ir = 0, als = 0, ps = 0;  // 定义传感器数据存储变量
    uint8_t displayBuff[20] = {0};     // 定义OLED显示缓冲区（20字节）

    // OLED初始化显示
    SSD1306_CLS();  // 清空OLED屏幕
    SSD1306_ShowStr(OLED_TEXT16_COLUMN_0, OLED_TEXT16_LINE_0, // 在屏幕第0行显示标题
                    " AP3216C Sensor ", TEXT_SIZE_16);

    while (1) {      // 任务主循环
        // 读取传感器数据（通过指针传递参数）
        AP3216C_ReadData(&ir, &als, &ps);

        /**************** 显示IR数据 ****************/
        memset_s(displayBuff, sizeof(displayBuff), 0, sizeof(displayBuff)); // 清空缓冲区
        sprintf_s(displayBuff, sizeof(displayBuff), " IR: %-5d ", ir);      // 格式化字符串
        SSD1306_ShowStr(OLED_TEXT16_COLUMN_0, OLED_TEXT16_LINE_1,           // 在第1行显示
                        displayBuff, TEXT_SIZE_16);

        /**************** 显示ALS数据 ****************/
        memset_s(displayBuff, sizeof(displayBuff), 0, sizeof(displayBuff));
        sprintf_s(displayBuff, sizeof(displayBuff), " ALS: %-5d ", als); 
        SSD1306_ShowStr(OLED_TEXT16_COLUMN_0, OLED_TEXT16_LINE_2, 
                        displayBuff, TEXT_SIZE_16);  // 在第2行显示

        /**************** 显示PS数据 ****************/
        memset_s(displayBuff, sizeof(displayBuff), 0, sizeof(displayBuff));
        sprintf_s(displayBuff, sizeof(displayBuff), " PS: %-5d ", ps);
        SSD1306_ShowStr(OLED_TEXT16_COLUMN_0, OLED_TEXT16_LINE_3, 
                        displayBuff, TEXT_SIZE_16);  // 在第3行显示

        sleep(1);    // 挂起任务1秒（每秒更新一次数据）
    }
}

static void SensorOledDemo(void)
{
    printf("Start Sensor OLED Demo!\r\n");  // 串口打印启动信息

    // 硬件初始化序列
    PCF8574_Init();     // 初始化PCF8574 I2C扩展芯片（必须最先初始化）
    AP3216C_Init();     // 初始化AP3216C三合一传感器（依赖I2C总线）
    SSD1306_Init();     // 初始化SSD1306 OLED显示屏（依赖I2C总线）

    // 创建传感器显示任务
    osThreadAttr_t attr = {            // 定义任务属性结构体
        .name = "SensorDisplayTask",   // 任务名称（调试用）
        .attr_bits = 0,                // 任务属性位（默认0）
        .cb_mem = NULL,                // 任务控制块内存地址（动态分配）
        .cb_size = 0,                  // 任务控制块大小
        .stack_mem = NULL,             // 任务栈内存地址（动态分配）
        .stack_size = TASK_STACK_SIZE, // 任务栈大小（1024字节）
        .priority = osPriorityNormal   // 任务优先级（普通优先级）
    };

    // 创建并启动任务
    SensorTask_ID = osThreadNew(SensorDisplayTask, // 任务函数指针
                                NULL,             // 任务参数（无）
                                &attr);           // 任务属性
    if (SensorTask_ID != NULL) {                  // 判断任务创建是否成功
        printf("Create SensorDisplayTask Success!\r\n"); // 串口输出成功信息
    }
}

// 系统启动时自动注册模块入口
SYS_RUN(SensorOledDemo);  // 使用OpenHarmony宏注册初始化函数



 typedef struct {
    const char*         name;       // 任务标识符
    uint32_t            attr_bits;  // 特殊属性标记
    void*               cb_mem;     // 控制块存储地址
    uint32_t            cb_size;    // 控制块大小
    void*               stack_mem;  // 栈存储地址
    uint32_t            stack_size; // 栈空间大小
    osPriority_t        priority;   // 执行优先级
  } osThreadAttr_t;
  

- **传感器数据结构体（隐式）**
  struct SensorData {
    uint16_t ir;        // 红外值（大端序）
    uint16_t als;       // 环境光值（小端序） 
    uint8_t ps;         // 接近检测值
    uint8_t _reserved;  // 对齐填充
  };