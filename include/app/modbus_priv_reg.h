#ifndef _MODBUS_PRIV_REG_H
#define _MODBUS_PRIV_REG_H

#include <stdint.h>
#include <stddef.h>

#define REG_NUM(end, start) ((end) - (start))

/*********************************************************************************/
//通用寄存器
typedef enum {
	REG_0000_0011_SOFTWARE_VERTION,
	REG_0000_0011_HARDWARE_VERTION,
	REG_0000_0011_PROTOCOL_VERTION,
	REG_0000_0011_FACTORT_ID,
	REG_0000_0011_VERTION_TYPE,
	REG_0000_0011_DEVICE_TYPE,
	REG_0000_0011_PROJECT_ID,
	REG_0000_0011_RESERVED, //保留值
	REG_0000_0011_RDWR_TEST = 9,
	REG_0000_0011_PWR_ON_FLAG,
	REG_0000_0011_CLEAR_TEXT_RESET,
	REG_0000_0011_END = REG_0000_0011_CLEAR_TEXT_RESET + 1,
} RTU_PRIV_REG_0000_0011_E;

typedef union {
	struct {
		uint16_t sv;
		uint16_t hv;
		uint16_t protocol_ver;
		uint16_t factory_id;
		uint16_t vertion_type;
		uint16_t device_type;
		uint16_t project_id;
		uint16_t reserved[2];
		uint16_t rdwr_test;
		uint16_t prower_on_flag;
		uint16_t reset;
	};
	uint16_t all_data[REG_NUM(REG_0000_0011_END, REG_0000_0011_SOFTWARE_VERTION)];
} reg_0000_0011;

/*********************************************************************************/
//设备明文参数
typedef enum {
	REG_1000_1199_BAT_ID = 1000,
	REG_1000_1199_TOTAL_VOLTAGE = 1003,
	REG_1000_1199_SOC,
	REG_1000_1199_CELL_TEMP,
	REG_1000_1199_ERRORO_STATUS = 1009,
	REG_1000_1199_CELL_VOLTAGE = 1011,
	REG_1000_1199_BAT_ENERGY = 1041,
	REG_1000_1199_BAT_CURRENT,
	REG_1000_1199_CALIBRATION_STATUS,
	REG_1000_1199_SOH,
	REG_1000_1199_RESERVER,
	REG_1000_1199_USER_ID = 1050,
	REG_1000_1199_LED_CTRL = 1053,
	REG_1000_1199_PWR_SUPPLY_REQ,
	REG_1000_1199_BAT_CAP_RATIO,
	REG_1000_1199_UNDER_VOLTAGE,
	REG_1000_1199_MAX_DISCHARGE_CUR,
	REG_1000_1199_MAX_CHARGE_CUR,
	REG_1000_1199_PROTECT_STATUS,
	REG_1000_1199_CHARGER_ERROR_STATUS, //充电机故障码,预留
	REG_1000_1199_MOS_TEMP,
	REG_1000_1199_EX_CLEAR_ERR_CODE_1 = 1065,
	REG_1000_1199_EX_CLEAR_ERR_CODE_2 = 1067,
	REG_1000_1199_LOAD_STATUS = 1069,

	REG_1000_1199_END = REG_1000_1199_LOAD_STATUS + 1,
} RTU_PRIV_REG_1000_1199_E;

#define BAT_INSIDE_STATUS 1190

typedef union {
	struct {
		uint16_t bat_id[3];
		uint16_t total_voltage;
		uint16_t soc;
		uint16_t cell_temperature[4];
		uint16_t error_status[2];
		uint16_t cell_voltage[30];
		uint16_t bat_energe;
		uint16_t bat_current;
		uint16_t calibration_status;
		uint16_t soh;
		uint16_t reserved[5]; //保留值
		uint16_t user_id[3];
		uint16_t led_ctrl_data;
		uint16_t power_supply_request;
		uint16_t bat_cap_ratio;
		uint16_t bat_uv_value;
		uint16_t max_dsg_crnt;
		uint16_t max_charge_crnt;
		uint16_t protect_status;
		uint16_t charger_error_status;
		uint16_t ex_temperature[4];
		uint16_t ex_error_code_1[2];
		uint16_t ex_error_code_2[2];
		uint16_t loader_status;
	};
	uint16_t all_data[REG_NUM(REG_1000_1199_END, REG_1000_1199_BAT_ID)];
} reg_1000_1199;

