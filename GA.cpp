#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<time.h>

//城市数量 N
#define N 48
//标识城市是否被访问,访问过置为 1
bool visit[N];
//城市距离矩阵
int distance[N][N];

//种群数量
#define POP_NUM 100
//迭代次数
#define GENERATE_COUNT 2000
#define GEN 10
//交叉概率
#define PC1 0.6
#define PC2 0.8
//变异概率
#define PM 0.2
//种群
int population[POP_NUM][N];
//保留的精英个体
int currentOpt[N];
int currentOptDistance;
int minIndex;
int maxIndex;
//种群平均距离和总距离
int avgDistance[POP_NUM];
int popDistance[POP_NUM];
int selectPop[POP_NUM][N];

void init()
{
	//城市的 x 和 y 坐标
	int x[N] = { 0 };
	int y[N] = { 0 };
	//从 data.txt 文件读取数据
	FILE* fp;
	if ((fp = fopen("..//att48.txt", "r")) == NULL)
	//if ((fp = fopen("..//kroB100.txt", "r")) == NULL)
	{
		printf("can not open the file!");
		exit(0);
	}
	while (!feof(fp))
	{
		int count;
		fscanf(fp, "%d", &count);
		fscanf(fp, "%d%d", &x[count - 1], &y[count - 1]);
	}
	fclose(fp);
	//计算城市之间距离
	for (int i = 0; i < N - 1; i++)
	{
		distance[i][i] = 0;				// 对角线为0
		for (int j = i + 1; j < N; j++)
		{
			double dis = sqrt((pow((double)x[i] - x[j], 2) / 10 + pow((double)y[i] - y[j], 2) / 10 ));
			int disInt = (int)dis;
			distance[i][j] = dis == disInt ? disInt : disInt + 1;
			distance[j][i] = distance[i][j];
		}
	}
	distance[N - 1][N - 1] = 0;
}

/* 贪心策略 */
void TSPGreedyAlgorithm()
{
	int totalDistance = 0;		//总路程
	//默认从 0 开始遍历
	int current = 0;			//当前选取结点	
	visit[0] = 1;
	printf("TSP 路径为：%d ->", 1);

	//遍历 N - 1 次
	for (int i = 1; i < N; i++)
	{
		//设置较大的距离初始值用来选取最近邻
		int min_distance = 0x7fffffff;
		//保存当前最近邻城市
		int temp;
		//循环选取城市
		for (int j = 1; j < N; j++)
		{
			if (!visit[j] && distance[current][j] < min_distance)
			{
				min_distance = distance[current][j];
				temp = j;
			}
		}
		visit[temp] = 1;
		current = temp;
		currentOpt[i] = temp;
		population[0][i] = temp;
		totalDistance += min_distance;
		printf(" %d ->", temp + 1);
	}
	totalDistance += distance[current][0];
	currentOptDistance = totalDistance;
	printf(" %d\n", 1);
}

/* 打印种群（测试） */
void printTest()
{
	for (int i = 0; i < POP_NUM; i++)
	{
		for (int j = 0; j < N; j++)
		{
			printf("%d ", population[i][j]);
		}
		printf("\n");
	}
}

/* 随机交换 num 次个体进行初始化 */
void swap(int* array, int num)
{
	int first, last, temp;
	for (int i = 0; i < num; i++)
	{
		first = rand() % N;
		last = rand() % N;
		temp = array[first];
		array[first] = array[last];
		array[last] = temp;
	}
}

/* 2-opt 交换 */
void reverse(int* array, int num)
{
	int first, last, temp;
	for (int i = 0; i < num; i++)
	{
		first = rand() % N;
		last = rand() % N;
		if (first > last)
		{
			temp = last;
			last = first;
			first = temp;
		}
		int sumBefore = 0;
		int sumAfter = 0;
		int sum = 0;
		for (int j = first; j < last; j++)
		{
			sum += distance[array[j]][array[j + 1]];
		}
		sumBefore = sum + distance[array[(first-1)%N]][array[first]] + distance[array[last]][array[(last + 1) % N]];
		sumAfter  = sum + distance[array[(first-1)%N]][array[last]] + distance[array[first]][array[(last + 1) % N]];
		if (sumAfter <= sumBefore)
		{
			for (int j = first; j < (last + first) / 2; j++)
			{
				temp = array[j];
				array[j] = array[first + last - j];
				array[first + last - j] = temp;
			}
		}
	}
}

