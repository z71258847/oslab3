#include <bits/stdc++.h>
#define processing 0
#define blocked 1
#define terminated 2
#define aborted 3
#define initiate 0
#define request 11
#define release 22
#define compute 33
#define terminate 44

using namespace std;
int n,m,k;

struct Op{
	int instr, r, num;
	Op(int w, int y, int z): instr(w), r(y), num(z){
	};
};

struct Task{
	vector<int> resource, max_r;
	bool finished = false;
	int wait_t = 0;
	int terminate_t = 0;
	int state = 0;
	bool operated;
	deque<Op> ops;
	Task(){
		resource.assign(m+1, 0);
		max_r.assign(m+1, 0);
	}
};


void optimistic(vector<Task> tasks, vector<int> resources){
	int count = 0;
	int cycle = 0;
	vector<int> pending;
	pending.assign(m+1, 0);
	vector<int> blocked_queue;
	while (count < n) {
		
		for (int i=1; i<=m; i++) resources[i]+=pending[i];
		
		pending.assign(m+1, 0);
		bool flag = false;
		//cout<<cycle << endl;
		
		for (int i=1; i<=n; i++) tasks[i].operated = false;
		
		
		for (int j=0; j<blocked_queue.size();j++) {
			int i = blocked_queue[j];
			if (tasks[i].state == aborted){
				blocked_queue.erase(blocked_queue.begin()+j);
				j--;
			}
			else {
				//cout << "blocked check " << i << endl;
				Op temp = tasks[i].ops.front();
				if (resources[temp.r] >= temp.num) {
					tasks[i].resource[temp.r] += temp.num;
					resources[temp.r] -= temp.num;
					tasks[i].ops.pop_front();
					tasks[i].state = processing;
					tasks[i].operated = true;
					blocked_queue.erase(blocked_queue.begin()+j);
					j--;
					flag = true;
				}
			}
		}
		
		for (int i=1; i<=n; i++){
			if (!tasks[i].finished) {
				if (tasks[i].state != blocked && !tasks[i].operated) {
					Op temp = tasks[i].ops.front();
					//cout << i << " " << temp.instr << " " <<temp.r << " " <<temp.num << endl;
					if (temp.instr == initiate) {
						tasks[i].max_r[temp.r] = temp.num;
						tasks[i].ops.pop_front();
						flag = true;
					}
					else if (temp.instr == terminate) {
						tasks[i].terminate_t = cycle;
						tasks[i].finished = true;
						count++;
						tasks[i].state = terminated;
						tasks[i].ops.pop_front();
						for (int j=1; j<=m; j++) {
							pending[j] += tasks[i].resource[j];
						} 
						flag = true;
					}
					else if (temp.instr == release) {
						tasks[i].resource[temp.r] -= temp.num;
						pending[temp.r] += temp.num;
						tasks[i].ops.pop_front();
						flag = true;
					}
					else if (temp.instr == compute) {
						tasks[i].ops[0].r--;
						if (tasks[i].ops[0].r == 0) {
							tasks[i].ops.pop_front();
						}
						flag = true;
					}
					else if (temp.instr == request){
						if (resources[temp.r] >= temp.num) {
							tasks[i].resource[temp.r] += temp.num;
							resources[temp.r] -= temp.num;
							tasks[i].ops.pop_front();	
							flag = true;					
						}
						else{
							tasks[i].state = blocked;
							//tasks[i].wait_t++;
							blocked_queue.push_back(i);
						}
					}
				}
				else{
					tasks[i].wait_t++;
				}
			}
		}
		
		if (!flag) {
			for (int i=1; i<=n; i++) {
				if (tasks[i].state == blocked) {
					Op temp = tasks[i].ops.front();
					if (pending[temp.r] + resources[temp.r] < temp.num){
						//cout << "abort " << i << endl;
						tasks[i].state = aborted;
						tasks[i].finished = true;
						count++;
						for (int q=1; q<=m; q++) {
							pending[q] += tasks[i].resource[q];
						}
					}
					else break;
				}
			}
		}
		cycle++;
	}
	cout << "FIFO" << endl;
	int tot_t=0, tot_wt=0;
	for (int i=1; i<=n; i++) {
		if (tasks[i].state!=aborted){
			cout << "Task " << i << " " << tasks[i].terminate_t << " " << tasks[i].wait_t << " ";
			printf("%.0f%\n", round(100.0*tasks[i].wait_t/tasks[i].terminate_t));
			tot_t += tasks[i].terminate_t;
			tot_wt += tasks[i].wait_t;
		}
		else cout << "Task " << i << " aborted" << endl;
	}
	cout <<  "total  " << tot_t << " " << tot_wt << " ";
	printf("%.0f%\n", round(100.0*tot_wt/tot_t));
	cout << endl; 
}