/*********************************************************************************/
//设备密文参数
typedef enum {
	REG_1200_1299_BAT_ID = 1200,
	REG_1200_1299_BAT_SECRET_KEY = 1203,
	REG_1200_1299_DSGHARGE_STATUS = 1211,
	REG_1200_1299_BLE_MAC,
	REG_1200_1299_PCB_SERIAL_NUM = 1215,
	REG_1200_1299_PACK_VOLTAGE = 1223,
	REG_1200_1299_SUPPLY_VOLTAGE_2G,
	REG_1200_1299_LED_SUPPLY_VOLTAGE,
	REG_1200_1299_LDR_SUPPLY_VOLTAGE,
	REG_1200_1299_NTC_COULOMB,
	REG_1200_1299_NTC_CHIP,
	REG_1200_1299_MODE,
	REG_1200_1299_ERROR_CODE,
	REG_1200_1299_HISTORT = 1232,
	REG_1200_1299_RESET_COULOMB,
	REG_1200_1299_USER_KEY,
	REG_1200_1299_RESET_BMS_CODE = 1242,
	REG_1200_1299_CHGGE_MOS_STATUS,
	REG_1200_1299_DSGHARGE_MOS_STATUS,
	REG_1200_1299_RUNNING_STATUS,
	REG_1200_1299_FAST_CHGGE_CMD,
	REG_1200_1299_FAST_CHGGE_MAX_VOLTAGE,
	REG_1200_1299_FAST_CHGGE_CURRENT,
	REG_1200_1299_FAST_CHGGE_ENABLE,
	REG_1200_1299_COULOMB_VERTION,
	REG_1200_1299_COULOMB_ALGORITHM_ENABLE,
	REG_1200_1299_CAPACITY_ACCURATE_FLAG,
	REG_1200_1299_CELL_TYPE,
	REG_1200_1299_BAT_CONFIG_INFO,
	REG_1200_1299_COULONB_VER_WR_STATUS,
	REG_1200_1299_DSGONNECT_DSGHAGRE_TIMEOUT,
	REG_1200_1299_CONNECT_DSGHAGRE_TIMEOUT,
	REG_1200_1299_NEED_SOC_CALIBRATION,
	REG_1200_1299_LOAD_STATUS,
	REG_1200_1299_UTC_TIMESTAMP,
	REG_1200_1299_BMS_PWR_OFF_CMD = 1262,
	REG_1200_1299_DEVICE_TYOE,
	REG_1200_1299_COMMUNICATION_TIMEOUT,
	REG_1200_1299_EX_SECRET_ERR_CODE_1,
	REG_1200_1299_EX_SECRET_ERR_CODE_2 = 1267,
	REG_1200_1299_KEEP_DSGGARGE_STATUS = 1269,
	REG_1200_1299_SECRET_PROTECT_STATUS,

	REG_1200_1299_END = REG_1200_1299_SECRET_PROTECT_STATUS + 1,
} RTU_PRIV_REG_1200_1299_E;

typedef union {
	struct {
		uint16_t bat_id[3];
		uint16_t bat_key[8];
		uint16_t discharge_status;
		uint16_t ble_mac_addr[3];
		uint16_t serial_num[8]; //电池秘钥
		uint16_t pack_voltage;
		uint16_t power_2g;
		uint16_t power_led;
		uint16_t power_ldr;
		uint16_t ntc_coloumn;
		uint16_t ntc_chip;
		uint16_t mode;
		uint16_t error_code[2];
		uint16_t history;
		uint16_t reset_voltameter;
		uint16_t user_key[8];
		uint16_t reset_bms;
		uint16_t charge_mos_status;
		uint16_t discharge_mos_status;
		uint16_t running_status;
		uint16_t fast_charge_cmd;
		uint16_t fast_charge_vol;
		uint16_t fasr_charge_current;
		uint16_t fast_charge_enable;
		uint16_t bq34_version;
		uint16_t voltameter_enable;
		uint16_t bq34_capacity_accurate_flag;
		uint16_t bq34_battery_version;
		uint16_t bq34_soft_version;
		uint16_t bq34_magic_number_status;
		uint16_t timeout_1;
		uint16_t timeout_2;
		uint16_t full_charge_status;
		uint16_t load_status;
		uint16_t utc_timestamp[2];
		uint16_t bms_poweroff_cmd;
		uint16_t connect_device_type;
		uint16_t keep_comm_time;
		uint16_t error_code_1[2];
		uint16_t error_code_2[2];
		uint16_t keep_discharge_status;
		uint16_t protect_status;
	};
	uint16_t all_data[REG_NUM(REG_1200_1299_END, REG_1200_1299_BAT_ID)];
} reg_1200_1299;

/*********************************************************************************/
//不懂
typedef enum {
	REG_2000_2099_FRAME_NUM = 2000,
	REG_2000_2099_FRAME_IDX,
	REG_2000_2099_VALID_REG_NUM,
	REG_2000_2099_FRAME_UTC_TIMESTAMP,
	REG_2000_2099_FRAME_TYPE = 2005,
	REG_2000_2099_SOFTWARE_VERTION,
	REG_2000_2099_HARDWARE_VERTION,
	REG_2000_2099_PROTOCOL_VERTION,
	REG_2000_2099_FACTORT_ID,
	REG_2000_2099_PROJECT_ID,
	REG_2000_2099_BAT_ID,
	REG_2000_2099_BAT_TOTAL_VOLTAGE = 2014,
	REG_2000_2099_SOC,
	REG_2000_2099_TEMP,
	REG_2000_2099_ERR_STATUS = 2020,
	REG_2000_2099_CELL_VOL = 2022,
	REG_2000_2099_BAT_ENERGY = 2052,
	REG_2000_2099_CURRENT,
	REG_2000_2099_FULL_CALIB,
	REG_2000_2099_SOH,
	REG_2000_2099_AI_CHGGE_STATUS,
	REG_2000_2099_FAST_MAX_CHGGE_VOL,
	REG_2000_2099_FAST_MAX_CHGGE_CUR,
	REG_2000_2099_PACK_VOL,
	REG_2000_2099_MOS_STATUS,
	REG_2000_2099_BAT_CAP_RATIO,
	REG_2000_2099_UVP_VALUE,
	REG_2000_2099_CONT_DSG_CUR,
	REG_2000_2099_MOS_TEMP,
	REG_2000_2099_EX_ERR_CODE_1 = 2068,
	REG_2000_2099_EX_ERR_CODE_2 = 2070,
	REG_2000_2099_KP_DSG_STA = 2072,
	REG_2000_2099_PROT_STATUS,
	REG_2000_2099_CYCLE_CHG_TIME,
	REG_2000_2099_BAT_CFG_INFO = 2076,

	REG_2000_2099_END = REG_2000_2099_BAT_CFG_INFO + 1,
} RTU_PRIV_REG_2000_2099_E;

