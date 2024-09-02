#include <iostream>
#include <vector>
#include <string>
#include <stdio.h>
#include <dirent.h>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "defines.h"


using namespace std;


void find_buildings (const char* loc, vector<string> &buildings_name) {
    string name;
    DIR *dir = opendir (loc);

    struct dirent *file = readdir( dir);

    while (file != NULL)
    {
        if (file->d_type == DT_DIR) {
            name = file->d_name;

            if (name != "." && name != "..") {
                buildings_name.push_back (name);
            }
        }
        
        file = readdir(dir);
    }

    closedir(dir);
}

void show_buildings (vector<string> &names) {
    int i;

    cout << "Nummber of buildings: " << names.size () << endl;

    for (i = 0; i < names.size (); i++) {
        cout << names[i] << " " << i << endl;
    }
}

void get_inputs (vector<int> &sources) {
    string line;
    string word;

    getline (cin, line);

    stringstream str (line);

    while (getline (str, word, ' ')) {
        if (word == "electricity") {
            sources.push_back (ELEC_CODE);
        }
        else if (word == "gas") {
            sources.push_back (GAS_CODE);
        }
        else {
            sources.push_back (WATER_CODE);
        }
    }
}

void send_message_to_building (int building_id, const vector<int> &pipes_fd) {
    int message;
    int i;

    for (i = 0; i < pipes_fd.size (); i++) {
        if (i == building_id) {
            message = SELECTED_MESSAGE;
            cout << "Sending message from main to building with pipe " << pipes_fd[i] << endl;
            write (pipes_fd[i], &message, sizeof (int));
        }
        else {
            cout << "Sending message from main to building with pipe " << pipes_fd[i] << endl;
            message = NOT_SELECTED_MESSAGE;
            write (pipes_fd[i], &message, sizeof (int));
        }
        }
}

void read_result (int pipe_fd, vector<int> &values) {
    int value;
    int i;

    for (i = 0; i < 12; i++) {
        cout << "Reading from pipe " << pipe_fd << endl;
        read (pipe_fd, &value, sizeof (int));
        values.push_back (value);
    }
}

void show_report (string title, vector<int> values) {
    int i;

    cout << title << endl;

    for (i = 0; i < values.size (); i++) {
        cout << "Mounth" << i + 1 << ": " << values[i] << endl;
    }
}

int main (int argc, char *argv[]) {
    int buildings_count;
    int building_id;
    int id;
    int num;
    int message;
    int i;
    int j;
    vector<int> sources;
    int sources_id[3] = {0, 0, 0};
    int result_pipe_fd[2];
    vector<string> buildings_name;
    vector<int> pipes_read_fd;
    vector<int> pipes_write_fd;
    vector<int> children_id;

    if (argc < 2) {
        cout << "Error happened" << endl;
        exit (0);
    }

    find_buildings (argv[1], buildings_name);

    buildings_count = buildings_name.size ();

    show_buildings (buildings_name);

    cout << "Creating result pipe" << endl;
    pipe (result_pipe_fd);

    for (i = 0; i < buildings_count; i++) {
        int pipe_fd[2];

        cout << "Creating pipe" << endl;
        pipe (pipe_fd);
        
        pipes_read_fd.push_back (pipe_fd[0]);
        pipes_write_fd.push_back (pipe_fd[1]);

        cout << "Creating child process" << endl;
        id = fork ();

        if (id == 0) {
            break;
        }
    }

    if (id != 0) {
        get_inputs (sources);
        cin >> building_id;

        cout << "Creating child process" << endl;
        int bill_id = fork ();

        if (bill_id == 0) {
            string file_name;

            file_name = argv[1];
            const char* path = file_name.c_str ();

            cout << "Running bill program" << endl;
            execl ("bill.out", "bill.out", path, NULL);

            exit (0);
        }

        vector<int> elec_res;
        vector<int> gas_res;
        vector<int> water_res;

        for (i = 0; i < sources.size (); i++) {
            sources_id[sources[i] - 1] = 1;
        }

        send_message_to_building (building_id, pipes_write_fd);

        for (i = 0; i < pipes_read_fd.size (); i++) {
            cout << "Closing pipes " << pipes_read_fd[i] << " " << pipes_write_fd[i] << endl;
            close (pipes_read_fd[i]);
            close (pipes_write_fd[i]);
        }

        cout << "Closing pipe " << result_pipe_fd[1] << endl;
        close (result_pipe_fd[1]);

        read_result (result_pipe_fd[0], elec_res);
        read_result (result_pipe_fd[0], gas_res);
        read_result (result_pipe_fd[0], water_res);

        cout << "Closing pipe " << result_pipe_fd[0] << endl;
        close (result_pipe_fd[0]);

        cout << "Waiting for child processes" << endl;
        int status = 0;
        pid_t wpid;
        while ((wpid = wait (&status)) > 0);

        if (sources_id[0] == 1) {
            show_report ("Electricity", elec_res);
        }
        if (sources_id[1] == 1) {
            show_report ("Gas", gas_res);
        }
        if (sources_id[2] == 1) {
            show_report ("Water", water_res);
        }
    }
    else {
        for (j = 0; j < pipes_read_fd.size () - 1; j++) {
            cout << "Closing pipes " << pipes_read_fd[j] << " " << pipes_write_fd[j] << endl;
            close (pipes_read_fd[j]);
            close (pipes_write_fd[j]);
        }

        cout << "Closing pipes " << pipes_write_fd[i] << " " << result_pipe_fd[0] << endl;
        close (pipes_write_fd[i]);
        close (result_pipe_fd[0]);

        cout << "Reading from pipe " << pipes_read_fd[i] << endl;
        read (pipes_read_fd[i], &message, sizeof (int));
        
        cout << "Closing pipes " << pipes_read_fd[i] << endl;
        close (pipes_read_fd[i]);

        if (message == 1) {
            string file_name;
            string delimiter = "/";

            file_name = argv[1] + delimiter + buildings_name[i];
            const char* path = file_name.c_str ();

            string pipe_write_fd = to_string (result_pipe_fd[1]);
            const char* fd = pipe_write_fd.c_str ();

            cout << "Running building program" << endl;
            execl ("building.out", "building.out", path, fd, NULL);
        }

        cout << "Finishing child process" << endl;
        exit (0);
    }

    return 0;
}