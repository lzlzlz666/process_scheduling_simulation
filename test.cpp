#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <queue>

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
    for (size_t i = 0; i < processes.size(); ++i) {
        if (i == 0) {
            processes[i].startTime = processes[i].arrivalTime;
        } else {
            processes[i].startTime = max(processes[i - 1].finishTime, processes[i].arrivalTime);
        }
        processes[i].finishTime = processes[i].startTime + processes[i].serviceTime;
        processes[i].turnaroundTime = processes[i].finishTime - processes[i].arrivalTime;
        processes[i].weightedTurnaroundTime = processes[i].turnaroundTime / processes[i].serviceTime;
        // 记录进程执行顺序
        for (float t = processes[i].startTime; t < processes[i].finishTime; t++) {
            scheduleOrder.push_back(processes[i].name);
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

    cout
        << setw(10) << "进程名"
        << setw(12) << "到达时间"
        << setw(12) << "服务时间"
        << setw(12) << "开始时间"
        << setw(12) << "结束时间"
        << setw(12) << "周转时间"
        << setw(12) << "带权周转时间" << endl;

    for (const auto& process : processes) {
        cout << setw(10) << process.name
             << setw(12) << process.arrivalTime
             << setw(12) << process.serviceTime
             << setw(12) << process.startTime
             << setw(12) << process.finishTime
             << setw(12) << process.turnaroundTime
             << setw(12) << process.weightedTurnaroundTime << endl;
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