typedef union {
	struct {
		uint16_t frame_total_num;
		uint16_t frame_serial_num;
		uint16_t reg_valid_num;
		uint16_t utc_timestamp[2];
		uint16_t data_type;
		uint16_t hv;
		uint16_t sv;
		uint16_t protocol_version;
		uint16_t factory_id;
		uint16_t project_id;
		uint16_t bat_id[3];
		uint16_t total_voltage;
		uint16_t soc;
		uint16_t temperature[4];
		uint16_t error_status[2];
		uint16_t cell_voltage[30];
		uint16_t battery_energy;
		uint16_t currents;
		uint16_t full_charge_status;
		uint16_t soh;
		uint16_t fast_charge_cmd;
		uint16_t fast_charge_vol;
		uint16_t fasr_charge_current;
		uint16_t pack_volt;
		uint16_t mos_status;
		uint16_t bat_cap_ratio;
		uint16_t uv_volt;
		uint16_t max_dsg_crnt;
		uint16_t ex_temperature[4];
		uint16_t ext_error_code_1[2];
		uint16_t ext_error_code_2[2];
		uint16_t keep_discharge_status;
		uint16_t protect_status;
		uint16_t cumulate_charge_discharge_filled_count[2];
		uint16_t bq34_soft_version;
	};
	uint16_t all_data[REG_NUM(REG_2000_2099_END, REG_2000_2099_FRAME_NUM)];
} reg_2000_2099;

/*********************************************************************************/
//不懂
typedef enum {
	REG_3000_3099_FRAME_NUM = 3000,
	REG_3000_3099_FRAME_IDX,
	REG_3000_3099_VALID_REG_NUM,
	REG_3000_3099_FRAME_UTC_TIMESTAMP,
	REG_3000_3099_FRAME_TYPE = 3005,
	REG_3000_3099_SOFTWARE_VERTION,
	REG_3000_3099_HARDWARE_VERTION,
	REG_3000_3099_PROTOCOL_VERTION,
	REG_3000_3099_FACTORT_ID,
	REG_3000_3099_PROJECT_ID,
	REG_3000_3099_BAT_ID,
	REG_3000_3099_ERR_CODE = 3014,
	REG_3000_3099_LAST_DSG_MAX_CUR = 3016,
	REG_3000_3099_LAST_DSG_MAX_TEMP,
	REG_3000_3099_LAST_CHG_MAX_CUR = 3021,
	REG_3000_3099_LAST_CHG_TEMP,
	REG_3000_3099_LAST_DSG_MIN_TEMP = 3026,
	REG_3000_3099_LAST_BEFORE_CHG_VOL = 3030,
	REG_3000_3099_LAST_AFTER_CHG_VOL,
	REG_3000_3099_LAST_CHGGE_TIME,
	REG_3000_3099_REALTIME_DIFF_MAX_VOL = 3034,
	REG_3000_3099_TOTAL_CHGGE_TIME,
	REG_3000_3099_TOTAL_DSGHARGE_TIME = 3037,
	REG_3000_3099_TOTAL_CYC_CHG_CAP = 3039,
	REG_3000_3099_DSG_DIFF_MAX_VOL = 3041,
	REG_3000_3099_FULL_DIFF_MAX_VOL = 3042,
	REG_3000_3099_HISTORT_MIN_VOL,
	REG_3000_3099_HISTORT_MAX_VOL,
	REG_3000_3099_HISTORT_MIN_CHG_TEMP = 3045,
	REG_3000_3099_HISTORT_MAX_CHG_TEMP = 3049,
	REG_3000_3099_HISTORT_MIN_DSG_TEMP = 3053,
	REG_3000_3099_HISTORT_MAX_DSG_TEMP = 3057,
	REG_3000_3099_HISTORT_MIN_IDLE_TEMP = 3061,
	REG_3000_3099_HISTORT_MAX_IDLE_TEMP = 3065,
	REG_3000_3099_HISTORT_MIN_DSG_CUR = 3069,
	REG_3000_3099_HISTORT_MAX_DSG_CUR,
	REG_3000_3099_DSG_MAX_DIF_VOL_MAP_CYC,
	REG_3000_3099_FULL_MAX_DIF_VOL_MAP_CYC = 3073,
	REG_3000_3099_HIST_MIN_VOL_MAP_CYC_CAP = 3075,
	REG_3000_3099_HIST_MAX_VOL_MAP_CYC_CAP = 3077,
	REG_3000_3099_HIST_MIN_CHG_TEMP_MAP_CYC_CAP = 3079,
	REG_3000_3099_HIST_MAX_CHG_TEMP_MAP_CYC_CAP = 3081,
	REG_3000_3099_HIST_MIN_DSG_TEMP_MAP_CYC_CAP = 3083,
	REG_3000_3099_HIST_MAX_DSG_TEMP_MAP_CYC_CAP = 3085,
	REG_3000_3099_HIST_MIN_IDLE_TEMP_MAP_CYC_CAP = 3087,
	REG_3000_3099_HIST_MAX_IDLE_TEMP_MAP_CYC_CAP = 3089,
	REG_3000_3099_HIST_MAX_DSG_CUR_MAP_CYC_CAP = 3091,
	REG_3000_3099_HIST_MAX_CHG_CUR_MAP_CYC_CAP = 3093,

	REG_3000_3099_END = REG_3000_3099_HIST_MAX_CHG_CUR_MAP_CYC_CAP + 2,
} RTU_PRIV_REG_3000_3099_E;

