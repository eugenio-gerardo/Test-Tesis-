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
        return time+exp_random(this->mean_service);
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
        this->arrival_time=time+exp_random(interarrival_mean);
        this->service_time=1000000;
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
public:
    Queue_System(double interarrival_mean, double service_mean, int limit_queue_size)
    {
        //state variables
        this->event=1;
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
        
        cout<<this->h_string()<<endl;
    }
    bool simulation_stop()
    {
        return this->num_cust_delayed==this->limit_queue_size;
    }
    void update_counters()
    {
        this->area_num_inq+=(this->queue_size)*this->d_sim_clock;
        this->area_server_status+=this->server.isBusy()*this->d_sim_clock;
    }
    void update_sim_clock(double time)
    {
        this->sim_clock_previous=this->sim_clock;
        this->sim_clock=time;
        this->d_sim_clock=this->sim_clock-this->sim_clock_previous;
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
                //this->custumer_queue.front().set_delay_time(this->sim_clock);
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
                this->total_of_delays+=this->custumer_queue.front().get_delay_time();
                this->num_cust_delayed++;
                this->custumer_queue.pop();
                this->custumer_queue.front().set_service_time(this->server.Generate_Service(this->sim_clock));
                this->custumer_queue.front().set_delay_time(this->sim_clock);
            }
    }
    void debug()
    {
        cout<<"Event Number:"<<this->event++<<endl;
        cout<<"Simulation Time Previous:"<<this->sim_clock_previous<<endl;
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
        cout<<"-----------------------------------------"<<endl;
        cout<<endl;
        //cin.get();
    }
    string h_string()
    {
        return "Event Number\tSimulation Time Previous\tSimulation Time\tDelta Simulation Time\tEvent\tServer Status\tQueue Size\tTotal Delay\tNum Cust Delayed\tNext Arrive\tNext Depart\tNext Event";
    }
    string r_string()
    {
        string r = to_string(this->event) + "\t"
        + to_string(this->sim_clock_previous) + "\t"
        + to_string(this->sim_clock) + "\t"
        + to_string(this->d_sim_clock) + "\t"
        + static_cast<char>(this->q_event) + "\t"
        + to_string(this->server.isBusy()) + "\t"
        + to_string(this->queue_size) + "\t"
        + to_string(this->total_of_delays) + "\t"
        + to_string(this->num_cust_delayed) + "\t"
        + to_string(this->custumer_queue.back().get_arrival_time()) + "\t"
        + to_string(this->custumer_queue.front().get_service_time()) + "\t"
        + static_cast<char>(this->next_q_event);
        return r;
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
        this->q_event=Queue_Event::INIT;
        cout<<this->r_string()<<endl;
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
                this->update_counters();
                this->next_event();
                cout<<this->r_string()<<endl;
            }
        this->report();
    }
    void report()
    {
        this->q_event=Queue_Event::END;
        cout<<this->r_string()<<endl;
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
    Queue_System q(1.0,.5,1000);
    q.simulate();
}
