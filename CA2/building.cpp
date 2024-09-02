#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "defines.h"

using namespace std;


typedef struct Source {
    string file_name;
    vector<vector<int>> values;
    vector<vector<int>> time_values;
    vector<int> mounth_values;
    vector<int> pick_times;
    vector<int> average_values;
} Source;

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
            row.erase (row.begin ());

            values.push_back (row);
        }

        line_num = line_num + 1;
	}
}

void find_time_values (const vector<vector<int>> &values, vector<vector<int>> &time_values) {
    int i;
    int j;
    int k;
    vector<int> time_sum;

    for (i = 0; i < MOUNTHS_NUMBER; i++) {
        time_sum = {0, 0, 0, 0, 0, 0};

        for (j = 0; j < DAYS_NUMBERS; j++) {
            for (k = 0; k < HOURS_NUMBER; k++) {
                time_sum[k] = time_sum[k] + values[i * MOUNTHS_NUMBER + j][k];
            }
        }

        time_values.push_back (time_sum);
    }
}

void read_from_pipe (int pipe_fd, vector<vector<int>> &values) {
    int value;
    vector<int> row;
    int i;
    int j;
    
    for (i = 0; i < MOUNTHS_NUMBER; i++) {
        row.clear ();

        for (j = 0; j < HOURS_NUMBER; j++) {
            cout << "Reading from pipe " << pipe_fd << endl;
            read (pipe_fd, &value, sizeof (int));
            row.push_back (value);
        }

        values.push_back (row);
    }
}

void write_to_fifo (int fifo_fd, const vector<vector<int>> &values) {
    int value;
    int i;
    int j;
    
    for (i = 0; i < values.size (); i++) {
        for (j = 0; j < values[i].size (); j++) {
            value = values[i][j];
            cout << "Sending from building to fifo " << fifo_fd << " value " << value << " " << i << " " << j << endl;
            write (fifo_fd, &value, sizeof (int));
        }
    }
}

void read_fifo (int fd, vector<int> &values) {
    int value;
    int i;

    for (i = 0; i < MOUNTHS_NUMBER; i++) {
        cout << "Reading from fifo in building " << fd << endl;
        read (fd, &value, sizeof (int));
        values.push_back (value);
    }
}

void write_to_pipe_type1 (int pipe_fd, const vector<int> &values) {
    int value;
    int i;

    for (i = 0; i < values.size (); i++) {
        value = values[i];
        cout << "Wrting to pipe in building " << pipe_fd << " " << i << endl;
        write (pipe_fd, &value, sizeof (int));
    }
}

void write_to_pipe_type2 (int pipe_fd, const vector<vector<int>> &values) {
    int value;
    int i;
    int j;
    cout << values.size () << endl;
    for (i = 0; i < values.size (); i++) {
        cout << values[i].size () << endl;
        for (j = 0; j < values[i].size (); j++) {
            value = values[i][j];
            cout << "Wrting to pipe in building " << pipe_fd << " " << i << " " << j << endl;
            write (pipe_fd, &value, sizeof (int));
        }
    }
}