typedef union {
	struct {
		uint16_t frame_total_num;
		uint16_t frame_serial_num;
		uint16_t reg_valid_num;
		uint16_t utc_timestamp[2];
		uint16_t data_type;
		uint16_t hv;
		uint16_t sv;
		uint16_t protocol_version;
		uint16_t factory_id;
		uint16_t project_id;
		uint16_t bat_id[3];
		uint16_t error_status[2];
		uint16_t discharge_current_max;
		uint16_t temperature_max[4];
		uint16_t last_time_charge_current_max;
		uint16_t last_time_charge_temp_max[4];
		uint16_t last_time_charge_temp_low[4];
		uint16_t last_time_before_charge_val;
		uint16_t last_time_after_charge_val;
		uint16_t last_time_charge_time[2];
		uint16_t batter_cell_diff_val_max;
		uint16_t cumulate_charge_time[2];
		uint16_t cumulate_discharge_time[2];
		uint16_t cumulate_charge_discharge_filled_count[2];
		uint16_t discharge_cell_diff_max;
		uint16_t full_cacpcity_cell_diff_max;
		uint16_t history_min_volt;
		uint16_t history_max_volt;
		uint16_t history_charge_min_temp[4];
		uint16_t history_charge_max_temp[4];
		uint16_t history_discharge_min_temp[4];
		uint16_t history_discharge_max_temp[4];
		uint16_t history_idle_min_temp[4];
		uint16_t history_idle_max_temp[4];
		uint16_t history_discharge_max_current;
		uint16_t history_charge_max_current;
		uint16_t discharge_cell_diff_max_for_charge_filled_count[2];
		uint16_t full_cacpcity_cell_diff_max_for_charge_filled_count[2];
		uint16_t history_min_volt_for_charge_filled_count[2];
		uint16_t history_max_volt_for_charge_filled_count[2];
		uint16_t history_charge_min_temp_for_charge_filled_count[2];
		uint16_t history_charge_max_temp_for_charge_filled_count[2];
		uint16_t history_discharge_min_temp_for_charge_filled_count[2];
		uint16_t history_discharge_max_temp_for_charge_filled_count[2];
		uint16_t history_idle_min_temp_for_charge_filled_count[2];
		uint16_t history_idle_max_temp_for_charge_filled_count[2];
		uint16_t history_discharge_max_current_for_charge_filled_count[2];
		uint16_t history_charge_max_current_for_charge_filled_count[2];
	};
	uint16_t all_data[REG_NUM(REG_3000_3099_END, REG_3000_3099_FRAME_NUM)];
} reg_3000_3099;

/*********************************************************************************/
//实时数据
typedef enum {
	REG_5000_5099_FRAME_NUM = 5000,
	REG_5000_5099_FRAME_IDX,
	REG_5000_5099_VALID_REG_NUM,
	REG_5000_5099_FRAME_UTC_TIMESTAMP,
	REG_5000_5099_FRAME_TYPE = 5005,
	REG_5000_5099_SOFTWARE_VERTION,
	REG_5000_5099_HARDWARE_VERTION,
	REG_5000_5099_PROTOCOL_VERTION,
	REG_5000_5099_FACTORT_ID,
	REG_5000_5099_PROJECT_ID,
	REG_5000_5099_BAT_ID,
	REG_5000_5099_REMAIN_RECORD_DATA = 5014,
	REG_5000_5099_DATA_IDX,
	REG_5000_5099_RELATIVE_TIMESTAMP = 5017,
	REG_5000_5099_ERR_CODE = 5019,
	REG_5000_5099_RUN_STATUS = 5021,
	REG_5000_5099_MOS_STATUS,
	REG_5000_5099_CPU_TEMP,
	REG_5000_5099_COULOMB_TEMP,
	REG_5000_5099_CELL_TEMP,
	REG_5000_5099_SOC = 5029,
	REG_5000_5099_CURRENT,
	REG_5000_5099_PACK_VOLTAGE,
	REG_5000_5099_CELL_VOLTAGE,
	REG_5000_5099_MAX_CYCLE_COUNT = 5062,
	REG_5000_5099_SOH = 5064,
	REG_5000_5099_ENERGY,
	REG_5000_5099_MOS_TEMP,
	REG_5000_5099_EX_ERR_COED_1 = 5070,
	REG_5000_5099_EX_ERR_COED_2 = 5072,
	REG_5000_5099_KEEP_DSG_STATUS = 5074,
	REG_5000_5099_PROT_STATUS,

	REG_5000_5099_END = REG_5000_5099_PROT_STATUS + 1,
} RTU_REALTIME_5000_5099_E;

