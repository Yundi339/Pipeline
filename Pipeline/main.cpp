#include<iostream>
#include<cstdio>
#include<bitset>
#include<fstream>
#include<vector>
using namespace std;
string inputPath[4] = { "General", "Datahazard", "Lwhazard","Branchhazard" };
string outputPath[4] = { "genResult.txt","dataResult.txt","loadResult.txt", "branchResult.txt" };
// regs[10] = { 0,9,5,7,1,2,3,4,5,6 };
// mem[5]	= { 5,9,4,8,7 };
struct IF_Path
{
	int PC = 0;
	int PC_T = 0;	//temp
	bitset<32> instruct;
	bool isRead = 1;
	bool isBranch = 0;
};

struct ID_Path
{
	int read_data1 = 0;
	int read_data2 = 0;
	int sign_ext = 0;
	int rs = 0;
	int rt = 0;
	int rd = 0;
	int control_T = 0;	//temp
	int control_signals = 0;
	int PC = 0;
	bool isRead = 1;
};

struct EX_Path
{
	int ALU = 0;
	int write_data = 0;
	int rt_rd = 0;
	int control_signals = 0;
	int rs = 0;
};

struct MEM_Path
{
	int read_data = 0;
	int ALU = 0;
	int rt_rd = 0;
	int control_signals = 0;
};

struct WB_Path
{
	int rt_rd = 0;
	int ALU = 0;
};

class Pipeline
{
private:
	// init
	int cc = 0;
	int count = 0;
	int max = 0;
	int regs[10] = { 0,9,5,7,1,2,3,4,5,6 };
	int mem[5] = { 5,9,4,8,7 };
	ifstream fin;
	ofstream fout;
	IF_Path IF;
	ID_Path ID;
	EX_Path EX;
	MEM_Path MEM;
	WB_Path WB;
	vector<bitset<32>> instruction;	//set

public:
	Pipeline(string input, string output) {
		fin.open(input);
		if (!fin) { cout << "Cannot open file" << endl; return; }
		fout.open(output);
		if (!fout) { cout << "Cannot out file" << endl; return; }
	}

	~Pipeline() {
		fin.close();
		fout.close();
	}

	void readFile()
	{
		char ch[33]; ch[32] = '\0';
		while (fin.peek() != EOF){
			fin.read(ch, 32);
			bitset<32> c(ch);
			instruction.push_back(c);
			cout << c << endl;
		}
		max = instruction.size() + 3;
	}

	void readInstruct()
	{
		if (IF.isRead){
			IF.PC_T = IF.PC;
			IF.PC += 4;
			if (count < instruction.size()) IF.instruct = instruction[count++];
			else IF.instruct = bitset<32>(0);
		}
	}

	void Display()
	{
		fout << "CC " << cc << ":\n\n"
			<< "Registers:"
			<< "\n$0:" << regs[0]
			<< "\n$1:" << regs[1]
			<< "\n$2:" << regs[2]
			<< "\n$3:" << regs[3]
			<< "\n$4:" << regs[4]
			<< "\n$5:" << regs[5]
			<< "\n$6:" << regs[6]
			<< "\n$7:" << regs[7]
			<< "\n$8:" << regs[8]
			<< "\n$9:" << regs[9]
			<< "\n\nData memory:"
			<< "\n0x00:" << mem[0]
			<< "\n0x04:" << mem[1]
			<< "\n0x08:" << mem[2]
			<< "\n0x0C:" << mem[3]
			<< "\n0x10:" << mem[4]
			<< "\n\nIF/ID:"
			<< "\nPC\t\t" << IF.PC
			<< "\nInstruction\t" << IF.instruct
			<< "\n\nID/EX :"
			<< "\nReadData1\t" << ID.read_data1
			<< "\nReadData2\t" << ID.read_data2
			<< "\nsign_ext\t" << ID.sign_ext
			<< "\nRs\t\t" << ID.rs
			<< "\nRt\t\t" << ID.rt
			<< "\nRd\t\t" << ID.rd
			<< "\nControl signals\t" << bitset<9>(ID.control_signals)
			<< "\n\nEX/MEM :"
			<< "\nALUout\t\t" << EX.ALU
			<< "\nWriteData\t" << EX.write_data
			<< "\nRt/Rd\t\t" << EX.rt_rd
			<< "\nControl signals\t" << bitset<5>(EX.control_signals)
			<< "\n\nMEM/WB :"
			<< "\nReadData\t" << MEM.read_data
			<< "\nALUout\t\t" << MEM.ALU
			<< "\nRt/Rd\t\t" << MEM.rt_rd
			<< "\nControl signals\t" << bitset<2>(MEM.control_signals)
			<< "\n=================================================================\n";
	}

	//clear forwarding
	void clear()
	{
		EX.ALU = 0;
		EX.write_data = 0;
		EX.rt_rd = 0;
		ID.control_signals = 0;
	}

	void MEM_WB()
	{
		WB.rt_rd = MEM.rt_rd;
		if (MEM.control_signals & 1)WB.ALU = MEM.read_data;
		else WB.ALU = MEM.ALU;
		if ((MEM.control_signals / 2) & 1) regs[MEM.rt_rd] = WB.ALU;
	}

