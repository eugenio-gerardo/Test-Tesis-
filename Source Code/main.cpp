#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <queue>
#include <fstream>
#include <cmath>
#include<stdlib.h>

using namespace std;

double u_random()
{
    return double(rand())/RAND_MAX;
}
double u_random(double const &a, double const &b)
{
    return a + (b-a)*u_random();
}
double exp_random(double const &l)
{
    return -l*log(u_random());
}

struct QueueParameters
{
    double  mean_interarrival;
    double  mean_service;
    int num_delays_required;
};

queue<double> A;
queue<double> S;

void set_A()
{
    A.push(.4);
    A.push(1.2);
    A.push(.5);
    A.push(1.7);
    A.push(.2);
    A.push(1.6);
    A.push(.2);
    A.push(1.4);
    A.push(1.9);
}

void set_S()
{
    S.push(2.0);
    S.push(.7);
    S.push(.2);
    S.push(1.1);
    S.push(3.7);
    S.push(0.6);
    S.push(100.0);
}

double next_arrive()
{
    double x=A.front();
    A.pop();
    return x;
}

double next_service()
{
    double x=S.front();
    S.pop();
    return x;
}

class File
{
private:
    string filename;
    char separator;
    vector<vector<string> > values;
public:
    File(string name, char separator)
    {
        this->filename=name;
        this->separator=separator;
    }
    void readfile()
    {
        ifstream f (this->filename);
        string line;
        while(getline(f, line))
            {
                string line_value;
                vector<string> line_values;
                stringstream ss(line);
                while(getline(ss, line_value, this->separator))
                    {
                        line_values.push_back(line_value);
                    }
                this->values.emplace_back(line_values);
            }
    }
    void print_list()
    {
        for(auto x:this->values)
            for(auto y:x)
                cout<<y<<endl;
    }
    void print_lines()
    {
        for(auto x:this->values)
            {
                string line="";
                for(auto y:x)
                    {
                        y+=this->separator;
                        line += y;
                    }
                cout<<" "<<line<<endl;
            }
    }
};

class Log
{
    private:
        ofstream l_file;
        char separator;
        string f_name;
        int counter;
        vector<string>header;
    public:
        Log(){};
        Log(string f_name, char separator, vector<string> header)
        {
            this->f_name=f_name;
            this->l_file=ofstream(f_name);
            this->separator=separator;
            this->log_line(header);
            this->counter=0;
            this->header=header;
        }

        void log_line(vector<string> v_line)
        {
            this->counter++;
            string s_line="";
            for(auto x:v_line)
            {
                s_line+=x;
                s_line+=this->separator;
            }
            s_line.erase(s_line.end()-1);
            l_file<<s_line<<endl;
        }
        bool check()
        {
            return this->counter==500000;
        }

};

enum class Server_Status {IDLE, BUSY};
enum class Queue_Event:char {ARRIVE='A', DEPART='D', INIT='I', END='E'};

class Server
{
private:
    double mean_service;
    Server_Status status;
public:
    Server() {}
    Server(double mean_service)
    {
        this->mean_service=mean_service;
        this->status=Server_Status::IDLE;
    }
    double Generate_Service (double time)
    {
        //double x=next_service();
        return time+exp_random(this->mean_service);//x;
    }
    bool isBusy()
    {
        return this->status==Server_Status::BUSY;
    }
    void setStatus(Server_Status status)
    {
        this->status=status;
    }

};

class Custumer
{
private:
    double arrival_time;
    double service_time;
    double delay_time;
public:
    Custumer(double interarrival_mean, double time)
    {
        this->arrival_time=time+exp_random(interarrival_mean);//next_arrive();//
        this->service_time=10000000000000;
        this->delay_time=0;
    }
    void set_service_time(double service_time)
    {
        this->service_time=service_time;
    }
    void set_delay_time(double delay_time)
    {
        this->delay_time=delay_time-this->arrival_time;
    }
    double get_arrival_time()
    {
        return this->arrival_time;
    }
    double get_service_time()
    {
        return this->service_time;
    }
    double get_delay_time()
    {
        return this->delay_time;
    }
};