typedef union {
	struct {
		uint16_t frame_total_num;
		uint16_t frame_serial_num;
		uint16_t reg_valid_num;
		uint16_t utc_timestamp[2];
		uint16_t data_type;
		uint16_t hv;
		uint16_t sv;
		uint16_t protocol_version;
		uint16_t factory_id;
		uint16_t project_id;
		uint16_t bat_id[3];

		uint16_t remains;
		uint16_t cur_index[2];
		uint16_t timestamp[2];
		uint16_t err_code[2];
		uint16_t run_status;
		uint16_t mos_status;
		uint16_t cpu_temp;
		uint16_t bq34_inter_temp;
		uint16_t bat_temp[4];
		uint16_t capacity;
		uint16_t current;

		uint16_t pack_volt;
		uint16_t cell_volt[30];
		uint16_t cumulate_charge_discharge_filled_count[2];
		uint16_t soh;
		uint16_t energy;
		uint16_t ex_temperature[4];
		uint16_t ext_error_code_1[2];
		uint16_t ext_error_code_2[2];
		uint16_t keep_discharge_status;
		uint16_t protect_status;
	};
	uint16_t all_data[REG_NUM(REG_5000_5099_END, REG_5000_5099_FRAME_NUM)];
} reg_5000_5099;

/*********************************************************************************/
//测试指令,参数设置
typedef enum {
	REG_55000_55229_BMS_MODE = 55000,
	REG_55000_55229_BMS_SN,
	REG_55000_55229_BAT_SN = 55050,
	REG_55000_55229_DEVICE_TRIPLE_CODE = 55100,
	REG_55000_55229_CHG_MOS_CTRL = 55200,
	REG_55000_55229_DSG_MOS_CTRL,
	REG_55000_55229_PDSG_MOS_CTRL,
	REG_55000_55229_BALANCE_CTRL_MODE,
	REG_55000_55229_BALANCE_CTRL_CMD,
	REG_55000_55229_SLEEP_SET_CMD = 55206,
	REG_55000_55229_PERIOD_RECORD_SAVE_TIME,
	REG_55000_55229_CLEAR_FAULT,
	REG_55000_55229_SET_PRODUCT_MODE,
	REG_55000_55229_SET_TIMESTAMP,
	REG_55000_55229_FIND_BUZZER = 55212,
	REG_55000_55229_V_CALIBRATE = 55220,
	REG_55000_55229_I_CALIBRATE,
	REG_55000_55229_SOC_CALIBRATE = 55223,
	REG_55000_55229_CAPACITY_SET,
	REG_55000_55229_ALARM_ARGV_SET,

	REG_55000_55229_END = REG_55000_55229_ALARM_ARGV_SET + 1,
} RTU_55000_55229_E;

//校准结果
#define REG_CALIBRATE_RESULT_ADDR 55502
#define REG_CALIBRATE_RESULT_SIZE 1

typedef union {
	struct {
		uint16_t mode;
		uint16_t bms_sn[30];
		uint16_t reserved_data_1[19]; //保留
		uint16_t bat_sn[30];
		uint16_t reserved_data_2[20]; //保留

		uint16_t device_id[10];
		uint16_t pid[5];
		uint16_t device_secret[16];

		uint16_t reserved_data_3[19]; //保留
		uint16_t reserved_data_4[50]; //保留
		uint16_t chg_mos_ctrl;
		uint16_t dsg_mos_ctrl;
		uint16_t pdsg_mos_ctrl;
		uint16_t balance_mode;
		uint16_t balance_ctrl[2];
		uint16_t sleep_mode_cmd;
		uint16_t period_record_time;
		uint16_t clear_fault;
		uint16_t set_product_mode;
		uint16_t set_time[2];
		uint16_t find_buzzer;
		uint16_t reserved_data_5[7]; //保留
		uint16_t cell_calibration;
		uint16_t current_calibration[2];
		uint16_t soc_calibration;
		uint16_t set_capacity;
		uint16_t alarm_argv_set;
	};
	uint16_t all_data[REG_NUM(REG_55000_55229_END, REG_55000_55229_BMS_MODE)];
} reg_55000_55229;

/*********************************************************************************/
//电池基础数据