bool banker_check(vector<Task> tasks, vector<int> resources, int x, int r, int num) {
	if (resources[r] < num) return false;
	bool flag = true;
	int count = 0;
	tasks[x].resource[r] += num;
	resources[r] -= num;
	for (int i=1; i<=n; i++) {
		if (tasks[i].finished || tasks[i].state == aborted) count++;
	}
	while (count < n && flag) {
		flag = false;
		/*for (int q=1; q<=m; q++) {
			cout << resources[q] << " ";
			for (int i=1; i<=n; i++) {
				cout << i << ":" << tasks[i].max_r[q]-tasks[i].resource[q] << " ";
			}
			cout << endl;
		}*/
		
		for (int i=1; i<=n; i++) {
			//cout << "check " << i << endl;
			if (!tasks[i].finished && !(tasks[i].state == aborted)){
				bool enough = true;
				for (int q=1; q<=m; q++) {
					if (resources[q] < tasks[i].max_r[q] - tasks[i].resource[q]) {
						enough = false;
					}
					//cout << i << ":" << tasks[i].max_r[q]-tasks[i].resource[q] << " ";
				}
				//cout << endl;
				if (enough) {
					tasks[i].finished = true;
					count++;
					for (int q=1; q<=m; q++) {
						resources[q] += tasks[i].resource[q];
					}
					flag = true;
				}
			}
		}
	}
	return flag;
}

