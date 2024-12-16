#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <queue>
#include <set>

using namespace std;

struct PCB {
    string name;
    float arrivalTime;
    float serviceTime;
    float remainingTime;  // 剩余服务时间
    float startTime=-1;
    float finishTime;
    float turnaroundTime;
    float weightedTurnaroundTime;
    int priority;  // 仅用于优先级抢占
};

// 打印队列状态
// 打印队列状态
void printQueueStatus(float currentTime, const vector<PCB>& processes, const vector<int>& readyQueue, const int runningProcess, const vector<int>& completedQueue) {
    cout << "时间 " << currentTime << " 时刻:" << endl;

    // 打印就绪队列
    cout << "就绪队列: ";
    for (const auto& index : readyQueue) {
        if (runningProcess != index ) cout << processes[index].name << " ";
    }
    cout << endl;

    // 打印运行队列
    cout << "运行队列: ";
    if (runningProcess != -1) {
        cout << processes[runningProcess].name << " ";
    }
    cout << endl;

    // 打印完成队列
    cout << "完成队列: ";
    for (const auto& index : completedQueue) {
        cout << processes[index].name << " ";
    }
    cout << endl;
}

// 输入进程信息
void input(vector<PCB>& processes, int algorithm) {
    cout << "请输入进程的名字、到达时间、服务时间";
    if (algorithm == 2) {
        cout << "和优先级（例如: a 0 100 2）：" << endl;
    } else {
        cout << "（例如: a 0 100）：" << endl;
    }
    for (size_t i = 0; i < processes.size(); ++i) {
        cout << "请输入进程 " << i + 1 << " 的信息：";
        cin >> processes[i].name >> processes[i].arrivalTime >> processes[i].serviceTime;
        processes[i].remainingTime = processes[i].serviceTime;  // 初始化剩余时间
        if (algorithm == 2) {  // 如果是优先级抢占算法，输入优先级
            cin >> processes[i].priority;
        }
    }
}

// 按照到达时间和服务时间排序
void sortProcesses(vector<PCB>& processes) {
    sort(processes.begin(), processes.end(), [](const PCB& a, const PCB& b) {
        return (a.arrivalTime < b.arrivalTime) ||
               (a.arrivalTime == b.arrivalTime && a.serviceTime < b.serviceTime);
    });
}


// 先进先出调度（FIFO）
void fifoSchedule(vector<PCB>& processes, vector<string>& scheduleOrder) {
    sortProcesses(processes);  // 按到达时间排序
    vector<int> readyQueue;  // 就绪队列
    vector<int> completedQueue;  // 完成队列
    int runningProcess;
    float currentTime = 0;  // 当前时间
    size_t completed = 0;  // 完成的进程数

    // 记录队列状态
    while (completed < processes.size()) {
        // 将所有到达的进程加入就绪队列
        for (size_t i = 0; i < processes.size(); ++i) {
            if (processes[i].arrivalTime <= currentTime && processes[i].remainingTime > 0 && find(readyQueue.begin(), readyQueue.end(), i) == readyQueue.end()) {
                readyQueue.push_back(i);  // 进程加入就绪队列
            }
        }

        // 只有当就绪队列不为空时，执行队头进程
        if (!readyQueue.empty()) {
            // 执行队头进程
            int index = readyQueue.front();  // 先进先出，队头进程
            // 打印队列状态
            printQueueStatus(currentTime, processes, readyQueue, index, completedQueue);

            readyQueue.erase(readyQueue.begin());  // 移除队头进程
            PCB& process = processes[index];

            if (process.startTime == -1) {
                process.startTime = currentTime;  // 进程开始执行时间
            }

            process.remainingTime = 0;  // 执行完毕
            currentTime += process.serviceTime;  // 更新时间

            process.finishTime = currentTime;
            process.turnaroundTime = process.finishTime - process.arrivalTime;
            process.weightedTurnaroundTime = process.turnaroundTime / process.serviceTime;

            completedQueue.push_back(index);  // 完成队列加入当前进程
            completed++;  // 更新完成的进程数
            if( completed == processes.size() ){
                // 打印队列状态
                printQueueStatus(currentTime, processes, readyQueue, -1, completedQueue);
            }

            // 记录进程执行顺序
            scheduleOrder.push_back(process.name);
        } else {
            // 如果没有进程可执行，则时间前进
            // 打印队列状态
            printQueueStatus(currentTime, processes, readyQueue, -1, completedQueue);
            currentTime++;
        }

    }
}



// 优先级抢占调度（Preemptive Priority）
void preemptivePrioritySchedule(vector<PCB>& processes, vector<string>& scheduleOrder) {
    sortProcesses(processes);  // 按到达时间排序
    priority_queue<pair<int, int>> pq;  // 优先级队列，存储 (优先级, 进程索引)
    float currentTime = 0;  // 当前时间
    size_t completed = 0;  // 完成的进程数

    while (completed < processes.size()) {
        priority_queue<pair<int, int>>().swap(pq);
        // 将所有到达的进程加入队列
        for (size_t i = 0; i < processes.size(); ++i) {
            if (processes[i].arrivalTime <= currentTime && processes[i].remainingTime > 0) {
                pq.push({-processes[i].priority, i});  // 注意：优先级越高，值越小，所以我们存负数
            }
        }

        if (!pq.empty()) {
            // 从队列中选取优先级最高的进程
            int index = pq.top().second;
            pq.pop();

            PCB& process = processes[index];

            // 如果该进程还没有开始，则开始执行
            if (process.startTime == -1) {
                process.startTime = currentTime;
            }

            // 执行该进程
            float execTime = min(process.remainingTime, 1.0f);  // 每次执行1时间单位
            process.remainingTime -= execTime;
            currentTime += execTime;

            // 记录进程执行顺序
            for (float t = currentTime - execTime; t < currentTime; t++) {
                scheduleOrder.push_back(process.name);
            }

            // 如果该进程执行完，更新结束时间
            if (process.remainingTime == 0) {
                process.finishTime = currentTime;
                process.turnaroundTime = process.finishTime - process.arrivalTime;
                process.weightedTurnaroundTime = process.turnaroundTime / process.serviceTime;
                completed++;
            }
        } else {
            // 如果没有进程可执行，则时间前进
            currentTime++;
        }
    }
}