typedef struct {
	uint16_t error_code[6];
	uint16_t cell_voltage[24];
	uint16_t cell_temperature[4];
	uint16_t current[2];
	uint16_t mos_temperature;
	uint16_t pdsg_mos_temperature;
	uint16_t max_cell_vol;
	uint16_t max_cell_vol_index;
	uint16_t min_cell_vol;
	uint16_t min_cell_vol_index;
	uint16_t max_temperature;
	uint16_t min_temperature;
	uint16_t soc;
	uint16_t soh;
	uint16_t max_allow_dsg_i[2];
	uint16_t max_allow_chg_i[2];
	uint16_t full_capacity;
	union {
		struct {
			uint16_t chg_mos_status : 1;
			uint16_t dsg_mos_status : 1;
			uint16_t pdsg_mos_status : 1;
		};
		uint16_t mos_data;
	} mos_status;
	uint16_t charge_detection;
	uint16_t loader_detection;
	uint16_t det_detction;
	uint16_t hv_adc;
	uint16_t pdsg_current;
	uint16_t comm_blue_status;
	uint16_t comm_4g_status;
	uint16_t self_check;
	uint16_t wakeup_reason;
	uint16_t heart_beat;
	uint16_t total_reset_time;
	uint16_t cycle_times;
	uint16_t total_chg_capacity[2];
	uint16_t total_dsg_capacity[2];
	uint16_t balance_status[2];
	uint16_t total_balance_time[20];
	uint16_t vol_diff;
	uint16_t total_voltage;
	uint16_t env_temperature;
	uint16_t reserved_data[7];
} bat_basic_data;

/*********************************************************************************/
//周期读取数据
typedef enum {
	REG_55600_55799_HV_VERSION = 55600,
	REG_55600_55799_SV_VERSION,
	REG_55600_55799_VENDOR_ID,
	REG_55600_55799_PID,
	REG_55600_55799_PROTOCOL_ID,

	BAT_BASIC_DATA_ERROR_CODE, //电池基础数据
	BAT_BASIC_DATA_CELL_VOL = BAT_BASIC_DATA_ERROR_CODE + 6,
	BAT_BASIC_DATA_CELL_TEMPERATURE = BAT_BASIC_DATA_CELL_VOL + 24,
	BAT_BASIC_DATA_CURRENT = BAT_BASIC_DATA_CELL_TEMPERATURE + 4,
	BAT_BASIC_DATA_MOS_TEMPERATURE = BAT_BASIC_DATA_CURRENT + 2,
	BAT_BASIC_DATA_PDSG_MOS_TEMPERATURE,
	BAT_BASIC_DATA_MAX_CELL_VOL,
	BAT_BASIC_DATA_MAX_CELL_INDEX,
	BAT_BASIC_DATA_MIN_CELL_VOL,
	BAT_BASIC_DATA_MIN_CELL_INDEX,
	BAT_BASIC_DATA_MAX_TEMPERATURE,
	BAT_BASIC_DATA_MIN_TEMPERATURE,
	BAT_BASIC_DATA_SOC,
	BAT_BASIC_DATA_SOH,
	BAT_BASIC_DATA_ALLOW_MAX_DSG_I,
	BAT_BASIC_DATA_ALLOW_MAX_CHG_I = BAT_BASIC_DATA_ALLOW_MAX_DSG_I + 2,
	BAT_BASIC_DATA_FULL_CAPACITY = BAT_BASIC_DATA_ALLOW_MAX_CHG_I + 2,
	BAT_BASIC_DATA_MOS_STATUS,
	BAT_BASIC_DATA_HCARGE_DETECTION,
	BAT_BASIC_DATA_LOADER_DETECTION,
	BAT_BASIC_DATA_DET_CONNECT_DETECTION,
	BAT_BASIC_DATA_HV_ADC,
	BAT_BASIC_DATA_PDSG_I_DETECTION,
	BAT_BASIC_DATA_BLE_COMMUNICATION_STATUS,
	BAT_BASIC_DATA_4G_COMMUNICATION_STATUS,
	BAT_BASIC_DATA_SELF_CHECK_RESUILT,
	BAT_BASIC_DATA_WAKEUP_REASON,
	BAT_BASIC_DATA_HEART_BEAT,
	BAT_BASIC_DATA_RESET_TIMES,
	BAT_BASIC_DATA_CYCLE_TIMES,
	BAT_BASIC_DATA_CHARGE_CAPACITY,
	BAT_BASIC_DATA_DISCHARGE_CAPACITY = BAT_BASIC_DATA_CHARGE_CAPACITY + 2,
	BAT_BASIC_DATA_BALANCE_STATUS = BAT_BASIC_DATA_DISCHARGE_CAPACITY + 2,
	BAT_BASIC_DATA_BALANCE_TIME = BAT_BASIC_DATA_BALANCE_STATUS + 2,
	BAT_BASIC_DATA_DIFF_VOL = BAT_BASIC_DATA_BALANCE_TIME + 20,
	BAT_BASIC_DATA_TOTAL_VOL,
	BAT_BASIC_DATA_ENV_TEMPERATURE,

	REG_55600_55799_END = BAT_BASIC_DATA_ENV_TEMPERATURE + 8
} RTU_REG_55600_55799_E;

typedef union {
	struct {
		uint16_t hv;
		uint16_t sv;
		uint16_t vendor_id;
		uint16_t project_id;
		uint16_t protocol_id;
		bat_basic_data bat_data;
	};
	uint16_t all_data[REG_NUM(REG_55600_55799_END, REG_55600_55799_HV_VERSION)];

} reg_55600_55799;

