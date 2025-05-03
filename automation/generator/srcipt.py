import pandas as pd
import re

# Helper function to split 96-bit ID into three 32-bit parts
def split_stm32_id(stm32id):
    # 如果是字符串且以 '0x' 或 '0X' 开头，则将其转换为整数
    if isinstance(stm32id, str) and stm32id.lower().startswith('0x'):
        try:
            stm32id = int(stm32id, 16)
        except ValueError:
            raise ValueError("Invalid hexadecimal string.")

    # 如果是浮点数，则转换为整数（注意这会丢失小数部分）
    elif isinstance(stm32id, float):
        stm32id = int(stm32id)
        if stm32id != stm32id:  # 检查是否发生了数据丢失
            raise ValueError("Floating point number cannot be accurately converted to an integer without data loss.")

    # 确保最终结果是一个整数
    if not isinstance(stm32id, int):
        raise TypeError("STM32 ID must be an integer or a hexadecimal string.")
    hex_str = format(stm32id, '024x')  # Ensure it's always 24 characters (96 bits)
    # Split the string into three 8-character (32-bit) chunks
    # part1 = int(hex_str[0:8], 16)
    # part2 = int(hex_str[8:16], 16)
    # part3 = int(hex_str[16:24], 16)

    part1 = f"0x{hex_str[0:8]}"
    part2 = f"0x{hex_str[8:16]}"
    part3 = f"0x{hex_str[16:24]}"
    return f"{{ {part1}, {part2}, {part3} }}"

def format_float(value, precision=10):
    rounded_value = round(float(value), precision)
    return f"{rounded_value:.{precision}f}f"

# Read Excel file
filename = 'data.xlsx'
data = pd.read_excel(filename)

# Initialize structure array
adc_cali_array = []

# Iterate over each row of data
for index, row in data.iterrows():
    # Extract and process data
    stm32id = split_stm32_id(row['stm32id'])
    # 使用辅助函数来格式化每个值,四舍五入到小数点后六位
    v_chassis_k = format_float(row['v_chassis_k'])
    v_chassis_b = format_float(row['v_chassis_b'])
    i_chassis_k = format_float(row['i_chassis_k'])
    i_chassis_b = format_float(row['i_chassis_b'])
    i_motor_k = format_float(row['i_motor_k'])
    i_motor_b = format_float(row['i_motor_b'])
    v_cap_k = format_float(row['v_cap_k'])
    v_cap_b = format_float(row['v_cap_b'])
    i_cap_k = format_float(row['i_cap_k'])
    i_cap_b = format_float(row['i_cap_b'])

    # Create struct members
    adc_calibration_t = lambda k, b: f"{{ {k}, {b} }}"
    
    board_adc_calibration_t = (
        f"{{ {stm32id}, \n"
        f"{adc_calibration_t(v_chassis_k, v_chassis_b)}, \n"
        f"{adc_calibration_t(i_chassis_k, i_chassis_b)}, \n"
        f"{adc_calibration_t(i_motor_k, i_motor_b)}, \n"
        f"{adc_calibration_t(v_cap_k, v_cap_b)}, \n"
        f"{adc_calibration_t(i_cap_k, i_cap_b)} }}"
    )

    adc_cali_array.append(board_adc_calibration_t)

# Generate array string
array_str = ',\n'.join(adc_cali_array)

print(f"Generated array string: \n{array_str}")

# Read C file
c_filename = 'abc.c'
with open(c_filename, 'r') as file:
    c_file_content = file.read()

# Insert generated content
start_tag = "// board_adc_calibration insert start"
end_tag = "// board_adc_calibration insert stop"
pattern = fr"\s*{re.escape(start_tag)}\n(.*?)\n\s*{re.escape(end_tag)}"
replacement = f"\n\t{start_tag}\n\t{array_str}\n\t{end_tag}"

c_file_content = re.sub(pattern, replacement, c_file_content, flags=re.DOTALL)

# Write modified content back to the C file
with open(c_filename, 'w') as file:
    file.write(c_file_content)

print(f"Data has been successfully inserted into {c_filename}")