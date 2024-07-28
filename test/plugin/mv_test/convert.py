import os
import pandas as pd
import csv
import json

# Function to convert CSV to JavaScript variable format
def convert_csv_to_js_variable(file_path):
    df = pd.read_csv(file_path)
    # Ensure 'ja' and 'en' columns exist, fill missing values with empty string
    if 'ja' not in df.columns:
        df['ja'] = ''
    if 'en' not in df.columns:
        df['en'] = ''
    
    df_extracted = df[['ja', 'en']].fillna('')
    # Convert to list of dictionaries
    test_map = df_extracted.to_dict(orient='records')
    # Create variable name from file name
    file_name = os.path.splitext(os.path.basename(file_path))[0]
    variable_name = f'test{file_name.capitalize()}'
    # Create JavaScript variable
    js_variable = f'const {variable_name} = {test_map};\n'
    return js_variable

# Function to find all CSV files in the same directory as the script
def find_csv_files(directory):
    return [os.path.join(directory, f) for f in os.listdir(directory) if f.endswith('.csv')]

# Main function
def main():
    script_directory = os.path.dirname(os.path.abspath(__file__)) + "/data/translate"

    csv_files = find_csv_files(script_directory)    
    if not csv_files:
        print("Not Found CSV File")
        return
    js_output = ''
    variable_names = []
    for csv_file in csv_files:
        js_variable = convert_csv_to_js_variable(csv_file)
        js_output += js_variable

    # Read the content of Langscore.test.js
    imports_file_path = os.path.join(script_directory, '../../Langscore.test.js')
    with open(imports_file_path, 'r', encoding='utf-8') as imports_file:
        existing_content = imports_file.read()

    # Find the range to replace
    start_marker = "// START OF GENERATED CONTENT"
    end_marker = "// END OF GENERATED CONTENT"
    start_idx = existing_content.find(start_marker)
    end_idx = existing_content.find(end_marker)

    if start_idx != -1 and end_idx != -1:
        new_content = existing_content[:start_idx + len(start_marker)] + "\n" + js_output + existing_content[end_idx:]
    else:
        print("Not Found Marker Command!!!")
        return

    # Write the new content back to Langscore.test.js
    with open(imports_file_path, 'w', encoding='utf-8') as imports_file:
        imports_file.write(new_content)
    print(f'JavaScript variables have been written to {imports_file_path}')

if __name__ == '__main__':
    main()