/*********************************************************************************/
//FLASH 存数数据
typedef enum {
	FLASH_RECORD_DATA_TOTAL_NUM,
	FLASH_RECORD_DATA_CUR_INDEX,
	FLASH_RECORD_DATA_DATA_TYPE,
	FLASH_RECORD_DATA_BAT_BASIC_DATA_START, //电池基础数据

	FLASH_RECORD_DATA_MAX_CELL_VOL = FLASH_RECORD_DATA_BAT_BASIC_DATA_START +
		(REG_55600_55799_END - BAT_BASIC_DATA_ERROR_CODE), //电池基础数据占100个寄存器
	FLASH_RECORD_DATA_MAX_CELL_VOL_INDEX,
	FLASH_RECORD_DATA_MIN_CELL_VOL,
	FLASH_RECORD_DATA_MIN_CELL_VOL_INDEX,
	FLASH_RECORD_DATA_MAX_DIFF_VOL,
	FLASH_RECORD_DATA_MAX_VOL_AT_MAX_DIFF_VOL,
	FLASH_RECORD_DATA_MAX_VOL_INDEX_AT_MAX_DIFF_VOL,
	FLASH_RECORD_DATA_MAX_CELL_TEMPERATURE,
	FLASH_RECORD_DATA_MAX_MOS_TEMPERATURE,
	FLASH_RECORD_DATA_MAX_PDSG_MOS_TEMPERATURE,
	FLASH_RECORD_DATA_MAX_I_AT_CHARGE,
	FLASH_RECORD_DATA_MAX_I_AT_DISCHARGE,
	FLASH_RECORD_DATA_START_SOC,
	FLASH_RECORD_DATA_END_SOC,
	FLASH_RECORD_DATA_TOTCL_CHG_CAPACITY,
	FLASH_RECORD_DATA_TOTCL_DSG_CAPACITY,
	FLASH_RECORD_DATA_START_TIME,
	FLASH_RECORD_DATA_STOP_TIME = FLASH_RECORD_DATA_START_TIME + 2,

	FLASH_RECORD_DATA_REVERSE = FLASH_RECORD_DATA_STOP_TIME + 2,

	FLASH_RECORD_DATA_END = FLASH_RECORD_DATA_REVERSE + 4,
} FLASH_RECORD_DATA;

typedef struct {
	uint16_t max_cell_vol_at_record;
	uint16_t max_cell_vol_index_at_record;
	uint16_t min_cell_vol_at_record;
	uint16_t min_cell_vol_index_at_record;
	uint16_t max_dif;
	uint16_t max_cell_vol_at_max_dif;
	uint16_t
		max_min_cell_vol_index_at_max_dif; //压差最大时最高电压序号（高字节）、最低电压序号（低字节）
	uint16_t max_temp_at_record;
	uint16_t max_mos_temp_at_record;
	uint16_t max_pdsg_mos_temp_at_record;
	uint16_t max_i_in_charge_at_record;
	uint16_t max_i_in_discharge_at_record;
	uint16_t start_soc;
	uint16_t end_soc;
	uint16_t total_chg_cap_at_record;
	uint16_t total_dsg_cap_at_record;
	uint16_t start_time[2];
	uint16_t end_time[2];
	uint16_t reserved_data[4];
} extreme_data_t;

typedef struct {
	uint16_t total_num;
	uint16_t cur_num;
	uint16_t data_type;
	bat_basic_data basic_data;
	extreme_data_t extre_data;
} flash_save_data;

/*********************************************************************************/
//故障记录
typedef enum {
	FAULT_RECORD_START = 57000,

	FAULT_RECORD_END = FAULT_RECORD_START + FLASH_RECORD_DATA_END,

} RTU_FAULT_RECORD_DATA_57000_57199_E;

typedef union {
	flash_save_data flash_data;
	uint16_t all_data[REG_NUM(FAULT_RECORD_END, FAULT_RECORD_START)];
} reg_57000_57199;

/*********************************************************************************/
//充放电记录
typedef enum {
	CHG_DSG_RECORD_DATA_START = 57200,

	CHG_DSG_RECORD_DATA_END = CHG_DSG_RECORD_DATA_START + FLASH_RECORD_DATA_END,
} RTU_CHG_DSG_RECORD_DATA_57200_57399_E;

typedef union {
	flash_save_data flash_data;
	uint16_t all_data[REG_NUM(CHG_DSG_RECORD_DATA_END, CHG_DSG_RECORD_DATA_START)];
} reg_57200_57399;

/*********************************************************************************/
//极值纪录
typedef enum {
	EXTREM_DATA_START = 57400,

	EXTREM_DATA_END = EXTREM_DATA_START + FLASH_RECORD_DATA_END,

} RTU_EXTREM_DATA_57400_57599_E;

typedef union {
	flash_save_data flash_data;
	uint16_t all_data[REG_NUM(EXTREM_DATA_END, EXTREM_DATA_START)];
} reg_57400_57599;

/*********************************************************************************/
//周期存储记录
typedef enum {
	PERIOD_DATA_START = 57600,

	PERIOD_DATA_END = PERIOD_DATA_START + FLASH_RECORD_DATA_END,

} RTU_PERIOD_DATA_57600_57799_E;

typedef union {
	flash_save_data flash_data;
	uint16_t all_data[REG_NUM(PERIOD_DATA_END, PERIOD_DATA_START)];
} reg_57600_57799;