class Queue_System
{
private:
    //state variables
    int event;
    double sim_clock_previous;
    double sim_clock;
    double d_sim_clock;
    Queue_Event q_event;
    int queue_size;
    double total_of_delays;
    double num_cust_delayed;
    Queue_Event next_q_event;
    //queue parameters
    double service_mean;
    double interarrival_mean;
    int limit_queue_size;
    //queue elements
    queue<Custumer> custumer_queue;
    Server server;
    //statical Counters
    double area_num_inq;
    double area_server_status;

    Log l;
    string control;
public:
    Queue_System(double interarrival_mean, double service_mean, int limit_queue_size)
    {
        //state variables
        this->event=0;
        this->sim_clock_previous=0;
        this->sim_clock=0;
        this->d_sim_clock=this->sim_clock;
        this->q_event=Queue_Event::INIT;
        this->queue_size=0;
        this->total_of_delays=0;
        this->num_cust_delayed=0;
        this->next_q_event=Queue_Event::ARRIVE;
        //queue parameters
        this->interarrival_mean=interarrival_mean;
        this->service_mean=service_mean;
        this->limit_queue_size=limit_queue_size;
        //init queue elements
        this->custumer_queue.push(Custumer(this->interarrival_mean, this->sim_clock));
        this->server=Server(this->service_mean);
        //statical counters
        this->area_num_inq=0;
        this->area_server_status=0;

        this->l=Log("qlog.csv",',', this->h_vector());
        this->control="";
        this->q_event=Queue_Event::INIT;
        this->l.log_line(this->r_vector());
    }
    bool simulation_stop()
    {
        return this->num_cust_delayed==this->limit_queue_size;
    }
    void update_counters()
    {
        this->d_sim_clock=this->sim_clock-this->sim_clock_previous;
        this->sim_clock_previous=this->sim_clock;
        this->area_num_inq+=(this->queue_size)*this->d_sim_clock;
        this->area_server_status+=this->server.isBusy()*this->d_sim_clock;
    }
    void update_sim_clock(double time)
    {
        this->sim_clock=time;
        this->update_counters();
    }
    void arrive()
    {
        this->event++;
        this->q_event=Queue_Event::ARRIVE;
        update_sim_clock(this->custumer_queue.back().get_arrival_time());
        this->custumer_queue.push(Custumer(this->interarrival_mean, this->sim_clock));
        if(this->server.isBusy())
            {
                this->queue_size++;
            }
        else
            {
                this->total_of_delays+=0;
                this->num_cust_delayed++;
                this->server.setStatus(Server_Status::BUSY);
                this->custumer_queue.front().set_service_time(this->server.Generate_Service(this->sim_clock));
            }
    }
    void depart()
    {
        this->event++;
        this->q_event=Queue_Event::DEPART;
        update_sim_clock(this->custumer_queue.front().get_service_time());
        if(this->queue_size==0)
            {
                this->server.setStatus(Server_Status::IDLE);
                this->custumer_queue.pop();
            }
        else
            {
                this->queue_size--;
                this->num_cust_delayed++;
                this->custumer_queue.pop();
                this->custumer_queue.front().set_service_time(this->server.Generate_Service(this->sim_clock));
                this->custumer_queue.front().set_delay_time(this->sim_clock);
                this->total_of_delays+=this->custumer_queue.front().get_delay_time();
            }
    }
    void debug()
    {
        cout<<"Event Number:"<<this->event<<endl;
        cout<<"Simulation Time Previous:"<<this->sim_clock_previous-this->d_sim_clock<<endl;
        cout<<"Simulation Time:"<<this->sim_clock<<endl;
        cout<<"Delta Simulation Time:"<<this->d_sim_clock<<endl;
        cout<<"Event:"<<static_cast<char>(this->q_event)<<endl;
        cout<<"Server Status:"<<server.isBusy()<<endl;
        cout<<"Queue  Size:"<<this->queue_size<<endl;
        auto custumer_queue_aux=this->custumer_queue;
        int i=1;
        while(!custumer_queue_aux.empty())
            {
                cout<<"\tClient:"<<i<<endl;
                cout<<"\t\tArrival_Time:"<<custumer_queue_aux.front().get_arrival_time()<<endl;
                cout<<"\t\tService_Time:"<<custumer_queue_aux.front().get_service_time()<<endl;
                cout<<"\t\tDelay:"<<custumer_queue_aux.front().get_delay_time()<<endl;
                custumer_queue_aux.pop();
                i++;
            }
            cout<<"Total Delay:"<<this->total_of_delays<<endl;
        cout<<"Next Arrive:"<<this->custumer_queue.back().get_arrival_time()<<endl;
        cout<<"Next Depart:"<<this->custumer_queue.front().get_service_time()<<endl;
        cout<<"Next Event:";
        if (this->custumer_queue.back().get_arrival_time()<this->custumer_queue.front().get_service_time())
            {
                cout<<"Arrive"<<endl;
            }
        else
            {
                cout<<"Depart"<<endl;
            }
        cout<<"Number Delayed:"<<this->num_cust_delayed<<endl;
        cout<<"Total Delay:"<<this->total_of_delays<<endl;
        cout<<"Area under Q(t):"<<this->area_num_inq<<endl;
        cout<<"Area under B(t):"<<this->area_server_status<<endl;
        cout<<"-----------------------------------------"<<endl;
        cout<<endl;
        //cin.get();
    }
    vector<string> h_vector()
    {
        vector<string> header;
        header.push_back("Event Number");
        header.push_back("Simulation Time Previous");
        header.push_back("Simulation Time");
        header.push_back("Delta Simulation Time");
        header.push_back("Event");
        header.push_back("Server Status");
        header.push_back("Queue Size");
        header.push_back("Total Delay");
        header.push_back("Num Cust Delayed");
        header.push_back("Next Arrive");
        header.push_back("Next Depart");
        header.push_back("Next Event");
        return header;
    }
    vector<string> r_vector()
    {
        vector<string> row;
        row.push_back(to_string(this->event));
        row.push_back(to_string(this->sim_clock_previous-this->d_sim_clock));
        row.push_back(to_string(this->sim_clock));
        row.push_back(to_string(this->d_sim_clock));
        row.push_back((static_cast<char>(this->q_event)=='A') ? "ARRIVE":"DEPART");
        row.push_back(to_string(this->server.isBusy()));
        row.push_back(to_string(this->queue_size));
        row.push_back(to_string(this->total_of_delays));
        row.push_back(to_string(this->num_cust_delayed));
        row.push_back(to_string(this->custumer_queue.back().get_arrival_time()));
        row.push_back(to_string(this->custumer_queue.front().get_service_time()));
        row.push_back((static_cast<char>(this->next_q_event)=='A')? "ARRIVE":"DEPART");
        return row;
    }
    void next_event()
    {
        if (this->custumer_queue.back().get_arrival_time()<this->custumer_queue.front().get_service_time())
            {
                this->next_q_event= Queue_Event::ARRIVE;
            }
        else
            {
                this->next_q_event= Queue_Event::DEPART;
            }
    }
    void simulate()
    {
        while(!simulation_stop())
            {
                if(this->next_q_event==Queue_Event::ARRIVE)
                    {
                        this->arrive();
                    }
                else
                    {
                        this->depart();
                    }
                this->next_event();
                this->log();
            }
        this->report();
    }
    void log()
    {
        if(this->l.check())
        {
            this->control+="x";
            this->l=Log("qlog"+control+".csv",',', this->h_vector());
            this->l.log_line(this->r_vector());
        }
        else
        {
            this->l.log_line(this->r_vector());
        }
    }
    void report()
    {
        cout<<"Interarrival mean:"<<this->interarrival_mean<<endl;
        cout<<"Service mean:"<<this->service_mean<<endl;
        cout<<"Limit size:"<<this->limit_queue_size<<endl;
        cout<<"Average Delay in Queue:"<<this->total_of_delays/this->num_cust_delayed<<endl;
        cout<<"Average Number in Queue:"<<this->area_num_inq/this->sim_clock<<endl;
        cout<<"Server  Utilization:"<<this->area_server_status/this->sim_clock<<endl;
    }
};

int main()
{

        set_A();
        set_S();
    Queue_System q(1.0,.5,1000000);
    q.simulate();
}
