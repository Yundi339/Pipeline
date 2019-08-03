# pipeline

#### 题目：
以C/C++/Java實作Pipeline處理器之指令運作模擬程式。需實作下列指令：<br>
lw, sw, add, addi, sub, and, andi, or, slt, beq<br>
並且能夠偵測與處理data hazard, hazard with load, branch hazard。 


#### 输出：
其中SampleInput.txt的初始化是根據Sample的初始data設置的，其餘都是按照題目要求設置的：<br>
regs[10] = { 0,9,5,7,1,2,3,4,5,6 };<br>
mem[5]	= { 5,9,4,8,7 };<br>


#### 程式:
輸入文件"General", "Datahazard", "Lwhazard", "Branchhazard"<br>
輸出文件"genResult.txt","dataResult.txt","loadResult.txt", "branchResult.txt"<br><br>

讀取文件(一次性read):<br>
readFile();<br><br>

存儲instruction:<br>
vector<bitset<32>> instruction;<br><br>

計算方法上使用五個struct和六個函數：<br>
struct IF_Path<br>
struct ID_Path<br>
struct EX_Path<br>
struct MEM_Path<br>
struct WB_Path<br><br>

### 主要类:
```
class Pipeline(inputFile, outputFile):
{
	readInstruct()；
	Display();
	MEM_WB();
	EX_MEM();
	ID_EX();
	IF_ID();
}
```