	void EX_MEM() {			
		MEM.rt_rd = EX.rt_rd;
		MEM.ALU = EX.ALU;
		// lw
		if ((EX.control_signals >> 3) & 1) MEM.read_data = mem[EX.ALU / 4];
		else MEM.read_data = 0;
		// sw
		if ((EX.control_signals / 4) & 1) mem[EX.ALU / 4] = EX.write_data;
		MEM.control_signals = EX.control_signals & 3;
	}

	void ID_EX() {
		int rs, rt;
		if ((ID.control_signals >> 8) & 1) EX.rt_rd = ID.rd;
		else EX.rt_rd = ID.rt;
		EX.rs = ID.rt, EX.write_data = ID.read_data2;
		// rs
		if ((EX.control_signals & 3) != 0 && ID.rs == MEM.rt_rd) rs = MEM.ALU;
		else if (ID.rs == WB.rt_rd)rs = WB.ALU;
		else rs = ID.read_data1;
		// rt
		if ((ID.control_signals >> 5) & 1) rt = ID.sign_ext;
		else rt = ID.read_data2;
		//data hazard
		if (rt == ID.read_data2){
			if (ID.rt == MEM.rt_rd && (EX.control_signals & 3) != 0) EX.write_data = rt = MEM.ALU;
			else if (ID.rt == WB.rt_rd) EX.write_data = rt = WB.ALU;
		}
		int alu = (ID.control_signals >> 6) & 3;
		if (alu == 3) EX.ALU = rs & rt;			//and:11
		else if (alu == 0) EX.ALU = rs + rt;	//add:00
		else if (alu == 1) EX.ALU = rs - rt;	//sub:01
		else if (alu == 2){
			int se = ID.sign_ext & 63;
			if (se == 32)EX.ALU = rs + rt;		//add:100000
			else if (se == 34)EX.ALU = rs - rt;	//sub:100010
			else if (se == 36)EX.ALU = rs & rt;	//and:100100
			else if (se == 37)EX.ALU = rs | rt;	//or:100101
			else if (se == 42)EX.ALU = rs < rt;	//slt:101010
		}
		EX.control_signals = (ID.control_signals & 31); //5bit
		//branch hazard
		if (IF.isBranch) {
			if (ID.control_signals == 80 && ID.read_data1 == ID.read_data2) { clear(); IF.PC = ID.PC; count += ID.sign_ext - 1; }//beq
			else if (ID.control_signals == 80 && ID.read_data1 != ID.read_data2) { clear(); IF.PC = ID.PC; count += ID.sign_ext - 1; }//bne
			IF.instruct = IF.isBranch = 0;
		}
	}

	void IF_ID()
	{
		// data hazard
		if (ID.isRead){
			ID.sign_ext = IF.instruct.to_ulong() & 65535;	//15-0
			ID.PC = IF.PC + ID.sign_ext * 4;
			ID.rd = (IF.instruct.to_ulong() >> 11) & 31;	//15-11
			ID.rt = (IF.instruct.to_ulong() >> 16) & 31;	//20-16
			ID.rs = (IF.instruct.to_ulong() >> 21) & 31;	//25-21
			ID.control_T = controlSignals((IF.instruct.to_ulong() >> 26) & 63);	//temp_controlSignal:31-25
			ID.read_data1 = regs[ID.rs];ID.read_data2 = regs[ID.rt];
			bool flag = ID.read_data1 ^ ID.read_data2;
			if (((IF.instruct.to_ulong() >> 26) & 63) == 4 && !flag)	//beq:000100
					IF.isBranch = 1;
			else if (((IF.instruct.to_ulong() >> 26) & 63) == 5 && flag)	//bne:000101
					IF.isBranch = 1;
		}
		//data hazard
		if (((EX.control_signals >> 3) & 1) && (EX.rs == ID.rs || EX.rs == ID.rt)) ID.control_signals = IF.isRead = 0;
		else ID.control_signals = ID.control_T, IF.isRead = 1;
	}

	int controlSignals(int op)
	{
		int cs = 0;
		if (op == 0 && (IF.instruct.to_ulong() & 63))	//r:000000
			cs = 0b110000010;//2
		else if (op == 35)		//lw:100011
			cs = 0b000101011;//0
		else if (op == 43)		//sw:101011
			cs = 0b000100100;//0
		else if (op == 4)		//beq:000100
			cs = 0b001010000;//1
		else if (op == 5)		//bne:000101
			cs = 0b001010001;//1
		else if (op == 8)		//addi:001000
			cs = 0b000100010;//0
		else if (op == 12)		//andi:001100
			cs = 0b011100010;//3
		return cs;
	}

	void run()	//execute
	{
		readFile();
		do{
			readInstruct();
			cc++;
			Display();
			MEM_WB();
			EX_MEM();
			ID_EX();
			IF_ID();
			if (IF.isRead == 0 || ID.isRead == 0) max++;// exist bubble
			if (IF.isBranch) max = max + 1 - ID.sign_ext;// exist branch
		} while (cc <= max);
	}
};

int main()
{
	for(int i = 0;i < 4; i++){
		cout << inputPath[i] << ":\n";
		Pipeline p(inputPath[i], outputPath[i]);
		p.run();
	}
	return 0;
}