/* 计算种群个体总距离,返回所有种群总距离 */
int calDistance()
{
	int minDistance = 0x7fffffff;
	int maxDistance = 0;
	int sum = 0;
	for (int i = 0; i < POP_NUM; i++)
	{
		popDistance[i] = 0;
		for (int j = 0; j < N - 1; j++)
		{
			popDistance[i] += distance[population[i][j]][population[i][j + 1]];
		}
		popDistance[i] += distance[population[i][N - 1]][population[i][0]];
		sum += popDistance[i];
		//计算最长、最短的个体路径长度
		if (popDistance[i] < minDistance)
		{
			minIndex = i;
			minDistance = popDistance[i];
		}
		if (popDistance[i] > maxDistance)
		{
			maxIndex = i;
			maxDistance = popDistance[i];
		}
	}
	//保存最优个体
	if (popDistance[minIndex] < currentOptDistance)
	{
		currentOptDistance = popDistance[minIndex];
		for (int i = 0; i < N; i++)
		{
			currentOpt[i] = population[minIndex][i];
		}
	}
	//淘汰最差的个体
	for (int i = 0; i < N; i++)
	{
		population[maxIndex][i] = currentOpt[i];
	}
	printf("当前最近距离为：%d\n", popDistance[minIndex]);
	return sum;
	//printf("当前最远距离为：%d\n", popDistance[maxIndex]);
}

/* 初始化种群 */
void initPop()
{
	//随机交换得到目标数量种群
	for (int i = 1; i < POP_NUM; i++)
	{
		for (int j = 0; j < N; j++)
		{
			population[i][j] = population[0][(j + i) % N];
		}
		swap(population[i], 1);
	}
	calDistance();
}

/* 选择：适应度代表个体被遗传到下一代群体中的概率
   轮盘赌选择方法的实现步骤:
	（1）计算群体中所有个体的适应度值；
	（2）计算每个个体的选择概率；
	（3）计算积累概率；
	（4）采用模拟赌盘操作（即生成0到1之间的随机数与每个个体遗传到下一代群体的概率进行匹配）来确定各个个体是否遗传到下一代群体中。
*/
void select()
{
	double popFit[POP_NUM];				//种群个体适应度
	double p[POP_NUM];					//种群个体的选择概率
	double sum = 0;
	//适应度
	for (int i = 0; i < POP_NUM; i++)
	{
		popFit[i] = 10000.0 / popDistance[i];	//适应度函数之为距离的倒数，注意分子为 double 结果才为 double
		sum += popFit[i];
	}
	//累计概率
	for (int i = 0; i < POP_NUM; i++)
	{
		p[i] = popFit[i] / sum;
	}
	//保留精英个体
	for (int k = 0; k < N; k++)
	{
		selectPop[0][k] = population[minIndex][k];
	}
	//轮盘赌选择
	for (int i = 1; i < POP_NUM; i++)
	{
		double temp = ((double)rand()) / RAND_MAX;
		for (int j = 0; j < POP_NUM; j++)
		{
			temp -= p[j];
			if (temp <= 0)
			{
				for (int k = 0; k < N; k++)
				{
					selectPop[i][k] = population[j][k];
				}
				break;
			}
		}
	}
}

/* 交叉：两个相互配对的染色体依据交叉概率 PC 按某种方式相互交换其部分基因，形成两个新的个体 
 * cross1():（1）随机选择一个父代的基因，在两个父代中找到该基因的位置，作为子代第一个基因；
 *			（2）判断下一个基因是否在子代中，分为 4 种情况并比较父代中这两个基因的距离，作为子代中的基因；
 *			（3）重复（2）步骤，直到形成个体
 *          （4）将子代逆转作为另一个子代
 * 该交叉运算基于贪心思想，可以继承父代优秀的基因，迅速收敛。
 *cross2()：单点交叉，扩展种群的覆盖范围
 */
