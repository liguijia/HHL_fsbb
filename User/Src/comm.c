#include "comm.h"
#include "fdcan.h"
#include "analog_signal.h"
#include "gpio.h"

#define my_hfdcan hfdcan1

DcdcOutputState dcdc_output_state = DCDC_OUTPUT_OUTPUT_DISABLED;
RxData can_rx_data                = {15, 0};
TxData can_tx_data;

static uint16_t can_recevie_cnt = 0;

DcdcOutputState UpdateDcdcOutputState(uint8_t IsEnabled)
{
    switch (dcdc_output_state) {
        case DCDC_OUTPUT_OUTPUT_DISABLED:
            if (IsEnabled) {
                dcdc_output_state = DCDC_OUTPUT_TRANSITION_TO_ENABLED;
            }
            break;
        case DCDC_OUTPUT_OUTPUT_ENABLED:
            if (!IsEnabled) {
                dcdc_output_state = DCDC_OUTPUT_TRANSITION_TO_DISABLED;
            }
            break;
        case DCDC_OUTPUT_TRANSITION_TO_ENABLED: {
            dcdc_output_state = DCDC_OUTPUT_OUTPUT_ENABLED;
            break;
        }

        case DCDC_OUTPUT_TRANSITION_TO_DISABLED: {
            dcdc_output_state = DCDC_OUTPUT_OUTPUT_DISABLED;
            break;
        }
        default: // 如果状态未知，返回关闭状态
        {
            dcdc_output_state = DCDC_OUTPUT_OUTPUT_DISABLED;
            break;
        }
    }

    // 如果没有状态变更，则保持原状态
    return dcdc_output_state;
}

void can_recevie_cnt_add(void)
{
    can_recevie_cnt++;
    if (can_recevie_cnt > CAN_DISCONNECT_MAX_COUNT) {
        can_recevie_cnt = CAN_DISCONNECT_MAX_COUNT + 1;
    }
}

void can_recevie_cnt_reset(void)
{
    can_recevie_cnt = 0;
}

uint16_t can_recevie_cnt_get(void)
{
    return can_recevie_cnt;
}

static int float2uint16_t(float x, float x_min, float x_max, int bits)
{
    if (x < x_min) {
        x = x_min;
    } else if (x > x_max) {
        x = x_max;
    }
    float span   = x_max - x_min;
    float offset = x_min;
    return (int)((x - offset) * ((float)((1 << bits) - 1)) / span);
}

// static uint16_t float2uint16_t(float value, float min, float max)
// {
//     float span = max - min;
//     if (span == 0)
//         return 0; // 防止除以零的情况
//     float offset = min;

//     // 将value从[min, max]范围映射到[0, 65535]
//     float mappedValue = ((value - offset) / span) * 65535.0f;

//     // 确保结果在uint16_t范围内
//     if (mappedValue < 0)
//         return 0;
//     if (mappedValue > 65535)
//         return 65535;

//     return (uint16_t)mappedValue;
// }

DcdcOutputState get_dcdc_output_state(void)
{
    return dcdc_output_state;
}