int main (int argc, char *argv[]) {
    string loc;
    int sources[3];
    int pipes_fd[3][2];
    int pid1;
    int pid2;
    int pid3;
    int value;
    int write_pipe_fd;
    int i;

    if (argc < 3) {
        cout << "Error happened" << endl;
        exit (0);
    }

    loc = argv[1];
    sscanf (argv[2], "%d", &write_pipe_fd);

    for (i = 0; i < SOURCES_NUMBER; i++) {
        pipe (pipes_fd[i]);
    }

    cout << "Creating child process" << endl;
    pid1 = fork ();

    if (pid1 == 0) {
        Source elec;

        cout << "Closing pipes" << endl;
        close (pipes_fd[0][0]);
        close (pipes_fd[1][0]);
        close (pipes_fd[1][1]);
        close (pipes_fd[2][0]);
        close (pipes_fd[2][1]);

        elec.file_name = loc + "/Electricity.csv";
        read_file (elec.file_name, elec.values);

        find_time_values (elec.values, elec.time_values);
    
        write_to_pipe_type2 (pipes_fd[0][1], elec.time_values);
        
        cout << "Closing pipe" << endl;
        close (pipes_fd[0][1]);

        cout << "Finishing child process" << endl;
        exit (0);
    }

    pid2 = fork ();

    if (pid2 == 0) {
        Source gas;

        cout << "Closing pipes" << endl;
        close (pipes_fd[1][0]);
        close (pipes_fd[0][0]);
        close (pipes_fd[0][1]);
        close (pipes_fd[2][0]);
        close (pipes_fd[2][1]);

        gas.file_name = loc + "/Gas.csv";
        read_file (gas.file_name, gas.values);

        find_time_values (gas.values, gas.time_values);

        write_to_pipe_type2 (pipes_fd[1][1], gas.time_values);

        cout << "Closing pipe" << endl;
        close (pipes_fd[1][1]);

        cout << "Finishing child process" << endl;
        exit (0);
    }

    pid3 = fork ();

    if (pid3 == 0) {
        Source water;

        cout << "Closing pipes" << endl;
        close (pipes_fd[2][0]);
        close (pipes_fd[0][0]);
        close (pipes_fd[0][1]);
        close (pipes_fd[1][0]);
        close (pipes_fd[1][1]);

        water.file_name = loc + "/Water.csv";
        
        cout << "Reading building file" << endl;
        read_file (water.file_name, water.values);

        find_time_values (water.values, water.time_values);
        
        write_to_pipe_type2 (pipes_fd[2][1], water.time_values);

        cout << "Closing pipe" << endl;
        close (pipes_fd[2][1]);

        cout << "Finishing child process" << endl;
        exit (0);
    }
    
    int fd;

    vector<vector <int>> elec_time_values;
    vector<vector <int>> gas_time_values;
    vector<vector <int>> water_time_values;
    vector<int> elec_res;
    vector<int> gas_res;
    vector<int> water_res;

    cout << "Closing pipes" << endl;
    close (pipes_fd[0][1]);
    close (pipes_fd[1][1]);
    close (pipes_fd[2][1]);
    
    read_from_pipe (pipes_fd[0][0], elec_time_values);
    read_from_pipe (pipes_fd[1][0], gas_time_values);
    read_from_pipe (pipes_fd[2][0], water_time_values);

    cout << "Closing pipes" << endl;
    close (pipes_fd[0][0]);
    close (pipes_fd[1][0]);
    close (pipes_fd[2][0]);

    cout << "Waiting for child processes" << endl;
    int status = 0;
    pid_t wpid;
    while ((wpid = wait (&status)) > 0);

    cout << "Creating values fifo in building" << endl;
    mkfifo ("values", 0666);

    cout << "Openning values fifo in building" << endl;
    int fifo_fd = open ("values", O_WRONLY);

    if (fifo_fd < 0) {
        cout << "Error happened" << endl;
        exit (0);
    }

    write_to_fifo (fifo_fd, elec_time_values);
    write_to_fifo (fifo_fd, gas_time_values);
    write_to_fifo (fifo_fd, water_time_values);

    cout << "Closing values fifo in building" << endl;
    close (fd);

    cout << "Openning results fifo in building" << endl;
    fifo_fd = open ("results", O_RDONLY);

    if (fifo_fd < 0) {
        cout << "Error happened" << endl;
        exit (0);
    }

    read_fifo (fifo_fd, elec_res);
    read_fifo (fifo_fd, gas_res);
    read_fifo (fifo_fd, water_res);

    cout << "Closing results fifo in building" << endl;
    close (fd);

    write_to_pipe_type1 (write_pipe_fd, elec_res);
    write_to_pipe_type1 (write_pipe_fd, gas_res);
    write_to_pipe_type1 (write_pipe_fd, water_res);

    return 0;
}