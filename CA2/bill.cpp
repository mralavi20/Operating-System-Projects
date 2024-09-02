#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include "defines.h"

using namespace std;


void read_file (string file_name, vector<vector<int>> &values) {
    vector<int> row;
    string word;
    string line;
    int line_num = 1;


    fstream file (file_name, ios::in);
    
    while(getline(file, line)) {
		row.clear();
 
		stringstream str(line);

        if (line_num != 1) {
		    while(getline(str, word, ',')) {
			    row.push_back(stoi (word));
            }

            row.erase (row.begin ());
            row.erase (row.begin ());
            
            values.push_back (row);
        }

        line_num = line_num + 1;
	}
}

void read_values (int fd, vector<vector<int>> &values) {
    int value;
    vector<int> row;
    int i;
    int j;

    for (i = 0; i < MOUNTHS_NUMBER; i++) {
        row.clear ();

        for (j = 0; j < HOURS_NUMBER; j++) {
            cout << "Reading from fifo in bill " << fd << " " << i << " " << j << endl;
            read (fd, &value, sizeof (int));
            row.push_back (value);
        }

        values.push_back (row);
    }
}

void write_values (int fd, const vector<int> &values) {
    int value;
    int i;

    for (i = 0; i < MOUNTHS_NUMBER; i++) {
        value = values[i];
        cout << "Writing to fifo in bill " << fd << " " << value << " " << i << endl;
        write (fd, &value, sizeof (int));
    }
}

void find_elec_bill (const vector<vector<int>> &values, const vector<int> &pick_times, vector<vector<int>> &cons, vector<int> &res) {
    int value;
    int mounth_sum;
    int i;
    int j;
    
    for (i = 0; i < values.size (); i++) {
        mounth_sum = 0;

        for (j = 0; j < values[i].size (); j++) {
            value = cons[i][2] * values[i][j];

            if (j == pick_times[i]) {
                value = 1.25 * value;
            }

            mounth_sum = mounth_sum + value;
        }
        
        res.push_back (mounth_sum);
    }
}

void find_gas_bill (const vector<vector<int>> &values, const vector<int> &pick_times, const vector<int> &average_values, vector<vector<int>> &cons, vector<int> &res) {
    int value;
    int mounth_sum;
    int i;
    int j;

    for (i = 0; i < values.size (); i++) {
        mounth_sum = 0;

        for (j = 0; j < values[i].size (); j++) {
            value = cons[i][1] * values[i][j];

            if (value < average_values[i]) {
                value = 0.75 * value;
            }

            if (j == pick_times[i]) {
                value = 1.25 * value;
            }

            mounth_sum = mounth_sum + value;
        }

        res.push_back (mounth_sum);
    }
}

void find_water_bill (const vector<vector<int>> &values, vector<vector<int>> &cons, vector<int> &res) {
    int value;
    int mounth_sum;
    int i;
    int j;

    for (i = 0; i < values.size (); i++) {
        mounth_sum = 0;

        for (j = 0; j < values[i].size (); j++) {
            value = cons[i][0] * values[i][j];

            mounth_sum = mounth_sum + value;
        }

        res.push_back (mounth_sum);
    }
}

int find_max_time (const vector<int> &times) {
    int max_value;
    int max_time;
    int i;

    max_value = 0;

    for (i = 0; i < times.size (); i++) {
        if (max_value < times[i]) {
            max_value = times[i];
            max_time = i;
        }
    }

    return max_time;
}

void find_pick_times (const vector<vector<int>> &time_values, vector<int> &pick_times) {
    int pick_time;
    int i;

    for (i = 0; i < time_values.size (); i++) {
        pick_time = find_max_time (time_values[i]);
        pick_times.push_back (pick_time);
    }
}

int find_average (const vector<int> &values) {
    int result;
    int sum;
    int i;

    sum = 0;

    for (i = 0; i < values.size (); i++) {
        sum = sum + values[i];
    }

    result = sum / values.size ();

    return result;
}

void find_average_values (const vector<vector<int>> &time_values, vector<int> &average_values) {
    int avg;
    int i;

    for (i = 0; i < time_values.size (); i++) {
        avg = find_average (time_values[i]);
        average_values.push_back (avg);
    }
}

void print_values1 (vector<int> values) {
    int i;

    for (i = 0; i < values.size (); i++) {
        cout << values[i] << " ";
    }

    cout << '\n' << endl;
}

void print_values2 (vector<vector<int>> values) {
    int i;
    int j;

    for (i = 0; i < values.size (); i++) {
        for (j = 0; j < values[i].size (); j++) {
            cout << values[i][j] << " ";
        }

        cout << "\n" << endl;
    }
}


int main (int argc, char *argv[]) {
    int fifo_fd;
    string file_name;
    vector<vector<int>> constants;
    vector<vector<int>> elec_time_values;
    vector<vector<int>> gas_time_values;
    vector<vector<int>> water_time_values;
    vector<int> elec_pick_times;
    vector<int> gas_pick_times;
    vector<int> water_pick_times;
    vector<int> elec_average_values;
    vector<int> gas_average_values;
    vector<int> water_average_values;
    vector<int> elec_res;
    vector<int> gas_res;
    vector<int> water_res;

    if (argc < 2) {
        cout << "Error happened" << endl;
        exit (0);
    }

    file_name = argv[1];
    file_name = file_name + "/bills.csv";

    cout << "Reading bill file" << endl;
    read_file (file_name, constants);

    cout << "Openning values fifo in bill" << endl;
    fifo_fd = open ("values", O_RDONLY);

    if (fifo_fd < 0) {
        cout << "Error happened" << endl;
        exit (0);
    }

    read_values (fifo_fd, elec_time_values);
    read_values (fifo_fd, gas_time_values);
    read_values (fifo_fd, water_time_values);

    cout << "Closing values fifo in bill" << endl;
    close (fifo_fd);

    find_pick_times (elec_time_values, elec_pick_times);
    find_pick_times (gas_time_values, gas_pick_times);
    find_pick_times (water_time_values, water_pick_times);

    find_average_values (elec_time_values, elec_average_values);
    find_average_values (gas_time_values, gas_average_values);
    find_average_values (water_time_values, water_average_values);

    find_elec_bill (elec_time_values, elec_pick_times, constants, elec_res);
    find_gas_bill (gas_time_values, gas_pick_times, gas_average_values, constants, gas_res);
    find_water_bill (water_time_values, constants, water_res);

    cout << "Creating results fifo in bill" << endl;
    mkfifo ("results", 0666);

    cout << "Openning results fifo in bill" << endl;
    fifo_fd = open ("results", O_WRONLY);

    if (fifo_fd < 0) {
        cout << "Error happened" << endl;
        exit (0);
    }

    write_values (fifo_fd, elec_res);
    write_values (fifo_fd, gas_res);
    write_values (fifo_fd, water_res);

    cout << "Closing results fifo in bill" << endl;
    close (fifo_fd);

    return 0;
}