/*********************************************************************************/
//4G模组
typedef enum {
	INFO_4G_RIGISTER_RESULT = 57800,
	INFO_4G_SIM_ID,
	INFO_4G_MODE_ID = 57811,
	INFO_4G_SIGNAL_POWER = 57819,
	INFO_4G_GPS_STATUS,
	INFO_4G_GPS_LONGTITUDE,
	INFO_4G_GPS_LATITUDE = 57823,
	INFO_4G_HV_VERSION = 57825,
	INFO_4G_SV_VERSION,
	INFO_4G_BLE_CONNECT_STATUS,
	INFO_4G_REVERSE,

	REG_57800_57849_END = INFO_4G_REVERSE + 2,
} RTU_INFO_4G_57800_57849_E;

typedef union {
	struct {
		uint16_t register_result;
		uint16_t ICCID[10];
		uint16_t IMEI[8];
		uint16_t signal_power;
		uint16_t gps_status;
		uint16_t longtitude[2];
		uint16_t latitude[2];
		uint16_t hv;
		uint16_t sv;
		uint16_t blue_connect_status;
		uint16_t reserved_data[2];
	};
	uint16_t all_data[REG_NUM(REG_57800_57849_END, INFO_4G_RIGISTER_RESULT)];
} reg_57800_57849;

/*********************************************************************************/
//文件信息
typedef enum {
	UPGRADE_INFO_FILE_SIZE = 60000,
	UPGRADE_INFO_FILE_NAME = 60002,

	REG_60000_60099_END = UPGRADE_INFO_FILE_NAME + 48,

} RTU_UPGRADE_INFO_60000_60099_E;

typedef union {
	struct {
		uint16_t file_size[2];
		uint16_t file_name[22];
		uint16_t reserved[26];
	};
	uint16_t all_data[REG_NUM(REG_60000_60099_END, UPGRADE_INFO_FILE_SIZE)];
} reg_60000_60099;

/*********************************************************************************/
//文件内容
typedef enum {
	UPGRADE_CONTENT_INDEX = 60100,
	UPGRADE_CONTENT = 60101,

	REG_60100_60200_END = UPGRADE_CONTENT + 64,

} RTU_UPGRADE_CONTENT_60100_60200_E;

/*********************************************************************************/
//升级结果
typedef enum {
	UPGRADE_STATUS = 60400,
	UPGRADE_RESULT_1,
	UPGRADE_RESULT_2,
	UPGRADE_RESULT_3,
	UPGRADE_RESULT_HV,
	UPGRADE_BOOT_SV = UPGRADE_RESULT_HV + 2,
	UPGRADE_APP_SV = UPGRADE_BOOT_SV + 2,

	UPGRADE_RESULT_END = UPGRADE_APP_SV + 2,

} RTU_UPGRADE_RESULT_60400_60409_E;

typedef union {
	struct {
		uint16_t upgrade_status;
		uint16_t upgrade_result_1;
		uint16_t upgrade_result_2;
		uint16_t upgrade_result_3;
		uint16_t hv[2];
		uint16_t boot_sv[2];
		uint16_t app_sv[2];
	};
	uint16_t all_data[REG_NUM(UPGRADE_RESULT_END, UPGRADE_STATUS)];
} reg_60400_60409;

#define REG_OFFSET(r, b) ((r) - (b))

#define PRI_0000_0011_REG_OFFSET(r) REG_OFFSET(r, REG_0000_0011_SOFTWARE_VERTION)
#define PRI_1000_1199_REG_OFFSET(r) REG_OFFSET(r, REG_1000_1199_BAT_ID)
#define PRI_1200_1299_REG_OFFSET(r) REG_OFFSET(r, REG_1200_1299_BAT_ID)
#define PRI_2000_2099_REG_OFFSET(r) REG_OFFSET(r, REG_2000_2099_FRAME_NUM)
#define PRI_3000_3099_REG_OFFSET(r) REG_OFFSET(r, REG_3000_3099_FRAME_NUM)
#define PRI_5000_5099_REG_OFFSET(r) REG_OFFSET(r, REG_5000_5099_FRAME_NUM)
#define PRI_55000_55229_REG_OFFSET(r) REG_OFFSET(r, REG_55000_55229_BMS_MODE)
#define PRI_55600_55799_REG_OFFSET(r) REG_OFFSET(r, REG_55600_55799_HV_VERSION)
#define PRI_57000_57199_REG_OFFSET(r) REG_OFFSET(r, FAULT_RECORD_START)
#define PRI_57200_57399_REG_OFFSET(r) REG_OFFSET(r, CHG_DSG_RECORD_DATA_START)
#define PRI_57400_57599_REG_OFFSET(r) REG_OFFSET(r, EXTREM_DATA_START)
#define PRI_57600_57799_REG_OFFSET(r) REG_OFFSET(r, PERIOD_DATA_START)
#define PRI_57800_57849_REG_OFFSET(r) REG_OFFSET(r, INFO_4G_RIGISTER_RESULT)
#define PRI_60000_60099_REG_OFFSET(r) REG_OFFSET(r, UPGRADE_INFO_FILE_SIZE)
#define PRI_60100_60200_REG_OFFSET(r) REG_OFFSET(r, UPGRADE_CONTENT_INDEX)
#define PRI_60400_60409_REG_OFFSET(r) REG_OFFSET(r, UPGRADE_STATUS)

#endif