void cross1()
{
	//pos1、pos2代表被选取的基因
	int pos1;
	int pos2;
	for (int k = 0; k < POP_NUM - 1; k += 2)
	{
		if (((double)rand()) / RAND_MAX < PC1)		//交叉概率
		{
			pos1 = 0;
			pos2 = 0;
			bool flag[N] = { 0 };
			//随机选取开始城市
			population[k][0] = rand() % N;
			flag[population[k][0]] = 1;
			for (int i = 0; i < N - 1; i++)
			{
				//找到相同的双亲节点
				for (int j = 0; j < N; j++)
				{
					if (population[k][i] == selectPop[k][j])
					{
						pos1 = j;
						break;
					}
				}
				for (int j = 0; j < N; j++)
				{
					if (population[k][i] == selectPop[k + 1][j])
					{
						pos2 = j;
						break;
					}
				}
				//printf("%d %d, %d, %d\n", pos1, pos2, population[k][i], i);
				bool add = false;
				while (!add)
				{
					if (!flag[selectPop[k + 1][(pos2 + 1) % N]] && !flag[selectPop[k][(pos1 + 1) % N]] && distance[population[k][i]][selectPop[k][(pos1 + 1) % N]] <= distance[population[k][i]][selectPop[k + 1][(pos2 + 1) % N]] || !flag[selectPop[k][(pos1 + 1) % N]] && flag[selectPop[k + 1][(pos2 + 1) % N]])
					{
						population[k][i + 1] = selectPop[k][(pos1 + 1) % N];
						flag[population[k][i + 1]] = 1;
						add = true;
					}
					else if (!flag[selectPop[k + 1][(pos2 + 1) % N]] && !flag[selectPop[k][(pos1 + 1) % N]] && distance[population[k][i]][selectPop[k][(pos1 + 1) % N]] > distance[population[k][i]][selectPop[k + 1][(pos2 + 1) % N]] || flag[selectPop[k][(pos1 + 1) % N]] && !flag[selectPop[k + 1][(pos2 + 1) % N]])
					{
						population[k][i + 1] = selectPop[k + 1][(pos2 + 1) % N];
						flag[population[k][i + 1]] = 1;
						add = true;
					}
					else if (!add && flag[selectPop[k][(pos1 + 1) % N]] && flag[selectPop[k + 1][(pos2 + 1) % N]])
					{
						pos1 = (pos1 + 1) % N;
						pos2 = (pos2 + 1) % N;
					}
				}
			}
			for (int i = 0; i < N; i++)
			{
				population[k + 1][i] = population[k][N - 1 - i];
			}
		}	
	}
}

void cross2()
{
	//交叉点位置
	int ranPos1;
	int ranPos2;
	int temp;
	for (int k = 0; k < POP_NUM - 1; k += 2)
	{
		ranPos1 = rand() % N;
		ranPos2 = rand() % N;
		if (((double)rand()) / RAND_MAX < PC2)		//交叉概率
		{
			if (ranPos1 > ranPos2)
			{
				temp = ranPos1;
				ranPos1 = ranPos2;
				ranPos2 = temp;
			}
			for (int i = ranPos1; i <= ranPos2; i++)
			{
				temp = population[k][i];
				population[k][i] = population[k + 1][i];
				population[k + 1][i] = temp;
			}
			int count1 = 0;
			int count2 = 0;
			int flag1[N];
			int flag2[N];
			for (int i = 0; i <= ranPos1 - 1; i++)
			{
				for (int j = ranPos1; j <= ranPos2; j++)
				{
					if (population[k][i] == population[k][j])
					{
						flag1[count1] = i;
						count1++;
					}
					if (population[k + 1][i] == population[k + 1][j])
					{
						flag2[count2] = i;
						count2++;
					}
				}
			}
			for (int i = ranPos2 + 1; i < N; i++)
			{
				for (int j = ranPos1; j <= ranPos2; j++)
				{
					if (population[k][i] == population[k][j])
					{
						flag1[count1] = i;
						count1++;
					}
					if (population[k + 1][i] == population[k + 1][j])
					{
						flag2[count2] = i;
						count2++;
					}
				}
			}
			if (count1 == count2 && count1 > 0)
			{
				for (int i = 0; i < count1; i++)
				{
					temp = population[k][flag1[i]];
					population[k][flag1[i]] = population[k + 1][flag2[i]];
					population[k + 1][flag2[i]] = temp;
				}
			}
		}
	}
}

/* 变异：改变个体编码串中的某些基因值，从而形成新的个体 */
void mutate(int param)
{
	for (int k = 0; k < POP_NUM; k++)
	{
		if (((double)rand()) / RAND_MAX < PM)	//变异概率
		{
			reverse(population[k], param);
		}
	}
}


int main()
{
	//初始化
	init();
	//当前时间作为随机数种子
	srand((unsigned)time(NULL));
	//贪心算法求解 TSP 
	TSPGreedyAlgorithm();
	
	initPop();
	for (int i = 0; i < GENERATE_COUNT; i++)
	{
		//计算当前种群平均距离	
		select();
		if (i >= GEN && abs(avgDistance[i] - avgDistance[i - GEN]) < 10 && popDistance[minIndex] == currentOptDistance)
		{
			cross2();
			mutate(N);
		}
		else
		{
			cross1();
			mutate(2);
		}
		//计算种群平均路径长度并更新最短路径
		avgDistance[i] = calDistance() / POP_NUM;
	}
	printf("\n最短路径长度为：%d\n", currentOptDistance);
	printf("TSP 路径为：");
	for (int i = 0; i < N; i++)
	{
		printf("%d -> ", currentOpt[i]);
	}
	printf("%d", currentOpt[0]);
	return 0;
}