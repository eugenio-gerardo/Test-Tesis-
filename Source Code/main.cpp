#include <iostream>
#include <fstream>
#include "random.h"

#include <string>
#include <sstream>
#include <queue>
#include <fstream>

using namespace std;

struct QueueParameters {
   double  mean_interarrival;
   double  mean_service;
   int num_delays_required;
};

class File {
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
        for(auto x:this->values){
            string line="";
            for(auto y:x){
                y+=this->separator;
                line += y;
                }
            cout<<" "<<line<<endl;
        }
    }
};

enum class Server_Status {IDLE, BUSY};

class Server
{
    private:
        double mean_service;
        Server_Status status;
    public:
        Server(){}
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
        }
        void set_service_time(double service_time)
        {
            this->service_time=service_time;
        }
        void set_delay_time(double delay_time)
        {
            this->delay_time=delay_time;
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
            return this->service_time-this->arrival_time;
        }
};

class Queue_System
{
    private:
        int queue_size;
        queue<Custumer> custumer_queue;
        Server server;
        double sim_clock;
        double sim_clock_previous;
        double interarrival_mean;
        //Statical Counters
        double num_cust_delayed;
        double total_of_delays;
        double area_num_inq;
        double area_server_status;

        int a;
        int d;

    public:
        Queue_System(double interarrival_mean, double service_mean)
        {
            this->server=Server(service_mean);
            this->sim_clock_previous=0;
            this->interarrival_mean=interarrival_mean;
            custumer_queue.push(Custumer(this->interarrival_mean, this->sim_clock));
            this->queue_size=0;
            this->sim_clock=this->custumer_queue.front().get_arrival_time();
            this->num_cust_delayed=0;
            this->total_of_delays=0;
            this->area_num_inq=0;
            this->area_server_status=0;
            a=0;
            d=0;
        }
        bool simulation_stop()
        {
            return this->num_cust_delayed==100;
        }
        void update_counters()
        {
            double time_since_last_event=this->sim_clock-this->sim_clock_previous;
            this->sim_clock_previous=this->sim_clock;
            this->area_num_inq+=(this->queue_size)*time_since_last_event;
            this->area_server_status+=this->server.isBusy()*time_since_last_event;
        }
        void arrive()
        {
            this->sim_clock=this->custumer_queue.back().get_arrival_time();
            this->custumer_queue.push(Custumer(this->interarrival_mean, this->sim_clock));
            if(this->server.isBusy())
            {
                cout<<"if"<<endl;
                this->queue_size++;
            }
            else
            {
                cout<<"else"<<endl;
                this->total_of_delays+=0;
                this->num_cust_delayed++;
                this->server.setStatus(Server_Status::BUSY);
                this->custumer_queue.front().set_service_time(this->server.Generate_Service(this->sim_clock));
            }
        }
        void depart()
        {
            this->sim_clock=this->custumer_queue.front().get_service_time();
            if(this->queue_size==0)
            {
                this->server.setStatus(Server_Status::IDLE);
                this->custumer_queue.pop();
            }
            else
            {
                this->queue_size--;
                this->total_of_delays+=this->custumer_queue.back().get_delay_time();
                ++this->num_cust_delayed;
                this->custumer_queue.pop();
                this->custumer_queue.front().set_service_time(this->server.Generate_Service(this->sim_clock));
            }
        }
        void debug()
        {
            cout<<"Sim_clock:"<<sim_clock<<endl;
            cout<<"size:"<<this->queue_size<<endl;
            cout<<"server status:"<<server.isBusy()<<endl;
            auto custumer_queue_aux=this->custumer_queue;
            while(!custumer_queue_aux.empty())
            {
                cout<<"Client"<<endl;
                cout<<"Arrival_Time:"<<custumer_queue_aux.front().get_arrival_time()<<endl;
                cout<<"Service_Time:"<<custumer_queue_aux.front().get_service_time()<<endl;
                custumer_queue_aux.pop();
            }
            cin.get();
        }
        void simulate()
        {
            debug();
            while(!simulation_stop())
            {
                this->update_counters();
                if(this->custumer_queue.front().get_service_time()>this->custumer_queue.back().get_arrival_time())
                    {
                        this->arrive();
                        cout<<"Arrive:"<<a++<<endl;
                        debug();
                    }
                else
                    {
                        this->depart();
                        cout<<"Depart:"<<d++<<endl;
                        debug();
                    }
            }
            this->report();
        }
        void report()
        {
            cout<<"Average Delay in Queue:"<<this->total_of_delays/this->num_cust_delayed<<endl;
            cout<<"Average Number in Queue:"<<this->area_num_inq/this->sim_clock<<endl;
            cout<<"Server  Utilization:"<<this->area_server_status/this->sim_clock<<endl;
        }
};

int main()
{
    /*
    QueueParameters a1, a2;
    ifstream fe("/home/eugenio/Downloads/mm12.out",ios::in | ios::binary);
    ofstream fsalida("/home/eugenio/Downloads/mm12.out", ios::out | ios::binary);
    a1.mean_interarrival=1;
    a1.mean_service=.5;
    a1.num_delays_required=1000;
    fsalida.write(reinterpret_cast<char *>(&a1), sizeof(QueueParameters));
    fsalida.close();
    fe.read(reinterpret_cast<char *>(&a2), sizeof(QueueParameters));
    cout<<a2.mean_interarrival<<endl;
    cout<<a2.mean_service<<endl;
    cout<<a2.num_delays_required<<endl;

    int x=5;
    for (int i=0; i<=100; i++)
    cout<<exp_random(5)<<endl;
    string files ="/home/eugenio/Downloads/mm12.in";
    char s=',';
    File x(files,s);
    x.readfile();
    x.print_list();
    //x.print_lines();*/
    Queue_System q(1,0.5);
    q.simulate();
}
