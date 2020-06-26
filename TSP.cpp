/*
	TSP 算例来自TSPLIB，att48.tsp 数据集，其中有 48 个城市
	TSPLIB is a library of sample instances for the TSP (and related problems)from various sources and of various types.
	目前最佳解总距离为 10628，其中距离的计算方式为 sqrt((x*x + y*y)/10)
	1. 使用贪心策略求解，解集总距离为 12813，可见贪心策略只是局部最优解
	2. 使用遗传算法求解，解集总距离（自测最优情况）为10789
*/
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
#define POP_NUM 250
//迭代次数
#define GENERATE_COUNT 10000
//交叉概率
#define PC 0.6
//变异概率
#define PM 0.1
//种群
int population[POP_NUM][N];
//种群总距离
int popDistance[POP_NUM];
int selectPop[POP_NUM][N];

/*
	贪心算法求解 TSP
*/

/* 初始化N个城市距离矩阵 */
void init()
{
	//城市的 x 和 y 坐标
	int x[N] = {0};
	int y[N] = {0};
	//从 data.txt 文件读取数据
	FILE *fp;
	if ((fp = fopen("..//att48.txt", "r")) == NULL)
	{
		printf("can not open the file!");
		exit(0);
	}
	while (!feof(fp))
	{
		int count;
		fscanf(fp, "%d", &count);
		fscanf(fp, "%d%d", &x[count-1], &y[count-1]);
	}
	fclose(fp);
	//计算城市之间距离
	for (int i = 0; i < N - 1; i++)
	{
		distance[i][i] = 0;				// 对角线为0
		for (int j = i + 1; j < N; j++)
		{
			distance[i][j] = (int)sqrt((pow((double)x[i] - x[j], 2) / 10 + pow((double)y[i] - y[j], 2) / 10));
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
		totalDistance += min_distance;
		printf(" %d ->", temp + 1);
	}
	totalDistance += distance[current][0];
	printf(" %d\n", 1);
	printf("总路程为：%d\n", totalDistance);
}

/*
	遗传算法求解 TSP
*/

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
void swap(int *array, int num)
{
	int first, last, temp;
	for(int i = 0; i < num; i++)
	{
		first = rand() % N;
		last = rand() % N;
		temp = array[first];
		array[first] = array[last];
		array[last] = temp;
	}
}

/* 初始化种群 */
void initPop()
{
	//当前时间作为随机数种子
	srand((unsigned)time(NULL));
	//随机交换得到目标数量种群
	for (int i = 0; i < POP_NUM; i++)
	{
		for (int j = 0; j < N; j++)
		{
			population[i][j] = j + 1;
		}
		swap(population[i], N);
	}
}

/* 计算种群个体总距离,返回最小距离数组下标 */
int calDistance()
{
	int minDistance = 0x7fffffff;
	int minIndex;
	for (int i = 0; i < POP_NUM; i++)
	{
		popDistance[i] = 0;
		for (int j = 0; j < N - 1; j++)
		{
			popDistance[i] += distance[population[i][j]][population[i][j + 1]];
		}
		popDistance[i] += distance[population[i][N - 1]][population[i][0]];
		if (popDistance[i] < minDistance)
		{
			minIndex = i;
			minDistance = popDistance[i];
		}
	}
	printf("当前最优距离为：%d\n", popDistance[minIndex]);
	return minIndex;
}

/* 选择：适应度代表个体被遗传到下一代群体中的概率
   轮盘赌选择方法的实现步骤:
	（1）计算群体中所有个体的适应度值；
	（2）计算每个个体的选择概率；
	（3）计算积累概率；
	（4）采用模拟赌盘操作（即生成0到1之间的随机数与每个个体遗传到下一代群体的概率进行匹配）来确定各个个体是否遗传到下一代群体中。
*/
void select(int index)
{
	double popFit[POP_NUM];				//种群个体适应度
	double p[POP_NUM];					//种群个体的选择概率
	double sum = 0;
	for (int i = 0; i < POP_NUM; i++)
	{		
		popFit[i] = 10000.0 / popDistance[i];	//适应度函数之为距离的倒数，注意分子为 double 结果才为 double
		sum += popFit[i];
	}
	for (int  i = 0; i < POP_NUM; i++)
	{
		p[i] = popFit[i] / sum;
	}
	for (int k = 0; k < N; k++)
	{
		selectPop[0][k] = population[index][k];
	}
	for (int i = 1; i < POP_NUM; i++)
	{
		double temp = ((double)rand())/ RAND_MAX;
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
	for (int i = 0; i < POP_NUM; i++)
	{
		for (int j = 0; j < POP_NUM; j++)
		{
			population[i][j] = selectPop[i][j];
		}
	}
}

/* 交叉：两个相互配对的染色体依据交叉概率 PC 按某种方式相互交换其部分基因，形成两个新的个体 */
void cross()
{
	//交叉点位置
	int ranPos1;
	int ranPos2;
	int temp;
	for (int k = 0; k < POP_NUM - 1; k += 2)
	{
		ranPos1 = rand() % N;
		ranPos2 = rand() % N;
		if (((double)rand()) / RAND_MAX < PC)		//交叉概率
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
					if (population[k+1][i] == population[k+1][j])
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
void mutation()
{
	for(int k = 0; k < POP_NUM; k++)
	{
		if (((double)rand()) / RAND_MAX < PM)	//变异概率
		{
			swap(population[k], 1);
		}
	}
}

/* 打印路径（测试）*/
void calTest(int array[])
{
	int sum = 0;
	for (int i = 0; i < N - 1; i++)
	{
		sum += distance[array[i]][array[i + 1]];
		printf("%d ", array[i]);
	}
	sum += distance[array[N-1]][array[0]];
	printf("总路程为：%d", sum);
}

int main()
{
	//初始化
	init();
	//贪心算法求解 TSP
	TSPGreedyAlgorithm();

	//遗传算法求解 TSP
	int resultIndex;		 //结果下标
	initPop();
	for (int i = 0; i < GENERATE_COUNT; i++)
	{
		resultIndex = calDistance();	
		select(resultIndex); //选择
		cross();			 //交叉
		mutation();			 //变异
		//print();
	}
	resultIndex = calDistance();
	printf("TSP 路径为：");
	for (int i = 0; i < N; i++)
	{
		printf("%d -> ", population[resultIndex][i]);
	}
	printf("%d", population[resultIndex][0]);
	//calTest(population[resultIndex]);
	return 0;
}