// 时间片轮转调度（Round Robin）
void roundRobinSchedule(vector<PCB>& processes, float timeQuantum, vector<string>& scheduleOrder) {
    sortProcesses(processes);  // 按到达时间排序
    queue<int> q;  // 队列用于存储待执行的进程索引
    float currentTime = 0;  // 当前时间
    size_t completed = 0;  // 完成的进程数
    bool flag = false;
    int pre_index;
    vector<bool> processesJudge(processes.size(), false);

    while (completed < processes.size()) {
        // 将所有到达的进程加入队列
        for (size_t i = 0; i < processes.size(); ++i) {
            if (processes[i].arrivalTime <= currentTime && processes[i].startTime == -1) {
                // 进程尚未开始且已到达
                processes[i].startTime = currentTime;  // 进程开始时间
                q.push(i);
            }
        }

        if(flag == true) {
            q.push(pre_index);
        }

        if (!q.empty()) {
            flag = false;
            // 执行队头进程
            int index = q.front();
            // 修改首次开始时间
            if (processesJudge[index] == false) {
                processes[index].startTime = currentTime;
                processesJudge[index] = true;
            }

            q.pop();
            PCB& process = processes[index];

            float execTime = min(process.remainingTime, timeQuantum);
            process.remainingTime -= execTime;
            currentTime += execTime;

            // 记录进程执行顺序
            for (float t = currentTime - execTime; t < currentTime; t++) {
                scheduleOrder.push_back(process.name);
            }

            // 如果进程执行完，更新结束时间
            if (process.remainingTime == 0) {
                process.finishTime = currentTime;
                process.turnaroundTime = process.finishTime - process.arrivalTime;
                process.weightedTurnaroundTime = process.turnaroundTime / process.serviceTime;
                completed++;
            } else {
                // 如果进程没有执行完，重新入队
                pre_index = index;
                flag = true;
            }
        } else {
            // 如果没有进程可执行，则时间前进
            currentTime++;
        }
    }
}


// 输出进程调度信息
void printSchedule(const vector<PCB>& processes, const vector<string>& scheduleOrder) {
    cout << "\n进程调度顺序：";
    for (const auto& name : scheduleOrder) {
        cout << name;
        if (&name != &scheduleOrder.back()) cout << " --> ";
    }
    cout << "\n\n";

    cout << left << setw(10) << "Name" << setw(12) << "arrivalTime" << setw(12) << "serviceTime"
         << setw(12) << "startTime" << setw(12) << "finishTime" << setw(12) << "aroundTime" << setw(20) << "weightedTurnaroundTime" << endl;

    for (const auto& process : processes) {
        cout << setw(10) << process.name
             << setw(12) << process.arrivalTime
             << setw(12) << process.serviceTime
             << setw(12) << process.startTime
             << setw(12) << process.finishTime
             << setw(12) << process.turnaroundTime
             << setw(20) << process.weightedTurnaroundTime << endl;
    }
}

// 计算平均周转时间和带权周转时间
void calculateAverage(const vector<PCB>& processes) {
    float totalTurnaroundTime = 0, totalWeightedTurnaroundTime = 0;
    for (const auto& process : processes) {
        totalTurnaroundTime += process.turnaroundTime;
        totalWeightedTurnaroundTime += process.weightedTurnaroundTime;
    }

    int N = processes.size();
    cout << "平均周转时间: " << fixed << setprecision(3) << totalTurnaroundTime / N << endl;
    cout << "平均带权周转时间: " << fixed << setprecision(3) << totalWeightedTurnaroundTime / N << endl;
}

int main() {
    int N, choice;
    float timeQuantum;

    cout << "<<---------- 进程调度算法 ---------->>" << endl;
    cout << "请输入进程数目: ";
    cin >> N;

    vector<PCB> processes(N);
    vector<string> scheduleOrder;  // 用于记录进程调度顺序

    cout << "请选择调度算法：" << endl;
    cout << "1. 先进先出（FIFO）" << endl;
    cout << "2. 优先级抢占" << endl;
    cout << "3. 时间片轮转" << endl;
    cout << "请输入您的选择（1/2/3）：";
    cin >> choice;

    input(processes, choice);

    switch (choice) {
        case 1:
            fifoSchedule(processes, scheduleOrder);
            break;
        case 2:
            preemptivePrioritySchedule(processes, scheduleOrder);
            break;
        case 3:
            cout << "请输入时间片长度: ";
            cin >> timeQuantum;
            roundRobinSchedule(processes, timeQuantum, scheduleOrder);
            break;
        default:
            cout << "无效选择！" << endl;
            return 1;
    }

    printSchedule(processes, scheduleOrder);
    calculateAverage(processes);

    return 0;
}