void comm_init(void)
{
    FDCAN_FilterTypeDef sFilterConfig;
    // 初始化滤波器
    // 配置FDCAN过滤器，仅接受来自RMCS的消息
    sFilterConfig.IdType       = FDCAN_STANDARD_ID;
    sFilterConfig.FilterIndex  = 0;
    sFilterConfig.FilterType   = FDCAN_FILTER_DUAL; // 使用双重过滤
    sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    sFilterConfig.FilterID1    = RMCS_ID;
    sFilterConfig.FilterID2    = LEGGED_ID;

    // 配置过滤器
    HAL_FDCAN_ConfigFilter(&my_hfdcan, &sFilterConfig);
    HAL_FDCAN_ConfigGlobalFilter(&my_hfdcan, FDCAN_REJECT, FDCAN_REJECT, FDCAN_REJECT_REMOTE, FDCAN_REJECT_REMOTE);

    // HAL_FDCAN_ConfigRxFifoOverwrite(&my_hfdcan, FDCAN_RX_FIFO0, FDCAN_RX_FIFO_OVERWRITE);
    // 激活过滤器
    HAL_FDCAN_ActivateNotification(&my_hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
    // 启动FDCAN外设
    HAL_FDCAN_Start(&my_hfdcan);
}

void can_send(void)
{
    FDCAN_TxHeaderTypeDef TxHeader;
    uint8_t data[8];

    // 设置消息头
    TxHeader.Identifier          = SUPERCAP_ID; // 消息ID
    TxHeader.IdType              = FDCAN_STANDARD_ID;
    TxHeader.TxFrameType         = FDCAN_DATA_FRAME;
    TxHeader.DataLength          = FDCAN_DLC_BYTES_8; // 数据长度码，这里是8字节
    TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    TxHeader.BitRateSwitch       = FDCAN_BRS_OFF;
    TxHeader.FDFormat            = FDCAN_CLASSIC_CAN;
    TxHeader.TxEventFifoControl  = FDCAN_NO_TX_EVENTS;
    TxHeader.MessageMarker       = 0;

    // 获取数据
    float chassis_power_temp    = 0.0f;
    float supercap_voltage_temp = 0.0f;
    float chassis_voltage_temp  = 0.0f;
    float chassis_current_temp  = 0.0f;

    uint16_t chassis_power;
    uint16_t supercap_voltage;
    uint16_t chassis_voltage;

    uint8_t IsDcdcEnabled = 0;

    // 获取数据
    supercap_voltage_temp = get_voltage_cap();
    chassis_voltage_temp  = get_voltage_motor();
    chassis_current_temp  = get_current_chassis();
    chassis_power_temp    = chassis_voltage_temp * chassis_current_temp;

    // 将数据转换为uint16_t类型
    chassis_power    = float2uint16_t(chassis_power_temp, 0.0f, 500.0f, 16);
    supercap_voltage = float2uint16_t(supercap_voltage_temp, 0.0f, 50.0f, 16);
    chassis_voltage  = float2uint16_t(chassis_voltage_temp, 0.0f, 50.0f, 16);

    // 判断DCDC是否开启
    if (DCDC_OUTPUT_OUTPUT_ENABLED == dcdc_output_state) {
        IsDcdcEnabled = 1;
    } else {
        IsDcdcEnabled = 0;
    }

    // 将txData结构体中的数据转换为字节数组
    data[1] = (uint8_t)(chassis_power >> 8);      // 高字节
    data[0] = (uint8_t)(chassis_power & 0xFF);    // 低字节
    data[3] = (uint8_t)(supercap_voltage >> 8);   // 高字节
    data[2] = (uint8_t)(supercap_voltage & 0xFF); // 低字节
    data[5] = (uint8_t)(chassis_voltage >> 8);    // 高字节
    data[4] = (uint8_t)(chassis_voltage & 0xFF);  // 低字节
    data[6] = IsDcdcEnabled;
    data[7] = 0x00; // unused

    // 发送数据
    if (HAL_FDCAN_GetTxFifoFreeLevel(&my_hfdcan) > 0) {
        HAL_FDCAN_AddMessageToTxFifoQ(&my_hfdcan, &TxHeader, data);
    }
}

// FDCAN接收中断处理函数
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{

    if (hfdcan->Instance == FDCAN1) {
        FDCAN_RxHeaderTypeDef RxHeader;
        uint8_t data[8];

        // 从FDCAN接收FIFO读取消息
        if (HAL_FDCAN_GetRxMessage(&my_hfdcan, FDCAN_RX_FIFO0, &RxHeader, data) == HAL_OK) {

            // 在这里处理接收到的数据
            if (RxHeader.Identifier == RMCS_ID) {
                can_recevie_cnt_reset();
                // 假设消息的前两个字节分别对应targetChassisPower和enabled

                // 正常使用
                // can_rx_data.targetChassisPower = data[6];
                // can_rx_data.enabled            = data[7];

                // test 测试
                can_rx_data.targetChassisPower = data[4];
                can_rx_data.enabled            = data[5];

                // 可以在这里添加更多的处理逻辑
                // HAL_GPIO_TogglePin(USR_LED_GPIO_Port, USR_LED_Pin);
                // 如果enabled不为1，则将其设置为0
                if (1 != can_rx_data.enabled) {
                    can_rx_data.enabled = 0;
                }
            }
        }
    }
}