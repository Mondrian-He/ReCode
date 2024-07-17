import random

# 设置参数
num_files = 10
total_entries_per_file = 10000
max_value = 1000000

common_entries = 500
unique_entries = total_entries_per_file - common_entries

# 生成共同的数据
common_data = set()
while len(common_data) < common_entries:
    common_data.add(f"element{random.randint(1, max_value)}")

# 生成文件
for i in range(num_files):
    file_name = f"set_{i}"
    # 创建唯一数据
    unique_data = set()
    while len(unique_data) < unique_entries:
        entry = f"element{random.randint(1, max_value)}"
        if entry not in common_data and entry not in unique_data:
            unique_data.add(entry)

    # 合并共同数据和唯一数据，并写入文件
    with open(file_name, 'w') as f:
        all_data = list(common_data) + list(unique_data)
        random.shuffle(all_data)  # 打乱数据的顺序
        for entry in all_data:
            f.write(f"{entry}\n")
    
    if i == 0:  
        with open('query', 'w') as query_file:
            query_file.writelines(f"{entry}\n" for entry in all_data)

print(f"Generated {num_files} files with {total_entries_per_file} entries each, including {common_entries} common entries.")