void banker(vector<Task> tasks, vector<int> resources) {
	int count = 0;
	int cycle = 0;
	vector<int> pending;
	pending.assign(m+1, 0);
	vector<int> blocked_queue;
	vector<int> init_resources = resources;
	while (count < n) {
		
		for (int i=1; i<=m; i++) resources[i]+=pending[i];
		
		pending.assign(m+1, 0);
		bool flag = false;
		//cout<<cycle << endl;
		
		for (int i=1; i<=n; i++) tasks[i].operated = false;
		
		
		for (int j=0; j<blocked_queue.size();j++) {
			int i = blocked_queue[j];
			//cout << "blocked check " << i << endl;
			Op temp = tasks[i].ops.front();
			if (banker_check(tasks, resources, i, temp.r, temp.num)) {
				tasks[i].resource[temp.r] += temp.num;
				resources[temp.r] -= temp.num;
				tasks[i].ops.pop_front();
				tasks[i].state = processing;
				tasks[i].operated = true;
				blocked_queue.erase(blocked_queue.begin()+j);
				j--;
				flag = true;
			}
		}
		
		for (int i=1; i<=n; i++){
			if (!tasks[i].finished) {
				if (tasks[i].state != blocked && !tasks[i].operated) {
					Op temp = tasks[i].ops.front();
					//cout << i << " " << temp.instr << " " <<temp.r << " " <<temp.num << endl;
					if (temp.instr == initiate) {
						if (temp.num > init_resources[temp.r]) {
							cout << "Banker aborts task "<< i << " before run begins:" << endl;
							cout << "    claim for resourse " << temp.r << "(" << temp.num<< ") exceeds number of units present (" << init_resources[temp.r] << ")" << endl;
							tasks[i].state = aborted;
							tasks[i].finished = true;
							count++;
							//cout << "abort " << i << endl;
							for (int q=1; q<=m; q++) {
								pending[q] += tasks[i].resource[q];
							}
						}
						else {
							tasks[i].max_r[temp.r] = temp.num;
							tasks[i].ops.pop_front();
							flag = true;
						}
					}
					else if (temp.instr == terminate) {
						tasks[i].terminate_t = cycle;
						tasks[i].finished = true;
						count++;
						tasks[i].state = terminated;
						tasks[i].ops.pop_front();
						for (int j=1; j<=m; j++) {
							pending[j] += tasks[i].resource[j];
						} 
						flag = true;
					}
					else if (temp.instr == release) {
						tasks[i].resource[temp.r] -= temp.num;
						pending[temp.r] += temp.num;
						tasks[i].ops.pop_front();
						flag = true;
					}
					else if (temp.instr == compute) {
						tasks[i].ops[0].r--;
						if (tasks[i].ops[0].r == 0) {
							tasks[i].ops.pop_front();
						}
						flag = true;
					}
					else if (temp.instr == request){
						if (tasks[i].resource[temp.r]+temp.num>tasks[i].max_r[temp.r]){
							cout << "During cycle " << i+1 << "-" << i+2 << " of Banker's algorithms" << endl;
							cout << "    Task "<<i << "'s request exceeds its claim; aborted; "<< tasks[i].resource[temp.r] <<" units available next cycle" << endl;
							tasks[i].state = aborted;
							tasks[i].finished = true;
							count++;
							//cout << "abort " << i << endl;
							for (int q=1; q<=m; q++) {
								pending[q] += tasks[i].resource[q];
							}
						}
						else if (banker_check(tasks, resources, i, temp.r, temp.num)) {
							tasks[i].resource[temp.r] += temp.num;
							resources[temp.r] -= temp.num;
							tasks[i].ops.pop_front();	
							flag = true;					
						}
						else{
							tasks[i].state = blocked;
							//tasks[i].wait_t++;
							blocked_queue.push_back(i);
						}
					}
				}
				else{
					tasks[i].wait_t++;
				}
			}
		}
		
		if (!flag) {
			for (int i=1; i<=n; i++) {
				if (tasks[i].state == blocked) {
					Op temp = tasks[i].ops.front();
					if (pending[temp.r] + resources[temp.r] < temp.num){
						cout << "abort " << i << endl;
						tasks[i].state = aborted;
						tasks[i].finished = true;
						count++;
						for (int q=1; q<=m; q++) {
							pending[q] += tasks[i].resource[q];
						}
					}
					else break;
				}
			}
		}
		cycle++;
	}
	
	cout << "Banker's" << endl;
	int tot_t=0, tot_wt=0;
	for (int i=1; i<=n; i++) {
		if (tasks[i].state!=aborted){
			cout << "Task " << i << " " << tasks[i].terminate_t << " " << tasks[i].wait_t << " ";
			printf("%.0f%\n", round(100.0*tasks[i].wait_t/tasks[i].terminate_t));
			tot_t += tasks[i].terminate_t;
			tot_wt += tasks[i].wait_t;
		}
		else cout << "Task " << i << " aborted" << endl;
	}
	cout <<  "total  " << tot_t << " " << tot_wt << " ";
	printf("%.0f%\n", round(100.0*tot_wt/tot_t));
}

int main(int argc, char** argv){
	ifstream input(argv[1]);
	vector<Task> tasks;
	vector<int> resources;
	input >> n >> m;
	tasks.assign(n+1, Task());
	resources.assign(m+1, 0);
	for (int i=1; i<=m; i++){
		int x;
		input >> x;
		resources[i] = x;
	}
	string s;
	int x, y, z;
	while (input >> s >> x >> y >> z) {
		if (s == "initiate") {
			tasks[x].ops.push_back(Op(initiate, y, z));
		}
		else if (s == "request") {
			tasks[x].ops.push_back(Op(request, y, z));
		}
		else if (s == "release") {
			tasks[x].ops.push_back(Op(release, y, z));
		}
		else if (s == "compute") {
			tasks[x].ops.push_back(Op(compute, y, z));
		} 
		else{
			tasks[x].ops.push_back(Op(terminate, y, z));
		}
	}
	
	optimistic(tasks, resources);
	banker(tasks, resources);
}
